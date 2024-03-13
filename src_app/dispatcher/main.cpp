#include "./dispatcher.hpp"

#include <core/executable.hpp>

int main(int argc, char ** argv) {

	auto CL = xel::xCommandLine(
		argc, argv,
		{
			{ 'p', nullptr, "producer_address", true },
			{ 'c', nullptr, "consumer_address", true },
		}
	);

	auto OptP = CL["producer_address"];
	auto OptC = CL["consumer_address"];
	if (!OptP() && !OptC()) {
		X_PERROR("missing input");
		return -1;
	}

	auto PA = xel::xNetAddress::Parse(*OptP);
	auto CA = xel::xNetAddress::Parse(*OptC);

	auto DPO = xel::xDispatcherOptions{
		.ProducerAddress = PA,
		.ConsumerAddress = CA,
	};
	auto DP  = xel::xDispatcherService();
	auto DPG = xel::xResourceGuard(DP, DPO);
	if (!DPG) {
		X_PERROR("Failed to init Dispatcher");
		return -1;
	}

	while (true) {
		DP.Tick();
	}

	return 0;
}
