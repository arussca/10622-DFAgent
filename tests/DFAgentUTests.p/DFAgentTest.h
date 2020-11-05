// Copyright (c) 2020 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved
//
//
// $Log$

#ifndef _DFAgentTest_h_
#define _DFAgentTest_h_

// Includes
#include "DFAgent.h"                      // DFAgent

#include <string>                         // std::string
#include <memory>                         // std::unique_ptr
#include <Trace.h>                        // Trace
#include <gtest/gtest.h>                  // ::testing*, TEST_*, ...

//----------------------------------------------------------------------
// Prototypes
//----------------------------------------------------------------------

/**
 * Main function to perform all unit test for DFAgentTest
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Array of arguments.
 * @return Status code of the program execution.
 * @retval Zero successful program execution status.
 * @retval Non-zero Unsuccessful execution of a program.
 */
int main(int argc, char** argv);

/**
 * Class reserved to perform unit tests of DFAgent class.
 */
class DFAgentTest: public ::testing::Test
{
  //--------------------------------------------------
  // public section
  //--------------------------------------------------
  public:

  //############################ PUBLIC TYPES ############################

  //########################## PUBLIC CONSTANTS ##########################

  //############# CONSTRUCTORS, DESTRUCTOR & SPECIAL METHODS #############

  /** Default constructor */
  DFAgentTest();

  /** Destructor */
  virtual ~DFAgentTest() override;

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
   * It is a wrapper function to be able to use the friend class declaration
   * in Google Test. So, it does the same that DFAgent::hash(const std::string &)
   * function. In other words, it computes the hash for the given string value.
   *
   * @param[in] obj Given value for computing its hash.
   * @return Computed hash of the given value.
   *
   * @attention It is a wrapper function.
   */
  static const DFAgent::Hash_t wrapper_for_hash(const std::string &obj);

  //########################## PROTECTED MEMBERS #########################
  // Objects declared here can be used by all tests in the test case    //
  // for DFAgent.                                                       //

  /** Member that allows you obtain more information about any test */
  static Trace trace_;


  //--------------------------------------------------
  // private section
  //--------------------------------------------------
  private:

  //############################ PRIVATE TYPES ###########################

  //########################## PRIVATE CONSTANTS #########################

  //########################### PRIVATE METHODS ##########################

  //########################### PRIVATE MEMBERS ##########################

};


#endif // _DFAgentTest_h_