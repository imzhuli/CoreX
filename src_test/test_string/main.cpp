#include <iostream>
#include <locale/char_util.hpp>
#include <map>

using namespace std;
using namespace xel;

int main() {

	auto M1 = std::map<char32_t, bool>{
		{ 0x40, false },
		{ 0x123, true },
		{ 0x30000, true },
		{ 0x40000, false },
	};

	for (auto & Item : M1) {
		if (IsUnicodeIdStart(Item.first) != Item.second) {
			cerr << "Failed at @" << hex << (int32_t)Item.first << endl;
			return -1;
		}
	}

	auto M2 = std::map<char32_t, bool>{
		{ 0x28, false }, { 0x33, true }, { 0x2f800, true }, { 0xe0100, true }, { 0xe01ef, true }, { 0xe01f0, false },
	};

	for (auto & Item : M2) {
		if (IsUnicodeIdContinue(Item.first) != Item.second) {
			cerr << "Failed at @" << hex << (int32_t)Item.first << endl;
			return -1;
		}
	}

	return 0;
}