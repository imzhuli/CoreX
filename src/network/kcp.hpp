/***
 *
 * a cpp rewriten version based on authorskywind3000/kcp
 *
 */

#pragma once
#include "../core/core_min.hpp"
#include "../core/list.hpp"

X_BEGIN

struct xKcpSegmentNode : xListNode {};
using xKcpSegmentList = xList<xKcpSegmentNode>;

struct xKcpSegment : xKcpSegmentNode {
	uint32_t conv;
	uint32_t cmd;
	uint32_t frg;
	uint32_t wnd;
	uint32_t ts;
	uint32_t sn;
	uint32_t una;
	uint32_t len;
	uint32_t resendts;
	uint32_t rto;
	uint32_t fastack;
	uint32_t xmit;
	ubyte    data[1];
};

struct xKcpControlBlock {
	uint32_t conv, mtu, mss, state;
	uint32_t snd_una, snd_nxt, rcv_nxt;
	uint32_t ts_recent, ts_lastack, ssthresh;
	uint32_t rx_rttval, rx_srtt, rx_rto, rx_minrto;  // signed
	uint32_t snd_wnd, rcv_wnd, rmt_wnd, cwnd, probe;
	uint32_t current, interval, ts_flush, xmit;
	uint32_t nrcv_buf, nsnd_buf;
	uint32_t nrcv_que, nsnd_que;
	uint32_t nodelay, updated;
	uint32_t ts_probe, probe_wait;
	uint32_t dead_link, incr;

	xKcpSegmentList snd_queue;
	xKcpSegmentList rcv_queue;
	xKcpSegmentList snd_buf;
	xKcpSegmentList rcv_buf;

	uint32_t * acklist;
	uint32_t   ackcount;
	uint32_t   ackblock;
	void *     user;
	ubyte *    buffer;
	ssize_t    fastresend;
	ssize_t    fastlimit;
	ssize_t    nocwnd, stream;
	uint32_t   logmask;

	ssize_t (*output)(const ubyte * buf, ssize_t len, xKcpControlBlock * kcp, void * user);
	void (*writelog)(const char * log, xKcpControlBlock * kcp, void * user);
};

#define IKCP_LOG_OUTPUT    (uint32_t(1))
#define IKCP_LOG_INPUT     (uint32_t(2))
#define IKCP_LOG_SEND      (uint32_t(4))
#define IKCP_LOG_RECV      (uint32_t(8))
#define IKCP_LOG_IN_DATA   (uint32_t(16))
#define IKCP_LOG_IN_ACK    (uint32_t(32))
#define IKCP_LOG_IN_PROBE  (uint32_t(64))
#define IKCP_LOG_IN_WINS   (uint32_t(128))
#define IKCP_LOG_OUT_DATA  (uint32_t(256))
#define IKCP_LOG_OUT_ACK   (uint32_t(512))
#define IKCP_LOG_OUT_PROBE (uint32_t(1024))
#define IKCP_LOG_OUT_WINS  (uint32_t(2048))

// create a new kcp control object, 'conv' must equal in two endpoint
// from the same connection. 'user' will be passed to the output callback
// output callback can be setup like this: 'kcp->output = my_udp_output'
X_API xKcpControlBlock * KcpCreate(uint32_t conv, void * user);

// release kcp control object
X_API void KcpDestroy(xKcpControlBlock * kcp);

// set output callback, which will be invoked by kcp
X_API void KcpSetOutput(xKcpControlBlock * kcp, ssize_t (*output)(const ubyte * buf, ssize_t len, xKcpControlBlock * kcp, void * user));

// user/upper level recv: returns size, returns below zero for EAGAIN
X_API ssize_t KcpRecv(xKcpControlBlock * kcp, void * buffer, ssize_t len);

// user/upper level send, returns below zero for error
X_API ssize_t KcpSend(xKcpControlBlock * kcp, const ubyte * buffer, ssize_t len);

// update state (call it repeatedly, every 10ms-100ms), or you can ask
// ikcp_check when to call it again (without ikcp_input/_send calling).
// 'current' - current timestamp in millisec.
X_API void KcpUpdate(xKcpControlBlock * kcp, uint32_t current);

// Determine when should you invoke ikcp_update:
// returns when you should invoke ikcp_update in millisec, if there
// is no ikcp_input/_send calling. you can call ikcp_update in that
// time, instead of call update repeatly.
// Important to reduce unnacessary ikcp_update invoking. use it to
// schedule ikcp_update (eg. implementing an epoll-like mechanism,
// or optimize ikcp_update when handling massive kcp connections)
X_API uint32_t KcpCheck(const xKcpControlBlock * kcp, uint32_t current);

// when you received a low level packet (eg. UDP packet), call it
X_API ssize_t KcpInput(xKcpControlBlock * kcp, const void * data, ssize_t size);

// flush pending data
X_API void KcpFlush(xKcpControlBlock * kcp);

// check the size of next message in the recv queue
X_API ssize_t KcpPeekSize(xKcpControlBlock * kcp);

// change MTU size, default is 1400
X_API bool KcpSetMtu(xKcpControlBlock * kcp, uint32_t mtu);

// set maximum window size: sndwnd=32, rcvwnd=32 by default
X_API void KcpSetWindowSize(xKcpControlBlock * kcp, ssize_t sndwnd, ssize_t rcvwnd);

// get how many packet is waiting to be sent
X_API uint32_t KcpGetWaitSend(const xKcpControlBlock * kcp);

// fastest: ikcp_nodelay(kcp, 1, 20, 2, 1)
// nodelay: <0: make no change, 0:disable(default), 1:enable
// interval: internal update timer interval in millisec, default is 100ms
// resend: <0: make no change, 0:disable fast resend(default), 1:enable fast resend
// nc: <0: make no change, 0:normal congestion control(default), 1:no congestion control
X_API void KcpNoDelay(xKcpControlBlock * kcp, int32_t nodelay, uint32_t interval, int32_t resend, int32_t nc);

// setup allocator
X_API void KcpSetAllocator(void * (*new_malloc)(size_t), void (*new_free)(void *));

// read conv
X_API uint32_t KcpGetConversation(const void * ptr);

X_END
