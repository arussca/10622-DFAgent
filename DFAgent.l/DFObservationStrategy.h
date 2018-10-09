/*
 * DFObservationStrategy.h
 *
 *  Created on: Jul 25, 2018
 *      Author: mhuertas
 */

#ifndef DFAGENT_L_DFOBSERVATIONSTRATEGY_H_
#define DFAGENT_L_DFOBSERVATIONSTRATEGY_H_

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <string>
#include <memory>

#include <json/json.h>
#include <GCSException.h>

class DFObservationStrategy {

public:

	//observation strategy
	struct ObservationStrategy
	{
	   int id;
	   bool is_leaf;
	   int observation_mode_id;
	   unsigned long observation_result_id;
	   unsigned int session_id;
	   std::string observation_mode;
	   int number_element_to_collect;
	   int element_counter;
	   std::vector<long> recipes_identifiers;
	   std::vector<int> next;
	};
	typedef struct  ObservationStrategy * ObservationStrategyPtr;

	DFObservationStrategy();
	virtual ~DFObservationStrategy();

	//build
	void 				build(std::string strategy);
	//next
	ObservationStrategyPtr next();
	//from observation result id
	ObservationStrategyPtr fromObservationResultId(unsigned long observation_result_id);
	//from  id
	ObservationStrategyPtr fromId(unsigned long id);
	//it is completed
	bool 				isComplete();
	//debug
	void 				debug();


private:

	std::map<int,ObservationStrategy> observation_strategy_map_;

	//unserializeStrategy_
	void unserializeStrategy_(Json::Value element);
	//debug_1
	void 				debug_1();

};

#endif /* DFAGENT_L_DFOBSERVATIONSTRATEGY_H_ */
