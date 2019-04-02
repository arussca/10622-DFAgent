// Copyright (c) 2019 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved
//
//
// $Log$

static const char rcsid[]="@(#) $Id$";

// Includes
#include "MonitorKeywordTest.h"           // MonitorKeywordTest
#include <MonitorKeyword.h>               // MonitorKeyword
#include <math.h>
#include <string>                         // std::string
#include <memory>                         // std::unique_ptr
#include <sstream>                        // std::stringstream, std::istringstream
#include <GCSException.h>                 // GCSException
#include <Trace.h>                        // Trace
#include <json/json.h>                    // Json::Value

/************************************************************************
*************************************************************************
**                                                                     **
**                                Macros                               **
**                                                                     **
*************************************************************************
************************************************************************/
//<editor-fold desc="Enable or disable debug trace via source code">
//#define MonitorKeywordTest_DEBUG // Enable debug trace
//#undef MonitorKeywordTest_DEBUG  // Disable debug trace
//</editor-fold>
//<editor-fold desc="Define MonitorKeywordTest_TRACE">
#if defined(MonitorKeywordTest_DEBUG)
#include <ace/Trace.h>                    // ACE_Trace
#define MonitorKeywordTest_TRACE \
  ACE_Trace ace_trace(__func__, __LINE__, __FILE__);\
  ace_trace.start_tracing();\
  ace_trace.set_nesting_indent(1);
#else
#define MonitorKeywordTest_TRACE \
  ;
#endif
//</editor-fold>

//----------------------------------------------------------------------
// main
//----------------------------------------------------------------------
int main(int argc, char **argv)
{
  MonitorKeywordTest_TRACE
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

Trace MonitorKeywordTest::trace_ = Trace("MonitorKeywordTest");

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
MonitorKeywordTest::MonitorKeywordTest()
:
  expected_(nullptr),
  current_(nullptr)
{
  MonitorKeywordTest_TRACE
}

//----------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------
MonitorKeywordTest::~MonitorKeywordTest()
{
  MonitorKeywordTest_TRACE
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
void MonitorKeywordTest::SetUp()
{
  MonitorKeywordTest_TRACE

  // Initialize expected_ member
  if (expected_ not_eq nullptr)
  {
    delete expected_;
    expected_ = nullptr;
  }
  expected_ = new /*(std::nothrow)*/ MonitorKeyword();
  expected_->id              = 1;
  expected_->keyName         = "KEYWORD";
  expected_->keyComment      = "number of event currenty proceed";
//expected_->keyUnit         = "";
  expected_->deviceName      = "LogManager";
  expected_->monitorName     = "numMessages";
  expected_->monitorType     = MonitorKeyword::MonitorType::SAMPLED;
  expected_->keyFormat       = MonitorKeyword::KeyFormat::ARRAY_TO_SCALAR;
  expected_->arrayRange      = MonitorKeyword::ArrayRange(0,60); // [i,j] = [1,61] => [i',j'] = [i-1,j-1] = [0,60]
  expected_->when            = MonitorKeyword::When::START;
//expected_->transposed      = false;
//expected_->keyInitialIndex = 0;
//expected_->arrayRange      = MonitorKeyword::ArrayRange();
//expected_->enumLabels      = MonitorKeyword::EnumLabels();
  // Update all given members.
  expected_->presentMembers_.clear();
  for ( const auto&item: MonitorKeyword::AllowedMembers::all )
  {
    expected_->presentMembers_[item] = false;
  }
  expected_->presentMembers_[MonitorKeyword::AllowedMembers::name     ] = true;
  expected_->presentMembers_[MonitorKeyword::AllowedMembers::comment  ] = true;
  expected_->presentMembers_[MonitorKeyword::AllowedMembers::component] = true;
  expected_->presentMembers_[MonitorKeyword::AllowedMembers::monitor  ] = true;
  expected_->presentMembers_[MonitorKeyword::AllowedMembers::type     ] = true;
  expected_->presentMembers_[MonitorKeyword::AllowedMembers::format   ] = true;
  expected_->presentMembers_[MonitorKeyword::AllowedMembers::when     ] = true;

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
void MonitorKeywordTest::TearDown()
{
  MonitorKeywordTest_TRACE
}

//----------------------------------------------------------------------
// constructPlainTextWapper_
//----------------------------------------------------------------------
bool MonitorKeywordTest::constructPlainTextWapper_(MonitorKeyword *&pointer)
{
  MonitorKeywordTest_TRACE

  if (pointer not_eq nullptr)
  {
    return false;
  }

  if (expected_ == nullptr)
  {
    return false;
  }

  // Build line
  std::string line("");
  line += expected_->keyName                             +  " "; // Keyword
  line += expected_->deviceName                          +  " "; // Component
  line += expected_->monitorName                         +  " "; // Monitor
  line += std::string(to_string(expected_->monitorType)) +  " "; // Type
  line += std::string(to_string(expected_->keyFormat)  ) +  " "; // Transformation
  // Comment
  line += std::string(MonitorKeyword::AllowedMembers::comment) + "=[" ;
  line += expected_->keyComment                                +  "] ";
  // When
  line += std::string(MonitorKeyword::AllowedMembers::when) + "=[" ;
  line += std::string(to_string(expected_->when))           +  "] ";

  // Build MonitorKeyword from plain text syntax
  try
  {
    pointer = new (std::nothrow) MonitorKeyword(
        line,          // line
        expected_->id, // lineNumber
        nullptr,       // logAgent
        trace_,        // trace
        0              // verbose
    );
  }
  catch (GCSException &ex)
  {
    std::cerr << __func__ << ": "<< ex.toString() << std::endl;
    return false;
  }
  catch(...)
  {
    std::cerr << __func__ << ": Exception" << std::endl;
    return false;
  }
  return pointer not_eq nullptr;
}

//----------------------------------------------------------------------
// constructJSONWapper_
//----------------------------------------------------------------------
bool MonitorKeywordTest::constructJSONWapper_(MonitorKeyword *&pointer)
{
  MonitorKeywordTest_TRACE

  if (pointer not_eq nullptr)
  {
    return false;
  }

  if (expected_ == nullptr)
  {
    return false;
  }

  // Build Json node
  Json::Value root;
  root[MonitorKeyword::AllowedMembers::name     ] = expected_->keyName;
  root[MonitorKeyword::AllowedMembers::component] = expected_->deviceName;
  root[MonitorKeyword::AllowedMembers::monitor  ] = expected_->monitorName;
  root[MonitorKeyword::AllowedMembers::type     ] = to_string(expected_->monitorType);
  root[MonitorKeyword::AllowedMembers::format   ] = to_string(expected_->keyFormat);
  root[MonitorKeyword::AllowedMembers::comment  ] = expected_->keyComment;
  root[MonitorKeyword::AllowedMembers::unit     ] = expected_->keyUnit;
  root[MonitorKeyword::AllowedMembers::when     ] = to_string(expected_->when);
  {
    Json::FastWriter fastWriter;
    std::string document = fastWriter.write(root);
    MonitorKeywordTest::trace_.out("Keyword to build is '%s'", document.data());
  }

  // Build MonitorKeyword from plain text syntax
  Trace trace("MonitorKeywordTest");
  try
  {
    pointer = new (std::nothrow) MonitorKeyword(
        root,          // JSON Value
        expected_->id, // lineNumber
        nullptr,       // logAgent
        trace_,        // trace
        0              // verbose
    );
  }
  catch (GCSException &ex)
  {
    std::cerr << __func__ << ": " << ex.toString() << std::endl;
    return false;
  }
  catch(...)
  {
    std::cerr << __func__ << ": Exception." << std::endl;
    return false;
  }
  return pointer not_eq nullptr;
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
TEST_F(MonitorKeywordTest, MonitorKeywordTest_DefaultConstructor_SetUp_TearDown_Destructor)
{
  MonitorKeywordTest_TRACE

  ;
}

//----------------------------------------------------------------------
// Tests that the MonitorKeyword constructor from plain text works
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, MonitorKeyword_DefaultConstructor_Destructor)
{
  MonitorKeywordTest_TRACE

  MonitorKeyword * pointer = new (std::nothrow) MonitorKeyword();
  ASSERT_TRUE( pointer not_eq nullptr );

  delete pointer;
  pointer = nullptr;
}

//----------------------------------------------------------------------
// Tests whether equality operator == works
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, EqualTo)
{
  MonitorKeywordTest_TRACE

  MonitorKeyword op1, op2;

  op1.deviceName = op2.deviceName = "device1";
  op1.monitorName = op2.monitorName = "monitor1";
  EXPECT_TRUE( op1 == op2 );

  op2.monitorName = "monitor2";
  EXPECT_FALSE( op1 == op2 );

  op2.monitorName = op1.monitorName;
  op2.deviceName = "device2";
  EXPECT_FALSE( op1 == op2 );
}

//----------------------------------------------------------------------
// Tests whether equality operator != works
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, NotEqualTo)
{
  MonitorKeywordTest_TRACE

  MonitorKeyword op1, op2;

  op1.deviceName = op2.deviceName = "device";
  op1.monitorName = "monitor1";
  op2.monitorName = "monitor2";
  EXPECT_TRUE( op1 not_eq op2 );

  op1.deviceName = "device1";
  op2.deviceName = "device2";
  op1.monitorName = op2.monitorName = "monitor";
  EXPECT_TRUE( op1 not_eq op2 );
}

//----------------------------------------------------------------------
// Tests whether relational operator < works
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, LessThan)
{
  MonitorKeywordTest_TRACE

  MonitorKeyword op1, op2;

  op1.deviceName = op2.deviceName = "device";
  op1.monitorName = "monitor1";
  op2.monitorName = "monitor2";
  EXPECT_TRUE( op1 < op2 );
  EXPECT_FALSE( op2 < op1 );

  op1.deviceName = "device1";
  op2.deviceName = "device2";
  op1.monitorName = op2.monitorName = "monitor";
  EXPECT_TRUE( op1 < op2 );
  EXPECT_FALSE( op2 < op1 );
}

//----------------------------------------------------------------------
// Tests whether relational operator > works
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, GreaterThan)
{
  MonitorKeywordTest_TRACE

  MonitorKeyword op1, op2;

  op1.deviceName = op2.deviceName = "device";
  op1.monitorName = "monitor1";
  op2.monitorName = "monitor2";
  EXPECT_TRUE( op2 > op1 );
  EXPECT_FALSE( op1 > op2 );

  op1.deviceName = "device1";
  op2.deviceName = "device2";
  op1.monitorName = op2.monitorName = "monitor";
  EXPECT_TRUE( op2 > op1 );
  EXPECT_FALSE( op1 > op2 );
}

//----------------------------------------------------------------------
// Tests whether relational operator <= works
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, LessThanOrEqualTo)
{
  MonitorKeywordTest_TRACE

  MonitorKeyword op1, op2;

  op1.deviceName = op2.deviceName = "device";
  op1.monitorName = "monitor1";
  op2.monitorName = "monitor2";
  EXPECT_TRUE( op1 <= op2 );
  EXPECT_FALSE( op2 <= op1 );

  op1.deviceName = "device1";
  op2.deviceName = "device2";
  op1.monitorName = op2.monitorName = "monitor";
  EXPECT_TRUE( op1 <= op2 );
  EXPECT_FALSE( op2 <= op1 );

  op1.deviceName = op2.deviceName = "device";
  op1.monitorName = op2.monitorName = "monitor";
  EXPECT_TRUE( op1 <= op2 );
  EXPECT_TRUE( op2 <= op1 );
}

//----------------------------------------------------------------------
// Tests whether relational operator >= works
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, GreaterThanOrEqualTo)
{
  MonitorKeywordTest_TRACE

  MonitorKeyword op1, op2;

  op1.deviceName = op2.deviceName = "device";
  op1.monitorName = "monitor1";
  op2.monitorName = "monitor2";
  EXPECT_TRUE( op2 >= op1 );
  EXPECT_FALSE( op1 >= op2 );

  op1.deviceName = "device1";
  op2.deviceName = "device2";
  op1.monitorName = op2.monitorName = "monitor";
  EXPECT_TRUE( op2 >= op1 );
  EXPECT_FALSE( op1 >= op2 );

  op1.deviceName =  op2.deviceName = "device";
  op1.monitorName = op2.monitorName = "monitor";
  EXPECT_TRUE( op2 >= op1 );
  EXPECT_TRUE( op1 >= op2 );
}

//----------------------------------------------------------------------
// Tests that from_string MonitorKeyword::MonitorType works
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, MonitorTypeFromString)
{
  MonitorKeywordTest_TRACE

  MonitorKeyword::MonitorType obj;
  std::string str;
  str = "NONE"    ; from_string(obj, str); EXPECT_EQ(MonitorKeyword::MonitorType::NONE    , obj );
  str = "SAMPLED" ; from_string(obj, str); EXPECT_EQ(MonitorKeyword::MonitorType::SAMPLED , obj );
  str = "DISCRETE"; from_string(obj, str); EXPECT_EQ(MonitorKeyword::MonitorType::DISCRETE, obj );
}

//----------------------------------------------------------------------
// Tests that from_string MonitorKeyword::When works
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, WhenFromString)
{
  MonitorKeywordTest_TRACE

  MonitorKeyword::When obj;
  std::string str;
  str = "NONE" ; from_string(obj, str); EXPECT_EQ(MonitorKeyword::When::NONE , obj );
  str = "START"; from_string(obj, str); EXPECT_EQ(MonitorKeyword::When::START, obj );
  str = "END"  ; from_string(obj, str); EXPECT_EQ(MonitorKeyword::When::END  , obj );
}

//----------------------------------------------------------------------
// Tests that from_string MonitorKeyword::KeyFormat works
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, KeyFormatFromString)
{
  MonitorKeywordTest_TRACE

  MonitorKeyword::KeyFormat obj;
  std::string str;
  str = "NONE"            ; from_string(obj, str); EXPECT_EQ(MonitorKeyword::KeyFormat::NONE           , obj );
  str = "RAD_TO_HMS"      ; from_string(obj, str); EXPECT_EQ(MonitorKeyword::KeyFormat::RAD_TO_HMS     , obj );
  str = "RAD_TO_DMS"      ; from_string(obj, str); EXPECT_EQ(MonitorKeyword::KeyFormat::RAD_TO_DMS     , obj );
  str = "RAD_TO_DEG"      ; from_string(obj, str); EXPECT_EQ(MonitorKeyword::KeyFormat::RAD_TO_DEG     , obj );
  str = "ARRAY_TO_SCALAR" ; from_string(obj, str); EXPECT_EQ(MonitorKeyword::KeyFormat::ARRAY_TO_SCALAR, obj );
}

//----------------------------------------------------------------------
// Tests that from_string MonitorKeyword::ArrayRange works
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, ArrayRangeFromString)
{
  MonitorKeywordTest_TRACE

  MonitorKeyword::ArrayRange obj;
  std::string str;
  str = "0,0" ; from_string(obj, str); EXPECT_EQ(0, obj.first); EXPECT_EQ(0, obj.second);
  str = "1,2" ; from_string(obj, str); EXPECT_EQ(1, obj.first); EXPECT_EQ(2, obj.second);
  str = "3,4" ; from_string(obj, str); EXPECT_EQ(3, obj.first); EXPECT_EQ(4, obj.second);
}

//----------------------------------------------------------------------
// Tests that from_string MonitorKeyword::EnumLabels works
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, EnumLabelsFromString)
{
  MonitorKeywordTest_TRACE

  static MonitorKeyword::EnumLabels expected({"IN", "OUT"});
  static MonitorKeyword::EnumLabels actual;
  std::string str("");
  for (const auto item: expected)
  {
    str += item + ",";
  }
  str = str.substr(0, str.size()-1);
  from_string(actual, str);
  ASSERT_EQ(expected.size(), actual.size());
  for (short i(-1); ++i <expected.size(); )
  {
    EXPECT_EQ(expected[i], actual[i]);
  }
}

//----------------------------------------------------------------------
// Tests that to_string MonitorKeyword::MonitorType works
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, MonitorTypeToString)
{
  MonitorKeywordTest_TRACE

  MonitorKeyword::MonitorType obj;
  std::string str;
  obj = MonitorKeyword::MonitorType::NONE    ; EXPECT_STREQ("NONE"    , to_string(obj) );
  obj = MonitorKeyword::MonitorType::SAMPLED ; EXPECT_STREQ("SAMPLED" , to_string(obj) );
  obj = MonitorKeyword::MonitorType::DISCRETE; EXPECT_STREQ("DISCRETE", to_string(obj) );
}

//----------------------------------------------------------------------
// Tests that to_string MonitorKeyword::When works
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, WhenToString)
{
  MonitorKeywordTest_TRACE

  MonitorKeyword::When obj;
  std::string str;
  obj = MonitorKeyword::When::NONE  ; EXPECT_STREQ("NONE" , to_string(obj) );
  obj = MonitorKeyword::When::START ; EXPECT_STREQ("START", to_string(obj) );
  obj = MonitorKeyword::When::END   ; EXPECT_STREQ("END"  , to_string(obj) );
}

//----------------------------------------------------------------------
// Tests that to_string MonitorKeyword::KeyFormat works
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, KeyFormatToString)
{
  MonitorKeywordTest_TRACE

  MonitorKeyword::KeyFormat obj;
  std::string str;
  obj = MonitorKeyword::KeyFormat::NONE           ; EXPECT_STREQ("NONE"           , to_string(obj) );
  obj = MonitorKeyword::KeyFormat::RAD_TO_HMS     ; EXPECT_STREQ("RAD_TO_HMS"     , to_string(obj) );
  obj = MonitorKeyword::KeyFormat::RAD_TO_DMS     ; EXPECT_STREQ("RAD_TO_DMS"     , to_string(obj) );
  obj = MonitorKeyword::KeyFormat::RAD_TO_DEG     ; EXPECT_STREQ("RAD_TO_DEG"     , to_string(obj) );
  obj = MonitorKeyword::KeyFormat::ARRAY_TO_SCALAR; EXPECT_STREQ("ARRAY_TO_SCALAR", to_string(obj) );
}

//----------------------------------------------------------------------
// Tests that to_string MonitorKeyword::ArrayRange works
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, ArrayRangeToString)
{
  MonitorKeywordTest_TRACE

  MonitorKeyword::ArrayRange obj;
  std::string str;
  obj = MonitorKeyword::ArrayRange(0,0); EXPECT_STREQ("0,0", to_string(obj) );
  obj = MonitorKeyword::ArrayRange(1,2); EXPECT_STREQ("1,2", to_string(obj) );
  obj = MonitorKeyword::ArrayRange(3,3); EXPECT_STREQ("3,3", to_string(obj) );
}

//----------------------------------------------------------------------
// Tests that to_string MonitorKeyword::EnumLabels works
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, EnumLabelsToString)
{
  MonitorKeywordTest_TRACE

  MonitorKeyword::EnumLabels obj ({"L1","L2","L3"});
  std::string str("");
  for (const auto item: obj)
  {
    str += item + ",";
  }
  str = str.substr(0, str.size()-1);
  EXPECT_STREQ(str.data(), to_string(obj) );
}

//----------------------------------------------------------------------
// Tests that all allowed field are recognized
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, isAnAllowedField)
{
  MonitorKeywordTest_TRACE

  ASSERT_TRUE(MonitorKeywordTest::expected_ not_eq nullptr);

  for (const std::string&allowedField: MonitorKeyword::AllowedMembers::all)
  {
    MonitorKeywordTest::trace_.out("Test that '%s' is an allowed field.", allowedField.data());
    EXPECT_TRUE(MonitorKeywordTest::expected_->isMemberAllowed(allowedField));
  }
  std::string notAllowedField("not-allowed-field");
  MonitorKeywordTest::trace_.out("Test that '%s' is not an allowed field.", notAllowedField.data());
  EXPECT_FALSE(MonitorKeywordTest::expected_->isMemberAllowed(notAllowedField));
}

//----------------------------------------------------------------------
// Tests that all given members are registered.
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, isMemberPresent)
{
  MonitorKeywordTest_TRACE

  MonitorKeywordTest::current_ = new (std::nothrow) MonitorKeyword();
  ASSERT_TRUE(MonitorKeywordTest::current_ not_eq nullptr);

  MonitorKeywordTest::trace_.out("The field '%s' must not be present.", MonitorKeyword::AllowedMembers::name );
  EXPECT_FALSE(MonitorKeywordTest::current_->isMemberPresent(MonitorKeyword::AllowedMembers::name));

  delete MonitorKeywordTest::current_;
  MonitorKeywordTest::current_ = nullptr;
}

//----------------------------------------------------------------------
// Tests that the MonitorKeyword constructor from plain text works
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, ConstructorPlainText)
{
  MonitorKeywordTest_TRACE

  // Construct from plain text
  ASSERT_TRUE(MonitorKeywordTest::constructPlainTextWapper_(MonitorKeywordTest::current_));
  ASSERT_TRUE(MonitorKeywordTest::current_ not_eq nullptr);

  delete MonitorKeywordTest::current_;
  MonitorKeywordTest::current_ = nullptr;
}

//----------------------------------------------------------------------
// Tests that the MonitorKeyword constructor from plain text works
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, ConstructorPlainTextAndComparedFields)
{
  MonitorKeywordTest_TRACE

  // Construct from plain text
  ASSERT_TRUE(MonitorKeywordTest::constructPlainTextWapper_(MonitorKeywordTest::current_));
  ASSERT_TRUE(MonitorKeywordTest::current_ not_eq nullptr);
  // Compare fields
  EXPECT_EQ( MonitorKeywordTest::expected_->id              , MonitorKeywordTest::current_->id              );
  EXPECT_EQ( MonitorKeywordTest::expected_->keyName         , MonitorKeywordTest::current_->keyName         );
  EXPECT_EQ( MonitorKeywordTest::expected_->keyComment      , MonitorKeywordTest::current_->keyComment      );
  EXPECT_EQ( MonitorKeywordTest::expected_->keyUnit         , MonitorKeywordTest::current_->keyUnit         );
  EXPECT_EQ( MonitorKeywordTest::expected_->deviceName      , MonitorKeywordTest::current_->deviceName      );
  EXPECT_EQ( MonitorKeywordTest::expected_->monitorName     , MonitorKeywordTest::current_->monitorName     );
  EXPECT_EQ( MonitorKeywordTest::expected_->monitorType     , MonitorKeywordTest::current_->monitorType     );
  EXPECT_EQ( MonitorKeywordTest::expected_->keyFormat       , MonitorKeywordTest::current_->keyFormat       );
  EXPECT_EQ( MonitorKeywordTest::expected_->when            , MonitorKeywordTest::current_->when            );
  EXPECT_EQ( MonitorKeywordTest::expected_->transposed      , MonitorKeywordTest::current_->transposed      );
  EXPECT_EQ( MonitorKeywordTest::expected_->keyInitialIndex , MonitorKeywordTest::current_->keyInitialIndex );
  EXPECT_EQ( MonitorKeywordTest::expected_->arrayRange      , MonitorKeywordTest::current_->arrayRange      );
  EXPECT_EQ( MonitorKeywordTest::expected_->enumLabels      , MonitorKeywordTest::current_->enumLabels      );

  delete MonitorKeywordTest::current_;
  MonitorKeywordTest::current_ = nullptr;
}

//----------------------------------------------------------------------
// Tests that the MonitorKeyword constructor from JSON syntax works
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, ConstructorJSON)
{
  MonitorKeywordTest_TRACE

  // Construct from plain text
  ASSERT_TRUE(MonitorKeywordTest::constructJSONWapper_(MonitorKeywordTest::current_));
  ASSERT_TRUE(MonitorKeywordTest::current_ not_eq nullptr);

  delete MonitorKeywordTest::current_;
  MonitorKeywordTest::current_ = nullptr;
}

//----------------------------------------------------------------------
// Tests that the MonitorKeyword constructor from plain text works
//----------------------------------------------------------------------
TEST_F(MonitorKeywordTest, ConstructorJSONAndComparedFields)
{
  MonitorKeywordTest_TRACE

  // Construct from plain text
  ASSERT_TRUE(MonitorKeywordTest::constructJSONWapper_(MonitorKeywordTest::current_));
  ASSERT_TRUE(MonitorKeywordTest::current_ not_eq nullptr);
  // Compare fields
  EXPECT_EQ( MonitorKeywordTest::expected_->id              , MonitorKeywordTest::current_->id              );
  EXPECT_EQ( MonitorKeywordTest::expected_->keyName         , MonitorKeywordTest::current_->keyName         );
  EXPECT_EQ( MonitorKeywordTest::expected_->keyComment      , MonitorKeywordTest::current_->keyComment      );
  EXPECT_EQ( MonitorKeywordTest::expected_->keyUnit         , MonitorKeywordTest::current_->keyUnit         );
  EXPECT_EQ( MonitorKeywordTest::expected_->deviceName      , MonitorKeywordTest::current_->deviceName      );
  EXPECT_EQ( MonitorKeywordTest::expected_->monitorName     , MonitorKeywordTest::current_->monitorName     );
  EXPECT_EQ( MonitorKeywordTest::expected_->monitorType     , MonitorKeywordTest::current_->monitorType     );
  EXPECT_EQ( MonitorKeywordTest::expected_->keyFormat       , MonitorKeywordTest::current_->keyFormat       );
  EXPECT_EQ( MonitorKeywordTest::expected_->when            , MonitorKeywordTest::current_->when            );
  EXPECT_EQ( MonitorKeywordTest::expected_->transposed      , MonitorKeywordTest::current_->transposed      );
  EXPECT_EQ( MonitorKeywordTest::expected_->keyInitialIndex , MonitorKeywordTest::current_->keyInitialIndex );
  EXPECT_EQ( MonitorKeywordTest::expected_->arrayRange      , MonitorKeywordTest::current_->arrayRange      );
  EXPECT_EQ( MonitorKeywordTest::expected_->enumLabels      , MonitorKeywordTest::current_->enumLabels      );

  delete MonitorKeywordTest::current_;
  MonitorKeywordTest::current_ = nullptr;
}

