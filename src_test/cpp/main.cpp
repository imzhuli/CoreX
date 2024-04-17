#include <condition_variable>
#include <core/core_min.hpp>
#include <core/string.hpp>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <server_arch/message.hpp>
#include <string>
#include <thread>

using namespace xel;
using namespace std;

struct xSSS : xBinaryMessage {
	void SerializeMembers() override {
		W(Address);
	}
	void DeserializeMembers() override {
		R(Address);
	}

	xNetAddress Address = {};
};

int main(int argc, char ** argv) {

	auto Addr = xNetAddress::Parse("192.168.123.1", 7788);
	Addr      = xNetAddress::Make6();
	Addr.Port = 1024;
	Touch(Addr);

	xSSS SSS;
	SSS.Address = Addr;

	ubyte Buffer[1024];
	auto  SS = SSS.Serialize(Buffer, sizeof(Buffer));
	cout << HexShow(Buffer, SS) << endl;

	xSSS RRR;
	RRR.Deserialize(Buffer, SS);
	cout << RRR.Address.ToString() << endl;

	return 0;
}
