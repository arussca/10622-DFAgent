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


void DFObservationStrategy::build(std::string strategy) {
	Json::Value root;
	Json::Reader reader;
	bool parsingSuccessful = reader.parse(strategy,root);

	if ( parsingSuccessful )
	{
		unserializeStrategy_(root);
	} else {
		throw InvalidValue("DFObservationStrategy::build", "Error parsing the strategy.");
	}
}

DFObservationStrategy::ObservationStrategyPtr DFObservationStrategy::next() {
	for (std::map<int,ObservationStrategy>::iterator iter = observation_strategy_map_.begin();
			iter != observation_strategy_map_.end(); ++iter) {
		if (iter->second.element_counter<iter->second.number_element_to_collect &&
				iter->second.is_leaf ) {
			return &(iter->second);
		}
	}
	throw NoEntries("Not more Observation Strategy nodes");
}

DFObservationStrategy::ObservationStrategyPtr DFObservationStrategy::fromObservationResultId(unsigned long observation_result_id) {
	for (std::map<int,ObservationStrategy>::iterator iter = observation_strategy_map_.begin();
			iter != observation_strategy_map_.end(); ++iter) {
		if (iter->second.observation_result_id==observation_result_id) {
			return &(iter->second);
		}
	}
	throw NotFound("Observation Strategy not found");
}


DFObservationStrategy::ObservationStrategyPtr DFObservationStrategy::fromId(unsigned long id) {
	for (std::map<int,ObservationStrategy>::iterator iter = observation_strategy_map_.begin();
			iter != observation_strategy_map_.end(); ++iter) {
		if (iter->second.id==id) {
			return &(iter->second);
		}
	}
	throw NotFound("Observation Strategy not found");
}

bool DFObservationStrategy::isComplete() {
	for (std::map<int,ObservationStrategy>::iterator iter = observation_strategy_map_.begin();
			iter != observation_strategy_map_.end(); ++iter) {
		if (iter->second.element_counter<iter->second.number_element_to_collect) {
			return false;
		}
	}
	return true;
}

void DFObservationStrategy::debug() {
	std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ " << std::endl;
	for (std::map<int,ObservationStrategy>::iterator iter = observation_strategy_map_.begin();
			iter != observation_strategy_map_.end(); ++iter) {

		std::cout << "id: " << iter->second.id;
		std::cout << " is_leaf: " << iter->second.is_leaf;
		std::cout << " observation_mode: " << iter->second.observation_mode;
		std::cout << " number_element_to_collect: " << iter->second.number_element_to_collect;
		std::cout << " element_counter: " << iter->second.element_counter;
		std::cout << " session_id: " << iter->second.session_id;
		std::cout << " observation_result_id: " << iter->second.observation_result_id;
		std::cout << " recipes_identifiers: [";
		for (int i=0;i<iter->second.recipes_identifiers.size();i++) {
			std::cout << iter->second.recipes_identifiers[i] << ",";
		}
		std::cout << " ]";

		std::cout << " next: [";
		for (int i=0;i<iter->second.next.size();i++) {
			std::cout << iter->second.next[i] << ",";
		}
		std::cout << " ]" << std::endl;

	}
	std::cout << "----------------------------------------------------------- " << std::endl;
}

void DFObservationStrategy::unserializeStrategy_(Json::Value element) {
	Json::Value  strategies = element["strategy"];
	for (int i = 0; i<strategies.size() ; i++ ) {
		int id = strategies[i]["id"].asInt();
		ObservationStrategy  observation_strategy;
		observation_strategy.observation_mode=strategies[i]["observation_mode"].asCString();
		observation_strategy.is_leaf=strategies[i]["is_leaf"].asBool();
		observation_strategy.id=id;
		observation_strategy.number_element_to_collect=strategies[i]["number_element_to_collect"].asInt();
		observation_strategy.element_counter=0;
		observation_strategy.observation_result_id=0;
		observation_strategy.session_id=0;
		observation_strategy_map_[id] = observation_strategy;
		Json::Value  next = strategies[i]["next"];
		for (int j=0;j<next.size();j++) {
			observation_strategy_map_[id].next.push_back(next[j].asInt());
		}
	}
}
