#include <core/core_min.hpp>
#include <core/core_stream.hpp>
#include <core/functional.hpp>
#include <core/indexed_storage.hpp>
#include <core/string.hpp>
#include <iostream>
#include <network/net_address.hpp>
#include <object/object.hpp>

using namespace xel;
using namespace std;

class xTest {
public:
	void foo() const { cout << "const foo" << endl; }
	void foo() { cout << "foo" << endl; }
};

int main(int argc, char ** argv) {

	auto A1 = xNetAddress::Parse("127.0.0.1");
	auto A2 = xNetAddress::Parse("127.0.0.1:80");
	auto A3 = xNetAddress::Parse("[2402:4e00:101a:f300:0:9f95:4b15:c0db]");
	auto A4 = xNetAddress::Parse("[2001:0db8:85a3:0000:0000:8a2e:0370:7334]:65535");
	auto A5 = xNetAddress::Parse("[::]:1024");
	auto A6 = xNetAddress::Parse("[2001:db8::1]:6553");

	cout << A1.ToString() << endl;
	cout << A2.ToString() << endl;
	cout << A3.ToString() << endl;
	cout << A4.ToString() << endl;
	cout << A5.ToString() << endl;
	cout << A6.ToString() << endl;

	xTest test;

	auto F = Delegate(&xTest::foo, &test);
	F();
	auto FC = Delegate(&xTest::foo, (const xTest *)&test);
	FC();

	return 0;
}
