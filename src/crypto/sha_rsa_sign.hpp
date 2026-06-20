#pragma once
#include "../core/core_min.hpp"

#include <filesystem>
#include <memory>

X_BEGIN

struct xSha256WithRsaContext;
struct xSha256WithRsaValidatorContext;

class xSha256WithRsa {
public:
	X_API_MEMBER bool Init(const std::filesystem::path & PriKeyPath);
	X_API_MEMBER void Clean();

	X_API_MEMBER bool Sign(void * Output, const void * Data, size_t Size);
	X_API_MEMBER bool Validate(const void * Data, size_t Size, const void * Signature);

private:
	std::unique_ptr<xSha256WithRsaContext> _Context;
};

class xSha256WithRsaValidator {
public:
	X_API_MEMBER bool Init(const std::filesystem::path & PubKeyPath);
	X_API_MEMBER void Clean();

	X_API_MEMBER bool operator()(const void * Data, size_t Size, const void * Signature);

private:
	std::unique_ptr<xSha256WithRsaValidatorContext> _Context;
};

X_END
