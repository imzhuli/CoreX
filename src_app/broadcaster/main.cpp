#include "./broadcaster.hpp"

#include <core/executable.hpp>

int main(int argc, char ** argv) {

	auto CL = xel::xCommandLine(
		argc, argv,
		{
			{ 'p', nullptr, "producer_address", true },
			{ 'o', nullptr, "observer_address", true },
		}
	);

	auto OptP = CL["producer_address"];
	auto OptO = CL["observer_address"];
	if (!OptP() && !OptO()) {
		X_PERROR("missing input");
		return -1;
	}

	auto PA = xel::xNetAddress::Parse(*OptP);
	auto CA = xel::xNetAddress::Parse(*OptO);

	auto DPO = xel::xBroadcasterOptions{
		.ProducerAddress = PA,
		.ObserverAddress = CA,
	};
	auto DP  = xel::xBroadcasterService();
	auto DPG = xel::xResourceGuard(DP, DPO);
	if (!DPG) {
		X_PERROR("Failed to init broadcaster");
		return -1;
	}

	while (true) {
		DP.Tick();
	}

	return 0;
}
