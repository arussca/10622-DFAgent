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

namespace DF
{

  class DFObservationStrategy
  {

  //--------------------------------------------------
  // public section
  //--------------------------------------------------
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
    typedef ObservationStrategy * ObservationStrategyPtr;



    DFObservationStrategy();
    virtual ~DFObservationStrategy();

    //build this object from a string representation in JSON.
    void         build(std::string strategy);
    //next strategy node
    ObservationStrategyPtr next();
    //strategy node from observation result id
    ObservationStrategyPtr fromObservationResultId(unsigned long observation_result_id);
    //strategy node  from  id
    ObservationStrategyPtr fromId(unsigned long id);
    //is strategy completed?
    bool         isComplete();
    //is the acquisition for the strategy completed?
    bool         isAcquisitionComplete();
    //debug
    void         debug();

    /**
     * Computes the total number of elements to be collected by
     * all the nodes that create this observation strategy.
     *
     * @return The summation of number_element_to_collect member of all nodes.
     */
    const std::size_t totalNumberOfElementsToCollect() /*const*/ /*noexcept*/;

    /**
     * Computes the number of elements already collected by
     * all the nodes that create this observation strategy.
     *
     * @return The summation of element_counter member of all nodes.
     */
    const std::size_t numberOfCollectedElements() /*const*/ /*noexcept*/;

    /**
     * Computes the current number of elements to be collected by
     * all the nodes that create this observation strategy.
     *
     * @return The summation of the following subtraction
     *         (\f{ number_element_to_collect - element_counter member \f}),
     *         for each node.
     */
    const std::size_t numberOfElementsLeftToCollect() /*const*/ /*noexcept*/;

    /**
     * Computes the total number of elements to be collected only by
     * those nodes that are leaf in this observation strategy.
     *
     * @return The summation of number_element_to_collect member of all leaf nodes.
     */
    const std::size_t totalNumberOfElementsToCollectForAcquisition() /*const*/ /*noexcept*/;

    /**
     * Computes the number of elements already collected by
     * those nodes that are leaf in this observation strategy.
     *
     * @return The summation of element_counter member of all leaf nodes.
     */
    const std::size_t numberOfCollectedElementsForAcquisition() /*const*/ /*noexcept*/;

    /**
     * Computes the current number of elements to be collected by
     * those nodes that are leaf in this observation strategy.
     *
     * @return The summation of the following subtraction
     *         (\f{ number_element_to_collect - element_counter member \f}),
     *         for each leaf node.
     */
    const std::size_t numberOfElementsLeftToCollectForAcquisition() /*const*/ /*noexcept*/;

    //Strategy session id
    long strategy_session_id;


  //--------------------------------------------------
  // private section
  //--------------------------------------------------
  private:

    //############################ PRIVATE TYPES ###########################

    /**
     * Decleration of an alias type to represent the relationship of
     * the strategies and theirs identifiers.
     */
    typedef std::map<int,ObservationStrategy> ObservationStrategyMap_t;

    ObservationStrategyMap_t observation_strategy_map_;
    //unserializeStrategy_
    void unserializeStrategy_(Json::Value element);

    /**
     * Forward class declaration, which will be reserved to
     * perform the unit test.
     */
    friend class DFObservationStrategyTest;

  }; // DFObservationStrategy

  /** Alias to define a pointer of DFObservationStrategy. */
  typedef DFObservationStrategy * DFObservationStrategyPtr;

  /** Alias to define a vector container of DFObservationStrategy pointers */
  typedef std::vector<DFObservationStrategyPtr> DFObservationStrategyList;

  /**
   * Find the first observation strategy in the given list, which is not completed
   * for acquisition.
   *
   * @param[in] list Container where the observation strategies are stored.
   * @return The first observation strategy, which belongs to the given list
   *         and it is not completed for acquisition.
   */
  DFObservationStrategyPtr find1stObsStrategyNotAcquisitionCompleted(DFObservationStrategyList& list);
}
#endif /* DFAGENT_L_DFOBSERVATIONSTRATEGY_H_ */
