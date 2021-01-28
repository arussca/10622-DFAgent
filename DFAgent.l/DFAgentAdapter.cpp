// Copyright (c) 2004 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved

// $Log$

static const char rcsid[]="@(#) $Id$";


// Includes
#include <DFAgentAdapter.h>
#include <DFAgent.h>
#include <IDL_Adapters_DGT.h>
//----------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------
DFAgentAdapter::DFAgentAdapter(DFAgent* device)
: DeviceAdapter(device)
{
  device_=device;
}

//----------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------
DFAgentAdapter::~DFAgentAdapter()
{
}


//----------------------------------------------------------------------
// Device specific operations
//----------------------------------------------------------------------
void DFAgentAdapter::receiveFrame(const DDPKF::Frame& idlNoKeywordsFrame, int currentWindow,
    int numWindows, const DGT::TinyVector& idlTopLeft,
    const DGT::TinyVector& idlBottomRight, const DGT::TimeValue& timestamp,
    const DGT::TimeValue& exposureTime, const char* cameraId)
{
  try
  {
    device_->receiveFrame(idlNoKeywordsFrame, currentWindow, numWindows, idlTopLeft, idlBottomRight,
        timestamp, exposureTime, cameraId);
  }
  catch(GCSException& ex)
  {
    throw toIDL(ex);
  }
}

void  DFAgentAdapter::receiveRecipeResult (const DDPKF::RecipeResult& result)
{
  std::string observingMode(result.obsMode);
  try
  {
    device_->receiveRecipeResult(observingMode, result.id, result.obsResultId, result.qc, result);
  }
  catch(GCSException& ex)
  {
    throw CORBA::BAD_PARAM();
  }
}


//----------------------------------------------------------------------
void DFAgentAdapter::setPrincipalInvestigatorName(const char* piName)
{
  try
  {
    device_->setPrincipalInvestigatorName(piName);
  }
  catch(GCSException& ex)
  {
    throw toIDL(ex);
  }
}

//----------------------------------------------------------------------
void DFAgentAdapter::setObservationType(DFAGENT::ObservationType obsType)
{
  try
  {
    device_->setObservationType((DFAgent::ObservationType)obsType);
  }
  catch(GCSException& ex)
  {
    throw toIDL(ex);
  }
}

//----------------------------------------------------------------------
void DFAgentAdapter::setObservationClass(DFAGENT::ObservationClass obsClass)
{
  try
  {
    device_->setObservationClass((DFAgent::ObservationClass)obsClass);
  }
  catch(GCSException& ex)
  {
    throw toIDL(ex);
  }
}

//----------------------------------------------------------------------
void DFAgentAdapter::setInstrumentMode(const char* insMode)
{
  try
  {
    device_->setInstrumentMode(insMode);
  }
  catch(GCSException& ex)
  {
    throw toIDL(ex);
  }
}

//----------------------------------------------------------------------
void DFAgentAdapter::setObserverName(const char* observerName)
{
  try
  {
    device_->setObserverName(observerName);
  }
  catch(GCSException& ex)
  {
    throw toIDL(ex);
  }
}

//----------------------------------------------------------------------
void DFAgentAdapter::setObjectName(const char* objectName)
{
  try
  {
    device_->setObjectName(objectName);
  }
  catch(GCSException& ex)
  {
    throw toIDL(ex);
  }
}

//----------------------------------------------------------------------
char* DFAgentAdapter::getObjectName()
{
  try
  {
    string tmpObjectName = device_->getObjectName();
    return CORBA::string_dup(tmpObjectName.c_str());
  }
  catch(GCSException& ex)
  {
    throw toIDL(ex);
  }
}

//----------------------------------------------------------------------
void DFAgentAdapter::setProgramId(const char* programId)
{
  try
  {
    device_->setProgramId(programId);
  }
  catch(GCSException& ex)
  {
    throw toIDL(ex);
  }
}

//----------------------------------------------------------------------
void DFAgentAdapter::setESOProgramId(const char* esoProgramId)
{
  try
  {
    device_->setESOProgramId(esoProgramId);
  }
  catch(GCSException& ex)
  {
    throw toIDL(ex);
  }
}

//----------------------------------------------------------------------
void DFAgentAdapter::setObservationBlockId(const char* obId)
{
  try
  {
    device_->setObservationBlockId(obId);
  }
  catch(GCSException& ex)
  {
    throw toIDL(ex);
  }
}

//----------------------------------------------------------------------
void DFAgentAdapter::setImageTitle(const char* imageTitle)
{
  try
  {
    device_->setImageTitle(imageTitle);
  }
  catch(GCSException& ex)
  {
    throw toIDL(ex);
  }
}

//----------------------------------------------------------------------
void DFAgentAdapter::setObservationComment(const char* observationComment)
{
  try
  {
    device_->setObservationComment(observationComment);
  }
  catch(GCSException& ex)
  {
    throw toIDL(ex);
  }
}

//----------------------------------------------------------------------
void DFAgentAdapter::setObsModeLowLevel(CORBA::Short obsMode)
{
  try
  {
    device_->setObsModeLowLevel(obsMode);
  }
  catch(GCSException& ex)
  {
    throw toIDL(ex);
  }
}

//----------------------------------------------------------------------
void DFAgentAdapter::setObservationMode(const char * obsMode)
{
  try
  {
    device_->setObservationMode(obsMode);
  }
  catch(GCSException& ex)
  {
    throw toIDL(ex);
  }

}

//----------------------------------------------------------------------
void DFAgentAdapter::setPropagateFrame(CORBA::Boolean doesPropagate)
{
  try
  {
    device_->setPropagateFrame(doesPropagate);
  }
  catch(GCSException& ex)
  {
    throw toIDL(ex);
  }
}

//----------------------------------------------------------------------
void DFAgentAdapter::setSaveFrame(CORBA::Boolean doesSave)
{
  try
  {
    device_->setSaveFrame(doesSave);
  }
  catch(GCSException& ex)
  {
    throw toIDL(ex);
  }
}

//----------------------------------------------------------------------
void DFAgentAdapter::setPropagateFITS(CORBA::Boolean doesPropagate)
{
  try
  {
    device_->setPropagateFITS(doesPropagate);
  }
  catch(GCSException& ex)
  {
    throw toIDL(ex);
  }
}

//----------------------------------------------------------------------
void DFAgentAdapter::setSaveFITS(CORBA::Boolean doesSave)
{
  try
  {
    device_->setSaveFITS(doesSave);
  }
  catch(GCSException& ex)
  {
    throw toIDL(ex);
  }
}

//----------------------------------------------------------------------
void DFAgentAdapter::setAutoReduce(CORBA::Boolean doesAuto)
{
  try
  {
    device_->setAutoReduce(doesAuto);
  }
  catch(GCSException& ex)
  {
    throw toIDL(ex);
  }
}

//----------------------------------------------------------------------
void DFAgentAdapter::setWriteKeywords(CORBA::Boolean doesWrite)
{
  try
  {
    device_->setWriteKeywords(doesWrite);
  }
  catch(GCSException& ex)
  {
    throw toIDL(ex);
  }
}

//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Monitorable attributes (device magnitudes)
//----------------------------------------------------------------------

// Monitor doesSaveFrame
CORBA::Boolean DFAgentAdapter::doesSaveFrame()
{
  return device_->doesSaveFrame();
}

// Monitor doesPropagateFrame
CORBA::Boolean DFAgentAdapter::doesPropagateFrame()
{
  return device_->doesPropagateFrame();
}

// Monitor doesSaveFITS
CORBA::Boolean DFAgentAdapter::doesSaveFITS()
{
  return device_->doesSaveFITS();
}

// Monitor doesPropagateFITS
CORBA::Boolean DFAgentAdapter::doesPropagateFITS()
{
  return device_->doesPropagateFITS();
}

// Monitor doesAutoReduce
CORBA::Boolean DFAgentAdapter::doesAutoReduce()
{
  return device_->doesAutoReduce();
}

// Monitor doesWriteKeywords
CORBA::Boolean DFAgentAdapter::doesWriteKeywords()
{
  return device_->doesWriteKeywords();
}


//----------------------------------------------------------------------
// Configurable attributes (device properties)
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// get offlineImagesPath
char* DFAgentAdapter::offlineImagesPath()
{
  return CORBA::string_dup(device_->offlineImagesPath());
}
//-------------------------------
// set offlineImagesPath
void DFAgentAdapter::offlineImagesPath(const char* value)
{
  try
  {
    device_->offlineImagesPath(value);
  }
  catch(GCSException& ex)
  {
    throw CORBA::BAD_PARAM();
  }
}

//----------------------------------------------------------------------
// get dfBranchInstanceName
char* DFAgentAdapter::dfBranchInstanceName()
{
  return CORBA::string_dup(device_->dfBranchInstanceName());
}
//-------------------------------
// set dfBranchInstanceName
void DFAgentAdapter::dfBranchInstanceName(const char* value)
{
  try
  {
    device_->dfBranchInstanceName(value);
  }
  catch(GCSException& ex)
  {
    throw CORBA::BAD_PARAM();
  }
}

// get dfManagerInstanceName
char* DFAgentAdapter::dfManagerInstanceName()
{
  //return CORBA::string_dup(device_->dfManagerInstanceName());
  return CORBA::string_dup("");
}

//-------------------------------
// set dfManagerInstanceName
void DFAgentAdapter::dfManagerInstanceName(const char* value)
{
  try
  {
    device_->dfManagerInstanceName(value);
  }
  catch(GCSException& ex)
  {
    throw CORBA::BAD_PARAM();
  }
}


//----------------------------------------------------------------------
// get dfAgentPartnerInstanceName
char* DFAgentAdapter::dfAgentPartnerInstanceName()
{
  return CORBA::string_dup(device_->dfAgentPartnerInstanceName());
}
//-------------------------------
// set dfAgentPartnerInstanceName
void DFAgentAdapter::dfAgentPartnerInstanceName(const char* value)
{
  try
  {
    device_->dfAgentPartnerInstanceName(value);
  }
  catch(GCSException& ex)
  {
    throw CORBA::BAD_PARAM();
  }
}

//----------------------------------------------------------------------
// get propagateFrame
CORBA::Short DFAgentAdapter::propagateFrame()
{
  return device_->propagateFrame();
}
//-------------------------------
// set propagateFrame
void DFAgentAdapter::propagateFrame(CORBA::Short value)
{
  try
  {
    device_->propagateFrame(value);
  }
  catch(GCSException& ex)
  {
    throw CORBA::BAD_PARAM();
  }
}

//----------------------------------------------------------------------
// get saveFrame
CORBA::Short DFAgentAdapter::saveFrame()
{
  return device_->saveFrame();
}
//-------------------------------
// set saveFrame
void DFAgentAdapter::saveFrame(CORBA::Short value)
{
  try
  {
    device_->saveFrame(value);
  }
  catch(GCSException& ex)
  {
    throw CORBA::BAD_PARAM();
  }
}

//----------------------------------------------------------------------
// get propagateFITS
CORBA::Short DFAgentAdapter::propagateFITS()
{
  return device_->propagateFITS();
}
//-------------------------------
// set propagateFITS
void DFAgentAdapter::propagateFITS(CORBA::Short value)
{
  try
  {
    device_->propagateFITS(value);
  }
  catch(GCSException& ex)
  {
    throw CORBA::BAD_PARAM();
  }
}

//----------------------------------------------------------------------
// get saveFITS
CORBA::Short DFAgentAdapter::saveFITS()
{
  return device_->saveFITS();
}
//-------------------------------
// set saveFITS
void DFAgentAdapter::saveFITS(CORBA::Short value)
{
  try
  {
    device_->saveFITS(value);
  }
  catch(GCSException& ex)
  {
    throw CORBA::BAD_PARAM();
  }
}

//----------------------------------------------------------------------
// get autoReduce
CORBA::Short DFAgentAdapter::autoReduce()
{
  return device_->autoReduce();
}
//-------------------------------
// set autoReduce
void DFAgentAdapter::autoReduce(CORBA::Short value)
{
  try
  {
    device_->autoReduce(value);
  }
  catch(GCSException& ex)
  {
    throw CORBA::BAD_PARAM();
  }
}

//----------------------------------------------------------------------
// get writeKeywords
CORBA::Short DFAgentAdapter::writeKeywords()
{
  return device_->writeKeywords();
}
//-------------------------------
// set writeKeywords
void DFAgentAdapter::writeKeywords(CORBA::Short value)
{
  try
  {
    device_->writeKeywords(value);
  }
  catch(GCSException& ex)
  {
    throw CORBA::BAD_PARAM();
  }
}

//----------------------------------------------------------------------
// get flippedImage
CORBA::Short DFAgentAdapter::flippedImage()
{
  return device_->flippedImage();
}
//-------------------------------
// set flippedImage
void DFAgentAdapter::flippedImage(CORBA::Short value)
{
  try
  {
    device_->flippedImage(value);
  }
  catch(GCSException& ex)
  {
    throw CORBA::BAD_PARAM();
  }
}

//----------------------------------------------------------------------
// get testingMode
CORBA::Short DFAgentAdapter::testingMode()
{
  return device_->testingMode();
}
//-------------------------------
// set testingMode
void DFAgentAdapter::testingMode(CORBA::Short value)
{
  try
  {
    device_->testingMode(value);
  }
  catch(GCSException& ex)
  {
    throw CORBA::BAD_PARAM();
  }
}

//----------------------------------------------------------------------
// get obsModeKeywordListPath
char* DFAgentAdapter::obsModeKeywordListPath()
{
  return CORBA::string_dup(device_->obsModeKeywordListPath());
}
//-------------------------------
// set obsModeKeywordListPath
void DFAgentAdapter::obsModeKeywordListPath(const char* value)
{
  try
  {
    device_->obsModeKeywordListPath(value);
  }
  catch(GCSException& ex)
  {
    throw CORBA::BAD_PARAM();
  }
}

//----------------------------------------------------------------------
// get recipeDictionaryPath
char* DFAgentAdapter::recipeDictionaryPath()
{
  return CORBA::string_dup(device_->recipeDictionaryPath());
}
//-------------------------------
// set recipeDictionaryPath
void DFAgentAdapter::recipeDictionaryPath(const char* value)
{
  try
  {
    device_->recipeDictionaryPath(value);
  }
  catch(GCSException& ex)
  {
    throw CORBA::BAD_PARAM();
  }
}

//----------------------------------------------------------------------
// get gcsComponent
char* DFAgentAdapter::gcsComponent()
{
  return CORBA::string_dup(device_->gcsComponent());
}
//-------------------------------
// set gcsComponent
void DFAgentAdapter::gcsComponent(const char* value)
{
  try
  {
    device_->gcsComponent(value);
  }
  catch(GCSException& ex)
  {
    throw CORBA::BAD_PARAM();
  }
}

//----------------------------------------------------------------------
// get cameraName
char* DFAgentAdapter::cameraName()
{
  return CORBA::string_dup(device_->cameraName());
}
//-------------------------------
// set cameraName
void DFAgentAdapter::cameraName(const char* value)
{
  try
  {
    device_->cameraName(value);
  }
  catch(GCSException& ex)
  {
    throw CORBA::BAD_PARAM();
  }
}

//----------------------------------------------------------------------
// get referencePixel
DGT::DoubleArray* DFAgentAdapter::referencePixel()
{
  DGT::DoubleArray* idl_prop = new DGT::DoubleArray;
  toIDL(device_->referencePixel(), *idl_prop);
  return idl_prop;
}
//-------------------------------
// set referencePixel
void DFAgentAdapter::referencePixel(const DGT::DoubleArray& value)
{
  try
  {
    DoubleArray1D prop;
    fromIDL(value,prop);
    device_->referencePixel(prop);
  }
  catch(GCSException& ex)
  {
    throw CORBA::BAD_PARAM();
  }
}

/**************************************************************
 *                private methods                             *
 **************************************************************/

