#include "./kcp.hpp"

#include "../core/core_byte.hpp"

#include <algorithm>
#include <limits>

X_BEGIN

static_assert(sizeof(ssize_t) >= sizeof(uint32_t));

//=====================================================================
// KCP BASIC
//=====================================================================
[[maybe_unused]] static constexpr const uint32_t IKCP_RTO_NDL       = 30;   // no delay min rto
[[maybe_unused]] static constexpr const uint32_t IKCP_RTO_MIN       = 100;  // normal min rto
[[maybe_unused]] static constexpr const uint32_t IKCP_RTO_DEF       = 200;
[[maybe_unused]] static constexpr const uint32_t IKCP_RTO_MAX       = 60000;
[[maybe_unused]] static constexpr const uint32_t IKCP_CMD_PUSH      = 81;  // cmd: push data
[[maybe_unused]] static constexpr const uint32_t IKCP_CMD_ACK       = 82;  // cmd: ack
[[maybe_unused]] static constexpr const uint32_t IKCP_CMD_WASK      = 83;  // cmd: window probe (ask)
[[maybe_unused]] static constexpr const uint32_t IKCP_CMD_WINS      = 84;  // cmd: window size (tell)
[[maybe_unused]] static constexpr const uint32_t IKCP_ASK_SEND      = 1;   // need to send IKCP_CMD_WASK
[[maybe_unused]] static constexpr const uint32_t IKCP_ASK_TELL      = 2;   // need to send IKCP_CMD_WINS
[[maybe_unused]] static constexpr const uint32_t IKCP_WND_SND       = 32;
[[maybe_unused]] static constexpr const uint32_t IKCP_WND_RCV       = 128;  // must >= max fragment size
[[maybe_unused]] static constexpr const uint32_t IKCP_MTU_DEF       = 1400;
[[maybe_unused]] static constexpr const uint32_t IKCP_ACK_FAST      = 3;
[[maybe_unused]] static constexpr const uint32_t IKCP_INTERVAL      = 100;
[[maybe_unused]] static constexpr const uint32_t IKCP_OVERHEAD      = 24;
[[maybe_unused]] static constexpr const uint32_t IKCP_DEADLINK      = 20;
[[maybe_unused]] static constexpr const uint32_t IKCP_THRESH_INIT   = 2;
[[maybe_unused]] static constexpr const uint32_t IKCP_THRESH_MIN    = 2;
[[maybe_unused]] static constexpr const uint32_t IKCP_PROBE_INIT    = 7000;    // 7 secs to probe window size
[[maybe_unused]] static constexpr const uint32_t IKCP_PROBE_LIMIT   = 120000;  // up to 120 secs to probe window
[[maybe_unused]] static constexpr const uint32_t IKCP_FASTACK_LIMIT = 5;       // max times to trigger fastack

//---------------------------------------------------------------------
// encode / decode
//---------------------------------------------------------------------
[[maybe_unused]] static inline uint32_t _ibound_(uint32_t lower, uint32_t middle, uint32_t upper) {
	return std::min(std::max(lower, middle), upper);
}

[[maybe_unused]] static inline int32_t _itimediff(uint32_t later, uint32_t earlier) {
	return ((int32_t)(later - earlier));
}

[[maybe_unused]] static void * (*ikcp_malloc_hook)(size_t) = nullptr;
[[maybe_unused]] static void (*ikcp_free_hook)(void *)     = nullptr;

// internal malloc
static void * ikcp_malloc(size_t size) {
	if (ikcp_malloc_hook) {
		return ikcp_malloc_hook(size);
	}
	return malloc(size);
}

// internal free
static void ikcp_free(void * ptr) {
	if (ikcp_free_hook) {
		ikcp_free_hook(ptr);
	} else {
		free(ptr);
	}
}

// redefine allocator
void KcpSetAllocator(void * (*new_malloc)(size_t), void (*new_free)(void *)) {
	ikcp_malloc_hook = new_malloc;
	ikcp_free_hook   = new_free;
}

// allocate a new kcp segment
static xKcpSegment * ikcp_segment_new(xKcpControlBlock * kcp, size_t size) {
	return (xKcpSegment *)ikcp_malloc(sizeof(xKcpSegment) + size);
}

// delete a segment
static void ikcp_segment_delete(xKcpControlBlock * kcp, xKcpSegment * seg) {
	ikcp_free(seg);
}

// write log
static void ikcp_log(xKcpControlBlock * kcp, int mask, const char * fmt, ...) {
	char    buffer[1024];
	va_list argptr;
	if ((mask & kcp->logmask) == 0 || kcp->writelog == 0) return;
	va_start(argptr, fmt);
	vsprintf(buffer, fmt, argptr);
	va_end(argptr);
	kcp->writelog(buffer, kcp, kcp->user);
}

// check log mask
static bool ikcp_canlog(const xKcpControlBlock * kcp, uint32_t mask) {
	if ((mask & kcp->logmask) == 0 || kcp->writelog == NULL) {
		return false;
	}
	return true;
}

// output segment
static int ikcp_output(xKcpControlBlock * kcp, const void * data, int size) {
	assert(kcp);
	assert(kcp->output);
	if (ikcp_canlog(kcp, IKCP_LOG_OUTPUT)) {
		ikcp_log(kcp, IKCP_LOG_OUTPUT, "[RO] %ld bytes", (long)size);
	}
	if (size == 0) return 0;
	return kcp->output((const ubyte *)data, size, kcp, kcp->user);
}

//---------------------------------------------------------------------
// create a new kcpcb
//---------------------------------------------------------------------
xKcpControlBlock * KcpCreate(uint32_t conv, void * user) {

	auto kcp = (xKcpControlBlock *)ikcp_malloc(sizeof(xKcpControlBlock));
	if (kcp == nullptr) {
		return nullptr;
	}
	kcp->conv       = conv;
	kcp->user       = user;
	kcp->snd_una    = 0;
	kcp->snd_nxt    = 0;
	kcp->rcv_nxt    = 0;
	kcp->ts_recent  = 0;
	kcp->ts_lastack = 0;
	kcp->ts_probe   = 0;
	kcp->probe_wait = 0;
	kcp->snd_wnd    = IKCP_WND_SND;
	kcp->rcv_wnd    = IKCP_WND_RCV;
	kcp->rmt_wnd    = IKCP_WND_RCV;
	kcp->cwnd       = 0;
	kcp->incr       = 0;
	kcp->probe      = 0;
	kcp->mtu        = IKCP_MTU_DEF;
	kcp->mss        = kcp->mtu - IKCP_OVERHEAD;
	kcp->stream     = 0;

	kcp->buffer = (ubyte *)ikcp_malloc((kcp->mtu + IKCP_OVERHEAD) * 3);
	if (kcp->buffer == NULL) {
		ikcp_free(kcp);
		return NULL;
	}

	Construct(kcp->snd_queue);
	Construct(kcp->rcv_queue);
	Construct(kcp->snd_buf);
	Construct(kcp->rcv_buf);

	kcp->nrcv_buf   = 0;
	kcp->nsnd_buf   = 0;
	kcp->nrcv_que   = 0;
	kcp->nsnd_que   = 0;
	kcp->state      = 0;
	kcp->acklist    = NULL;
	kcp->ackblock   = 0;
	kcp->ackcount   = 0;
	kcp->rx_srtt    = 0;
	kcp->rx_rttval  = 0;
	kcp->rx_rto     = IKCP_RTO_DEF;
	kcp->rx_minrto  = IKCP_RTO_MIN;
	kcp->current    = 0;
	kcp->interval   = IKCP_INTERVAL;
	kcp->ts_flush   = IKCP_INTERVAL;
	kcp->nodelay    = 0;
	kcp->updated    = 0;
	kcp->logmask    = 0;
	kcp->ssthresh   = IKCP_THRESH_INIT;
	kcp->fastresend = 0;
	kcp->fastlimit  = IKCP_FASTACK_LIMIT;
	kcp->nocwnd     = 0;
	kcp->xmit       = 0;
	kcp->dead_link  = IKCP_DEADLINK;
	kcp->output     = NULL;
	kcp->writelog   = NULL;

	return kcp;
}

//---------------------------------------------------------------------
// release a new kcpcb
//---------------------------------------------------------------------
void KcpDestroy(xKcpControlBlock * kcp) {
	assert(kcp);
	if (kcp) {
		while (auto seg = kcp->snd_buf.PopHead()) {
			ikcp_segment_delete(kcp, (xKcpSegment *)seg);
		}
		while (auto seg = kcp->rcv_buf.PopHead()) {
			ikcp_segment_delete(kcp, (xKcpSegment *)seg);
		}
		while (auto seg = kcp->snd_queue.PopHead()) {
			ikcp_segment_delete(kcp, (xKcpSegment *)seg);
		}
		while (auto seg = kcp->rcv_queue.PopHead()) {
			ikcp_segment_delete(kcp, (xKcpSegment *)seg);
		}
		if (kcp->buffer) {
			ikcp_free(kcp->buffer);
		}
		if (kcp->acklist) {
			ikcp_free(kcp->acklist);
		}

		kcp->nrcv_buf = 0;
		kcp->nsnd_buf = 0;
		kcp->nrcv_que = 0;
		kcp->nsnd_que = 0;
		kcp->ackcount = 0;
		kcp->buffer   = NULL;
		kcp->acklist  = NULL;
		ikcp_free(kcp);
	}
}

//---------------------------------------------------------------------
// set output callback, which will be invoked by kcp
//---------------------------------------------------------------------
void KcpSetOutput(xKcpControlBlock * kcp, ssize_t (*output)(const ubyte * buf, ssize_t len, xKcpControlBlock * kcp, void * user)) {
	// not thread safe
	kcp->output = output;
}

//---------------------------------------------------------------------
// peek data size
//---------------------------------------------------------------------
ssize_t KcpPeekSize(xKcpControlBlock * kcp) {

	assert(kcp);
	auto seg = static_cast<xKcpSegment *>(kcp->rcv_queue.Head());
	if (!seg) {
		return -1;
	}
	if (seg->frg == 0) {
		return seg->len;
	}

	if (kcp->nrcv_que < seg->frg + 1) {
		return -1;
	}

	ssize_t length = 0;
	for (const auto & Node : kcp->rcv_queue) {
		auto SegNode = static_cast<const xKcpSegment &>(Node);
		length      += SegNode.len;
		if (SegNode.frg == 0) {
			break;
		}
	}
	return length;
}

//---------------------------------------------------------------------
// user/upper level recv: returns size, returns below zero for EAGAIN
//---------------------------------------------------------------------
ssize_t KcpRecv(xKcpControlBlock * kcp, void * buffer_, ssize_t len) {
	assert(kcp);

	auto buffer  = static_cast<ubyte *>(buffer_);
	bool ispeek  = len < 0;
	bool recover = false;

	if (kcp->rcv_queue.IsEmpty()) {
		return -1;
	}

	if (ispeek) {
		len = -len;
	}

	auto peeksize = KcpPeekSize(kcp);
	if (peeksize < 0) {
		return -2;
	}
	if (peeksize > len) {
		return -3;
	}
	if (kcp->nrcv_que >= kcp->rcv_wnd) {
		recover = true;
	}

	// merge fragment
	len = 0;
	for (auto & node : kcp->rcv_queue) {
		auto & seg = static_cast<xKcpSegment &>(node);
		if (buffer) {
			memcpy(buffer, seg.data, seg.len);
			buffer += seg.len;
		}
		len          += seg.len;
		auto fragment = seg.frg;

		if (ikcp_canlog(kcp, IKCP_LOG_RECV)) {
			ikcp_log(kcp, IKCP_LOG_RECV, "recv sn=%lu", (unsigned long)seg.sn);
		}

		if (!ispeek) {
			xKcpSegmentList::Remove(seg);
			ikcp_segment_delete(kcp, &seg);
			kcp->nrcv_que--;
		}

		if (fragment == 0) {
			break;
		}
	}
	assert(len == peeksize);

	// move available data from rcv_buf -> rcv_queue
	while (auto seg = static_cast<xKcpSegment *>(kcp->rcv_buf.Head())) {
		if (seg->sn == kcp->rcv_nxt && kcp->nrcv_que < kcp->rcv_wnd) {
			kcp->rcv_queue.GrabTail(*seg);
			kcp->nrcv_buf--;
			kcp->nrcv_que++;
			kcp->rcv_nxt++;
		} else {
			break;
		}
	}

	// fast recover
	if (kcp->nrcv_que < kcp->rcv_wnd && recover) {
		// ready to send back IKCP_CMD_WINS in ikcp_flush
		// tell remote my window size
		kcp->probe |= IKCP_ASK_TELL;
	}

	return len;
}

//---------------------------------------------------------------------
// user/upper level send, returns below zero for error
//---------------------------------------------------------------------
ssize_t KcpSend(xKcpControlBlock * kcp, const ubyte * buffer, ssize_t len) {
	// xKcpSegment * seg;
	ssize_t count;
	ssize_t sent = 0;

	assert(kcp->mss > 0);
	if (len < 0) {
		return -1;
	}

	// append to previous segment in streaming mode (if possible)
	if (kcp->stream != 0) {
		if (auto old = static_cast<xKcpSegment *>(kcp->snd_queue.Head())) {
			if (old->len < kcp->mss) {
				auto capacity = kcp->mss - old->len;
				auto extend   = (len < capacity) ? len : capacity;

				auto seg = ikcp_segment_new(kcp, old->len + extend);
				if (!seg) {
					return -2;
				}
				kcp->snd_queue.AddTail(*seg);
				memcpy(seg->data, old->data, old->len);
				if (buffer) {
					memcpy(seg->data + old->len, buffer, extend);
					buffer += extend;
				}
				seg->len = old->len + extend;
				seg->frg = 0;
				len     -= extend;

				xKcpSegmentList::Remove(*old);
				ikcp_segment_delete(kcp, old);
				sent = extend;
			}
		}
		if (len <= 0) {
			return sent;
		}
	}

	if (len <= (int)kcp->mss) {
		count = 1;
	} else {
		count = (len + kcp->mss - 1) / kcp->mss;
	}

	if (count >= IKCP_WND_RCV) {
		if (kcp->stream != 0 && sent > 0) {
			return sent;
		}
		return -2;
	}

	if (count == 0) {
		count = 1;
	}

	// fragment
	for (decltype(count) i = 0; i < count; i++) {
		ssize_t size = (len > kcp->mss) ? kcp->mss : len;
		auto    seg  = ikcp_segment_new(kcp, size);
		if (!seg) {
			return -2;
		}
		if (buffer && len > 0) {
			memcpy(seg->data, buffer, size);
		}
		seg->len = size;
		seg->frg = (kcp->stream == 0) ? (count - i - 1) : 0;
		kcp->snd_queue.AddTail(*seg);
		kcp->nsnd_que++;
		if (buffer) {
			buffer += size;
		}
		len  -= size;
		sent += size;
	}

	return sent;
}

//---------------------------------------------------------------------
// parse ack
//---------------------------------------------------------------------
static void ikcp_update_ack(xKcpControlBlock * kcp, uint32_t rtt) {
	uint32_t rto = 0;
	if (kcp->rx_srtt == 0) {
		kcp->rx_srtt   = rtt;
		kcp->rx_rttval = rtt / 2;
	} else {
		auto delta = SignedDiff(rtt, kcp->rx_srtt);
		if (delta < 0) {
			delta = -delta;
		}
		kcp->rx_rttval = (3 * kcp->rx_rttval + delta) / 4;
		kcp->rx_srtt   = (7 * kcp->rx_srtt + rtt) / 8;
		if (kcp->rx_srtt < 1) kcp->rx_srtt = 1;
	}
	rto         = kcp->rx_srtt + std::max(kcp->interval, 4 * kcp->rx_rttval);
	kcp->rx_rto = _ibound_(kcp->rx_minrto, rto, IKCP_RTO_MAX);
}

static void ikcp_shrink_buf(xKcpControlBlock * kcp) {
	if (auto seg = static_cast<xKcpSegment *>(kcp->snd_buf.Head())) {
		kcp->snd_una = seg->sn;
	} else {
		kcp->snd_una = kcp->snd_nxt;
	}
}

static void ikcp_parse_ack(xKcpControlBlock * kcp, uint32_t sn) {
	if (_itimediff(sn, kcp->snd_una) < 0 || _itimediff(sn, kcp->snd_nxt) >= 0) {
		return;
	}
	for (auto & node : kcp->snd_buf) {
		auto seg = static_cast<xKcpSegment *>(&node);
		if (sn == seg->sn) {
			xKcpSegmentList::Remove(*seg);
			ikcp_segment_delete(kcp, seg);
			kcp->nsnd_buf--;
			break;
		}
		if (_itimediff(sn, seg->sn) < 0) {
			break;
		}
	}
}

static void ikcp_parse_una(xKcpControlBlock * kcp, uint32_t una) {
	for (auto & node : kcp->snd_buf) {
		auto seg = static_cast<xKcpSegment *>(&node);
		if (_itimediff(una, seg->sn) > 0) {
			xKcpSegmentList::Remove(*seg);
			ikcp_segment_delete(kcp, seg);
			kcp->nsnd_buf--;
		} else {
			break;
		}
	}
}

static void ikcp_parse_fastack(xKcpControlBlock * kcp, uint32_t sn, uint32_t ts) {
	if (_itimediff(sn, kcp->snd_una) < 0 || _itimediff(sn, kcp->snd_nxt) >= 0) {
		return;
	}

	for (auto & node : kcp->snd_buf) {
		auto seg = static_cast<xKcpSegment *>(&node);
		if (_itimediff(sn, seg->sn) < 0) {
			break;
		} else if (sn != seg->sn) {
#ifndef IKCP_FASTACK_CONSERVE
			seg->fastack++;
#else
			if (_itimediff(ts, seg->ts) >= 0) seg->fastack++;
#endif
		}
	}
}

//---------------------------------------------------------------------
// ack append
//---------------------------------------------------------------------
[[maybe_unused]] static void ikcp_ack_push(xKcpControlBlock * kcp, uint32_t sn, uint32_t ts) {
	uint32_t   newsize = kcp->ackcount + 1;
	uint32_t * ptr;

	if (newsize > kcp->ackblock) {
		uint32_t * acklist;
		uint32_t   newblock;

		for (newblock = 8; newblock < newsize; newblock <<= 1) {
			// continue
		}
		acklist = (uint32_t *)ikcp_malloc(newblock * sizeof(uint32_t) * 2);
		assert(acklist);

		if (kcp->acklist != NULL) {
			uint32_t x;
			for (x = 0; x < kcp->ackcount; x++) {
				acklist[x * 2 + 0] = kcp->acklist[x * 2 + 0];
				acklist[x * 2 + 1] = kcp->acklist[x * 2 + 1];
			}
			ikcp_free(kcp->acklist);
		}

		kcp->acklist  = acklist;
		kcp->ackblock = newblock;
	}

	ptr    = &kcp->acklist[kcp->ackcount * 2];
	ptr[0] = sn;
	ptr[1] = ts;
	kcp->ackcount++;
}

[[maybe_unused]] static void ikcp_ack_get(const xKcpControlBlock * kcp, int p, uint32_t * sn, uint32_t * ts) {
	if (sn) sn[0] = kcp->acklist[p * 2 + 0];
	if (ts) ts[0] = kcp->acklist[p * 2 + 1];
}

//---------------------------------------------------------------------
// parse data
//---------------------------------------------------------------------
void ikcp_parse_data(xKcpControlBlock * kcp, xKcpSegment * newseg) {
	uint32_t sn     = newseg->sn;
	ssize_t  repeat = 0;

	if (_itimediff(sn, kcp->rcv_nxt + kcp->rcv_wnd) >= 0 || _itimediff(sn, kcp->rcv_nxt) < 0) {
		ikcp_segment_delete(kcp, newseg);
		return;
	}

	xKcpSegment * InsertPoint = nullptr;
	for (auto Iter = kcp->rcv_buf.rbegin(), End = kcp->rcv_buf.rend(); Iter != End; ++Iter) {
		InsertPoint = &static_cast<xKcpSegment &>(*Iter);
		if (InsertPoint->sn == sn) {
			repeat = 1;
			break;
		}
		if (_itimediff(sn, InsertPoint->sn) > 0) {
			break;
		}
	}

	if (!repeat) {
		if (InsertPoint) {
			xKcpSegmentList::InsertBefore(*newseg, *InsertPoint);
		} else {
			kcp->rcv_buf.AddHead(*newseg);
		}
		kcp->nrcv_buf++;
	} else {
		ikcp_segment_delete(kcp, newseg);
	}

	// move available data from rcv_buf -> rcv_queue
	for (auto & Node : kcp->rcv_buf) {
		auto Seg = &static_cast<xKcpSegment &>(Node);
		if (Seg->sn == kcp->rcv_nxt && kcp->nrcv_que < kcp->rcv_wnd) {
			kcp->rcv_queue.GrabTail(*Seg);
			kcp->nrcv_buf--;
			kcp->nrcv_que++;
			kcp->rcv_nxt++;
		} else {
			break;
		}
	}
}

//---------------------------------------------------------------------
// input data
//---------------------------------------------------------------------
ssize_t KcpInput(xKcpControlBlock * kcp, const void * data, ssize_t size) {
	auto prev_una = kcp->snd_una;
	auto maxack = 0, latest_ts = 0;
	bool flag = false;

	if (ikcp_canlog(kcp, IKCP_LOG_INPUT)) {
		ikcp_log(kcp, IKCP_LOG_INPUT, "[RI] %zi bytes", size);
	}

	if (!data || size < (ssize_t)IKCP_OVERHEAD) {
		return -1;
	}

	while (true) {
		uint32_t      conv, ts, sn, una, len;
		uint16_t      wnd;
		uint8_t       cmd, frg;
		xKcpSegment * seg;

		if (size < (ssize_t)IKCP_OVERHEAD) {
			break;
		}

		auto R = xStreamReader(data);
		conv   = R.R4L();
		if (conv != kcp->conv) {
			return -1;
		}
		cmd = R.R1L();
		frg = R.R1L();
		wnd = R.R2L();
		ts  = R.R4L();
		sn  = R.R4L();
		una = R.R4L();
		len = R.R4L();

		size -= IKCP_OVERHEAD;

		if (size < (ssize_t)len || MakeSigned(len) < 0) {
			return -2;
		}

		if (cmd != IKCP_CMD_PUSH && cmd != IKCP_CMD_ACK && cmd != IKCP_CMD_WASK && cmd != IKCP_CMD_WINS) {
			return -3;
		}

		kcp->rmt_wnd = wnd;
		ikcp_parse_una(kcp, una);
		ikcp_shrink_buf(kcp);

		if (cmd == IKCP_CMD_ACK) {
			if (_itimediff(kcp->current, ts) >= 0) {
				ikcp_update_ack(kcp, _itimediff(kcp->current, ts));
			}
			ikcp_parse_ack(kcp, sn);
			ikcp_shrink_buf(kcp);
			if (!flag) {
				flag      = true;
				maxack    = sn;
				latest_ts = ts;
			} else {
				if (_itimediff(sn, maxack) > 0) {
#ifndef IKCP_FASTACK_CONSERVE
					maxack    = sn;
					latest_ts = ts;
#else
					if (_itimediff(ts, latest_ts) > 0) {
						maxack    = sn;
						latest_ts = ts;
					}
#endif
				}
			}
			if (ikcp_canlog(kcp, IKCP_LOG_IN_ACK)) {
				ikcp_log(
					kcp, IKCP_LOG_IN_ACK, "input ack: sn=%lu rtt=%ld rto=%ld", (unsigned long)sn, (long)_itimediff(kcp->current, ts),
					(long)kcp->rx_rto
				);
			}
		} else if (cmd == IKCP_CMD_PUSH) {
			if (ikcp_canlog(kcp, IKCP_LOG_IN_DATA)) {
				ikcp_log(kcp, IKCP_LOG_IN_DATA, "input psh: sn=%lu ts=%lu", (unsigned long)sn, (unsigned long)ts);
			}
			if (_itimediff(sn, kcp->rcv_nxt + kcp->rcv_wnd) < 0) {
				ikcp_ack_push(kcp, sn, ts);
				if (_itimediff(sn, kcp->rcv_nxt) >= 0) {
					seg       = ikcp_segment_new(kcp, len);
					seg->conv = conv;
					seg->cmd  = cmd;
					seg->frg  = frg;
					seg->wnd  = wnd;
					seg->ts   = ts;
					seg->sn   = sn;
					seg->una  = una;
					seg->len  = len;

					if (len > 0) {
						R.R(seg->data, len);
					}

					ikcp_parse_data(kcp, seg);
				}
			}
		} else if (cmd == IKCP_CMD_WASK) {
			// ready to send back IKCP_CMD_WINS in ikcp_flush
			// tell remote my window size
			kcp->probe |= IKCP_ASK_TELL;
			if (ikcp_canlog(kcp, IKCP_LOG_IN_PROBE)) {
				ikcp_log(kcp, IKCP_LOG_IN_PROBE, "input probe");
			}
		} else if (cmd == IKCP_CMD_WINS) {
			// do nothing
			if (ikcp_canlog(kcp, IKCP_LOG_IN_WINS)) {
				ikcp_log(kcp, IKCP_LOG_IN_WINS, "input wins: %lu", (unsigned long)(wnd));
			}
		} else {
			return -3;
		}

		data  = R();
		size -= len;
	}

	if (flag) {
		ikcp_parse_fastack(kcp, maxack, latest_ts);
	}

	if (_itimediff(kcp->snd_una, prev_una) > 0) {
		if (kcp->cwnd < kcp->rmt_wnd) {
			auto mss = kcp->mss;
			if (kcp->cwnd < kcp->ssthresh) {
				kcp->cwnd++;
				kcp->incr += mss;
			} else {
				if (kcp->incr < mss) kcp->incr = mss;
				kcp->incr += (mss * mss) / kcp->incr + (mss / 16);
				if ((kcp->cwnd + 1) * mss <= kcp->incr) {
					kcp->cwnd = (kcp->incr + mss - 1) / ((mss > 0) ? mss : 1);
				}
			}
			if (kcp->cwnd > kcp->rmt_wnd) {
				kcp->cwnd = kcp->rmt_wnd;
				kcp->incr = kcp->rmt_wnd * mss;
			}
		}
	}

	return 0;
}

//---------------------------------------------------------------------
// ikcp_encode_seg
//---------------------------------------------------------------------
[[maybe_unused]] static ubyte * ikcp_encode_seg(void * ptr, const xKcpSegment * seg) {
	auto W = xStreamWriter(ptr);
	W.W4L(seg->conv);
	W.W1L(seg->cmd);
	W.W1L(seg->frg);
	W.W2L(seg->wnd);
	W.W4L(seg->ts);
	W.W4L(seg->sn);
	W.W4L(seg->una);
	W.W4L(seg->len);
	return W;
}

[[maybe_unused]] static int ikcp_wnd_unused(const xKcpControlBlock * kcp) {
	if (kcp->nrcv_que < kcp->rcv_wnd) {
		return kcp->rcv_wnd - kcp->nrcv_que;
	}
	return 0;
}

//---------------------------------------------------------------------
// ikcp_flush
//---------------------------------------------------------------------
void KcpFlush(xKcpControlBlock * kcp) {
	uint32_t    current = kcp->current;
	ubyte *     buffer  = kcp->buffer;
	ubyte *     ptr     = buffer;
	int         count, size, i;
	uint32_t    resent, cwnd;
	uint32_t    rtomin;
	int         change = 0;
	int         lost   = 0;
	xKcpSegment seg;

	// 'ikcp_update' haven't been called.
	if (kcp->updated == 0) {
		return;
	}

	seg.conv = kcp->conv;
	seg.cmd  = IKCP_CMD_ACK;
	seg.frg  = 0;
	seg.wnd  = ikcp_wnd_unused(kcp);
	seg.una  = kcp->rcv_nxt;
	seg.len  = 0;
	seg.sn   = 0;
	seg.ts   = 0;

	// flush acknowledges
	count = kcp->ackcount;
	for (i = 0; i < count; i++) {
		size = (int)(ptr - buffer);
		if (size + (int)IKCP_OVERHEAD > (int)kcp->mtu) {
			ikcp_output(kcp, buffer, size);
			ptr = buffer;
		}
		ikcp_ack_get(kcp, i, &seg.sn, &seg.ts);
		ptr = ikcp_encode_seg(ptr, &seg);
	}

	kcp->ackcount = 0;

	// probe window size (if remote window size equals zero)
	if (kcp->rmt_wnd == 0) {
		if (kcp->probe_wait == 0) {
			kcp->probe_wait = IKCP_PROBE_INIT;
			kcp->ts_probe   = kcp->current + kcp->probe_wait;
		} else {
			if (_itimediff(kcp->current, kcp->ts_probe) >= 0) {
				if (kcp->probe_wait < IKCP_PROBE_INIT) kcp->probe_wait = IKCP_PROBE_INIT;
				kcp->probe_wait += kcp->probe_wait / 2;
				if (kcp->probe_wait > IKCP_PROBE_LIMIT) kcp->probe_wait = IKCP_PROBE_LIMIT;
				kcp->ts_probe = kcp->current + kcp->probe_wait;
				kcp->probe   |= IKCP_ASK_SEND;
			}
		}
	} else {
		kcp->ts_probe   = 0;
		kcp->probe_wait = 0;
	}

	// flush window probing commands
	if (kcp->probe & IKCP_ASK_SEND) {
		seg.cmd = IKCP_CMD_WASK;
		size    = (int)(ptr - buffer);
		if (size + (int)IKCP_OVERHEAD > (int)kcp->mtu) {
			ikcp_output(kcp, buffer, size);
			ptr = buffer;
		}
		ptr = ikcp_encode_seg(ptr, &seg);
	}

	// flush window probing commands
	if (kcp->probe & IKCP_ASK_TELL) {
		seg.cmd = IKCP_CMD_WINS;
		size    = (int)(ptr - buffer);
		if (size + (int)IKCP_OVERHEAD > (int)kcp->mtu) {
			ikcp_output(kcp, buffer, size);
			ptr = buffer;
		}
		ptr = ikcp_encode_seg(ptr, &seg);
	}

	kcp->probe = 0;

	// calculate window size
	cwnd = std::min(kcp->snd_wnd, kcp->rmt_wnd);
	if (kcp->nocwnd == 0) cwnd = std::min(kcp->cwnd, cwnd);

	// move data from snd_queue to snd_buf
	while (_itimediff(kcp->snd_nxt, kcp->snd_una + cwnd) < 0) {
		auto newseg = static_cast<xKcpSegment *>(kcp->snd_queue.Head());
		if (!newseg) {
			break;
		}
		kcp->snd_buf.GrabTail(*newseg);

		kcp->nsnd_que--;
		kcp->nsnd_buf++;

		newseg->conv     = kcp->conv;
		newseg->cmd      = IKCP_CMD_PUSH;
		newseg->wnd      = seg.wnd;
		newseg->ts       = current;
		newseg->sn       = kcp->snd_nxt++;
		newseg->una      = kcp->rcv_nxt;
		newseg->resendts = current;
		newseg->rto      = kcp->rx_rto;
		newseg->fastack  = 0;
		newseg->xmit     = 0;
	}

	// calculate resent
	resent = (kcp->fastresend > 0) ? (uint32_t)kcp->fastresend : 0xffffffff;
	rtomin = (kcp->nodelay == 0) ? (kcp->rx_rto >> 3) : 0;

	// flush data segments
	// for (p = kcp->snd_buf.next; p != &kcp->snd_buf; p = p->next) {
	for (auto & Node : kcp->snd_buf) {
		auto segment  = static_cast<xKcpSegment *>(&Node);
		bool needsend = false;
		if (segment->xmit == 0) {
			needsend = true;
			segment->xmit++;
			segment->rto      = kcp->rx_rto;
			segment->resendts = current + segment->rto + rtomin;
		} else if (_itimediff(current, segment->resendts) >= 0) {
			needsend = true;
			segment->xmit++;
			kcp->xmit++;
			if (kcp->nodelay == 0) {
				segment->rto += std::max(segment->rto, kcp->rx_rto);
			} else {
				uint32_t step = (kcp->nodelay < 2) ? segment->rto : kcp->rx_rto;
				segment->rto += step / 2;
			}
			segment->resendts = current + segment->rto;
			lost              = 1;
		} else if (segment->fastack >= resent) {
			if ((int)segment->xmit <= kcp->fastlimit || kcp->fastlimit <= 0) {
				needsend = 1;
				segment->xmit++;
				segment->fastack  = 0;
				segment->resendts = current + segment->rto;
				change++;
			}
		}

		if (needsend) {
			int need;
			segment->ts  = current;
			segment->wnd = seg.wnd;
			segment->una = kcp->rcv_nxt;

			size = (int)(ptr - buffer);
			need = IKCP_OVERHEAD + segment->len;

			if (size + need > (int)kcp->mtu) {
				ikcp_output(kcp, buffer, size);
				ptr = buffer;
			}

			ptr = ikcp_encode_seg(ptr, segment);

			if (segment->len > 0) {
				memcpy(ptr, segment->data, segment->len);
				ptr += segment->len;
			}

			if (segment->xmit >= kcp->dead_link) {
				kcp->state = (uint32_t)-1;
			}
		}
	}

	// flash remain segments
	size = (int)(ptr - buffer);
	if (size > 0) {
		ikcp_output(kcp, buffer, size);
	}

	// update ssthresh
	if (change) {
		uint32_t inflight = kcp->snd_nxt - kcp->snd_una;
		kcp->ssthresh     = inflight / 2;
		if (kcp->ssthresh < IKCP_THRESH_MIN) kcp->ssthresh = IKCP_THRESH_MIN;
		kcp->cwnd = kcp->ssthresh + resent;
		kcp->incr = kcp->cwnd * kcp->mss;
	}

	if (lost) {
		kcp->ssthresh = cwnd / 2;
		if (kcp->ssthresh < IKCP_THRESH_MIN) kcp->ssthresh = IKCP_THRESH_MIN;
		kcp->cwnd = 1;
		kcp->incr = kcp->mss;
	}

	if (kcp->cwnd < 1) {
		kcp->cwnd = 1;
		kcp->incr = kcp->mss;
	}
}

//---------------------------------------------------------------------
// update state (call it repeatedly, every 10ms-100ms), or you can ask
// ikcp_check when to call it again (without ikcp_input/_send calling).
// 'current' - current timestamp in millisec.
//---------------------------------------------------------------------
void KcpUpdate(xKcpControlBlock * kcp, uint32_t current) {
	kcp->current = current;

	if (kcp->updated == 0) {
		kcp->updated  = 1;
		kcp->ts_flush = kcp->current;
	}

	auto slap = _itimediff(kcp->current, kcp->ts_flush);

	if (slap >= 10000 || slap < -10000) {
		kcp->ts_flush = kcp->current;
		slap          = 0;
	}

	if (slap >= 0) {
		kcp->ts_flush += kcp->interval;
		if (_itimediff(kcp->current, kcp->ts_flush) >= 0) {
			kcp->ts_flush = kcp->current + kcp->interval;
		}
		KcpFlush(kcp);
	}
}

//---------------------------------------------------------------------
// Determine when should you invoke ikcp_update:
// returns when you should invoke ikcp_update in millisec, if there
// is no ikcp_input/_send calling. you can call ikcp_update in that
// time, instead of call update repeatly.
// Important to reduce unnacessary ikcp_update invoking. use it to
// schedule ikcp_update (eg. implementing an epoll-like mechanism,
// or optimize ikcp_update when handling massive kcp connections)
//---------------------------------------------------------------------
uint32_t KcpCheck(const xKcpControlBlock * kcp, uint32_t current) {
	uint32_t ts_flush = kcp->ts_flush;

	if (kcp->updated == 0) {
		return current;
	}

	if (_itimediff(current, ts_flush) >= 10000 || _itimediff(current, ts_flush) < -10000) {
		ts_flush = current;
	}

	if (_itimediff(current, ts_flush) >= 0) {
		return current;
	}

	auto tm_flush  = _itimediff(ts_flush, current);
	auto tm_packet = std::numeric_limits<decltype(tm_flush)>::min();
	for (const auto & Node : kcp->snd_buf) {
		auto seg  = static_cast<const xKcpSegment *>(&Node);
		auto diff = _itimediff(seg->resendts, current);
		if (diff <= 0) {
			return current;
		}
		if (diff < tm_packet) {
			tm_packet = diff;
		}
	}

	auto minimal = std::min(tm_packet, tm_flush);
	if (minimal >= kcp->interval) {
		minimal = kcp->interval;
	}

	return current + minimal;
}

bool KcpSetMtu(xKcpControlBlock * kcp, uint32_t mtu) {
	assert(mtu <= std::numeric_limits<std::make_signed_t<decltype(kcp->mtu)>>::max());
	if (mtu < 50 || mtu < IKCP_OVERHEAD) {
		return false;
	}
	auto buffer = (ubyte *)ikcp_malloc((mtu + IKCP_OVERHEAD) * 3);
	if (buffer == NULL) {
		return false;
	}
	kcp->mtu = mtu;
	kcp->mss = kcp->mtu - IKCP_OVERHEAD;
	ikcp_free(kcp->buffer);
	kcp->buffer = buffer;
	return true;
}

void ikcp_interval(xKcpControlBlock * kcp, uint32_t interval) {
	if (interval > 5000) {
		interval = 5000;
	} else if (interval < 10) {
		interval = 10;
	}
	kcp->interval = interval;
	return;
}

void KcpNoDelay(xKcpControlBlock * kcp, int32_t nodelay, uint32_t interval, int32_t resend, int32_t nc) {
	if (nodelay >= 0) {
		kcp->nodelay = nodelay;
		if (nodelay) {
			kcp->rx_minrto = IKCP_RTO_NDL;
		} else {
			kcp->rx_minrto = IKCP_RTO_MIN;
		}
	}
	if (interval >= 0) {
		if (interval > 5000)
			interval = 5000;
		else if (interval < 10)
			interval = 10;
		kcp->interval = interval;
	}
	if (resend >= 0) {
		kcp->fastresend = resend;
	}
	if (nc >= 0) {
		kcp->nocwnd = nc;
	}
}

void KcpSetWindowSize(xKcpControlBlock * kcp, ssize_t sndwnd, ssize_t rcvwnd) {
	assert(sndwnd <= std::numeric_limits<std::make_signed_t<decltype(kcp->snd_wnd)>>::max());
	assert(rcvwnd <= std::numeric_limits<std::make_signed_t<decltype(kcp->rcv_wnd)>>::max());
	assert(kcp && sndwnd > 0 && rcvwnd > 0);
	if (sndwnd > 0) {
		kcp->snd_wnd = sndwnd;
	}
	if (rcvwnd > 0) {  // must >= max fragment size
		kcp->rcv_wnd = std::max(rcvwnd, (ssize_t)IKCP_WND_RCV);
	}
}

uint32_t KcpGetWaitSend(const xKcpControlBlock * kcp) {
	// get how many packet is waiting to be sent
	return kcp->nsnd_buf + kcp->nsnd_que;
}

// read conv
uint32_t KcpGetConversation(const void * ptr) {
	auto R = xStreamReader(ptr);
	return R.R4L();
}

X_END
