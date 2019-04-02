// Copyright (c) 2019 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved
//
//
// $Log$

#ifndef _MonitorKeywordTest_h_
#define _MonitorKeywordTest_h_

// Includes
#include "MonitorKeyword.h"               // MonitorKeyword

#include <string>                         // std::string
#include <memory>                         // std::unique_ptr
#include <Trace.h>                        // Trace
#include <gtest/gtest.h>                  // ::testing*, TEST_*, ...

//----------------------------------------------------------------------
// Prototypes
//----------------------------------------------------------------------

/**
 * Main function to perform all unit test for MonitorKeywordTest
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Array of arguments.
 * @return Status code of the program execution.
 * @retval Zero successful program execution status.
 * @retval Non-zero Unsuccessful execution of a program.
 */
int main(int argc, char** argv);

/**
 * Class reserved to perform unit tests of MonitorKeyword class.
 */
class MonitorKeywordTest: public ::testing::Test
{
  //--------------------------------------------------
  // public section
  //--------------------------------------------------
  public:

  //############################ PUBLIC TYPES ############################

  //########################## PUBLIC CONSTANTS ##########################

  //############# CONSTRUCTORS, DESTRUCTOR & SPECIAL METHODS #############

  /** Default constructor */
  MonitorKeywordTest();

  /** Destructor */
  virtual ~MonitorKeywordTest() override;

  //########################## PUBLIC METHODS ############################

  //########################## PUBLIC MEMBERS ############################

  //--------------------------------------------------
  // protected section
  //--------------------------------------------------
  protected:

  //########################### PROTECTED TYPES ##########################

  //######################### PROTECTED CONSTANTS ########################

  //########################## PROTECTED METHODS #########################
  // If the constructor and destructor are not enough for setting up    //
  // and cleaning up each test, you can define the following methods:   //
  // - Setup(), which will be called immediately after the constructor. //
  // - TearDown(), which will be called immediately after each test.    //

  /**
   * This method will be called immediately after the constructor (right
   * before each test).
   */
  void SetUp() override;

  /**
   * This method will be called immediately after each test (right
   * before the destructor).
   */
  void TearDown() override;

  /**
   * This method allocates memory for the given MonitorKeyword pointer
   * and invokes the MonitorKeyword constructor, which builds the object
   * using the plain text format as input parameter.
   *
   * @note
   * - The information of the monitor is extracted from the internal
   * member called expected_. For that reason, it's a pre-condition
   * that it have been initialized before calling this method.
   *
   * @related expected_
   *
   * @param[in] pointer MonitorKeyword pointer to build from plain text
   * @return true, if everything was OK. false, otherwise.
   */
  bool constructPlainTextWapper_(MonitorKeyword *&pointer);

  /**
   * This method allocates memory for the given MonitorKeyword pointer
   * and invokes the MonitorKeyword constructor, which builds the object
   * using the JSON format as input parameter.
   *
   * @note
   * - The information of the monitor is extracted from the internal
   * member called expected_. For that reason, it's a pre-condition
   * that it have been initialized before calling this method.
   *
   * @related expected_
   *
   * @param[in] pointer MonitorKeyword pointer to build from plain text
   * @return true, if everything was OK. false, otherwise.
   */
  bool constructJSONWapper_(MonitorKeyword *&pointer);

  //########################## PROTECTED MEMBERS #########################
  // Objects declared here can be used by all tests in the test case    //
  // for MonitorKeyword.                                                //

  /** Member that allows you obtain more information about any test */
  static Trace trace_;

  /**
   * Pointer of MonitorKeyword class, whose main aim is be
   * a fixed object to compare results
   */
  MonitorKeyword *expected_;

  /**
   * Pointer of MonitorKeyword class, whose main aim is be
   * built on several tests
   */
  MonitorKeyword *current_;


  //--------------------------------------------------
  // private section
  //--------------------------------------------------
  private:

  //############################ PRIVATE TYPES ###########################

  //########################## PRIVATE CONSTANTS #########################

  //########################### PRIVATE METHODS ##########################

  //########################### PRIVATE MEMBERS ##########################

};


#endif // _MonitorKeywordTest_h_