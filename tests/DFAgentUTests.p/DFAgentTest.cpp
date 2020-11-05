// Copyright (c) 2019 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved
//
//
// $Log$

static const char rcsid[]="@(#) $Id$";

// Includes
#include "DFAgentTest.h"           // DFAgentTest
#include <Trace.h>                 // Trace
#include <DFAgent.h>               // DFAgent


/************************************************************************
*************************************************************************
**                                                                     **
**                                Macros                               **
**                                                                     **
*************************************************************************
************************************************************************/

//<editor-fold desc="Enable or disable debug trace via source code">
//#define DFAgentTest_DEBUG // Enable debug trace
//#undef DFAgentTest_DEBUG  // Disable debug trace
//</editor-fold>

//<editor-fold desc="Define DFAgentTest_TRACE">
#if defined(DFAgentTest_DEBUG)
#include <ace/Trace.h>                    // ACE_Trace
#define DFAgentTest_TRACE \
  ACE_Trace ace_trace(__func__, __LINE__, __FILE__);\
  ace_trace.start_tracing();\
  ace_trace.set_nesting_indent(1);
#else
#define DFAgentTest_TRACE \
  ;
#endif
//</editor-fold>

//----------------------------------------------------------------------
// main
//----------------------------------------------------------------------
int main(int argc, char **argv)
{
  DFAgentTest_TRACE
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

Trace DFAgentTest::trace_ = Trace("DFAgentTest");

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
DFAgentTest::DFAgentTest()
{
  DFAgentTest_TRACE
}

//----------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------
DFAgentTest::~DFAgentTest()
{
  DFAgentTest_TRACE
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
void DFAgentTest::SetUp()
{
  DFAgentTest_TRACE
}

//----------------------------------------------------------------------
// Code here will be called immediately after each test (right
// before the destructor).
//----------------------------------------------------------------------
void DFAgentTest::TearDown()
{
  DFAgentTest_TRACE
}

//----------------------------------------------------------------------
// Code here will be called immediately after each test (right
// before the destructor).
//----------------------------------------------------------------------
const DFAgent::Hash_t DFAgentTest::wrapper_for_hash(const std::string &obj)
{
  DFAgentTest_TRACE
  return DFAgent::hash(obj);
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
TEST_F(DFAgentTest, DFAgentTest_DefaultConstructor_SetUp_TearDown_Destructor)
{
  DFAgentTest_TRACE

  ;
}

//----------------------------------------------------------------------
// Tests that the DFAgent constructor from plain text works
//----------------------------------------------------------------------
TEST_F(DFAgentTest, DFAgent_DefaultConstructor_Destructor)
{
  DFAgentTest_TRACE
}

//----------------------------------------------------------------------
// Tests that the DFAgent constructor from plain text works
//----------------------------------------------------------------------
TEST_F(DFAgentTest, DFAgent_hash_string)
{
  DFAgentTest_TRACE
  const std::string parameter("1");
  const DFAgent::Hash_t expected (177622);
  DFAgent::Hash_t actual (0);
  ASSERT_NO_THROW(actual = wrapper_for_hash(parameter););
  ASSERT_EQ(expected, actual);
}
