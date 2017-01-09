// Copyright (c) 2004 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved

// $Log$

static const char rcsid[]="@(#) $Id$";


// Includes
#include <DFAgent.h>
#include <DFAgentAdapter.h>
#include <DFAgentConfigAgent.h>

//@ipc
#include <MonitorHubFactory.h>
#include <algorithm>
#include <IDL_Adapters_RF.h>
#include <IDL_Adapters_DPKF.h>
#include <DataPackInfo.h>
#include <CORBARef.h>
#include <sys/types.h>
#include <sys/stat.h>
//@epc

//----------------------------------------------------------------------
// Device Constructor
//----------------------------------------------------------------------
DFAgent::DFAgent(string name)
 : Device(name)
{
  trace_.out("constructor\n");
  className_ = "DFAgent";
  isComposite_ = FALSE;
  
  // Init some properties with hardcoded default values
  offlineImagesPath_         = "offlineDirectory";
  dfBranchInstanceName_      = "DF/Manager1/BranchAG";
  dfAgentPartnerInstanceName_  = "DF/SFSNB_DFAgentPartner";
  obsModeKeywordListPath_    = "Path";
  recipeDictionaryPath_      = "dictionary";
  gcsComponent_              = "gcsComponent";
  cameraName_                = "cameraName";
  dfSessionId_               = 0;
  shortObsMode_              = 0;
  for(int w=0; w<referencePixel_.size(); w++) referencePixel_[w]=0;  

  //initialize variables  
  referencePixel_[0] = 512.0;
  referencePixel_[1] = 512.0;
    
  //Initialize ObsType variable to default value
  obsType_ = OBJECTS;
  //initialize map with ObservationType and string names..

  obsTypeName_[BIAS] = "BIAS";
  obsTypeName_[DARK] = "DARK";
  obsTypeName_[SKY_FLAT] = "SKY_FLAT";
  obsTypeName_[DOME_FLAT] = "DOME_FLAT";
  obsTypeName_[SPECTRAL_FLAT] = "SPECTRAL_FLAT";
  obsTypeName_[ARC] = "ARC";
  obsTypeName_[SKY] = "SKY";
  obsTypeName_[OBJECTS] = "OBJECT";

  //Initialize ObsClass variable to default value
  obsClass_ = SCIENCE;
  //initialize map with ObservationClass and string names..
  obsClassName_[SCIENCE] = "Science";
  obsClassName_[CALIB] = "Calib";
  
  //@ipc
  //init pointer to devices to null
  dfBranchInstance_ = DF::Branch_ifce::_nil();
  dfAgentPartnerInstance_ = DFAGENT::DFAgent_ifce::_nil();
  
  //init some custom variables
  rawFrame_ = NULL;
  isSessionRunning_ = false;
  //@epc
}

//----------------------------------------------------------------------
// Device destructor
//----------------------------------------------------------------------
DFAgent::~DFAgent()
{
  trace_.out("destructor\n");
  // Disconnect and delete aggregated Devices
}

/************************************************************************
*************************************************************************
**                                                                     **
**            Virtual methods inherited from Device interface          **
**                                                                     **
*************************************************************************
************************************************************************/
  
//----------------------------------------------------------------------
// Connects device with GCS services
//----------------------------------------------------------------------
void DFAgent::connect(string orbname,unsigned threads,long priority,
                                bool mode,bool update)
{
  Device::connect(orbname,threads,priority,mode,update);
  
  // Create IDL/CORBA Adapter
  try
  {
    trace_.out("creating IDL adapter\n");
    adapter_ = new DFAgentAdapter(this);
  }
  catch(GCSException& ex)
  {
    ex.addToTrace("DFAgent::connect()"); 
    trace_.err("IDL adapter can't be created\n");
    throw;
  }
  
  // Connect adapter with CORBA context
  try
  {
    trace_.out("connecting adapter with CORBA\n");
    adapter_->setupCorba((PortableServer::Servant)adapter_,corbaContext_);
  }
  catch(GCSException& ex)
  {
    ex.addToTrace("DFAgent::connect()"); 
    trace_.err("IDL adapter can't be connected to CORBAContext\n");
    throw;
  }
    
  // Create the ConfigAgent for this class
  try
  {
    trace_.out("creating configuration agent\n");
    configAgent_ = new DFAgentConfigAgent(this,mode_,lowPriorityTimer_);
  }
  catch(GCSException& ex)
  {
    ex.addToTrace("DFAgent::connect()"); 
    if(ex.isA("CorbaException")) trace_.err("ConfigManager is down or unreachable\n");
      else trace_.err("unable to create config agent\n");
    throw;
  }
  
  // Update profile if required
  if (update) updateProfile_();
  
  trace_.out("connected\n");
}

//----------------------------------------------------------------------
// Performs starting procedure: OFF-->STARTING-->ON
//----------------------------------------------------------------------
void DFAgent::start()
{
  trace_.out("start()\n");
  goStarting_();
  
  // Configure device
  try
  {
    configure("Default");
  }
  catch(GCSException& ex)
  {
    goFault_();
    ex.addToTrace("DFAgent::start()"); 
    logError_("unable to configure device");
    throw;
  }
  
  createMonitors_();
  
  // ## Create other classes required for this Device
  
  // ## Create other threads required for this Device
  
  // 
  goOn_();
}

//----------------------------------------------------------------------
// Performs initialization procedure: ON-->INITIALISING-->READY
//----------------------------------------------------------------------
void DFAgent::init()
{
  trace_.out("init()\n");
  goInitialising_();
  
  // ## Initialization procedure
  
  // Init aggregated Devices
  goIdle_();  // or goRunning_();
}

//----------------------------------------------------------------------
// Performs halting procedure: READY-->HALTING-->ON
//----------------------------------------------------------------------
void DFAgent::halt()
{
  trace_.out("halt()\n");
  goHalting_();
  
  // ## Halting procedure
  
  // Halt aggregated Devices
  goOn_();
}

//----------------------------------------------------------------------
// Performs shutdown procedure: ON-->SHUTTING_DOWN-->OFF
//----------------------------------------------------------------------
void DFAgent::shutdown()
{
  trace_.out("shutdown()\n");
  goShuttingDown_();
  // //if started previously
  
  // ## Shutdown procedure
  
  // Shutdown aggregated Devices
  goOff_();
}

//----------------------------------------------------------------------
// Performs reset procedure: FAULT-->RESETTING-->ON
//----------------------------------------------------------------------
void DFAgent::reset()
{
  trace_.out("reset()\n");
  goResetting_();
  
  // ## Resetting procedure
  
  // Reset aggregated Devices
  goOn_();
}

//----------------------------------------------------------------------
// Performs restore procedure: goes to previous state
//----------------------------------------------------------------------
void DFAgent::restore()
{
  trace_.out("restore()\n");
  if (isFault()==FALSE)
  {
    logError_("Restore is only posible in FAULT state");
    throw WrongState(name_.c_str(),"Not in FAULT state. Unable to restore");
  }
  try
  { //It is a timeout, try to reconnect
    initDFBranchInstance_();
    initDFAgentPartnerInstance_();
    goPreviousState_();
    cancelAlarm_(const_cast<char*>("DeviceFault"),const_cast<char*>("Device is restored"));
  }
  catch (GCSException& ex)
  {
    ex.addToTrace ("DFAgent::restore()");
    logError_ ("ERROR when connecting to CORBA Pointers: %s", ex.toString());
    goFault_();
    throw;
  }
}

//----------------------------------------------------------------------
// Performs test procedure: only possible in ON state
//----------------------------------------------------------------------
void DFAgent::test()
{
  trace_.out("test()\n");
  if (isOn()==FALSE)
  {
    logError_("Test is only posible in ON state");
    throw WrongState(name_.c_str(),"Not in ON state. Unable to test");
  }
  
  // ## Test procedure
  
  // ## Test aggregated Devices
}

//----------------------------------------------------------------------
// Performs abort: interrupt any current operation and goes to FAULT
//----------------------------------------------------------------------
void DFAgent::abort()
{
  trace_.out("abort()\n");
  
  // ## Abort procedure
  
  // Abort aggregated Devices
  goFault_();
}

//----------------------------------------------------------------------
// Report device status: returns static memory, do not delete! 
//----------------------------------------------------------------------
char* DFAgent::report(short level)
{
  trace_.out("report(%d)\n",level);
  char* dev_reply = new char[DEVICE_REPORT_SIZE_];
  ostrstream ost(dev_reply, DEVICE_REPORT_SIZE_);
  memset(dev_reply,0,DEVICE_REPORT_SIZE_);
  
  // Generic Device info
  ost << Device::report(level);
  
  // ## Use level parameter to show more/less info
  // Current Device report
  ost << endl << "Device Properties" << endl;   
  ost << "-------------------------------------------------------" << endl;
  ost << "offlineImagesPath   =" << offlineImagesPath() << endl;
  ost << "dfBranchInstanceName=" << dfBranchInstanceName() << endl;
  ost << "dfAgentPartnerInstanceName=" << dfAgentPartnerInstanceName()  << endl;
  ost << "propagateFrame      =" << propagateFrame()    << endl;
  ost << "saveFrame           =" << saveFrame()         << endl;
  ost << "propagateFITS       =" << propagateFITS()     << endl;
  ost << "saveFITS            =" << saveFITS()          << endl;
  ost << "autoReduce            =" << autoReduce()          << endl;  
  ost << "writeKeywords            =" << writeKeywords()          << endl;    
  ost << "flippedImage            =" << flippedImage()          << endl;  
  ost << "testingMode            =" << testingMode()          << endl;  
  ost << "obsModeKeywordListPath =" << obsModeKeywordListPath() << endl;
  ost << "recipeDictionaryPath=" << recipeDictionaryPath() << endl;
  ost << "gcsComponent        =" << gcsComponent()      << endl;
  ost << "cameraName          =" << cameraName()        << endl;
  // ## Add here your other properties
  
  ost << endl << "Device Magnitudes" << endl;   
  ost << "-------------------------------------------------------" << endl;
  ost << "doesSaveFrame       :" << doesSaveFrame()     << endl;
  ost << "doesPropagateFrame  :" << doesPropagateFrame()<< endl;
  ost << "doesSaveFITS        :" << doesSaveFITS()      << endl;
  ost << "doesPropagateFITS   :" << doesPropagateFITS() << endl;
  ost << "doesAutoReduce      :" << doesAutoReduce() << endl;
  ost << "doesWriteKeywords   :" << doesWriteKeywords() << endl;  
  // ## Add here your other monitors
  
  // ## Add more information if required
  
  // Add report of aggregated Devices if required
  
  return(dev_reply);
}

//----------------------------------------------------------------------
// Returns RCSID. Returns static memory; do not delete!  
//----------------------------------------------------------------------
const char* DFAgent::version()
{
  trace_.out("version()\n");
  return(rcsid);
}

//----------------------------------------------------------------------
// Create monitors of this Device  
//----------------------------------------------------------------------
void DFAgent::createMonitors_()
{
  trace_.out("creating monitors\n");
}

//----------------------------------------------------------------------
// Starts the monitors of this Device 
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// Stops the monitors of this Device 
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// Create aggregated Devices  
//----------------------------------------------------------------------
void DFAgent::createDevices_(bool update)
{
  trace_.out("creating aggregated Devices\n");
}

/************************************************************************
*************************************************************************
**                                                                     **
**                 Specific methods of this Device                     **
**                                                                     **
*************************************************************************
*************************************************************************
* All specific methods:                                                 *
* - Shall make a transition to RUNNING (even when is already running)   *
* - Shall make a transition to IDLE when the Device in really idle      *
* - Use lock_()/unlock_() only for thread safe operations               *
* - If an error is detected use logError_() & throw GCSException        *
* - If an alarm is detected use sendAlarm_()                            *
* - If a failure is detected use goFault_(method)                       *
* - Acces to a DeviceDriver or aggregated Device can be required        *
************************************************************************/

//----------------------------------------------------------------------
// getWithFrameRecipeElement_
//----------------------------------------------------------------------
DDPKF::DataElement *DFAgent::getWithFrameRecipeElement_(const string & elementName)
{
  //virtual function to be implemented by 
  return NULL;  
}  

//----------------------------------------------------------------------
// getBeforeFrameRecipeElement_
//----------------------------------------------------------------------
DDPKF::DataElement *DFAgent::getBeforeFrameRecipeElement_(const string & elementName)
{
  //virtual function to be implemented by 
  return NULL;  
}  

//----------------------------------------------------------------------
// belongsToRecipe_
//----------------------------------------------------------------------
bool DFAgent::belongsToRecipe_(const string& elementName)
{
  bool status(false);
  string elementTraslationName(recipeDictionary_.fetchTranslation(elementName, obsModeName_[shortObsMode_]) );
  vector<string>::iterator itResult(find(recipeElementNames_.begin(), recipeElementNames_.end(), elementTraslationName));
  if(itResult != recipeElementNames_.end())
  {
    status = true;
  }
  return status;
}

//----------------------------------------------------------------------
// sendElement_
//----------------------------------------------------------------------
void DFAgent::sendElement_(string elementRecipeName, bool hasFrameArrived)
{
  DDPKF::DataElement * pDataElement = NULL;
  try
  {      
     //check dfBranchInstance_ isn't null, otherwise reconnect to it.
    initDFBranchInstance_();
  
    // If pointer to corba object isn't NULL, send element to DF
    if (!CORBA::is_nil(const_cast<const DF::Branch_ifce_var&>(dfBranchInstance_)))
    {
      //Translate recipe specific term into generic agent varaible name
      //agentName is the same as recipeName by default    
      string elementName(recipeDictionary_.fetchOriginal(elementRecipeName, obsModeName_[shortObsMode_]) );
      if(hasFrameArrived)
      {
        pDataElement = getWithFrameRecipeElement_(elementName);
      }
      else
      {
        pDataElement = getBeforeFrameRecipeElement_(elementName);
      }
      
      if( pDataElement != NULL)
      {
        dfBranchInstance_->receiveORParam( *pDataElement, dfBranchInstanceName_, dfSessionId_);
        //cout << elementName << " sent!!" << endl;
        delete pDataElement;
      }
    }
  }
  catch(const CORBA::Exception &ex)
  {
     GCSException gcsex = DException::mapCORBAException(const_cast<CORBA::Exception&>(ex));
     if (gcsex.isA ("CorbaException"))
     {  //It is a timeout, was a communications error
       dfBranchInstance_ = DF::Branch_ifce::_nil();
     }
     delete pDataElement;
     throw gcsex;
  }
  catch( GCSException& ex )
  {
    ex.addToTrace("DFAgent::sendElement_(const string&)");
    logError_ ("Error while sending element: %s", ex.toString());
    delete pDataElement;
    throw;
  }
}

//----------------------------------------------------------------------
// sendElement_
//----------------------------------------------------------------------
void DFAgent::sendElement_(string elementRecipeName)
{
  DDPKF::DataElement * pDataElement = NULL;
  try
  {      
     //check dfBranchInstance_ isn't null, otherwise reconnect to it.
    initDFBranchInstance_();
  
    // If pointer to corba object isn't NULL, send element to DF
    if (!CORBA::is_nil(const_cast<const DF::Branch_ifce_var&>(dfBranchInstance_)))
    {
      //Translate recipe specific term into generic agent varaible name
      //agentName is the same as recipeName by default    
      string elementName(recipeDictionary_.fetchOriginal(elementRecipeName, obsModeName_[shortObsMode_]) );
      pDataElement = getWithFrameRecipeElement_(elementName);
      if(pDataElement == NULL)
      { //try BeforeFrame, just in case it is not an withFrame element
        pDataElement = getBeforeFrameRecipeElement_(elementName);
      }
      
      if( pDataElement != NULL)
      {
        dfBranchInstance_->receiveORParam( *pDataElement, dfBranchInstanceName_, dfSessionId_);
        //cout << elementName << " sent!!" << endl;
        delete pDataElement;
      }
    }
  }
  catch(const CORBA::Exception &ex)
  {
     GCSException gcsex = DException::mapCORBAException(const_cast<CORBA::Exception&>(ex));
     if (gcsex.isA ("CorbaException"))
     {  //It is a timeout, was a communications error
       dfBranchInstance_ = DF::Branch_ifce::_nil();
     }
     delete pDataElement;
     throw gcsex;
  }
  catch( GCSException& ex )
  {
    ex.addToTrace("DFAgent::sendElement_(const string&)");
    logError_ ("Error while sending element: %s", ex.toString());
    delete pDataElement;
    throw;
  }
}

//----------------------------------------------------------------------
// initDFBranchInstance
//----------------------------------------------------------------------
void DFAgent::initDFBranchInstance_()
{  
  try
  {
    if(CORBA::is_nil(const_cast<const DF::Branch_ifce_var&>(dfBranchInstance_)))
    { // Initializing pointer to data factory corba object
      logInfo_ ("Connecting to Data Factory Branch");
      trace_.out ("Connecting to Data Factory Branch");
      CORBARef<DF::Branch_ifce> df_ref(dfBranchInstanceName_, 0);
      df_ref.roundtripTimeout( TimeValue(60, 0) );
      dfBranchInstance_ = df_ref.typedRef();
      dfBranchInstance_->ping();
      logInfo_("Connected to DF Branch!");
      trace_.out ("Connected to DF Branch!");
      logInfo_("Opening session with default mode");
      setObsModeLowLevel (shortObsMode_);
    }
  }
  catch(const CORBA::Exception &ex)
  {
    GCSException gcsex = DException::mapCORBAException(const_cast<CORBA::Exception&>(ex));
    cout << "ERROR: " << gcsex << endl;
    if (gcsex.isA ("CorbaException"))
    {  //It is a timeout, set null the corba pointer
      dfBranchInstance_ = DF::Branch_ifce::_nil();
    }
    throw;
  }
  catch (GCSException& ex)
  {
    ex.addToTrace ("DFAgent::initDFBranchInstance_()");
    logError_ ("ERROR when connecting to DFBranch: %s", ex.toString());
    dfBranchInstance_ = DF::Branch_ifce::_nil();
    throw;
  }        
}

//----------------------------------------------------------------------
// initDFAgentPartnerInstance_
//----------------------------------------------------------------------
void DFAgent::initDFAgentPartnerInstance_()
{
  try
  {
    if((dfAgentPartnerInstanceName_.length() != 0) && CORBA::is_nil(const_cast<const DFAGENT::DFAgent_ifce_var&>(dfAgentPartnerInstance_)))

    {  //blank string means no agent partner supported for the specific agent
      // Initializing pointer to data factory corba object
      logInfo_ ("Connecting to DFAgent Partner");
      trace_.out ("Connecting to DFAgent Partner");
      CORBARef<DFAGENT::DFAgent_ifce> ap_ref(dfAgentPartnerInstanceName_, 0);
      ap_ref.roundtripTimeout( TimeValue(60, 0) );
      dfAgentPartnerInstance_ = ap_ref.typedRef();
      dfAgentPartnerInstance_->ping();
      logInfo_("Connected to DFAgent Partner!");
      trace_.out ("Connected to DFAgent Partner!");
    }
  }
  catch(const CORBA::Exception &ex)
  {
    GCSException gcsex = DException::mapCORBAException(const_cast<CORBA::Exception&>(ex));
    cout << "ERROR: " << gcsex << endl;
    if (gcsex.isA ("CorbaException"))
    {  //It is a timeout, set null the corba pointer
      dfAgentPartnerInstance_ = DFAGENT::DFAgent_ifce::_nil();
    }
    throw;
  }
  catch (GCSException& ex)
  {
    ex.addToTrace ("DFAgent::initDFAgentPartnerInstance_()");
    logError_ ("ERROR when connecting to DFAgentPartner: %s", ex.toString());
    dfAgentPartnerInstance_ = DFAGENT::DFAgent_ifce::_nil();
    throw;
  }   
}

//----------------------------------------------------------------------
// writeStandardKeywords_
//----------------------------------------------------------------------
void DFAgent::writeStandardKeywords_(DataDescription& frame)
{
  try
  {
    try
    {
      frame.addKeyword(DPKF::Keyword<string>("OBJECT", objectName_.c_str(), "Object name", ""));
    }
    catch(AlreadyExists &ex)
    {
      frame.modifyKeyword("OBJECT", objectName_);
    }
    try
    {
      frame.addKeyword(DPKF::Keyword<string>("INSMODE", instrumentMode_.c_str(), "Instrument Mode", ""));
    }
    catch(AlreadyExists &ex)
    {
      frame.modifyKeyword("INSMODE", instrumentMode_);
    }
    try
    {
      frame.addKeyword(DPKF::Keyword<string>("OBSTYPE", obsTypeName_[obsType_].c_str(), "Observation Type", ""));
    }
    catch(AlreadyExists &ex)
    {
      frame.modifyKeyword("OBSTYPE", obsTypeName_[obsType_]);
    }    
    try
    {
      frame.addKeyword(DPKF::Keyword<string>("PI", principalInvestigatorName_.c_str(), "Principal Investigator", ""));
    }
    catch(AlreadyExists &ex)
    {
      frame.modifyKeyword("PI", principalInvestigatorName_);
    }
    try
    {
      frame.addKeyword(DPKF::Keyword<string>("OBSERVER", observerName_.c_str(), "Observer Name (SA = support astronomer)", ""));
    }
    catch(AlreadyExists &ex)
    {
      frame.modifyKeyword("OBSERVER", observerName_);
    }
    try
    {
      frame.addKeyword(DPKF::Keyword<string>("GTCPRGID", programId_.c_str(), "Program Identifier", ""));
    }
    catch(AlreadyExists &ex)
    {
      frame.modifyKeyword("GTCPRGID", programId_);
    }
//   
    if(esoProgramId_ != "")
    {  
      try
      {
        frame.addKeyword(DPKF::Keyword<string>("ESOPRGID", esoProgramId_, "ESO Program Identifier", ""));
      }
      catch(AlreadyExists &ex)
      {
        frame.modifyKeyword("ESOPRGID", esoProgramId_);
      }    
    }
//    
    try
    {
      frame.addKeyword(DPKF::Keyword<string>("GTCOBID", observationBlockId_, "Observation Block ID", ""));
    }
    catch(AlreadyExists &ex)
    {
      frame.modifyKeyword("GTCOBID", observationBlockId_);
    }
    try
    {
      frame.addKeyword(DPKF::Keyword<string>("TELESCOP", "GTC", "Telescope id.", ""));
    }
    catch(AlreadyExists &ex)
    {
      frame.modifyKeyword<string>("TELESCOP", "GTC");
    }
    try
    {
      frame.addKeyword(DPKF::Keyword<string>("OBSMODE", obsModeName_[shortObsMode_], "Observation Mode", ""));
    }
    catch(AlreadyExists &ex)
    {
      frame.modifyKeyword("OBSMODE", obsModeName_[shortObsMode_]);
    }
    try
    {
      frame.addKeyword(DPKF::Keyword<string>("ORIGIN", "GRANTECAN", "FITS file originator", ""));
    }
    catch(AlreadyExists &ex)
    {
      frame.modifyKeyword<string>("ORIGIN", "GRANTECAN");
    }
    try
    {
      frame.addKeyword(DPKF::Keyword<string>("OBSERVAT", "ORM", "Name of observatory (IRAF style)", ""));
    }
    catch(AlreadyExists &ex)
    {
      frame.modifyKeyword<string>("OBSERVAT", "ORM");
    }
    try
    {
      frame.addKeyword(DPKF::Keyword<string>("LATITUDE", "+28:45:43.2", "Telescope latitude (degrees), +28:45:43.2", ""));      
    }
    catch(AlreadyExists &ex)
    {
      frame.modifyKeyword<string>("LATITUDE", "+28:45:43.2");
    }
    try
    {
      frame.addKeyword(DPKF::Keyword<string>("LONGITUD", "+17:52:39.5", "Telescope longitude (degrees), +17:52:39.5", ""));
    }
    catch(AlreadyExists &ex)
    {
      frame.modifyKeyword<string>("LONGITUD", "+17:52:39.5");
    }
    try
    {
      frame.addKeyword(DPKF::Keyword<long>("HEIGHT", 2348, "Telescope height above sea level (m)", ""));
    }
    catch(AlreadyExists &ex)
    {
      frame.modifyKeyword("HEIGHT", (long)2348);
    }
  }
  catch( GCSException& ex )
  {
    cerr << "ERROR while writing the standar Keywords" << endl;
    ex.addToTrace("writeStandardKeywords_(Frame<double,2>)");      
    cerr << ex << endl;
  }  
}

//----------------------------------------------------------------------
// writeWCSKeywords_
//----------------------------------------------------------------------
void DFAgent::writeWCSKeywords_(DataDescription& frame, TinyVector<unsigned long, 2>& frameSize)
{
  // Implemented if needed by inherited class
}

//----------------------------------------------------------------------
// writeAdditionalKeywords_
//----------------------------------------------------------------------
void DFAgent::writeAdditionalKeywords_(DataDescription& frame)
{
  trace_.out("writeAdditionalKeywords_()\n");
  // Implemented if needed by inherited class
}

//----------------------------------------------------------------------
// applyObsModeSettings_
//----------------------------------------------------------------------
void DFAgent::applyObsModeSettings_(const std::vector<bool> & settings)
{  // this method is virtual, so it can be redefined in the derived class
   // the behaviour can be different from this,
   // different settings order, or different settings magnitudes for instance
   // default behaviour is the following:
   // saveFrame / saveFits / propagateFrame / propagateFits / autoReduce / writeKeywords
   if(settings.size() != 6)
   {
     throw InvalidValue("applyObsModeSettings_(vector<bool>)", "Wrong number of settings");
   }
   doesSaveFrame_ = settings[0];
   doesSaveFITS_ = settings[1];
   doesPropagateFrame_ = settings[2];
   doesPropagateFITS_ = settings[3];
   doesAutoReduce_ = settings[4];
   doesWriteKeywords_ = settings[5];
   
   magnitudeChange_("doesSaveFrame", doesSaveFrame_);
   propertyChange_("saveFrame");
   magnitudeChange_("doesSaveFITS", doesSaveFITS_);
   propertyChange_("saveFITS");
   magnitudeChange_("doesPropagateFrame", doesPropagateFrame_);
   propertyChange_("propagateFrame");
   magnitudeChange_("doesPropagateFITS", doesPropagateFITS_);
   propertyChange_("propagateFITS");
   magnitudeChange_("doesAutoReduce", doesAutoReduce_);
   propertyChange_("autoReduce");
   magnitudeChange_("doesWriteKeywords", doesWriteKeywords_);
   propertyChange_("writeKeywords");
}

//----------------------------------------------------------------------
// receiveFrame 
//----------------------------------------------------------------------
void DFAgent::receiveFrame( const DDPKF::Frame& idlNoKeywordsFrame, int currentWindow, 
                            int numWindows, const DGT::TinyVector& idlTopLeft, 
                            const DGT::TinyVector& idlBottomRight, const DGT::TimeValue& idlTimestamp, 
                            const DGT::TimeValue& idlExposureTime, const char* cameraId )
{
  trace_.out("receiveFrame()\n");
  
  //check dfBranchInstance_, aren't null, otherwise reconnect to them.
  initDFBranchInstance_();
             
  try
  {
    //copy arguments into member variables
    currentWindow_ = currentWindow;
    numWindows_ = numWindows;
    idlTopLeft_ = idlTopLeft;
    idlBottomRight_ = idlBottomRight;
    timestamp_ = idlTimestamp;
    exposureTime_ = idlExposureTime;
    cameraId_ = string(cameraId);

    //extract the header
    DataDescription header;
    fromIDL(idlNoKeywordsFrame.dataDesc, header);
    trace_.out ("Gathering monitor keywords values...");
    TimeValue expStartTime ( fromIDL(idlTimestamp) );

    obsModeKeywordList_[shortObsMode_].gatherMonitorKeywords(expStartTime, fromIDL(idlExposureTime), monitorAgent_, header);

    writeStandardKeywords_(header);
    TinyVector<unsigned long, 2> sizeF;
    if(idlNoKeywordsFrame.dataArray.size_of_dimensions.N == 2)
    {  //frame data array has more than two dimensions don't even try to call writeWCSKeywords_
      fromIDL (idlNoKeywordsFrame.dataArray.size_of_dimensions, sizeF);
      writeWCSKeywords_(header, sizeF);
    }
    writeAdditionalKeywords_(header);
   
    //####Temporal fix solution#######
    try
    {
      header.addKeyword(DPKF::Keyword<double>("EXPTIME", (fromIDL(idlExposureTime)).toFloat(), "", ""));
    }
    catch(AlreadyExists &ex)
    {
      header.modifyKeyword("EXPTIME", (fromIDL(idlExposureTime)).toFloat());
    }
    catch( GCSException& ex )
    {
      cerr << "ERROR while writing the EXPTIME keyword" << endl;
      ex.addToTrace("DFAgent::receiveFrame(Frame<double,2>)");      
      cerr << ex << endl;
    }
    //################################

    //construct an idl frame with the new header
    
    DDPKF::Frame idlFrame;    
    toIDL(header, idlFrame.dataDesc);
    idlFrame.errorFlag = idlNoKeywordsFrame.errorFlag;
    idlFrame.dataArray.size_of_dimensions = idlNoKeywordsFrame.dataArray.size_of_dimensions;
    idlFrame.dataArray.memoryRep = idlNoKeywordsFrame.dataArray.memoryRep;
    idlFrame.dataArray.data.replace (idlNoKeywordsFrame.dataArray.data.maximum(), 
                                     idlNoKeywordsFrame.dataArray.data.length(),
                                     const_cast<CORBA::Octet*>(idlNoKeywordsFrame.dataArray.data.get_buffer()),
                                     false);
    idlFrame.errorArray.size_of_dimensions = idlNoKeywordsFrame.errorArray.size_of_dimensions;
    idlFrame.errorArray.memoryRep = idlNoKeywordsFrame.errorArray.memoryRep;
    idlFrame.errorArray.data.replace (idlNoKeywordsFrame.errorArray.data.maximum(), 
                                     idlNoKeywordsFrame.errorArray.data.length(),
                                     const_cast<CORBA::Octet*>(idlNoKeywordsFrame.errorArray.data.get_buffer()),
                                     false);
    idlFrame.extensions.replace     (idlNoKeywordsFrame.extensions.maximum(), 
                                     idlNoKeywordsFrame.extensions.length(),
                                     const_cast<DDPKF::Frame&>(idlNoKeywordsFrame).extensions.get_buffer(0),
                                     false);    

    
    
    if (!CORBA::is_nil(const_cast<const DF::Branch_ifce_var&>(dfBranchInstance_)))
    {
      vector<string>::iterator it, itBegin = recipeElementNames_.begin(), itEnd = recipeElementNames_.end();
      for(it = itBegin; it != itEnd; it++)
      {//hasFrameArrived = true
        sendElement_(*it, true);
      }
      
      if(multiFrameRecipe_)
      {
        //add frame item to the list
        dfBranchInstance_->addORParamItemAcquiredFrame( recipeDictionary_.fetchTranslation("rawFrame", obsModeName_[shortObsMode_]).c_str(),
                                                        idlFrame, currentWindow, numWindows, idlTopLeft, 
                                                        idlBottomRight, idlTimestamp, idlExposureTime, doesPropagateFrame_, 
                                                        doesSaveFrame_, dfBranchInstanceName_, dfSessionId_ );
      }
      else
      {  //receive frme just received
        dfBranchInstance_->receiveORParamAcquiredFrame( recipeDictionary_.fetchTranslation("rawFrame", obsModeName_[shortObsMode_]).c_str(),
                                                        idlFrame, currentWindow, numWindows, idlTopLeft, 
                                                        idlBottomRight, idlTimestamp, idlExposureTime, doesPropagateFrame_, 
                                                        doesSaveFrame_, dfBranchInstanceName_, dfSessionId_ );
      }
    }    
  }
  catch (const CORBA::Exception &ex)
  {  
     GCSException gcsex = DException::mapCORBAException(const_cast<CORBA::Exception&>(ex));
     gcsex.addToTrace("DFAgent::receiveFrame()");  
     logError_ ("Error when receiving frame: %s", gcsex.toString());
     if (gcsex.isA ("CorbaException"))
     {  //It is a timeout, was a communications error
       dfBranchInstance_ = DF::Branch_ifce::_nil();
     }  
     goIdle_();
     throw;
  }
  catch( GCSException& ex )
  {
     ex.addToTrace("DFAgent::receiveFrame()");
     logError_ ("Error while receiveing raw frame: %s", ex.toString());
     goIdle_();
     throw;
  }    
}

//----------------------------------------------------------------------
// setObservationType
//----------------------------------------------------------------------
void DFAgent::setObservationType(ObservationType obsType)
{
  trace_.out("setObservationType(%s)\n", obsTypeName_[obsType].c_str());
  //## check if input parameters are valid here
  lock_();
//  goRunning_();
  
  obsType_ = obsType;
  
//  goIdle_();
  unlock_();
}

//----------------------------------------------------------------------
// setObservationClass
//----------------------------------------------------------------------
void DFAgent::setObservationClass(ObservationClass obsClass)
{
  trace_.out("setObservationClass(%s)\n", obsClassName_[obsClass].c_str());
  //## check if input parameters are valid here
  lock_();
//  goRunning_();
  
  obsClass_ = obsClass;
  
//  goIdle_();
  unlock_();
}


//----------------------------------------------------------------------
// setInstrumentMode
//----------------------------------------------------------------------
void DFAgent::setInstrumentMode(const char* insMode)
{
  trace_.out("setInstrumentMode(%s)\n",insMode);
  //## check if input parameters are valid here
  lock_();
  goRunning_();
  
  instrumentMode_ = insMode;
  
  goIdle_();
  unlock_();
}

//----------------------------------------------------------------------
// setPrincipalInvestigatorName 
//----------------------------------------------------------------------
void DFAgent::setPrincipalInvestigatorName(const char* piName)
{
  trace_.out("setPrincipalInvestigatorName(%s)\n",piName);
  //## check if input parameters are valid here
  lock_();
  goRunning_();
  
  principalInvestigatorName_ = piName;
  
  goIdle_();
  unlock_();
}

//----------------------------------------------------------------------
// setObserverName 
//----------------------------------------------------------------------
void DFAgent::setObserverName(const char* observerName)
{
  trace_.out("setObserverName(%s)\n",observerName);
  //## check if input parameters are valid here
  lock_();
  goRunning_();
  
  observerName_ = observerName;
  
  goIdle_();
  unlock_();
}

//----------------------------------------------------------------------
// setObjectName 
//----------------------------------------------------------------------
void DFAgent::setObjectName(const char* objectName)
{
  trace_.out("setObjectName(%s)\n",objectName);
  //## check if input parameters are valid here
  lock_();
  goRunning_();
  
  objectName_ = objectName;
  
  goIdle_();
  unlock_();
}

//----------------------------------------------------------------------
// getObjectName 
//----------------------------------------------------------------------
string DFAgent::getObjectName()
{
  return objectName_;
}

//----------------------------------------------------------------------
// setProgramId 
//----------------------------------------------------------------------
void DFAgent::setProgramId(const char* programId)
{
  trace_.out("setProgramId(%s)\n",programId);
  //## check if input parameters are valid here
  lock_();
  goRunning_();
  
  programId_ = programId;
  
  goIdle_();
  unlock_();
}

//----------------------------------------------------------------------
// setESOProgramId 
//----------------------------------------------------------------------
void DFAgent::setESOProgramId(const char* esoProgramId)
{
  trace_.out("setESOProgramId(%s)\n",esoProgramId);
  //## check if input parameters are valid here
  lock_();
  goRunning_();
  
  esoProgramId_ = esoProgramId;
  
  goIdle_();
  unlock_();
}

//----------------------------------------------------------------------
// setObservationBlockId 
//----------------------------------------------------------------------
void DFAgent::setObservationBlockId(const char* obId)
{
  trace_.out("setObservationBlockId(%s)\n",obId);
  //## check if input parameters are valid here
  lock_();
  goRunning_();
  
  observationBlockId_ = obId;
  
  goIdle_();
  unlock_();
}

//----------------------------------------------------------------------
// setImageTitle 
//----------------------------------------------------------------------
void DFAgent::setImageTitle(const char* imageTitle)
{
  trace_.out("setImageTitle(%s)\n",imageTitle);
  //## check if input parameters are valid here
  lock_();
  goRunning_();
  
  imageTitle_ = imageTitle;
  
  goIdle_();
  unlock_();
}

//----------------------------------------------------------------------
// setObservationComment 
//----------------------------------------------------------------------
void DFAgent::setObservationComment(const char* observationComment)
{
  trace_.out("setObservationComment(%s)\n",observationComment);
  //## check if input parameters are valid here
  lock_();
  goRunning_();
  
  observationComment_ = observationComment;
  
  goIdle_();
  unlock_();
}

//----------------------------------------------------------------------
// fetchORInfo_
//----------------------------------------------------------------------
void DFAgent::fetchORInfo_()
{
  try
  {
    //check dfBranchInstance_ isn't null, otherwise reconnect to it.
    initDFBranchInstance_();
    
    //Get info about Obs Result elements
    DF::DataPackInfo_var dataInfoIDL = 
    dfBranchInstance_->getObsResultInfo(dfBranchInstanceName_, dfSessionId_);
    DataPackInfo dataInfo;
    fromIDL(dataInfoIDL.in(), dataInfo);
    recipeElementNames_ = dataInfo.elementsName();
  
    string strRawFrame(recipeDictionary_.fetchTranslation("rawFrame", obsModeName_[shortObsMode_]) );
    for(unsigned int c=0; c < recipeElementNames_.size(); c++)
    {
      if(strRawFrame == recipeElementNames_[c])
      {
        multiFrameRecipe_ = ( dataInfo.elementType(c+1) == DF::TYPE_FRAME_LIST );
        break;
      }
    }
  }
  catch(const CORBA::Exception &ex)
  {
     GCSException gcsex = DException::mapCORBAException(const_cast<CORBA::Exception&>(ex));
     if (gcsex.isA ("CorbaException"))
     {  //It is a timeout, was a communications error
       dfBranchInstance_ = DF::Branch_ifce::_nil();
     }
     throw gcsex;
  }
  catch( GCSException& ex )
  {
    ex.addToTrace("DFAgent::fetchORInfo_()");
    throw;
  }
}

//----------------------------------------------------------------------
// setObsModeLowLevel 
//----------------------------------------------------------------------
void DFAgent::setObsModeLowLevel(short obsMode)
{
  //@ipc
  try
  {
    //check dfBranchInstance_ isn't null, otherwise reconnect to it.
    initDFBranchInstance_();
    
    //Checked already that the same obs mode isn't running
    if(shortObsMode_ != obsMode)
    {
      applyObsModeSettings_( obsModeSettings_[obsMode] );
    }  
    shortObsMode_ = obsMode;
    if(isSessionRunning_)
    { // Agent can manage only one session at a time, if it exists, it must be closed before opening a new one
      try
      {
         dfBranchInstance_->closeObsMode(dfSessionId_);
      }
      catch (...) {}	 
    }
    //Subscribe to monitor needed for this obsMode, using .spec keywords. Agent will read from them shortly
    dfSessionId_ = dfBranchInstance_->configureObsMode( obsModeName_[shortObsMode_].c_str() );
    dfBranchInstance_->setInstrumentId (gcsComponent_, cameraName_, dfBranchInstanceName_, dfSessionId_);
    dfBranchInstance_->setSessionAutoReduce(doesAutoReduce_, dfBranchInstanceName_, dfSessionId_);
    isSessionRunning_ = true;
    fetchORInfo_();
 
    //send early elements, the ones which can be sent while waiting for the rawFrame
    vector<string>::iterator it, itBegin = recipeElementNames_.begin(), itEnd = recipeElementNames_.end();
    for(it = itBegin; it != itEnd; it++)
    {  //hasFrameArrived = false
      sendElement_(*it, false);
    }    
    
//    for_each(recipeElementNames_.begin(),recipeElementNames_.end(),bind2nd( mem_fun( &DFAgent::sendElement_ ), this ));

  }
  catch(const CORBA::Exception &ex)
  {
     GCSException gcsex = DException::mapCORBAException(const_cast<CORBA::Exception&>(ex));
     if (gcsex.isA ("CorbaException"))
     {  //It is a timeout, was a communications error
       dfBranchInstance_ = DF::Branch_ifce::_nil();
     }
     logError_ ("Error reading initializing ObsMode: %s", gcsex.toString());
     throw gcsex;
  }
  catch( GCSException& ex )
  {
    ex.addToTrace("DFAgent::setObsModeLowLevel(short)");
    logError_ ("Error reading initializing ObsMode: %s", ex.toString());
    throw;
  }
  //@epc
}

//----------------------------------------------------------------------
// setPropagateFrame 
//----------------------------------------------------------------------
void DFAgent::setPropagateFrame(bool doesPropagate)
{
  trace_.out("setPropagateFrame(%hd)\n",doesPropagate);
  //## check if input parameters are valid here
  lock_();
  goRunning_();
  
  doesPropagateFrame_ = doesPropagate;
  propertyChange_("propagateFrame");  
  magnitudeChange_("doesPropagateFrame",doesPropagate);
  
  goIdle_();
  unlock_();
}

//----------------------------------------------------------------------
// setSaveFrame 
//----------------------------------------------------------------------
void DFAgent::setSaveFrame(bool doesSave)
{
  trace_.out("setSaveFrame(%hd)\n",doesSave);
  //## check if input parameters are valid here
  lock_();
  goRunning_();
  
  doesSaveFrame_ = doesSave;
  propertyChange_("saveFrame");    
  magnitudeChange_("doesSaveFrame",doesSave);
  
  goIdle_();
  unlock_();
}

//----------------------------------------------------------------------
// setPropagateFITS 
//----------------------------------------------------------------------
void DFAgent::setPropagateFITS(bool doesPropagate)
{
  trace_.out("setPropagateFITS(%hd)\n",doesPropagate);
  //## check if input parameters are valid here
  lock_();
  goRunning_();
  
  doesPropagateFITS_ = doesPropagate;
  propertyChange_("propagateFITS");  
  magnitudeChange_("doesPropagateFITS", doesPropagate);
  
  goIdle_();
  unlock_();
}

//----------------------------------------------------------------------
// setSaveFITS 
//----------------------------------------------------------------------
void DFAgent::setSaveFITS(bool doesSave)
{
  trace_.out("setSaveFITS(%hd)\n",doesSave);
  //## check if input parameters are valid here
  lock_();
  goRunning_();
  
  doesSaveFITS_ = doesSave;
  propertyChange_("saveFITS");    
  magnitudeChange_("doesSaveFITS", doesSave);
  
  goIdle_();
  unlock_();
}

//----------------------------------------------------------------------
// setAutoReduce
//----------------------------------------------------------------------
void DFAgent::setAutoReduce(bool doesAuto)
{
  trace_.out("setAutoReduce(%hd)\n",doesAuto);
  //## check if input parameters are valid here
  lock_();
  goRunning_();
  
  doesAutoReduce_ = doesAuto;
  propertyChange_("autoReduce");    
  magnitudeChange_("doesAutoReduce", doesAuto);
  
  goIdle_();
  unlock_();
}

//----------------------------------------------------------------------
// setWriteKeywords
//----------------------------------------------------------------------
void DFAgent::setWriteKeywords(bool doesWrite)
{
  trace_.out("setWriteKeywords(%hd)\n",doesWrite);
  //## check if input parameters are valid here
  lock_();
  goRunning_();
  
  doesWriteKeywords_ = doesWrite;
  propertyChange_("writeKeywords");    
  magnitudeChange_("doesWriteKeywords", doesWrite);
  
  goIdle_();
  unlock_();
}

//----------------------------------------------------------------------
// createArchiveFits 
//----------------------------------------------------------------------
void DFAgent::createArchiveFits()
{
  trace_.out("createArchiveFits()\n");
  //## check if input parameters are valid here
  lock_();
  goRunning_();
  
  //## Add code as required
  
  goIdle_();
  unlock_();
}

//----------------------------------------------------------------------
// importFitsFile 
//----------------------------------------------------------------------
void DFAgent::importFitsFile()
{
  trace_.out("importFitsFile()\n");
  //## check if input parameters are valid here
  lock_();
  goRunning_();
  
  //## Add code as required
  
  goIdle_();
  unlock_();
}

//----------------------------------------------------------------------
// commitArchiveFits 
//----------------------------------------------------------------------
void DFAgent::commitArchiveFits()
{
  trace_.out("commitArchiveFits()\n");
  //## check if input parameters are valid here
  lock_();
  goRunning_();
  
  //## Add code as required
  
  goIdle_();
  unlock_();
}

//----------------------------------------------------------------------
// setReductionRecipe 
//----------------------------------------------------------------------
void DFAgent::setReductionRecipe()
{
  trace_.out("setReductionRecipe()\n");
  //## check if input parameters are valid here
  lock_();
  goRunning_();
  
  //## Add code as required
  
  goIdle_();
  unlock_();
}

//----------------------------------------------------------------------
// commitOR 
//----------------------------------------------------------------------
void DFAgent::commitOR()
{
  trace_.out("commitOR()\n");
  //## check if input parameters are valid here
  lock_();
  goRunning_();    
  
  //## Add code as required
    
  goIdle_();
  unlock_();
}

/************************************************************************
*************************************************************************
**                                                                     **
**           Monitorable attributes (device magnitudes)                **
**                                                                     **
*************************************************************************
*************************************************************************
* All monitor methods:                                                  *
* - Can be invoked in any state of the Device                           *
* - Do not modify the state of the Device                               *
* - It is required to check the ranges of the monitored values          *
* - If an alarm is detected use sendAlarm_()                            *
* - Shall be as fast as possible (due to high monitoring rate)          *
* - Acces to a DeviceDriver or aggregated Device can be required        *
* - In bool & enum monitors use magnitudeChange_() to inform of change  *
************************************************************************/
  
//----------------------------------------------------------------------
// Monitor doesSaveFrame
//----------------------------------------------------------------------
bool DFAgent::doesSaveFrame()
{
  return doesSaveFrame_;
}

//----------------------------------------------------------------------
// Monitor doesPropagateFrame
//----------------------------------------------------------------------
bool DFAgent::doesPropagateFrame()
{
  return doesPropagateFrame_;
}

//----------------------------------------------------------------------
// Monitor doesSaveFITS
//----------------------------------------------------------------------
bool DFAgent::doesSaveFITS()
{
  return doesSaveFITS_;
}

//----------------------------------------------------------------------
// Monitor doesPropagateFITS
//----------------------------------------------------------------------
bool DFAgent::doesPropagateFITS()
{
  return doesPropagateFITS_;
}

//----------------------------------------------------------------------
// Monitor doesAutoReduce
//----------------------------------------------------------------------
bool DFAgent::doesAutoReduce()
{
  return doesAutoReduce_;
}

//----------------------------------------------------------------------
// Monitor doesWriteKeywords
//----------------------------------------------------------------------
bool DFAgent::doesWriteKeywords()
{
  return doesWriteKeywords_;
}


/************************************************************************
*************************************************************************
**                                                                     **
**          Configurable attributes (device properties)                **
**                                                                     **
*************************************************************************
*************************************************************************
* All property methods:                                                 *
* - Used ONLY in STARTING,ON,IDLE,FAULT. Use checkConfigurability_()    *
* - Do not modify the state of the Device                               *
* - Both get/set methods shall be provided for each property            *
* - Properties shall have a default value initialized at startup        *
* - It is required to check the ranges before to apply properties       *
* - If an error is detected: use logError_() & throw GCSException       *
* - Inform that a property is changed: use propertyChange_()            *
************************************************************************/
  
//----------------------------------------------------------------------
// Get offlineImagesPath property
//----------------------------------------------------------------------
const char* DFAgent::offlineImagesPath()
{
  trace_.out("offlineImagesPath()\n");
  return offlineImagesPath_.in();
}

//----------------------------------------------------------------------
// Set offlineImagesPath property
//----------------------------------------------------------------------
void DFAgent::offlineImagesPath(const char* new_offlineImagesPath)
{
  trace_.out("offlineImagesPath(%s)\n",new_offlineImagesPath);
  // Change property
  offlineImagesPath_ = new_offlineImagesPath;
  
  propertyChange_("offlineImagesPath");
}

//----------------------------------------------------------------------
// Get dfBranchInstanceName property
//----------------------------------------------------------------------
const char* DFAgent::dfBranchInstanceName()
{
  trace_.out("dfBranchInstanceName()\n");
  return dfBranchInstanceName_.in();
}

//----------------------------------------------------------------------
// Set dfBranchInstanceName property
//----------------------------------------------------------------------
void DFAgent::dfBranchInstanceName(const char* new_dfBranchInstanceName)
{
  trace_.out("dfBranchInstanceName(%s)\n",new_dfBranchInstanceName);
  // Change property
  dfBranchInstanceName_ = new_dfBranchInstanceName;
  if(isIdle())
  {
    dfBranchInstance_ = DF::Branch_ifce::_nil();
    initDFBranchInstance_();
  }

  propertyChange_("dfBranchInstanceName");
}

//----------------------------------------------------------------------
// Get dfAgentPartnerInstanceName property
//----------------------------------------------------------------------
const char* DFAgent::dfAgentPartnerInstanceName()
{
  trace_.out("dfAgentPartnerInstanceName()\n");
  return dfAgentPartnerInstanceName_.in();
}

//----------------------------------------------------------------------
// Set dfAgentPartnerInstanceName property
//----------------------------------------------------------------------
void DFAgent::dfAgentPartnerInstanceName(const char* new_dfAgentPartnerInstanceName)
{
  trace_.out("dfAgentPartnerInstanceName(%s)\n",new_dfAgentPartnerInstanceName);
  // Change property
  dfAgentPartnerInstanceName_ = new_dfAgentPartnerInstanceName;
  if(isIdle())
  {
    dfAgentPartnerInstance_ = DFAGENT::DFAgent_ifce::_nil();
    initDFAgentPartnerInstance_();
  }

  propertyChange_("dfAgentPartnerInstanceName");
}

//----------------------------------------------------------------------
// Get propagateFrame property
//----------------------------------------------------------------------
short DFAgent::propagateFrame()
{
  trace_.out("propagateFrame()\n");
  return doesPropagateFrame_;
}

//----------------------------------------------------------------------
// Set propagateFrame property
//----------------------------------------------------------------------
void DFAgent::propagateFrame(short new_propagateFrame)
{
  trace_.out("propagateFrame(%hd)\n",new_propagateFrame);
  // Change property
  doesPropagateFrame_ = new_propagateFrame;
 
  magnitudeChange_("doesPropagateFrame",new_propagateFrame); 
  propertyChange_("propagateFrame");
}

//----------------------------------------------------------------------
// Get saveFrame property
//----------------------------------------------------------------------
short DFAgent::saveFrame()
{
  trace_.out("saveFrame()\n");
  return doesSaveFrame_;
}

//----------------------------------------------------------------------
// Set saveFrame property
//----------------------------------------------------------------------
void DFAgent::saveFrame(short new_saveFrame)
{
  trace_.out("saveFrame(%hd)\n",new_saveFrame);
  // Change property
  doesSaveFrame_ = new_saveFrame;
  
  magnitudeChange_("doesSaveFrame",new_saveFrame); 
  propertyChange_("saveFrame");
}

//----------------------------------------------------------------------
// Get propagateFITS property
//----------------------------------------------------------------------
short DFAgent::propagateFITS()
{
  trace_.out("propagateFITS()\n");
  return doesPropagateFITS_;
}

//----------------------------------------------------------------------
// Set propagateFITS property
//----------------------------------------------------------------------
void DFAgent::propagateFITS(short new_propagateFITS)
{
  trace_.out("propagateFITS(%hd)\n",new_propagateFITS);
  // Change property
  doesPropagateFITS_ = new_propagateFITS;
  
  magnitudeChange_("doesPropagateFITS",new_propagateFITS);
  propertyChange_("propagateFITS");
}

//----------------------------------------------------------------------
// Get saveFITS property
//----------------------------------------------------------------------
short DFAgent::saveFITS()
{
  trace_.out("saveFITS()\n");
  return doesSaveFITS_;
}

//----------------------------------------------------------------------
// Set saveFITS property
//----------------------------------------------------------------------
void DFAgent::saveFITS(short new_saveFITS)
{
  trace_.out("saveFITS(%hd)\n",new_saveFITS);
  // Change property
  doesSaveFITS_ = new_saveFITS;
  
  magnitudeChange_("doesSaveFITS",new_saveFITS);
  propertyChange_("saveFITS");
}

//----------------------------------------------------------------------
// Get autoReduce property
//----------------------------------------------------------------------
short DFAgent::autoReduce()
{
  trace_.out("autoReduce()\n");
  return doesAutoReduce_;
}

//----------------------------------------------------------------------
// Set autoReduce property
//----------------------------------------------------------------------
void DFAgent::autoReduce(short new_autoReduce)
{
  trace_.out("autoReduce(%hd)\n",new_autoReduce);
  // Change property
  doesAutoReduce_ = new_autoReduce;
  
  magnitudeChange_("doesAutoReduce",new_autoReduce);
  propertyChange_("autoReduce");
}

//----------------------------------------------------------------------
// Get writeKeywords property
//----------------------------------------------------------------------
short DFAgent::writeKeywords()
{
  trace_.out("writeKeywords()\n");
  return doesWriteKeywords_;
}

//----------------------------------------------------------------------
// Set writeKeywords property
//----------------------------------------------------------------------
void DFAgent::writeKeywords(short new_writeKeywords)
{
  trace_.out("writeKeywords(%hd)\n",new_writeKeywords);
  // Change property
  doesWriteKeywords_ = new_writeKeywords;
  
  magnitudeChange_("doesWriteKeywords",new_writeKeywords);
  propertyChange_("writeKeywords");
}

//----------------------------------------------------------------------
// Get flippedImage property
//----------------------------------------------------------------------
short DFAgent::flippedImage()
{
  trace_.out("flippedImage()\n");
  return flippedImage_;
}

//----------------------------------------------------------------------
// Set flippedImage property
//----------------------------------------------------------------------
void DFAgent::flippedImage(short new_flippedImage)
{
  trace_.out("flippedImage(%hd)\n",new_flippedImage);
  // Change property
  flippedImage_ = new_flippedImage;
  
  propertyChange_("flippedImage");
}

//----------------------------------------------------------------------
// Get testingMode property
//----------------------------------------------------------------------
short DFAgent::testingMode()
{
  trace_.out("testingMode()\n");
  return testingMode_;
}

//----------------------------------------------------------------------
// Set testingMode property
//----------------------------------------------------------------------
void DFAgent::testingMode(short new_testingMode)
{
  trace_.out("testingMode(%hd)\n",new_testingMode);
  // Change property
  testingMode_ = new_testingMode;
  
  propertyChange_("testingMode");
}

//----------------------------------------------------------------------
// Get obsModeKeywordListPath property
//----------------------------------------------------------------------
const char* DFAgent::obsModeKeywordListPath()
{
  trace_.out("obsModeKeywordListPath()\n");
  return obsModeKeywordListPath_.in();
}

//----------------------------------------------------------------------
// Set obsModeKeywordListPath property
//----------------------------------------------------------------------
void DFAgent::obsModeKeywordListPath(const char* new_obsModeKeywordListPath)
{
  trace_.out("obsModeKeywordListPath(%s)\n",new_obsModeKeywordListPath);
  // Change property
  obsModeKeywordListPath_ = new_obsModeKeywordListPath;
  //@ipc
  //load fits header keywords for each Obs Mode
  std::string fitsHeaderPath;
  std::ifstream inputFile(obsModeKeywordListPath_.c_str());
  
  if(inputFile)
  {
    while(inputFile >> fitsHeaderPath)
    {
      trace_.out ("Parsing keyword specification...");
      //header keyword files are read in the same order than ObservationMode enumeration
      FITSHeaderInfo fitsHeaderInfo;
      fitsHeaderInfo.parseKeywordsFile(fitsHeaderPath.c_str());
      obsModeKeywordList_.push_back(fitsHeaderInfo);
    }
    if (obsModeKeywordList_.size() == 0)
    {
      throw InvalidValue("obsModeKeywordListPath","No keywords configuration found.");
    }
  }
  else
  {
    throw OperationFailed("DFAgent::obsModeKeywordListPath(const char*)",
                          ("Missing fileName: " + obsModeKeywordListPath_).c_str());
  }
  //@epc  
  propertyChange_("obsModeKeywordListPath");
}

//----------------------------------------------------------------------
// Get recipeDictionaryPath property
//----------------------------------------------------------------------
const char* DFAgent::recipeDictionaryPath()
{
  trace_.out("recipeDictionaryPath()\n");
  return recipeDictionaryPath_.in();
}

//----------------------------------------------------------------------
// Set recipeDictionaryPath property
//----------------------------------------------------------------------
void DFAgent::recipeDictionaryPath(const char* new_recipeDictionaryPath)
{
  trace_.out("recipeDictionaryPath(%s)\n",new_recipeDictionaryPath);
  recipeDictionaryPath_ = new_recipeDictionaryPath;
  //@ipc
  try
  {  
    recipeDictionary_.loadFromFile(recipeDictionaryPath_.c_str() );
  }
  catch( GCSException& ex )
  {
    ex.addToTrace("DFAgent::recipeDictionaryPath(const char*)");
    throw;
  }
  //@epc
  
  propertyChange_("recipeDictionaryPath");
}

//----------------------------------------------------------------------
// Get gcsComponent property
//----------------------------------------------------------------------
const char* DFAgent::gcsComponent()
{
  trace_.out("gcsComponent()\n");
  return gcsComponent_.in();
}

//----------------------------------------------------------------------
// Set gcsComponent property
//----------------------------------------------------------------------
void DFAgent::gcsComponent(const char* new_gcsComponent)
{
  trace_.out("gcsComponent(%s)\n",new_gcsComponent);
  
  // checkConfigurability_();
  // ## check new values before to apply the property
  
  // Change property
  gcsComponent_ = new_gcsComponent;
  
  propertyChange_("gcsComponent");
}

//----------------------------------------------------------------------
// Get cameraName property
//----------------------------------------------------------------------
const char* DFAgent::cameraName()
{
  trace_.out("cameraName()\n");
  return cameraName_.in();
}

//----------------------------------------------------------------------
// Set cameraName property
//----------------------------------------------------------------------
void DFAgent::cameraName(const char* new_cameraName)
{
  trace_.out("cameraName(%s)\n",new_cameraName);
  
  // checkConfigurability_();
  // ## check new values before to apply the property
  
  // Change property
  cameraName_ = new_cameraName;
  
  propertyChange_("cameraName");
}

//----------------------------------------------------------------------
// Get referencePixel property
//----------------------------------------------------------------------
const DoubleArray1D DFAgent::referencePixel()
{
  trace_.out("referencePixel()\n");
  DoubleArray1D doubleArray1D(referencePixel_.size());
  for(unsigned int w=0; w<referencePixel_.size(); w++) 
  { 
    doubleArray1D[w] = referencePixel_[w];
  }

  return doubleArray1D;
}

//----------------------------------------------------------------------
// Set referencePixel property
//----------------------------------------------------------------------
void DFAgent::referencePixel(const DoubleArray1D& new_referencePixel)
{
  trace_.out("referencePixel(double[%ld])\n",new_referencePixel.width());
  
  // Check if is a valid array
  if (new_referencePixel.width()!=referencePixel_.size())
  {
    logError_("Array property is out of range");
    throw InvalidValue(name_.c_str(),"Index out of range in array");
  }
  
  // Change property
  for(unsigned int w=0; w<new_referencePixel.width(); w++)
  {
    referencePixel_[w] = new_referencePixel[w];
  }
  
  propertyChange_("referencePixel");
}

