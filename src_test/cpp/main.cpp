#include <algorithm/binary_exponentiation.hpp>
#include <atomic>
#include <core/core_min.hpp>
#include <core/core_value_util.hpp>
#include <core/version.hpp>
#include <iostream>
#include <network/net_address.hpp>

using namespace std;
using namespace xel;

int main(int argc, char ** argv) {

	auto S1 = "[fc00:0db8:85a3:0000:0000:8a2e:0370:7334]:1234";
	auto A1 = xNetAddress::Parse(S1);
	cout << A1.IpToString() << endl;
	cout << A1.ToString() << endl;
	A1.Port = 65535;

	auto S2 = A1.IpToString();
	auto A2 = xNetAddress::Parse(S2);
	cout << A2.IpToString() << endl;
	cout << A2.ToString() << endl;

	auto S3 = "192.168.1.1:7334";
	auto A3 = xNetAddress::Parse(S3);
	cout << A3.IpToString() << endl;
	cout << A3.ToString() << endl;

	auto S4 = A3.IpToString();
	auto A4 = xNetAddress::Parse(S4);
	cout << A4.IpToString() << endl;
	cout << A4.ToString() << endl;
	return 0;
}
