// Copyright (c) 2004 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved

// $Log$

static const char rcsid[]="@(#) $Id$";


// GCS Includes
#include "dfAgentLCU.h"
#include "dfAgentApp.h"
#include "TimeService.h"
#include "ace/ACE.h"
#ifdef VXWORKS
#include "DDFactory.h"
#endif
#include <string>

// Constructor
dfAgentLCU::dfAgentLCU(char* name) 
              :LCU(name)
{
}

// Destructor
dfAgentLCU::~dfAgentLCU()
{
}

// Create the Devices of the lcu
int dfAgentLCU::start(int update)
{
  printf("-------------------------------------------------------------------------------\n");
  printf("Starting %s services...\n",name_);
  printf("-------------------------------------------------------------------------------\n");
#ifdef VXWORKS
  try
  {
    driverFactory_ = DDFactory::instance();
  }
  catch(GCSException& ex)
  {
    ex.addToTrace("dfAgentLCU::start");
    ex.addToTrace("Error initialising DDFactory");
    cerr << ex;
  }
  cerr << driverFactory_->report();
#endif

  //-------------------
  // Start ACE services
  //-------------------
  ACE::init();

  //---------------------------------------
  // Initialising Time Service for this LCU
  //---------------------------------------
  try
  {
    timeService_ = TimeService::instance();
  }
  catch(GCSException& ex)
  {
    ex.addToTrace("dfAgentLCU::start");
    ex.addToTrace("Error initialising TimeService");
    cerr << ex;
  }

  // Start your Applications and Devices here
  printf("-------------------------------------------------------------\n");
  printf("Starting %s Applications...\n",name_);
  printf("-------------------------------------------------------------\n");
  try
  {
    dfAgentApp_ = new dfAgentApp(const_cast<char*>("dfAgentApp"));
  }
  catch(GCSException& ex)
  {
    ex.addToTrace("dfAgentLCU::start");
    ex.addToTrace("Error creating application");
    cerr << ex;
  }
  try
  {
    dfAgentApp_->run(update);
  }
  catch(GCSException& ex)
  {
    ex.addToTrace("dfAgentLCU::start");
    ex.addToTrace("Error running application");
    cerr << ex;
  }

  return 0;
}

// Stop services and applications
int dfAgentLCU::stop()
{
  printf("-------------------------------------------------------------------------------\n");
  printf("Stopping %s...\n",name_);
  printf("-------------------------------------------------------------------------------\n");

  dfAgentApp_->quit();
  delete dfAgentApp_;

  // Stop services here
  //timeService_->fini();
  //delete timeService_;
  // ACE, DDFactory...
  return 0;
}

#ifdef VXWORKS
//---------------------------------------------------------------------------
// These functions can be called from the VxWorks shell or VxWorks scripts
//---------------------------------------------------------------------------
dfAgentLCU* _dfAgentLCU_;

extern "C" void start_LCU(int update=0)
{
  _dfAgentLCU_ = new dfAgentLCU("dfAgentLCU");
  _dfAgentLCU_->start(update); 
}

extern "C" void stop_LCU()
{
  _dfAgentLCU_->stop(); 
  delete _dfAgentLCU_;
}

extern "C" void reboot_LCU()
{
  _dfAgentLCU_->reBoot(); 
}
#else
int main(int argc, char** argv)
{
  dfAgentLCU* _dfAgentLCU_;
  int update=0;
  CommandLine::current(argc, argv);
  CommandLine cl(argc, argv);
  update = cl.getFlag("-update");
  _dfAgentLCU_ = new dfAgentLCU(const_cast<char*>("dfAgentLCU"));
  _dfAgentLCU_->start(update); 
  // Leave the program running
  while(true)
  {
    sleep(2);
  };
}
#endif

