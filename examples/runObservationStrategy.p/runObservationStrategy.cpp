#include <iostream>
#include <string>
#include "DFObservationStrategy.h"

int main() {

	std::cout << "Hello!" << std::endl;

	std::string strategy= "{ \"strategy\" : [ {\"id\":1,\"observation_mode\":\"r1_1\",\"number_element_to_collect\":2,\"next\":[5] }, { \"id\":2,\"observation_mode\":\"r1_2\",\"number_element_to_collect\":2,\"next\":[5] }, { \"id\":3,\"observation_mode\":\"r1_3\",\"number_element_to_collect\":2,\"next\":[6] }, { \"id\":4,\"observation_mode\":\"r1_4\",\"number_element_to_collect\":2,\"next\":[6] },{ \"id\":5,\"observation_mode\":\"r2_1\",\"number_element_to_collect\":2,\"next\":[7] },{ \"id\":6,\"observation_mode\":\"r2_2\",\"number_element_to_collect\":2,\"next\":[7] },{ \"id\":7,\"observation_mode\":\"r3_1\",\"number_element_to_collect\":2,\"next\":[] } ] }";

	DF::DFObservationStrategy observation_strategy;

	try {
		observation_strategy.build(strategy);
	} catch (...) {
		std::cout << "NO OK" << std::endl;
	}

}
