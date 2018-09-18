/*
 * DFObservationStrategy.cpp
 *
 *  Created on: Jul 25, 2018
 *      Author: mhuertas
 */

#include "DFObservationStrategy.h"

DFObservationStrategy::DFObservationStrategy() {
}

DFObservationStrategy::~DFObservationStrategy() {
}


DFObservationStrategy::ObservationStrategy DFObservationStrategy::unserializeStrategy_(Json::Value element)
{
	ObservationStrategy observation_strategy;
	observation_strategy.observation_mode=element["observation_mode"].asCString();
	observation_strategy.number_element_to_collect=element["number_element_to_collect"].asInt();
	observation_strategy.element_counter=0;

	Json::Value  clindrens = element["clindrens"];
	for (int i = 0; i<clindrens.size() ; i++ )
	{
		ObservationStrategy observation_strategy_children = unserializeStrategy_(clindrens[i]);
		observation_strategy.clindrens.push_back(observation_strategy_children);
	}

	return observation_strategy;
}

void DFObservationStrategy::build(std::string strategy) {
	Json::Value root;
	Json::Reader reader;
	bool parsingSuccessful = reader.parse(strategy,root);

	if ( parsingSuccessful )
	{
		observation_strategy = unserializeStrategy_(root);
	} else {
		throw InvalidValue("DFObservationStrategy::build", "Error parsing the strategy.");
	}
}

int DFObservationStrategy::getObservationMode() {
	return observation_strategy_stack.top().observation_mode_id;
}

std::string DFObservationStrategy::getObservationModeName() {
	return observation_strategy_stack.top().observation_mode;
}

void DFObservationStrategy::increaseElementCounter() {
	observation_strategy_stack.top().element_counter++;
}

bool DFObservationStrategy::isFilledUp() {
    if ( observation_strategy_stack.top().element_counter ==
    		observation_strategy_stack.top().number_element_to_collect )  {
    	return true;
    }
    return false;
}

void DFObservationStrategy::storeId(int id) {
	observation_strategy_stack.top().recipes_identifiers.push_back(id);
}

std::vector<long> DFObservationStrategy::getStoreIds() {
	return observation_strategy_stack.top().recipes_identifiers;
}

void DFObservationStrategy::update() {
	ObservationStrategy observation_strategy_current = observation_strategy;
	if (!observation_strategy_stack.empty()) {
		observation_strategy_current = observation_strategy_stack.top();
		observation_strategy_stack.pop();
	}

	bool end = false;
	do {
		if (observation_strategy_current.element_counter < observation_strategy_current.number_element_to_collect) {
			observation_strategy_stack.push(observation_strategy_current);
			if (observation_strategy_current.clindrens.size() == 0  )  {
				end = true;
			} else {
				observation_strategy_current = observation_strategy_current.clindrens[observation_strategy_current.element_counter];
			}
		}
	} while (!end);
}

void DFObservationStrategy::removeElement() {
	observation_strategy_stack.pop();
}

bool DFObservationStrategy::isComplete() {
	return observation_strategy_stack.empty();
}

void DFObservationStrategy::debug() {
	debug_1(observation_strategy,observation_strategy_stack);
}

void DFObservationStrategy::debug_1(const ObservationStrategy & ob,std::stack<ObservationStrategy> observation_strategy_stack) {
	std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
	while (!observation_strategy_stack.empty()) {
		ObservationStrategy  observation_strategy_current = observation_strategy_stack.top();
		std::cout << "observation_mode:" << observation_strategy_stack.top().observation_mode << std::endl;
		std::cout << "element_counter:" << observation_strategy_stack.top().element_counter << std::endl;
		std::cout << "number_element_to_collect:" << observation_strategy_stack.top().number_element_to_collect << std::endl;
		std::cout << "-------------------------------------------------------" << std::endl;
		observation_strategy_stack.pop();
	 }
}
