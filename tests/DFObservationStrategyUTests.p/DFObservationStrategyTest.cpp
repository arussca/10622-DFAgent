// Copyright (c) 2019 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved
//
//
// $Log$

static const char rcsid[]="@(#) $Id$";

// Includes
#include "DFObservationStrategyTest.h"    // DFObservationStrategyTest
#include <DFObservationStrategy.h>        // DFObservationStrategy
#include <json/json.h>                    // Json::Value

/************************************************************************
*************************************************************************
**                                                                     **
**                                Macros                               **
**                                                                     **
*************************************************************************
************************************************************************/
//<editor-fold desc="Enable or disable debug trace via source code">
//#define DFObservationStrategyTest_DEBUG // Enable debug trace
//#undef DFObservationStrategyTest_DEBUG  // Disable debug trace
//</editor-fold>

//<editor-fold desc="Define DFObservationStrategyTest_TRACE">
#if defined(DFObservationStrategyTest_DEBUG)
#include <ace/Trace.h>                    // ACE_Trace
#define DFObservationStrategyTest_TRACE \
  ACE_Trace ace_trace(__func__, __LINE__, __FILE__);\
  ace_trace.start_tracing();\
  ace_trace.set_nesting_indent(1);
#else
#define DFObservationStrategyTest_TRACE \
  ;
#endif
//</editor-fold>

//----------------------------------------------------------------------
// main
//----------------------------------------------------------------------
int main(int argc, char **argv)
{
  DFObservationStrategyTest_TRACE
  try
  {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
  }
  catch (...)
  {
    std::cerr << "An exception occurs." << std::endl;
    return -1;
  }
}

/************************************************************************
*************************************************************************
**                                                                     **
**                     Static and constants members                    **
**                                                                     **
*************************************************************************
************************************************************************/

Trace DFObservationStrategyTest::trace_ = Trace("DFObservationStrategyTest");

/************************************************************************
*************************************************************************
**                                                                     **
**                     Constructors and destructor                     **
**                                                                     **
*************************************************************************
************************************************************************/

//----------------------------------------------------------------------
// Default constructor
//----------------------------------------------------------------------
DFObservationStrategyTest::DFObservationStrategyTest()
:
  strategyJSON_()
{
  DFObservationStrategyTest_TRACE
}

//----------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------
DFObservationStrategyTest::~DFObservationStrategyTest()
{
  DFObservationStrategyTest_TRACE
  strategyJSON_.clear();
}

/************************************************************************
*************************************************************************
**                                                                     **
**                           Public methods                            **
**                                                                     **
*************************************************************************
************************************************************************/

/************************************************************************
*************************************************************************
**                                                                     **
**                         Protected methods                           **
**                                                                     **
*************************************************************************
************************************************************************/

//----------------------------------------------------------------------
// Code here will be called immediately after the constructor (right
// before each test).
//----------------------------------------------------------------------
void DFObservationStrategyTest::SetUp()
{
  DFObservationStrategyTest_TRACE

  // Initialize strategyJSON_ member
  strategyJSON_.clear();
  Json::Value recipe;
  // recipe 1
  recipe["id"]                        = 1;
  recipe["observation_mode"]          = "r1";
  recipe["is_leaf"]                   = true;
  recipe["number_element_to_collect"] = 2;
  recipe.removeMember("next");
  recipe["next"].append(3);
  strategyJSON_["strategy"].append(recipe);
  // recipe 2
  recipe["id"]                        = 2;
  recipe["observation_mode"]          = "r2";
  recipe["is_leaf"]                   = true;
  recipe["number_element_to_collect"] = 2;
  recipe.removeMember("next");
  recipe["next"].append(3);
  strategyJSON_["strategy"].append(recipe);
  // recipe 3
  recipe["id"]                        = 3;
  recipe["observation_mode"]          = "r3";
  recipe["is_leaf"]                   = false;
  recipe["number_element_to_collect"] = 2;
  recipe.removeMember("next");
  recipe["next"].append(Json::Value());
  strategyJSON_["strategy"].append(recipe);
}

//----------------------------------------------------------------------
// Code here will be called immediately after each test (right
// before the destructor).
//----------------------------------------------------------------------
void DFObservationStrategyTest::TearDown()
{
  DFObservationStrategyTest_TRACE
}

//----------------------------------------------------------------------
// buildObservationStrategy_
//----------------------------------------------------------------------
bool DFObservationStrategyTest::buildObservationStrategy_(DF::DFObservationStrategy *&pointer)
{
  if ( pointer == nullptr )
  {
    return false;
  }

  try
  {
    Json::FastWriter fastWriter;
    std::string strategy_str = fastWriter.write(strategyJSON_);
    DFObservationStrategyTest::trace_.out("Strategy to build is '%s'", strategy_str.data());
    pointer->build(strategy_str);
  }
  catch (GCSException& ex)
  {
    ex.addToTrace(__PRETTY_FUNCTION__);
    ex.addText("Unable to build strategy.");
    trace_.err(ex.text());
    throw ;
  }
  return true;

}

/************************************************************************
*************************************************************************
**                                                                     **
**                           Private methods                           **
**                                                                     **
*************************************************************************
************************************************************************/

//----------------------------------------------------------------------
//----------------------------------------------------------------------


/************************************************************************
*************************************************************************
**                                                                     **
**                              Unit test                              **
**                                                                     **
*************************************************************************
************************************************************************/

//----------------------------------------------------------------------
// Tests whether the default constructor and destructor works
//----------------------------------------------------------------------
TEST_F(DFObservationStrategyTest, DFObservationStrategyTest_DefaultConstructor_SetUp_TearDown_Destructor)
{
  DFObservationStrategyTest_TRACE

  ;
}

//----------------------------------------------------------------------
// Tests that the DFObservationStrategy constructor from plain text works
//----------------------------------------------------------------------
TEST_F(DFObservationStrategyTest, DFObservationStrategy_DefaultConstructor_Destructor)
{
  DFObservationStrategyTest_TRACE

  const auto * pointer ( new (std::nothrow) DF::DFObservationStrategy() );
  ASSERT_TRUE( pointer not_eq nullptr );

  delete pointer;
  pointer = nullptr;
}

//----------------------------------------------------------------------
// Tests that DFObservationStrategy::build method works
//----------------------------------------------------------------------
TEST_F(DFObservationStrategyTest, DFObservationStrategy_build)
{
  DFObservationStrategyTest_TRACE

  DF::DFObservationStrategy * pointer ( new (std::nothrow) DF::DFObservationStrategy() );
  ASSERT_TRUE( pointer not_eq nullptr );

  ASSERT_TRUE(buildObservationStrategy_(pointer));

  delete pointer;
  pointer = nullptr;
}

//----------------------------------------------------------------------
// Tests that DFObservationStrategy::next method works
//----------------------------------------------------------------------
TEST_F(DFObservationStrategyTest, DFObservationStrategy_next)
{
  DFObservationStrategyTest_TRACE

  DF::DFObservationStrategy * pointer ( new (std::nothrow) DF::DFObservationStrategy() );
  ASSERT_TRUE( pointer not_eq nullptr );

  ASSERT_TRUE(buildObservationStrategy_(pointer));

  DF::DFObservationStrategy::ObservationStrategyPtr firstNonCompletedLeafNode (nullptr);

  // Obtain and check the first leaf node marked as incomplete.
  firstNonCompletedLeafNode = pointer->next();
  ASSERT_TRUE( firstNonCompletedLeafNode not_eq nullptr );
  ASSERT_TRUE( firstNonCompletedLeafNode->is_leaf );
  ASSERT_TRUE( firstNonCompletedLeafNode->element_counter < firstNonCompletedLeafNode->number_element_to_collect );
  ASSERT_EQ( 1, firstNonCompletedLeafNode->id );

  // Mark leaf node as completed
  firstNonCompletedLeafNode->element_counter = firstNonCompletedLeafNode->number_element_to_collect;

  // Obtain and check the first leaf node marked as incomplete.
  firstNonCompletedLeafNode = pointer->next();
  ASSERT_TRUE( firstNonCompletedLeafNode not_eq nullptr );
  ASSERT_TRUE( firstNonCompletedLeafNode->is_leaf );
  ASSERT_TRUE( firstNonCompletedLeafNode->element_counter < firstNonCompletedLeafNode->number_element_to_collect );
  ASSERT_EQ( 2, firstNonCompletedLeafNode->id );
  ASSERT_TRUE( pointer not_eq nullptr );
  firstNonCompletedLeafNode->element_counter = firstNonCompletedLeafNode->number_element_to_collect;

  // No more incomplete leaf nodes.
  ASSERT_ANY_THROW( pointer->next(); );

  delete pointer;
  pointer = nullptr;
}

//----------------------------------------------------------------------
// Tests that DFObservationStrategy::totalNumberOfElementsToCollect method works
//----------------------------------------------------------------------
TEST_F(DFObservationStrategyTest, DFObservationStrategy_totalNumberOfElementsToCollect)
{
  DFObservationStrategyTest_TRACE

  DF::DFObservationStrategy * pointer ( new (std::nothrow) DF::DFObservationStrategy() );
  ASSERT_TRUE( pointer not_eq nullptr );

  ASSERT_TRUE(buildObservationStrategy_(pointer));

  EXPECT_EQ(6, pointer->totalNumberOfElementsToCollect());

  delete pointer;
  pointer = nullptr;
}

//----------------------------------------------------------------------
// Tests that DFObservationStrategy::numberOfCollectedElements method works
//----------------------------------------------------------------------
TEST_F(DFObservationStrategyTest, DFObservationStrategy_numberOfCollectedElements)
{
  DFObservationStrategyTest_TRACE

  DF::DFObservationStrategy * pointer ( new (std::nothrow) DF::DFObservationStrategy() );
  ASSERT_TRUE( pointer not_eq nullptr );

  ASSERT_TRUE(buildObservationStrategy_(pointer));

  EXPECT_EQ(0, pointer->numberOfCollectedElements());

  // Obtain and check the first leaf node marked as incomplete.
  DF::DFObservationStrategy::ObservationStrategyPtr firstNonCompletedLeafNode (nullptr);

  firstNonCompletedLeafNode = pointer->next();
  ASSERT_TRUE(firstNonCompletedLeafNode not_eq nullptr);
  firstNonCompletedLeafNode->element_counter++;
  EXPECT_EQ(1, pointer->numberOfCollectedElements());
  firstNonCompletedLeafNode->element_counter++;
  EXPECT_EQ(2, pointer->numberOfCollectedElements());

  firstNonCompletedLeafNode = pointer->next();
  ASSERT_TRUE(firstNonCompletedLeafNode not_eq nullptr);
  firstNonCompletedLeafNode->element_counter++;
  EXPECT_EQ(3, pointer->numberOfCollectedElements());
  firstNonCompletedLeafNode->element_counter++;
  EXPECT_EQ(4, pointer->numberOfCollectedElements());

  delete pointer;
  pointer = nullptr;
}

//----------------------------------------------------------------------
// Tests that DFObservationStrategy::numberOfElementsLeftToCollect method works
//----------------------------------------------------------------------
TEST_F(DFObservationStrategyTest, DFObservationStrategy_numberOfElementsLeftToCollect)
{
  DFObservationStrategyTest_TRACE

  DF::DFObservationStrategy * pointer ( new (std::nothrow) DF::DFObservationStrategy() );
  ASSERT_TRUE( pointer not_eq nullptr );

  ASSERT_TRUE(buildObservationStrategy_(pointer));

  EXPECT_EQ(6, pointer->numberOfElementsLeftToCollect());

  // Obtain and check the first leaf node marked as incomplete.
  DF::DFObservationStrategy::ObservationStrategyPtr firstNonCompletedLeafNode (nullptr);

  firstNonCompletedLeafNode = pointer->next();
  ASSERT_TRUE(firstNonCompletedLeafNode not_eq nullptr);
  firstNonCompletedLeafNode->element_counter++;
  EXPECT_EQ(5, pointer->numberOfElementsLeftToCollect());
  firstNonCompletedLeafNode->element_counter++;
  EXPECT_EQ(4, pointer->numberOfElementsLeftToCollect());

  firstNonCompletedLeafNode = pointer->next();
  ASSERT_TRUE(firstNonCompletedLeafNode not_eq nullptr);
  firstNonCompletedLeafNode->element_counter++;
  EXPECT_EQ(3, pointer->numberOfElementsLeftToCollect());
  firstNonCompletedLeafNode->element_counter++;
  EXPECT_EQ(2, pointer->numberOfElementsLeftToCollect());

  delete pointer;
  pointer = nullptr;
}

//----------------------------------------------------------------------
// Tests that DFObservationStrategy::totalNumberOfElementsToCollectForAcquisition method works
//----------------------------------------------------------------------
TEST_F(DFObservationStrategyTest, DFObservationStrategy_totalNumberOfElementsToCollectForAcquisition)
{
  DFObservationStrategyTest_TRACE

  DF::DFObservationStrategy * pointer ( new (std::nothrow) DF::DFObservationStrategy() );
  ASSERT_TRUE( pointer not_eq nullptr );

  ASSERT_TRUE(buildObservationStrategy_(pointer));

  EXPECT_EQ(4, pointer->totalNumberOfElementsToCollectForAcquisition());

  delete pointer;
  pointer = nullptr;
}

//----------------------------------------------------------------------
// Tests that DFObservationStrategy::numberOfCollectedElementsForAcquisition method works
//----------------------------------------------------------------------
TEST_F(DFObservationStrategyTest, DFObservationStrategy_numberOfCollectedElementsForAcquisition)
{
  DFObservationStrategyTest_TRACE

  DF::DFObservationStrategy * pointer ( new (std::nothrow) DF::DFObservationStrategy() );
  ASSERT_TRUE( pointer not_eq nullptr );

  ASSERT_TRUE(buildObservationStrategy_(pointer));

  EXPECT_EQ(0, pointer->numberOfCollectedElementsForAcquisition());

  // Obtain and check the first leaf node marked as incomplete.
  DF::DFObservationStrategy::ObservationStrategyPtr firstNonCompletedLeafNode (nullptr);

  firstNonCompletedLeafNode = pointer->next();
  ASSERT_TRUE(firstNonCompletedLeafNode not_eq nullptr);
  firstNonCompletedLeafNode->element_counter++;
  EXPECT_EQ(1, pointer->numberOfCollectedElementsForAcquisition());
  firstNonCompletedLeafNode->element_counter++;
  EXPECT_EQ(2, pointer->numberOfCollectedElementsForAcquisition());

  firstNonCompletedLeafNode = pointer->next();
  ASSERT_TRUE(firstNonCompletedLeafNode not_eq nullptr);
  firstNonCompletedLeafNode->element_counter++;
  EXPECT_EQ(3, pointer->numberOfCollectedElementsForAcquisition());
  firstNonCompletedLeafNode->element_counter++;
  EXPECT_EQ(4, pointer->numberOfCollectedElementsForAcquisition());

  delete pointer;
  pointer = nullptr;
}

//----------------------------------------------------------------------
// Tests that DFObservationStrategy::numberOfElementsLeftToCollectForAcquisition method works
//----------------------------------------------------------------------
TEST_F(DFObservationStrategyTest, DFObservationStrategy_numberOfElementsLeftToCollectForAcquisition)
{
  DFObservationStrategyTest_TRACE

  DF::DFObservationStrategy * pointer ( new (std::nothrow) DF::DFObservationStrategy() );
  ASSERT_TRUE( pointer not_eq nullptr );

  ASSERT_TRUE(buildObservationStrategy_(pointer));

  EXPECT_EQ(4, pointer->numberOfElementsLeftToCollectForAcquisition());

  // Obtain and check the first leaf node marked as incomplete.
  DF::DFObservationStrategy::ObservationStrategyPtr firstNonCompletedLeafNode (nullptr);

  firstNonCompletedLeafNode = pointer->next();
  ASSERT_TRUE(firstNonCompletedLeafNode not_eq nullptr);
  firstNonCompletedLeafNode->element_counter++;
  EXPECT_EQ(3, pointer->numberOfElementsLeftToCollectForAcquisition());
  firstNonCompletedLeafNode->element_counter++;
  EXPECT_EQ(2, pointer->numberOfElementsLeftToCollectForAcquisition());

  firstNonCompletedLeafNode = pointer->next();
  ASSERT_TRUE(firstNonCompletedLeafNode not_eq nullptr);
  firstNonCompletedLeafNode->element_counter++;
  EXPECT_EQ(1, pointer->numberOfElementsLeftToCollectForAcquisition());
  firstNonCompletedLeafNode->element_counter++;
  EXPECT_EQ(0, pointer->numberOfElementsLeftToCollectForAcquisition());

  delete pointer;
  pointer = nullptr;
}

//----------------------------------------------------------------------
// Tests that DF::find1stObsStrategyNotAcquisitionCompleted method works
//----------------------------------------------------------------------
TEST_F(DFObservationStrategyTest, DF_find1stObsStrategyNotAcquisitionCompleted)
{
  DFObservationStrategyTest_TRACE

  DF::DFObservationStrategyPtr
    p1 ( new (std::nothrow) DF::DFObservationStrategy() ),
    p2 ( new (std::nothrow) DF::DFObservationStrategy() ),
    result (nullptr);
  ASSERT_TRUE( p1 not_eq nullptr );
  ASSERT_TRUE( p2 not_eq nullptr );
  ASSERT_TRUE( result == nullptr );

  ASSERT_TRUE(buildObservationStrategy_(p1));
  ASSERT_TRUE(buildObservationStrategy_(p2));

  EXPECT_FALSE(p1->isAcquisitionComplete());
  EXPECT_FALSE(p2->isAcquisitionComplete());

  DF::DFObservationStrategyList list;
  list.emplace_back(p1);
  list.emplace_back(p2);

  EXPECT_EQ(2, list.size());

  result = DF::find1stObsStrategyNotAcquisitionCompleted(list);
  EXPECT_FALSE( result->isAcquisitionComplete() );
  EXPECT_TRUE( result == p1 );
  list.erase(list.begin());
  EXPECT_EQ(1, list.size());

  result = DF::find1stObsStrategyNotAcquisitionCompleted(list);
  EXPECT_FALSE( result->isAcquisitionComplete() );
  EXPECT_TRUE( result == p2 );
  list.erase(list.begin());
  EXPECT_EQ(0, list.size());

  list.emplace_back(p1);
  list.emplace_back(p2);
  EXPECT_EQ(2, list.size());

  // Force manually that observation strategy p1 to be completed for acquisition.
  DF::DFObservationStrategy::ObservationStrategyPtr node (nullptr);
  for (; true;)
  {
    try
    {
      node = p1->next();
      node->element_counter = node->number_element_to_collect;
    }
    catch (...)
    {
      break;
    }
  }
  EXPECT_TRUE( p1->isAcquisitionComplete() );

  // Check that the first element on the list is p1.
  result = list.front();
  EXPECT_TRUE( result == p1 );

  // Find 1st non-completed observation strategy for acquisition.
  result = DF::find1stObsStrategyNotAcquisitionCompleted(list);
  EXPECT_FALSE( result->isAcquisitionComplete() );
  EXPECT_TRUE( result == p2 );

  list.clear();
  delete p1;
  p1 = nullptr;
  delete p2;
  p2 = nullptr;
}
