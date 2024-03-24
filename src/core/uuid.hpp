#include "./core_min.hpp"
#include "./string.hpp"

#include <compare>

X_BEGIN

class xUuid {
public:
	using xRawType = ubyte[16];

	X_INLINE const xRawType & GetData() const {
		return _Data;
	}
	X_INLINE constexpr size_t GetSize() const {
		return sizeof(xRawType);
	}
	X_INLINE std::string ToString() const {
		return StrToHex(_Data, sizeof(_Data));
	}

	X_INLINE xUuid()
		: _Data{} {
	}
	X_INLINE xUuid(const xNoInit &) {
	}
	X_INLINE xUuid(const xGeneratorInit &) {
		Generate();
	}
	X_INLINE xUuid(const xRawType & RawData) {
		memcpy(&_Data, &RawData, sizeof(xRawType));
	}

	X_INLINE std::strong_ordering operator<=>(const xUuid & Other) const {
		return memcmp(_Data, Other._Data, sizeof(xRawType)) <=> 0;
	}

	X_API_MEMBER bool Generate();

private:
	xRawType _Data;
};

X_END
