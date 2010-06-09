// Copyright (c) 2004 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved

// $Log$

static const char rcsid[]="@(#) $Id$";


// GCS Includes
#include "dfAgentApp.h"
#include "DFAgent.h"

// Constructor
dfAgentApp::dfAgentApp(char* name) 
              :Application(name)
{
}

// Destructor
dfAgentApp::~dfAgentApp()
{
}

// Create the Devices of the application
int dfAgentApp::run(int update)
{
  // Create Devices
  try
  {
    // Create Device 1
    dfAgent_  = new DFAgent("DevLib/DFAgent");
    dfAgent_->connect("orb1",4,50,Device::INTEGRATED,update);
  }
  catch(GCSException& ex)
  {
    ex.addToTrace("dfAgentApp:run");
    cerr << "Error creating Device for this application" << endl;
    throw;
  }
  // Start Devices (if required)
  try
  {
    // dfAgent_->start();
  }
  catch(GCSException& ex)
  {
    ex.addToTrace("dfAgentApp:run");
    cerr << "Error starting Device" << endl;
    throw;
  }
  // Init Devices (if required)
  try
  {
    // dfAgent_->init();
  }
  catch(GCSException& ex)
  {
    ex.addToTrace("dfAgentApp:run");
    cerr << "Error initialising Device" << endl;
    throw;
  }
  return(0);
}

// Delete the Devices of the family 
int dfAgentApp::quit()
{
  try
  {
    // Delete Device 
    // dfAgent_->halt();
    // dfAgent_->shutdown();
    dfAgent_->disconnect();
    delete dfAgent_;
  }
  catch(GCSException& ex)
  {
    ex.addToTrace("dfAgentApp:quit");
    cerr << ex;
  }
  return(0);
}

#ifdef VXWORKS
//---------------------------------------------------------------------------
// These functions can be called from the VxWorks shell or VxWorks scripts
//---------------------------------------------------------------------------
dfAgentApp* _dfAgentApp_;

extern "C" void run_dfAgentApp(int update=0)
{
  _dfAgentApp_ = new dfAgentApp("dfAgentApp");
  _dfAgentApp_->run(update); 
}

extern "C" void quit_dfAgentApp(void)
{
  _dfAgentApp_->quit(); 
  delete _dfAgentApp_;
}
#endif
