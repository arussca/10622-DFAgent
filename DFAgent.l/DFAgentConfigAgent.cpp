// Copyright (c) 2004 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved

// $Log$

static const char rcsid[]="@(#) $Id$";


// Includes
#include <DFAgentConfigAgent.h>

//----------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------
DFAgentConfigAgent::DFAgentConfigAgent(DFAgent* device, const int mode, TimerEngine* timer)
 : ConfigAgent(device->name(),device->className(),mode,timer),trace_(device->name())
{
  device_ = device;

  // Assign default values to properties so they never contain garbage
  offlineImagesPath_   = "offlineDirectory";
  dfBranchInstanceName_  = "DF/DFBranch";
  dfAgentPartnerInstanceName_  = "DF/SFSNB_DFAgentPartner";  
  propagateFrame_      = 1;
  saveFrame_           = 1;
  propagateFITS_       = 1;
  saveFITS_            = 1;
  autoReduce_          = 1;  
  writeKeywords_       = 1;  
  flippedImage_        = 1;  
  testingMode_        = 0;  
  obsModeKeywordListPath_  = "Path";
  recipeDictionaryPath_  = "dictionary";
  gcsComponent_        = "gcsComponent";
  cameraName_          = "cameraName";
}

//----------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------
DFAgentConfigAgent::~DFAgentConfigAgent()
{
}

//----------------------------------------------------------------------
// applyProperties: applies the stored configuration to the Device
//----------------------------------------------------------------------
void DFAgentConfigAgent::applyProperties()
{
  trace_.out("DFAgentConfigAgent::applyProperties()");
  try
  {
    // Apply properties one by one
    device_->offlineImagesPath(this->offlineImagesPath_);
    device_->dfBranchInstanceName(this->dfBranchInstanceName_);
    device_->dfAgentPartnerInstanceName(this->dfAgentPartnerInstanceName_);
    device_->propagateFrame(this->propagateFrame_);
    device_->saveFrame(this->saveFrame_);
    device_->propagateFITS(this->propagateFITS_);
    device_->saveFITS(this->saveFITS_);
    device_->autoReduce(this->autoReduce_);
    device_->writeKeywords(this->writeKeywords_);    
    device_->flippedImage(this->flippedImage_);        
    device_->testingMode(this->testingMode_);    
    device_->obsModeKeywordListPath(this->obsModeKeywordListPath_);
    device_->recipeDictionaryPath(this->recipeDictionaryPath_);
    device_->gcsComponent(this->gcsComponent_);
    device_->cameraName(this->cameraName_);
    device_->referencePixel(this->referencePixel_);    
  }
  catch (GCSException& ex)
  {
    ex.addToTrace("DFAgentConfigAgent::applyProperties");
    throw;
  }
}

//----------------------------------------------------------------------
// readProperties: obtains the property values from the Device
//----------------------------------------------------------------------
int DFAgentConfigAgent::readProperties()
{
  trace_.out("DFAgentConfigAgent::readProperties()");
  try
  {
    // Read properties one by one
    this->offlineImagesPath_   = device_->offlineImagesPath();
    this->dfBranchInstanceName_  = device_->dfBranchInstanceName();
    this->dfAgentPartnerInstanceName_  = device_->dfAgentPartnerInstanceName();
    this->propagateFrame_      = device_->propagateFrame();
    this->saveFrame_           = device_->saveFrame();
    this->propagateFITS_       = device_->propagateFITS();
    this->saveFITS_            = device_->saveFITS();
    this->autoReduce_            = device_->autoReduce();    
    this->writeKeywords_            = device_->writeKeywords();    
    this->flippedImage_            = device_->flippedImage();
    this->testingMode_            = device_->testingMode();    
    this->obsModeKeywordListPath_  = device_->obsModeKeywordListPath();
    this->recipeDictionaryPath_  = device_->recipeDictionaryPath();
    this->gcsComponent_        = device_->gcsComponent();
    this->cameraName_          = device_->cameraName();
    this->referencePixel_      = device_->referencePixel();    
  }
  catch (GCSException& ex)
  {
    ex.addToTrace("DFAgentConfigAgent::readProperties");
    throw;
  }
  return(0);
}

//----------------------------------------------------------------------
// toConfiguration: Converts the Device properties to a Configuration
//----------------------------------------------------------------------
void DFAgentConfigAgent::toConfiguration(CONFIG::Configuration& c)
{
  trace_.out("DFAgentConfigAgent::toConfiguration()");
  try
  {
    // Let ConfigAgent base class do the common stuff
    this->ConfigAgent::toConfiguration(c);

    addProperty(c, "offlineImagesPath", offlineImagesPath_);
    addProperty(c, "dfBranchInstanceName", dfBranchInstanceName_);
    addProperty(c, "dfAgentPartnerInstanceName", dfAgentPartnerInstanceName_);
    addProperty(c, "propagateFrame", propagateFrame_);
    addProperty(c, "saveFrame", saveFrame_);
    addProperty(c, "propagateFITS", propagateFITS_);
    addProperty(c, "saveFITS", saveFITS_);
    addProperty(c, "autoReduce", autoReduce_);    
    addProperty(c, "writeKeywords", writeKeywords_);
    addProperty(c, "flippedImage", flippedImage_);
    addProperty(c, "testingMode", testingMode_);    
    addProperty(c, "obsModeKeywordListPath", obsModeKeywordListPath_);
    addProperty(c, "recipeDictionaryPath", recipeDictionaryPath_);
    addProperty(c, "gcsComponent", gcsComponent_);
    addProperty(c, "cameraName", cameraName_);
    addProperty(c, "referencePixel", referencePixel_);    
  }
  catch (GCSException& ex)
  {
    ex.addToTrace("DFAgentConfigAgent::toConfiguration");
    throw;
  }
}

//----------------------------------------------------------------------
// fromConfiguration: Converts the configuration to the Device format
//----------------------------------------------------------------------
void DFAgentConfigAgent::fromConfiguration(const CONFIG::Configuration& c)
{
  trace_.out("DFAgentConfigAgent::fromConfiguration()");
  try
  {
    // Let ConfigAgent base class do the common stuff
    this->ConfigAgent::fromConfiguration(c);

    extractProperty(c, "offlineImagesPath", offlineImagesPath_);
    extractProperty(c, "dfBranchInstanceName", dfBranchInstanceName_);
    extractProperty(c, "dfAgentPartnerInstanceName", dfAgentPartnerInstanceName_);
    extractProperty(c, "propagateFrame", propagateFrame_);
    extractProperty(c, "saveFrame", saveFrame_);
    extractProperty(c, "propagateFITS", propagateFITS_);
    extractProperty(c, "saveFITS", saveFITS_);
    extractProperty(c, "autoReduce", autoReduce_);
    extractProperty(c, "writeKeywords", writeKeywords_);    
    extractProperty(c, "flippedImage", flippedImage_);
    extractProperty(c, "testingMode", testingMode_);    
    extractProperty(c, "obsModeKeywordListPath", obsModeKeywordListPath_);
    extractProperty(c, "recipeDictionaryPath", recipeDictionaryPath_);
    extractProperty(c, "gcsComponent", gcsComponent_);
    extractProperty(c, "cameraName", cameraName_);
    extractProperty(c, "referencePixel", referencePixel_);    
  }
  catch (GCSException& ex)
  {
    ex.addToTrace("DFAgentConfigAgent::fromConfiguration");
    throw;
  }
}

