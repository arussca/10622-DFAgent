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

//#define DFObservationStrategy_DEBUG 1

class DFObservationStrategy {

public:

	//observation strategy node
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

	//build this object from a string representation in JSON.
	void 				build(std::string strategy);
	//next strategy node
	ObservationStrategyPtr next();
	//strategy node from observation result id
	ObservationStrategyPtr fromObservationResultId(unsigned long observation_result_id);
	//strategy node  from  id
	ObservationStrategyPtr fromId(unsigned long id);
	//is strategy completed?
	bool 				isComplete();
	//is the acquisition for the strategy completed?
	bool 				isAcquisitionComplete();
	//debug
	void 				debug();

	//Strategy session id
	long strategy_session_id;
private:
	std::map<int,ObservationStrategy> observation_strategy_map_;
	//unserializeStrategy_
	void unserializeStrategy_(Json::Value element);
};

#endif /* DFAGENT_L_DFOBSERVATIONSTRATEGY_H_ */
