/*
 * DFObservationStrategy.cpp
 *
 *  Created on: Jul 25, 2018
 *      Author: mhuertas
 */

#include "DFObservationStrategy.h"

namespace DF {

DFObservationStrategy::DFObservationStrategy() : strategy_session_id(0) {
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
		throw InvalidValue(__PRETTY_FUNCTION__,"Error parsing the strategy.");
	}
}

DFObservationStrategy::ObservationStrategyPtr DFObservationStrategy::next() {
	for (ObservationStrategyMap_t::iterator iter = observation_strategy_map_.begin();
			iter != observation_strategy_map_.end(); ++iter) {
		if (iter->second.element_counter<iter->second.number_element_to_collect &&
				iter->second.is_leaf ) {
			return &(iter->second);
		}
	}
	throw NoEntries(__PRETTY_FUNCTION__,"Not more Observation Strategy nodes");
}

DFObservationStrategy::ObservationStrategyPtr DFObservationStrategy::fromObservationResultId(unsigned long observation_result_id) {
	for (ObservationStrategyMap_t::iterator iter = observation_strategy_map_.begin();
			iter != observation_strategy_map_.end(); ++iter) {
		if (iter->second.observation_result_id==observation_result_id) {
			return &(iter->second);
		}
	}
	throw NotFound(__PRETTY_FUNCTION__,"Observation Strategy not found");
}

DFObservationStrategy::ObservationStrategyPtr DFObservationStrategy::fromId(unsigned long id) {
	for (ObservationStrategyMap_t::iterator iter = observation_strategy_map_.begin();
			iter != observation_strategy_map_.end(); ++iter) {
		if (iter->second.id==id) {
			return &(iter->second);
		}
	}
	throw NotFound(__PRETTY_FUNCTION__,"Observation Strategy not found");
}

bool DFObservationStrategy::isComplete() {
	for (ObservationStrategyMap_t::iterator iter = observation_strategy_map_.begin();
			iter != observation_strategy_map_.end(); ++iter) {
		if (iter->second.element_counter<iter->second.number_element_to_collect) {
			return false;
		}
	}
	return true;
}

bool DFObservationStrategy::isAcquisitionComplete() {
	for (ObservationStrategyMap_t::iterator iter = observation_strategy_map_.begin();
			iter != observation_strategy_map_.end(); ++iter) {
		if (iter->second.element_counter<iter->second.number_element_to_collect &&
				iter->second.is_leaf ) {
			return false;
		}
	}
	return true;
}

void DFObservationStrategy::debug() {

#ifdef DFObservationStrategy_DEBUG
	std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ " << std::endl;
	std::cout << " strategy_session_id: " << strategy_session_id << std::endl;
	for (ObservationStrategyMap_t::iterator iter = observation_strategy_map_.begin();
			iter != observation_strategy_map_.end(); ++iter) {
		std::cout << "id: " << iter->second.id;
		std::cout << " is_leaf: " << iter->second.is_leaf;
		std::cout << " obs_mode: " << iter->second.observation_mode;
		std::cout << " num_element_to_collect: " << iter->second.number_element_to_collect;
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
#endif
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

//----------------------------------------------------------------------
// totalNumberOfElementsToCollect
//----------------------------------------------------------------------
const std::size_t DFObservationStrategy::totalNumberOfElementsToCollect() /*const*/ /*noexcept*/
{
	std::size_t retVal(0);
	for (ObservationStrategyMap_t::iterator it = observation_strategy_map_.begin();
			 it != observation_strategy_map_.end();
			 ++it)
	{
		retVal += it->second.number_element_to_collect;
	}
	return retVal;
}

//----------------------------------------------------------------------
// numberOfCollectedElements
//----------------------------------------------------------------------
const size_t DFObservationStrategy::numberOfCollectedElements() /*const*/ /*noexcept*/
{
	std::size_t retVal(0);
	for (ObservationStrategyMap_t::iterator it = observation_strategy_map_.begin();
			 it != observation_strategy_map_.end();
			 ++it)
	{
		retVal += it->second.element_counter;
	}
	return retVal;
}

//----------------------------------------------------------------------
// numberOfElementsLeftToCollect
//----------------------------------------------------------------------
const size_t DFObservationStrategy::numberOfElementsLeftToCollect() /*const*/ /*noexcept*/
{
	std::size_t retVal(0);
	for (ObservationStrategyMap_t::iterator it = observation_strategy_map_.begin();
			 it != observation_strategy_map_.end();
			 ++it)
	{
		retVal += (it->second.number_element_to_collect - it->second.element_counter);
	}
	return retVal;
}

//----------------------------------------------------------------------
// totalNumberOfElementsToCollectForAcquisition
//----------------------------------------------------------------------
const std::size_t DFObservationStrategy::totalNumberOfElementsToCollectForAcquisition() /*const*/ /*noexcept*/
{
	std::size_t retVal(0);
	for (ObservationStrategyMap_t::iterator it = observation_strategy_map_.begin();
			 it != observation_strategy_map_.end();
			 ++it)
	{
		
		if ( it->second.is_leaf )
		{
			retVal += it->second.number_element_to_collect;
		}
	}
	return retVal;
}

//----------------------------------------------------------------------
// numberOfCollectedElementsForAcquisition
//----------------------------------------------------------------------
const std::size_t DFObservationStrategy::numberOfCollectedElementsForAcquisition() /*const*/ /*noexcept*/
{
	std::size_t retVal(0);
	for (ObservationStrategyMap_t::iterator it = observation_strategy_map_.begin();
			 it != observation_strategy_map_.end();
			 ++it)
	{
		if ( it->second.is_leaf )
		{
			retVal += it->second.element_counter;
		}
	}
	return retVal;
}

//----------------------------------------------------------------------
// numberOfElementsLeftToCollectForAcquisition
//----------------------------------------------------------------------
const std::size_t DFObservationStrategy::numberOfElementsLeftToCollectForAcquisition() /*const*/ /*noexcept*/
{
	std::size_t retVal(0);
	for (ObservationStrategyMap_t::iterator it = observation_strategy_map_.begin();
			 it != observation_strategy_map_.end();
			 ++it)
	{
		if ( it->second.is_leaf )
		{
			retVal += (it->second.number_element_to_collect - it->second.element_counter);
		}
	}
	return retVal;
}

//----------------------------------------------------------------------
// find1stObsStrategyNotAcquisitionCompleted
//----------------------------------------------------------------------
DFObservationStrategyPtr find1stObsStrategyNotAcquisitionCompleted(DFObservationStrategyList &list) 
{
	DFObservationStrategyList::iterator it (list.begin());
	for (; it not_eq list.end(); ++it)
	{
		if ( not (*it)->isAcquisitionComplete() )
		{
			break;
		}
	}

	if ( it == list.end() )
	{
		throw NoEntries(__PRETTY_FUNCTION__,"Observation Strategy not found");

	} 

	return (*it);
}

};
