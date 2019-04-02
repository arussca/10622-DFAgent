// Copyright (c) 2019 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved
//
//
// $Log$

static const char rcsid[]="@(#) $Id$";

// Includes
#include "FITSHeaderInfoTest.h"           // FITSHeaderInfoTest
#include <FITSHeaderInfo.h>               // FITSHeaderInfo

#include <math.h>
#include <string>                         // std::string
#include <memory>                         // std::unique_ptr
#include <sstream>                        // std::stringstream, std::istringstream
#include <GCSException.h>                 // GCSException
#include <json/json.h>                    // Json::Value

/************************************************************************
*************************************************************************
**                                                                     **
**                                Macros                               **
**                                                                     **
*************************************************************************
************************************************************************/
//<editor-fold desc="Enable or disable debug trace via source code">
//#define FITSHeaderInfoTest_DEBUG // Enable debug trace
//#undef FITSHeaderInfoTest_DEBUG  // Disable debug trace
//</editor-fold>
//<editor-fold desc="Define FITSHeaderInfoTest_TRACE">
#if defined(FITSHeaderInfoTest_DEBUG)
#include <ace/Trace.h>                    // ACE_Trace
#define FITSHeaderInfoTest_TRACE \
  ACE_Trace ace_trace(__func__, __LINE__, __FILE__);\
  ace_trace.start_tracing();\
  ace_trace.set_nesting_indent(1);
#else
#define FITSHeaderInfoTest_TRACE \
  ; // #ToDo: implement
#endif
//</editor-fold>

//----------------------------------------------------------------------
// main
//----------------------------------------------------------------------
int main(int argc, char **argv)
{
  FITSHeaderInfoTest_TRACE
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

Trace FITSHeaderInfoTest::trace_ = Trace("FITSHeaderInfoTest");

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
FITSHeaderInfoTest::FITSHeaderInfoTest()
:
  expected_(nullptr),
  current_(nullptr)
{
  FITSHeaderInfoTest_TRACE
}

//----------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------
FITSHeaderInfoTest::~FITSHeaderInfoTest()
{
  FITSHeaderInfoTest_TRACE
  if (expected_ not_eq nullptr)
  {
    delete expected_;
    expected_ = nullptr;
  }
  if (current_ not_eq nullptr)
  {
    delete current_;
    current_ = nullptr;
  }
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
void FITSHeaderInfoTest::SetUp()
{
  FITSHeaderInfoTest_TRACE

  // Initialize expected_ member
  if (expected_ not_eq nullptr)
  {
    delete expected_;
    expected_ = nullptr;
  }
  //expected_ = new /*(std::nothrow)*/ FITSHeaderInfo();

  // Destroy current_ member.
  if (current_ not_eq nullptr)
  {
    delete current_;
    current_ = nullptr;
  }
}

//----------------------------------------------------------------------
// Code here will be called immediately after each test (right
// before the destructor).
//----------------------------------------------------------------------
void FITSHeaderInfoTest::TearDown()
{
  FITSHeaderInfoTest_TRACE
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
TEST_F(FITSHeaderInfoTest, DefaultConstructor_SetUp_TearDown_Destructor)
{
  FITSHeaderInfoTest_TRACE
  // Nothing to do. Setup and TearDown methods are invoked automatically.
}

//----------------------------------------------------------------------
// Tests that Default constructor works
//----------------------------------------------------------------------
TEST_F(FITSHeaderInfoTest, DefaultConstructor)
{
  FITSHeaderInfoTest_TRACE

  FITSHeaderInfoTest::current_ = new (std::nothrow) FITSHeaderInfo();
  ASSERT_TRUE(FITSHeaderInfoTest::current_ not_eq nullptr);

  delete FITSHeaderInfoTest::current_;
  FITSHeaderInfoTest::current_ = nullptr;
}

//----------------------------------------------------------------------
// Tests that ParseFITSHeaderKeywordsFromFile works
//----------------------------------------------------------------------
TEST_F(FITSHeaderInfoTest, ParseFITSHeaderKeywordsFromFile)
{
  FITSHeaderInfoTest_TRACE

  FITSHeaderInfoTest::current_ = new (std::nothrow) FITSHeaderInfo();
  ASSERT_TRUE(FITSHeaderInfoTest::current_ not_eq nullptr);

  ; // #ToDo: implement

  delete FITSHeaderInfoTest::current_;
  FITSHeaderInfoTest::current_ = nullptr;
}

//----------------------------------------------------------------------
// Tests that ParseFITSHeaderKeywordsFromInline works
//----------------------------------------------------------------------
TEST_F(FITSHeaderInfoTest, ParseFITSHeaderKeywordsFromInline)
{
  FITSHeaderInfoTest_TRACE
  ; // #ToDo: implement
}

//----------------------------------------------------------------------
// Tests that Merge works
//----------------------------------------------------------------------
TEST_F(FITSHeaderInfoTest, Merge)
{
  FITSHeaderInfoTest_TRACE
  ; // #ToDo: implement
}

//----------------------------------------------------------------------
// Tests that GatherFixedKeywords works
//----------------------------------------------------------------------
TEST_F(FITSHeaderInfoTest, GatherFixedKeywords)
{
  FITSHeaderInfoTest_TRACE
  ; // #ToDo: implement
}

//----------------------------------------------------------------------
// Tests that GatherMonitorKeywords works
//----------------------------------------------------------------------
TEST_F(FITSHeaderInfoTest, GatherMonitorKeywords)
{
  FITSHeaderInfoTest_TRACE
  ; // #ToDo: implement
}

//----------------------------------------------------------------------
// Tests that GatherMonitorKeywords works
//----------------------------------------------------------------------
TEST_F(FITSHeaderInfoTest, GatherMonitorKeywordsMonitorAgent)
{
  FITSHeaderInfoTest_TRACE
  ; // #ToDo: implement
}

//----------------------------------------------------------------------
// Tests that WriteHeaderToFile works
//----------------------------------------------------------------------
TEST_F(FITSHeaderInfoTest, WriteHeaderToFile)
{
  FITSHeaderInfoTest_TRACE
  ; // #ToDo: implement
}

//----------------------------------------------------------------------
// Tests that WriteHeaderToFile works
//----------------------------------------------------------------------
TEST_F(FITSHeaderInfoTest, WriteHeaderToFileMonitorAgent)
{
  FITSHeaderInfoTest_TRACE
  ; // #ToDo: implement
}

//----------------------------------------------------------------------
// Tests that ForceSampleData works
//----------------------------------------------------------------------
TEST_F(FITSHeaderInfoTest, ForceSampleData)
{
  FITSHeaderInfoTest_TRACE
  ; // #ToDo: implement
}

//----------------------------------------------------------------------
// Tests that ForceSampleData works
//----------------------------------------------------------------------
TEST_F(FITSHeaderInfoTest, ForceSampleDataMonitorAgent)
{
  FITSHeaderInfoTest_TRACE
  ; // #ToDo: implement
}

//----------------------------------------------------------------------
// Tests that GetKeywordNameList works
//----------------------------------------------------------------------
TEST_F(FITSHeaderInfoTest, GetKeywordNameList)
{
  FITSHeaderInfoTest_TRACE
  ; // #ToDo: implement
}

//----------------------------------------------------------------------
// Tests that GetMonitorKeywordList works
//----------------------------------------------------------------------
TEST_F(FITSHeaderInfoTest, GetMonitorKeywordList)
{
  FITSHeaderInfoTest_TRACE
  ; // #ToDo: implement
}

//----------------------------------------------------------------------
// Tests that GetParsedKeywords works
//----------------------------------------------------------------------
TEST_F(FITSHeaderInfoTest, GetParsedKeywords)
{
  FITSHeaderInfoTest_TRACE
  ; // #ToDo: implement
}

//----------------------------------------------------------------------
// Tests that GetNotParsedNodesID works
//----------------------------------------------------------------------
TEST_F(FITSHeaderInfoTest, GetNotParsedNodesID)
{
  FITSHeaderInfoTest_TRACE
  ; // #ToDo: implement
}

//----------------------------------------------------------------------
// Tests that GetNotWrittenKeywords works
//----------------------------------------------------------------------
TEST_F(FITSHeaderInfoTest, GetNotWrittenKeywords)
{
  FITSHeaderInfoTest_TRACE
  ; // #ToDo: implement
}

//----------------------------------------------------------------------
// Tests that GetNumberOfDiscreteKeywords works
//----------------------------------------------------------------------
TEST_F(FITSHeaderInfoTest, GetNumberOfDiscreteKeywords)
{
  FITSHeaderInfoTest_TRACE
  ; // #ToDo: implement
}

//----------------------------------------------------------------------
// Tests that GetNumberOfSampledKeywords works
//----------------------------------------------------------------------
TEST_F(FITSHeaderInfoTest, GetNumberOfSampledKeywords)
{
  FITSHeaderInfoTest_TRACE
  ; // #ToDo: implement
}
