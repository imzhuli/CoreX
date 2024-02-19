#include <core/executable.hpp>
#include <core/string.hpp>
#include <crypto/base64.hpp>
#include <crypto/sha_rsa_sign.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace xel;

int main(int argc, char ** argv) {

	auto CL = xCommandLine(
		argc, argv,
		{
			{ 'k', "pri-keyfile", "pri-keyfile", true },
			{ 'v', "validator-keyfile", "validator-keyfile", true },
		}
	);

	auto OptKey = CL["pri-keyfile"];
	if (!OptKey()) {
		cerr << "A private key file must be provided" << endl;
		return -1;
	}

	string Source      = "abcdefg";
	auto   Signer      = xSha256WithRsa();
	auto   SignerGuard = xResourceGuard(Signer, *OptKey);
	if (!SignerGuard) {
		cerr << "failed to load key file" << endl;
		return -1;
	}

	auto SignView = Signer(Source.data(), Source.length());
	cout << "SignLength: " << SignView.Size() << endl;

	auto Validate = Signer.Validate(Source.data(), Source.length(), SignView.Data());
	cout << "Validate: " << Validate << endl;

	auto Encoded = Base64Encode(SignView.Data(), SignView.Size());
	cout << "Encode: " << Encoded << endl;

	auto Decoded = Base64Decode(Encoded.data(), Encoded.size());
	cout << "Decoded length: " << Decoded.length() << endl;

	auto Validate2 = Signer.Validate(Source.data(), Source.length(), Decoded.data());
	cout << "Validate2: " << Validate2 << endl;

	auto OptValidatorKey = CL["validator-keyfile"];
	if (OptValidatorKey()) {
		auto Validator      = xSha256WithRsaValidator();
		auto ValidatorGuard = xResourceGuard(Validator, *OptValidatorKey);

		auto ValidateResult = Validator(Source.data(), Source.length(), Decoded.data());
		cout << "Validator: " << ValidateResult << endl;
	}

	return 0;
}
