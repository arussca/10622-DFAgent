// Copyright (c) 2019 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved
//
//
// $Log$

#ifndef _FITSHeaderInfoTest_h_
#define _FITSHeaderInfoTest_h_

// Includes
#include "FITSHeaderInfo.h"               // FITSHeaderInfo

#include <string>                         // std::string
#include <memory>                         // std::unique_ptr
#include <Trace.h>                        // Trace
#include <gtest/gtest.h>                  // ::testing*, TEST_*, ...

//----------------------------------------------------------------------
// Prototypes
//----------------------------------------------------------------------

/**
 * Main function to perform all unit test for FITSHeaderInfoTest
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Array of arguments.
 * @return Status code of the program execution.
 * @retval Zero successful program execution status.
 * @retval Non-zero Unsuccessful execution of a program.
 */
int main(int argc, char** argv);

/**
 * Class reserved to perform unit tests of FITSHeaderInfo class.
 */
class FITSHeaderInfoTest: public ::testing::Test
{
  //--------------------------------------------------
  // public section
  //--------------------------------------------------
  public:

  //############################ PUBLIC TYPES ############################

  //########################## PUBLIC CONSTANTS ##########################

  //############# CONSTRUCTORS, DESTRUCTOR & SPECIAL METHODS #############

  /** Default constructor */
  FITSHeaderInfoTest();

  /** Destructor */
  virtual ~FITSHeaderInfoTest();

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

  //########################## PROTECTED MEMBERS #########################
  // Objects declared here can be used by all tests in the test case    //
  // for FITSHeaderInfo.                                                //

  /** Member that allows you obtain more information about any test */
  static Trace trace_;

  /**
   * Pointer of FITSHeaderInfo class, whose main aim is be
   * a fixed object to compare results
   */
  FITSHeaderInfo *expected_;

  /**
   * Pointer of FITSHeaderInfo class, whose main aim is be
   * built on several tests
   */
  FITSHeaderInfo *current_;


  //--------------------------------------------------
  // private section
  //--------------------------------------------------
  private:

  //############################ PRIVATE TYPES ###########################

  //########################## PRIVATE CONSTANTS #########################

  //########################### PRIVATE METHODS ##########################

  //########################### PRIVATE MEMBERS ##########################

};

#endif // _FITSHeaderInfoTest_h_