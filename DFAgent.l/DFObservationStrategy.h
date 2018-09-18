/*
 * DFObservationStrategy.h
 *
 *  Created on: Jul 25, 2018
 *      Author: mhuertas
 */

#ifndef DFAGENT_L_DFOBSERVATIONSTRATEGY_H_
#define DFAGENT_L_DFOBSERVATIONSTRATEGY_H_

#include <stack>
#include <string>
#include <vector>
#include <iostream>
#include <string>

#include <json/json.h>

#include <GCSException.h>


class DFObservationStrategy {
public:
	DFObservationStrategy();
	virtual ~DFObservationStrategy();

	void 				build(std::string strategy);
	int 				getObservationMode();
	std::string 		getObservationModeName();
	void 				increaseElementCounter();
	bool 				isFilledUp();
	void 				storeId(int id);
	std::vector<long> 	getStoreIds();
	void 				update();
	void 				removeElement();
	void 				debug();
	bool 				isComplete();

	struct ObservationStrategy
	{
	   int observation_mode_id;
	   std::string observation_mode;
	   int number_element_to_collect;
	   int element_counter;
	   std::vector<long> recipes_identifiers;
	   std::vector<ObservationStrategy> clindrens;
	};
	ObservationStrategy observation_strategy;

	std::stack<ObservationStrategy> observation_strategy_stack;
private:

	ObservationStrategy unserializeStrategy_(Json::Value element);
	void 				debug_1(const  ObservationStrategy &ob,std::stack<ObservationStrategy> observation_strategy_stack);

};

#endif /* DFAGENT_L_DFOBSERVATIONSTRATEGY_H_ */

