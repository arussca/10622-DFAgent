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
#include <sstream>                         // std::stringstream
#include <algorithm>                       // std::find
#include <IDL_Adapters_RF.h>
#include <IDL_Adapters_DPKF.h>
#include <DataPackInfo.h>
#include <CORBARef.h>                      // CORBARef
#include <sys/types.h>
#include <sys/stat.h>
#include <PropertyParser.h>               // SyntaxError
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
	//<editor-fold desc="#ToDo: This section could be removed. See [[deprecated]] comments next to members definition on header.">
	//initialize map with ObservationType and string names..

	obsTypeName_[BIAS] = "BIAS";
	obsTypeName_[DARK] = "DARK";
	obsTypeName_[SKY_FLAT] = "SKY_FLAT";
	obsTypeName_[DOME_FLAT] = "DOME_FLAT";
	obsTypeName_[SPECTRAL_FLAT] = "SPECTRAL_FLAT";
	obsTypeName_[ARC] = "ARC";
	obsTypeName_[SKY] = "SKY";
	obsTypeName_[OBJECTS] = "OBJECT";
	//</editor-fold>

	//Initialize ObsClass variable to default value
	obsClass_ = SCIENCE;
	//<editor-fold desc="#ToDo: This section could be removed. See [[deprecated]] comments next to members definition on header.">
	//initialize map with ObservationClass and string names..
	obsClassName_[SCIENCE] = "Science";
	obsClassName_[CALIB] = "Calib";
	//</editor-fold>

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
  	obsModeKeywordList_.clear();
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
// receiveRecipeResult
//----------------------------------------------------------------------
void DFAgent::receiveRecipeResult(std::string observingMode,
		unsigned long id,
		unsigned long obsResultId,
		DF::RecipeQualityControlType qc,
		const DDPKF::RecipeResult& result)
{

	ACE_Guard<ACE_Thread_Mutex> guard(dfagentGuardMutex_);

	trace_.out("receiveRecipeResult [obs_mode=%s] [id=%lu] [obs_result_id=%lu]\n",
			observingMode.c_str() ,
			id,
			obsResultId);

	DF::DFObservationStrategy::ObservationStrategyPtr observation_strategy_node;
	//Get the observation strategy node from the observation result id.
	DF::DFObservationStrategyList::iterator observation_strategies_iter = observation_strategies_.begin();
	for(;observation_strategies_iter!=observation_strategies_.end();++observation_strategies_iter) {
		try {
			observation_strategy_node = (*observation_strategies_iter)->fromObservationResultId(obsResultId);
			break;
		} catch (NotFound &exp) {
			trace_.out("Not observation strategy node found in this strategy");
		}
	}
	//Check if we found the element, we should.
	if ( observation_strategies_iter == observation_strategies_.end() )
	{
		trace_.out("Not observation strategy node found in ANY strategy");
		return;
	}
	//Print debug info
	(*observation_strategies_iter)->debug();
	//Unsubscribe to reduction results
	unsubscribeToReductionResult_(observation_strategy_node->observation_mode.c_str());
	//Close observation mode session
	trace_.out("Close observation mode session [id=%lu]\n",observation_strategy_node->session_id);
	dfBranchInstance_->closeObsMode(observation_strategy_node->session_id);
	//If the strategy session is complete, close observation strategy session
	if ((*observation_strategies_iter)->isComplete())
	{
		trace_.out("Close observation strategy session [id=%lu]\n",(*observation_strategies_iter)->strategy_session_id);
		dfBranchInstance_->closeObsStrategy((*observation_strategies_iter)->strategy_session_id);
		//Remove from vector
		observation_strategies_.erase(observation_strategies_iter);

	} else {

		for (int next_id = 0; next_id < observation_strategy_node->next.size(); next_id++) {
			//Get the observation strategy parent
			DF::DFObservationStrategy::ObservationStrategyPtr observation_strategy_next_node =
					(*observation_strategies_iter)->fromId(observation_strategy_node->next[next_id]);
			//Increase element counter
			observation_strategy_next_node->element_counter++;
			//Store recipe id
			observation_strategy_next_node->recipes_identifiers.push_back(id);
			//If the observation strategy parent is completed
			if (observation_strategy_next_node->element_counter ==
					observation_strategy_next_node->number_element_to_collect)
			{
				trace_.out("Commit Observation Result\n");
				subscribeToReductionResult_(observation_strategy_next_node->observation_mode.c_str());
				configureObsMode_(observation_strategy_next_node->observation_mode.c_str(),(*observation_strategies_iter)->strategy_session_id);
				observation_strategy_next_node->session_id=dfSessionId_;

				DDPKF::DataElement dataElement;
				dataElement.name = CORBA::string_dup("resultIds");
				dataElement.type = DF::TYPE_LONG_LIST;
				dataElement.value.lList(DGT::LongList());
				toIDL(observation_strategy_next_node->recipes_identifiers, dataElement.value.lList());

				initDFBranchInstance_(false);
				dfBranchInstance_->receiveORParam(dataElement, dfBranchInstanceName_, dfSessionId_);
				observation_strategy_next_node->observation_result_id = dfBranchInstance_->commitOR(true, true,
						dfBranchInstanceName_,
						observation_strategy_next_node->session_id);
				trace_.out("Observation result [id=%ul]\n",observation_strategy_next_node->observation_result_id );
			}
			//Print debug info
			(*observation_strategies_iter)->debug();
		}


	}
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
// setPrincipalInvestigatorName
//----------------------------------------------------------------------
void DFAgent::setPrincipalInvestigatorName(const char* piName)
{
	trace_.out("setPrincipalInvestigatorName(%s)\n",piName);

	ACE_Guard<ACE_Thread_Mutex> guard(deviceGuardMutex_);

	principalInvestigatorName_ = piName;
}

//----------------------------------------------------------------------
// setObservationType
//----------------------------------------------------------------------
void DFAgent::setObservationType(ObservationType obsType)
{
	trace_.out("setObservationType(%s)\n", obsTypeName_[obsType].c_str());
	//trace_.out("setObservationType(%s)\n", to_string(obsType));

	ACE_Guard<ACE_Thread_Mutex> guard(deviceGuardMutex_);

	obsType_ = obsType;
}

//----------------------------------------------------------------------
// setObservationClass
//----------------------------------------------------------------------
void DFAgent::setObservationClass(ObservationClass obsClass)
{
	trace_.out("setObservationClass(%s)\n", obsClassName_[obsClass].c_str());
	//trace_.out("setObservationClass(%s)\n", to_string(obsClass));

	ACE_Guard<ACE_Thread_Mutex> guard(deviceGuardMutex_);

	obsClass_ = obsClass;
}

//----------------------------------------------------------------------
// setInstrumentMode
//----------------------------------------------------------------------
void DFAgent::setInstrumentMode(const char* insMode)
{
	trace_.out("setInstrumentMode(%s)\n",insMode);

	ACE_Guard<ACE_Thread_Mutex> guard(deviceGuardMutex_);

	instrumentMode_ = insMode;
}

//----------------------------------------------------------------------
// setObserverName
//----------------------------------------------------------------------
void DFAgent::setObserverName(const char* observerName)
{
	trace_.out("setObserverName(%s)\n",observerName);

	ACE_Guard<ACE_Thread_Mutex> guard(deviceGuardMutex_);

	observerName_ = observerName;
}

//----------------------------------------------------------------------
// setObjectName
//----------------------------------------------------------------------
void DFAgent::setObjectName(const char* objectName)
{
	trace_.out("setObjectName(%s)\n",objectName);

	ACE_Guard<ACE_Thread_Mutex> guard(deviceGuardMutex_);

	objectName_ = objectName;
}

//----------------------------------------------------------------------
// getObjectName
//----------------------------------------------------------------------
string DFAgent::getObjectName()
{
	trace_.out("getObjectName\n");

	ACE_Guard<ACE_Thread_Mutex> guard(deviceGuardMutex_);

	return objectName_;
}

//----------------------------------------------------------------------
// setProgramId
//----------------------------------------------------------------------
void DFAgent::setProgramId(const char* programId)
{
	trace_.out("setProgramId(%s)\n",programId);

	ACE_Guard<ACE_Thread_Mutex> guard(deviceGuardMutex_);

	programId_ = programId;
}

//----------------------------------------------------------------------
// setESOProgramId
//----------------------------------------------------------------------
void DFAgent::setESOProgramId(const char* esoProgramId)
{
	trace_.out("setESOProgramId(%s)\n",esoProgramId);

	ACE_Guard<ACE_Thread_Mutex> guard(deviceGuardMutex_);

	esoProgramId_ = esoProgramId;
}

//----------------------------------------------------------------------
// setObservationBlockId
//----------------------------------------------------------------------
void DFAgent::setObservationBlockId(const char* obId)
{
	trace_.out("setObservationBlockId(%s)\n",obId);

	ACE_Guard<ACE_Thread_Mutex> guard(deviceGuardMutex_);

	observationBlockId_ = obId;
}

//----------------------------------------------------------------------
// setImageTitle
//----------------------------------------------------------------------
void DFAgent::setImageTitle(const char* imageTitle)
{
	trace_.out("setImageTitle(%s)\n",imageTitle);

	ACE_Guard<ACE_Thread_Mutex> guard(deviceGuardMutex_);

	imageTitle_ = imageTitle;
}

//----------------------------------------------------------------------
// setObservationComment
//----------------------------------------------------------------------
void DFAgent::setObservationComment(const char* observationComment)
{
	trace_.out("setObservationComment(%s)\n",observationComment);

	ACE_Guard<ACE_Thread_Mutex> guard(deviceGuardMutex_);

	observationComment_ = observationComment;
}

//----------------------------------------------------------------------
// setObsModeLowLevel
//----------------------------------------------------------------------
void DFAgent::setObsModeLowLevel(short obsMode)
{

	trace_.out("setObsModeLowLevel(%d)\n",obsMode);

	ACE_Guard<ACE_Thread_Mutex> guard(deviceGuardMutex_);
	//@ipc
	try
	{
		//check dfBranchInstance_ isn't null, otherwise reconnect to it.
		initDFBranchInstance_(false);

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
		dfSessionId_ = dfBranchInstance_->configureObsMode( obsModeName_[shortObsMode_].c_str(),0L);
		dfBranchInstance_->setInstrumentId (gcsComponent_, cameraName_, dfBranchInstanceName_, dfSessionId_);
		dfBranchInstance_->setSessionAutoReduce(doesAutoReduce_, dfBranchInstanceName_, dfSessionId_);
		isSessionRunning_ = true;

		fetchORInfo_();

		//send early elements, the ones which can be sent while waiting for the rawFrame
		vector<string>::iterator it, itBegin = recipeElementNames_.begin(), itEnd = recipeElementNames_.end();
		for(it = itBegin; it != itEnd; it++)
		{
			//hasFrameArrived = false
			sendElement_(*it, false);
		}
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
// setObsMode
//----------------------------------------------------------------------
void DFAgent::setObservationMode(const char * obsMode) {

	ACE_Guard<ACE_Thread_Mutex> guard(dfagentGuardMutex_);

	trace_.out("setObservationMode(%s)\n",obsMode);

	//Get the availables observation modes
	std::vector<std::string> observation_modes = getAvailableObsMode_();
	if ( find(observation_modes.begin(), observation_modes.end(), obsMode) != observation_modes.end() ) {
		//Get the observation strategy description
		std::string strategy = dfBranchInstance_->getObsStrategyDetailts(obsMode);
		//Create & Build the configuration strategy.
		DF::DFObservationStrategy * observation_strategy = new DF::DFObservationStrategy();
		observation_strategy->build(strategy);
		//Print debug info
		observation_strategy->debug();
		//Open strategy session
		observation_strategy->strategy_session_id = dfBranchInstance_->configureObsStrategy(obsMode);
		//Store the observation strategy
		observation_strategies_.push_back(observation_strategy);
		//Print debug info
		observation_strategy->debug();
	} else {
		throw InvalidValue("DFAgent::setObservationMode", "Observation Mode not defined in the Pipeline.");
	}
}

//----------------------------------------------------------------------
// setPropagateFrame
//----------------------------------------------------------------------
void DFAgent::setPropagateFrame(bool doesPropagate)
{
	trace_.out("setPropagateFrame(%hd)\n",doesPropagate);

	ACE_Guard<ACE_Thread_Mutex> guard(deviceGuardMutex_);

	doesPropagateFrame_ = doesPropagate;
	propertyChange_("propagateFrame");
	magnitudeChange_("doesPropagateFrame",doesPropagate);

}

//----------------------------------------------------------------------
// setSaveFrame
//----------------------------------------------------------------------
void DFAgent::setSaveFrame(bool doesSave)
{
	trace_.out("setSaveFrame(%hd)\n",doesSave);

	ACE_Guard<ACE_Thread_Mutex> guard(deviceGuardMutex_);

	doesSaveFrame_ = doesSave;
	propertyChange_("saveFrame");
	magnitudeChange_("doesSaveFrame",doesSave);
}

//----------------------------------------------------------------------
// setPropagateFITS
//----------------------------------------------------------------------
void DFAgent::setPropagateFITS(bool doesPropagate)
{
	trace_.out("setPropagateFITS(%hd)\n",doesPropagate);

	ACE_Guard<ACE_Thread_Mutex> guard(deviceGuardMutex_);

	doesPropagateFITS_ = doesPropagate;
	propertyChange_("propagateFITS");
	magnitudeChange_("doesPropagateFITS", doesPropagate);
}

//----------------------------------------------------------------------
// setSaveFITS
//----------------------------------------------------------------------
void DFAgent::setSaveFITS(bool doesSave)
{
	trace_.out("setSaveFITS(%hd)\n",doesSave);

	ACE_Guard<ACE_Thread_Mutex> guard(deviceGuardMutex_);

	doesSaveFITS_ = doesSave;
	propertyChange_("saveFITS");
	magnitudeChange_("doesSaveFITS", doesSave);
}

//----------------------------------------------------------------------
// setAutoReduce
//----------------------------------------------------------------------
void DFAgent::setAutoReduce(bool doesAuto)
{
	trace_.out("setAutoReduce(%hd)\n",doesAuto);

	ACE_Guard<ACE_Thread_Mutex> guard(deviceGuardMutex_);

	doesAutoReduce_ = doesAuto;
	propertyChange_("autoReduce");
	magnitudeChange_("doesAutoReduce", doesAuto);
}

//----------------------------------------------------------------------
// setWriteKeywords
//----------------------------------------------------------------------
void DFAgent::setWriteKeywords(bool doesWrite)
{
	trace_.out("setWriteKeywords(%hd)\n",doesWrite);

	ACE_Guard<ACE_Thread_Mutex> guard(deviceGuardMutex_);

	doesWriteKeywords_ = doesWrite;
	propertyChange_("writeKeywords");
	magnitudeChange_("doesWriteKeywords", doesWrite);
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
// Get dfBranchInstanceName property
//----------------------------------------------------------------------
const char* DFAgent::dfManagerInstanceName()
{
	trace_.out("dfManagerInstanceName()\n");
	return dfManagerInstanceName_.in();
}

//----------------------------------------------------------------------
// Set dfBranchInstanceName property
//----------------------------------------------------------------------
void DFAgent::dfManagerInstanceName(const char* new_dfManagerInstanceName)
{
	trace_.out("dfManagerInstanceName(%s)\n",new_dfManagerInstanceName);
	// Change property
	dfManagerInstanceName_ = new_dfManagerInstanceName;
	if(isIdle())
	{

		dfManagerInstance_ = DF::DataAcquisitionManager_ifce::_nil();
		initDFManagerInstance_();
	}
	propertyChange_("dfManagerInstanceName");
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

/************************************************************************
 *************************************************************************
 **                                                                     **
 **          Implementation of specific private methods                 **
 **                                                                     **
 *************************************************************************
 ************************************************************************/


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

std::vector<std::string> DFAgent::getAvailableObsMode_()
{
	initDFBranchInstance_(false);

	try {

		DGT::StringList_var idl_string_list = dfBranchInstance_->getAvailableObsMode();
		std::vector<std::string> string_list;
		fromIDL(idl_string_list,string_list);

		return string_list;
	} catch (GCSException& ex ) {
		ex.addToTrace("DFAgent::getAvailableObsMode_()");
		throw;
	}
}

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
			{
				//try BeforeFrame, just in case it is not an withFrame element
				pDataElement = getBeforeFrameRecipeElement_(elementName);
			}

			if( pDataElement != NULL)
			{
				dfBranchInstance_->receiveORParam( *pDataElement, dfBranchInstanceName_, dfSessionId_);
				delete pDataElement;
			}
		}
	}
	catch(const CORBA::Exception &ex)
	{
		GCSException gcsex = DException::mapCORBAException(const_cast<CORBA::Exception&>(ex));
		if (gcsex.isA ("CorbaException"))
		{
			//It is a timeout, was a communications error
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
void DFAgent::initDFBranchInstance_(bool init_obsmode_lowlevel)
{
	try
	{
		if(CORBA::is_nil(const_cast<const DF::Branch_ifce_var&>(dfBranchInstance_)))
		{
			// Initializing pointer to data factory corba object
			logInfo_ ("Connecting to Data Factory Branch");
			trace_.out ("Connecting to Data Factory Branch");
			CORBARef<DF::Branch_ifce> df_ref(dfBranchInstanceName_, 0);
			df_ref.roundtripTimeout( TimeValue(60, 0) );
			dfBranchInstance_ = df_ref.typedRef();
			dfBranchInstance_->ping();
			logInfo_("Connected to DF Branch!");
			trace_.out ("Connected to DF Branch!");
			logInfo_("Opening session with default mode");

			if (init_obsmode_lowlevel) {
				setObsModeLowLevel (shortObsMode_);
			}
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
// initDFManagerInstance
//----------------------------------------------------------------------
void DFAgent::initDFManagerInstance_()
{
	try
	{
		if(CORBA::is_nil(const_cast<const DF::DataAcquisitionManager_ifce_var&>(dfManagerInstance_)))
		{
			//Initializing pointer to data factory corba object
			logInfo_ ("Connecting to Data Factory Manager");
			trace_.out ("Connecting to Data Factory Manager");

			CORBARef<DF::DataAcquisitionManager_ifce> dfmanager_ref(dfManagerInstanceName_, 0);
			dfmanager_ref.roundtripTimeout( TimeValue(60, 0) );
			dfManagerInstance_ = dfmanager_ref.typedRef();
			logInfo_("Connected to DF Manager!");
			trace_.out ("Connected to DF Manager!");
			//Pointer to adapter
			consumerId_ = dynamic_cast<DFAgentAdapter*>(adapter_)->_this();
		}
	}
	catch(const CORBA::Exception &ex)
	{
		GCSException gcsex = DException::mapCORBAException(const_cast<CORBA::Exception&>(ex));
		cout << "ERROR: " << gcsex << endl;
		if (gcsex.isA ("CorbaException"))
		{  //It is a timeout, set null the corba pointer
			dfManagerInstance_ = DF::DataAcquisitionManager_ifce::_nil();
		}
		throw;
	}
	catch (GCSException& ex)
	{
		ex.addToTrace ("DFAgent::initDFManagerInstance_()");
		logError_ ("ERROR when connecting to DFManager: %s", ex.toString());
		dfManagerInstance_ = DF::DataAcquisitionManager_ifce::_nil();
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
			//frame.addKeyword(DPKF::Keyword<string>("OBSTYPE", to_string(obsType_), "Observation Type", ""));
		}
		catch(AlreadyExists &ex)
		{
			frame.modifyKeyword("OBSTYPE", obsTypeName_[obsType_]);
			//frame.modifyKeyword("OBSTYPE", std::string(to_string(obsType_)));
		}
		try
		{
		  frame.addKeyword(DPKF::Keyword<string>("OBSCLASS", obsClassName_[obsClass_].c_str(), "Observation Class", ""));
		  //frame.addKeyword(DPKF::Keyword<string>("OBSCLASS", to_string(obsClass_), "Observation Class", ""));
		}
		catch(AlreadyExists &ex)
		{
		  frame.modifyKeyword("OBSCLASS", obsClassName_[obsClass_]);
		  //frame.modifyKeyword("OBSCLASS", std::string(to_string(obsClass_)));
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

	trace_.out("writeWCSKeywords_()\n");
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

void DFAgent::receiveFrameObsStrategy(const DDPKF::Frame& idlFrame,
		int currentWindow,
		int numWindows,
		const DGT::TinyVector& idlTopLeft,
		const DGT::TinyVector& idlBottomRight,
		const DGT::TimeValue& timestamp,
		const DGT::TimeValue& exposureTime,
		bool propagate,
		bool save,
		bool raw_frame) {

	ACE_Guard<ACE_Thread_Mutex> guard(dfagentGuardMutex_);

	trace_.out("receiveFrame\n");

	//Get the first observation strategy node with room for new images.
	DF::DFObservationStrategyPtr strategyPtr (nullptr);
	strategyPtr = DF::find1stObsStrategyNotAcquisitionCompleted(observation_strategies_);

	//Print debug info
	strategyPtr->debug();

	//Obtain the next Observation Strategy
	DF::DFObservationStrategy::ObservationStrategyPtr observation_strategy_node (strategyPtr->next());

	//If a reduction session has not been opened
	if(observation_strategy_node->session_id == 0) {
		subscribeToReductionResult_(observation_strategy_node->observation_mode.c_str());
		configureObsMode_(observation_strategy_node->observation_mode.c_str(), strategyPtr->strategy_session_id);
		observation_strategy_node->session_id=dfSessionId_;
	}

	//Raw frame, it does not belong to observation result
	if (raw_frame) {
		dfBranchInstance_->receiveFrame(idlFrame,
				currentWindow,
				numWindows,
				idlTopLeft,
				idlBottomRight,
				timestamp,
				exposureTime,
				propagate,
				save,
				observation_strategy_node->session_id );
	} else {
		dfBranchInstance_->receiveORParamAcquiredFrame(
				"rawFrame",
				idlFrame,
				currentWindow,
				numWindows,
				idlTopLeft,
				idlBottomRight,
				timestamp,
				exposureTime,
				doesPropagateFrame_,
				doesSaveFrame_,
				dfBranchInstanceName_,
				observation_strategy_node->session_id);
		//Increase element counter
		observation_strategy_node->element_counter++;
		//It is the observation strategy completed?
		if(observation_strategy_node->element_counter==observation_strategy_node->number_element_to_collect) {
			trace_.out("Commit Observation Result\n");
			observation_strategy_node->observation_result_id = dfBranchInstance_->commitOR(true, true,
					dfBranchInstanceName_,
					observation_strategy_node->session_id);
			trace_.out("Observation result [id=%ul]\n",observation_strategy_node->observation_result_id);
		}
	}
	//Print debug info
	strategyPtr->debug();
}

void DFAgent::configureObsMode_(const char * obsMode, long observation_strategy_session)
{

	trace_.out("DFAgent::configureObsMode_ [obs_mode=%s]\n",obsMode);

	initDFBranchInstance_(false);

	try {
		dfSessionId_ = dfBranchInstance_->configureObsMode( obsMode,observation_strategy_session );
		dfBranchInstance_->setInstrumentId (gcsComponent_, cameraName_, dfBranchInstanceName_, dfSessionId_);
		dfBranchInstance_->setSessionAutoReduce(doesAutoReduce_, dfBranchInstanceName_, dfSessionId_);
		//isSessionRunning_ = true;
	} catch (GCSException& ex ) {
		ex.addToTrace("DFAgent::configureObsMode_()");
		throw;
	}
}

void DFAgent::unsubscribeToReductionResult_(const char * recipe_name)
{
	try
	{
		trace_.out("Unsubscribe to Reduction Result [name=%s]\n", recipe_name);

		dfManagerInstance_->unsubscribeFromRecipeResults(gcsComponent_,
				cameraName_,
				recipe_name,
				recipe_name,
				consumerId_);
	} catch(...) {
	}
}

void DFAgent::subscribeToReductionResult_(const char * recipe_name)
{
	initDFManagerInstance_();

	unsubscribeToReductionResult_(recipe_name);
	try {
		trace_.out("Subscribe to reduction result[id=%s]\n",recipe_name);
		dfManagerInstance_->subscribeToRecipeResults(gcsComponent_,
				cameraName_,
				recipe_name,
				recipe_name,
				consumerId_);
	} catch (GCSException& ex ) {
		ex.addToTrace("DFAgent::subscribeToReductionResult_()");
		throw;
	}
}

//----------------------------------------------------------------------
// parseFITSHeaderKeywordsFromFile_
//----------------------------------------------------------------------
void DFAgent::parseFITSHeaderKeywordsFromFile_(std::string filePath)
{
  if ( filePath.empty() )
  {
    filePath = std::string(obsModeKeywordListPath_.c_str());
  }

  // Check if the DFBranch is available or not.
  initDFBranchInstance_(false);

  // Check if file can be opened
  {
    std::ifstream file(filePath);
    if ( not file.is_open() )
    {
      NotFound exception(__PRETTY_FUNCTION__);
      exception.addText("Unable to open file '%s' where the monitor-keywords are specified.", filePath.c_str() );
      throw exception;
    }
    else
    {
      file.close();
    }
  }

  // Define an auxiliary function as static lambda function
  static auto endWith = [](const std::string &base, const std::string &toMatch) -> bool
  {
    return toMatch.size() <= base.size() and std::equal(toMatch.rbegin(),
                                                        toMatch.rend(),
                                                        base.rbegin());
  };

  // Determine the file format by its extension
  if ( endWith (filePath, ".json") )
  {
    parseFITSHeaderKeywordsFromJSONFile_(filePath);
  }// JSON
  else
  {
    parseFITSHeaderKeywordsFromPlainTextFile_(filePath);
  }// PlainText
}

//----------------------------------------------------------------------
// parseFITSHeaderKeywordsFromPlainTextFile_
//----------------------------------------------------------------------
void DFAgent::parseFITSHeaderKeywordsFromPlainTextFile_(const std::string &filePath)
{
  // Check pre-condition
  if ( obsModeName_.size() < 1 )
  {
    NoEntries exception(__PRETTY_FUNCTION__);
    exception.addText("obsModeName_ member must be initialized with all observation mode names before.");
    throw exception;
  }

  // Load FITS Header keywords for each Obs Mode
  std::ifstream inputFile(filePath.c_str());

  if( inputFile.is_open() )
  {
    obsModeKeywordList_.clear();
    obsModeHash_.clear();
    std::string fitsHeaderPath;
    hash_t identifier(0);
    while(inputFile >> fitsHeaderPath)
    {
      
      trace_.out ("Parsing keyword specification line %d ...", identifier);
      // Check pre-condition
      if ( obsModeName_.count(identifier) < 1 )
      {
        NoEntries exception(__PRETTY_FUNCTION__);
        exception.addText("There is not defined an observation mode name for id number '%d'.", identifier);
        throw exception;
      }
      //header keyword files are read in the same order than ObservationMode enumeration
      FITSHeaderInfo fitsHeaderInfo;
      fitsHeaderInfo.parseFITSHeaderKeywordsFromFile(fitsHeaderPath.c_str());
      obsModeKeywordList_.emplace(identifier, fitsHeaderInfo);
      obsModeHash_.emplace(obsModeName_.at(identifier), identifier);
      ++identifier;
    }
    if (obsModeKeywordList_.size() == 0)
    {
      InvalidValue exception (__PRETTY_FUNCTION__);
      exception.addText("No keywords configuration found.");
      throw exception;
    }
    inputFile.close();
  }
  else
  {
    OperationFailed exception(__PRETTY_FUNCTION__);
    exception.addText("Unable to open file '%s'.", filePath.data());
    throw exception;
  }
}

//----------------------------------------------------------------------
// parseFITSHeaderKeywordsFromJSONFile_
//----------------------------------------------------------------------
void DFAgent::parseFITSHeaderKeywordsFromJSONFile_(const std::string &filePath)
{
  std::ifstream inputFile(filePath.data());
  if ( not inputFile.is_open() )
  {
    OperationFailed exception(__PRETTY_FUNCTION__);
    exception.addText("Unable to open file '%s'.", filePath.data());
    throw exception;
  }

  //<editor-fold desc="Parse JSON file">
  Json::Value root;
  try
  {
    inputFile >> root;
  }
  catch (...)
  {
    SyntaxError exception(__PRETTY_FUNCTION__);
    exception.addText("Error when parsing JSON file ('%s'). Suggestion: Check JSON syntax.", filePath.data() );
    throw exception;
  }
  inputFile.close();
  //</editor-fold>

  // Check type of root node
  if ( not root.isObject() )
  {
    throw SyntaxError(__PRETTY_FUNCTION__, "The root JSON node must be an object.");
  }

  static const std::vector<std::string> fieldsLevel1 ({ "keyword-sections", "observation-modes" });
  static const std::vector<std::string> fieldsLevel2 ({ "name", "keywords" });

  //<editor-fold desc="Check if all fields, which belong to 1st level, exist or not">
  {
    for (const std::string&item : fieldsLevel1)
    {
      //<editor-fold desc="Check if item is present or not">
      if ( not root.isMember(item) )
      {
        SyntaxError exception(__PRETTY_FUNCTION__);
        exception.addText("The root JSON node must have defined the field called '%s'.", item.data());
        trace_.err( exception.text() );
        if ( logAgent_ not_eq nullptr )
          logAgent_->logError( exception.text() );
        throw exception;
      }
      //</editor-fold>

      //<editor-fold desc="Check if item node is an array or not">
      if ( not root[item].isArray() )
      {
        SyntaxError exception(__PRETTY_FUNCTION__);
        exception.addText("The field called '%s' must be an array type.", item.data());
        trace_.err( exception.text() );
        if ( logAgent_ not_eq nullptr )
          logAgent_->logError( exception.text() );
        throw exception;
      }
      //</editor-fold>
    }
  }
  //</editor-fold>

  // Define aliases
  Json::Value
    &nodeKeywordSections( root["keyword-sections"]),
    &nodeKeywordObsModes( root["observation-modes"])
  ;

  //<editor-fold desc="Parse sections of keywords.">
  trace_.out("Parse sections of keywords.");
  using KeywordSectionContainer = std::map<std::string, FITSHeaderInfo>;
  KeywordSectionContainer keywordSections;
  {
    for ( Json::ArrayIndex indexSection (-1); ++indexSection < nodeKeywordSections.size(); )
    {
      unsigned int id (indexSection + 1);
      Json::Value & node ( nodeKeywordSections[indexSection] );
      try
      {
        //<editor-fold desc="Check if the mandatory field exists or not.">
        for (const auto &item : fieldsLevel2)
        {
          if ( not node.isMember(item) )
          {
            SyntaxError exception (__PRETTY_FUNCTION__);
            exception.addText("The node number '%03d' doesn't contain the field '%s'", id, item.data());
            trace_.err( exception.text() );
            if ( logAgent_ not_eq nullptr )
              logAgent_->logError( exception.text() );
            throw exception;
          }
        }
        //</editor-fold>

        // Define aliases
        Json::Value
          & nodeSectionName     (node["name"]),
          & nodeSectionKeywords (node["keywords"])
        ;

        //<editor-fold desc="Check fields types">
        if ( not nodeSectionName.isString() )
        {
          SyntaxError exception (__PRETTY_FUNCTION__);
          exception.addText("Error in the node number '%03d'. The value of the field 'name' must be a string.'", id);
          trace_.err( exception.text() );
          if ( logAgent_ not_eq nullptr )
            logAgent_->logError( exception.text() );
          throw exception;
        }
        if ( not nodeSectionKeywords.isString() and not nodeSectionKeywords.isArray() )
        {
          SyntaxError exception (__PRETTY_FUNCTION__);
          exception.addText("Error in the node number '%03d'. The value of the field 'keywords' must be a string or an array.'", id);
          trace_.err( exception.text() );
          if ( logAgent_ not_eq nullptr )
            logAgent_->logError( exception.text() );
          throw exception;
        }
        //</editor-fold>

        KeywordSectionContainer::key_type    sectionName     ( nodeSectionName.asString() );
        KeywordSectionContainer::mapped_type sectionKeywords (monitorAgent_, logAgent_, trace_);

        //<editor-fold desc="Parse section of keywords">
        // Inline keywords definition or not
        if ( nodeSectionKeywords.isString() )
        {
          // Keywords are defined in a file
          sectionKeywords.parseFITSHeaderKeywordsFromFile( nodeSectionKeywords.asString() );
        }
        else if ( nodeSectionKeywords.isArray() )
        {
          // Keywords are defined inline
          sectionKeywords.parseFITSHeaderKeywordsFromInline( nodeSectionKeywords );
        }
        else
        {
          // Save guard!
          UnknownException exception(__PRETTY_FUNCTION__);
          exception.addText("Software error. Line='%d'", __LINE__);
          trace_.err( exception.text() );
          if ( logAgent_ not_eq nullptr )
            logAgent_->logError( exception.text() );
          throw exception;
        }
        //</editor-fold>

        keywordSections.emplace( sectionName, sectionKeywords );
      }
      catch (GCSException &exception)
      {
        exception.addToTrace(__PRETTY_FUNCTION__);
        throw;
      }
      catch (...)
      {
        UnknownException exception(__PRETTY_FUNCTION__);
        exception.addText( "Problems to parse the node number '%03d'.", id );
        trace_.err( exception.text() );
        if ( logAgent_ not_eq nullptr )
        {
          logAgent_->logError( exception.text() );
        }
        throw;
      }
    }
  }
  //</editor-fold>

  //<editor-fold desc="Get all available observation modes">
  std::vector<std::string> availableObsModeList (getAvailableObsMode_() );
  //</editor-fold>

  //<editor-fold desc="Parse observation mode keywords">
  {
    trace_.out("Parse observation mode keywords.");
    obsModeKeywordList_.clear();
    obsModeHash_.clear();
    obsModeName_.clear();
    for ( Json::ArrayIndex indexObsMode (-1); ++indexObsMode < nodeKeywordObsModes.size(); )
    {
      unsigned int id (indexObsMode + 1);
      Json::Value & node ( nodeKeywordObsModes[indexObsMode] );
      try
      {
        //<editor-fold desc="Check if the mandatory field exists or not">
        for (const auto &item : fieldsLevel2)
        {
          if ( not node.isMember(item) )
          {
            SyntaxError exception (__PRETTY_FUNCTION__);
            exception.addText("The node number '%03d' doesn't contain the field '%s'.", id, item.data());
            trace_.err( exception.text() );
            if ( logAgent_ not_eq nullptr )
              logAgent_->logError( exception.text() );
            throw exception;
          }
        }
        //</editor-fold>

        const bool & isIDPresent(node.isMember("id"));
        const Json::Value
            & nodeObsModeName   (node["name"]),
            & nodeObsModeLabels (node["keywords"]),
            & nodeObsModeID (node["id"])
        ;

        //<editor-fold desc="Check fields types">
        if ( not nodeObsModeName.isString() )
        {
          SyntaxError exception (__PRETTY_FUNCTION__);
          exception.addText("Error in the node number '%03d'. The value of the field 'name' must be a string.'", id);
          trace_.err( exception.text() );
          if ( logAgent_ not_eq nullptr )
            logAgent_->logError( exception.text() );
          throw exception;
        }
        if ( isIDPresent and not nodeObsModeID.isUInt() )
        {
          SyntaxError exception (__PRETTY_FUNCTION__);
          exception.addText("The node number '%03d'. The value of the field 'id' must be a natural number.", id);
          trace_.err( exception.text() );
          if ( logAgent_ not_eq nullptr )
            logAgent_->logError( exception.text() );
          throw exception;
        }
        if ( not nodeObsModeLabels.isArray() )
        {
          SyntaxError exception (__PRETTY_FUNCTION__);
          exception.addText("Error in the node number '%03d'. The value of the field 'keywords' must be an array.'", id);
          trace_.err( exception.text() );
          if ( logAgent_ not_eq nullptr )
            logAgent_->logError( exception.text() );
          throw exception;
        }
        //</editor-fold>

        const std::string obsModeName     ( nodeObsModeName.asString() );
        const hash_t obsModeID            ( isIDPresent ? nodeObsModeID.asUInt() : DFAgent::hash(obsModeName) );
        FITSHeaderInfo    obsModeKeywords (monitorAgent_, logAgent_, trace_);

        //<editor-fold desc="Check if the given observation mode name is available or not">
        if ( availableObsModeList.end() == std::find(availableObsModeList.begin(),
                                                     availableObsModeList.end(),
                                                     obsModeName)
           )
        {
          InvalidValue exception(__PRETTY_FUNCTION__);
          exception.addText("Observation mode '%s' is not defined in the Pipeline.", obsModeName.data());
          throw exception;
        }
        //</editor-fold>

        //<editor-fold desc="Collect and merge all keywords sections">
        for (Json::ArrayIndex indexSectionLabel (-1); ++indexSectionLabel < nodeObsModeLabels.size();)
        {
          //<editor-fold desc="Check type">
          if ( not nodeObsModeLabels[indexSectionLabel].isString() )
          {
            SyntaxError exception (__PRETTY_FUNCTION__);
            exception.addText("Error in the node number '%03d'.", id);
            exception.addText("Unable to parse the element number '%d', it must be a string.'", (indexSectionLabel+1) );
            trace_.err( exception.text() );
            if ( logAgent_ not_eq nullptr )
              logAgent_->logError( exception.text() );
            throw exception;
          }
          //</editor-fold>
          //<editor-fold desc="Check if label exists or not">
          const KeywordSectionContainer::key_type & label (nodeObsModeLabels[indexSectionLabel].asString());
          if ( keywordSections.count(label) < 1 )
          {
            SyntaxError exception(__PRETTY_FUNCTION__);
            exception.addText("Unable to find the section '%s'.", label.data() );
            trace_.err( exception.text() );
            if ( logAgent_ not_eq nullptr )
              logAgent_->logError( exception.text() );
            throw exception;
          }
          //</editor-fold>
          //<editor-fold desc="Merge">
          obsModeKeywords.merge(keywordSections.at(label));
          //</editor-fold>
        }
        //</editor-fold>

        //<editor-fold desc="Register parsed keywords for the specific observation mode.">
        obsModeKeywordList_.emplace(obsModeID, obsModeKeywords);
        obsModeHash_.emplace(obsModeName, obsModeID);
        obsModeName_.emplace(obsModeID, obsModeName);
        //</editor-fold>
      }
      catch (GCSException &exception)
      {
        exception.addToTrace(__PRETTY_FUNCTION__);
        throw;
      }
      catch (...)
      {

        UnknownException exception(__PRETTY_FUNCTION__);
        exception.addText( "Problems to parse the node number '%03d'.", id );
        trace_.err( exception.text() );
        if ( logAgent_ not_eq nullptr )
        {
          logAgent_->logError( exception.text() );
        }
        throw;
      }
    }
  }
  //</editor-fold>
}

//----------------------------------------------------------------------
// hash
//----------------------------------------------------------------------
template<typename T>
const DFAgent::hash_t DFAgent::hash(const T &obj) noexcept
{
  static_assert(
      (std::is_enum<T>::value or                          // is T an enum data type?
       std::is_same<int short, T>::value or               // is T an int short data type?
       std::is_same<int, T>::value or                     // is T an int data type?
       std::is_same<int long, T>::value or                // is T an int long data type?
       std::is_same<int long long , T>::value or          // is T an int long long data type?
       std::is_same<unsigned int short, T>::value or      // is T an unsigned int short data type?
       std::is_same<unsigned int, T>::value or            // is T an unsigned int data type?
       std::is_same<unsigned int long, T>::value or       // is T an unsigned int long data type?
       std::is_same<unsigned int long long , T>::value or // is T an unsigned int long long data type?
       std::is_same<std::string, T>::value                // is T a  string data type?
      ),
      "Invalid given type for T typename."
  );
  const static std::hash<T> hash_fn;
  return (hash_fn(obj));
}

//----------------------------------------------------------------------
// gatherKeywords_
//----------------------------------------------------------------------
bool DFAgent::gatherKeywords_(const TimeValue &time,
		                      const TimeValue &expTime,
		                      DataDescription &header)
{
	bool retVal {true};
	retVal &= gatherFixedKeywords_(header);
	retVal &= gatherMonitorKeywords_(time, expTime, header);
	return retVal;
}

//----------------------------------------------------------------------
// gatherFixedKeywords_
//----------------------------------------------------------------------
bool DFAgent::gatherFixedKeywords_(DataDescription &header)
{
	bool retVal {true};
	const DFAgent::hash_t && hash { getHashOf1stAcquiringObsStrategyNode_() };
	retVal = DFAgent::obsModeKeywordList_[hash].gatherFixedKeywords(header);
	return retVal;
}

//----------------------------------------------------------------------
// gatherMonitorKeywords_
//----------------------------------------------------------------------
bool DFAgent::gatherMonitorKeywords_(const TimeValue &time,
		                             const TimeValue &expTime,
		                             DataDescription &header)

{
	bool retVal {true};
	const DFAgent::hash_t && hash { getHashOf1stAcquiringObsStrategyNode_() };
	retVal = DFAgent::obsModeKeywordList_[hash]
			.gatherMonitorKeywords(time, expTime, DFAgent::monitorAgent_, header);
	return retVal;
}

//----------------------------------------------------------------------
// getNameOf1stAcquiringObsStrategyNode_
//----------------------------------------------------------------------
const char * DFAgent::getNameOf1stAcquiringObsStrategyNode_()
{
	DF::DFObservationStrategyPtr strategyPtr (nullptr);
	strategyPtr = DF::find1stObsStrategyNotAcquisitionCompleted(DFAgent::observation_strategies_);
	//Print debug info
	strategyPtr->debug();
	DF::DFObservationStrategy::ObservationStrategyPtr observation_strategy_node (strategyPtr->next());
	return observation_strategy_node->observation_mode.data();
}

//----------------------------------------------------------------------
// getHashOf1stAcquiringObsStrategyNode_
//----------------------------------------------------------------------
DFAgent::hash_t DFAgent::getHashOf1stAcquiringObsStrategyNode_()
{
	const char* obsModeNodeName ( DFAgent::getNameOf1stAcquiringObsStrategyNode_() );
	if ( DFAgent::obsModeHash_.count(obsModeNodeName) < 1 )
	{
		NoEntries exception(__PRETTY_FUNCTION__);
		exception.addText("First observation mode for acquisition '%s' has not a registered identifier.", obsModeNodeName);
		throw exception;
	}
	DFAgent::shortObsMode_ = DFAgent::obsModeHash_.at(obsModeNodeName);
	return DFAgent::shortObsMode_;
}


/************************************************************************
*************************************************************************
**                                                                     **
**                           Other functions                           **
**                                                                     **
*************************************************************************
************************************************************************/

//----------------------------------------------------------------------
// to_string
//----------------------------------------------------------------------
const char* to_string(const DFAgent::ObservationType &obj)
{
  switch (obj)
  {
    case DFAgent::ObservationType::BIAS:          return "BIAS";
    case DFAgent::ObservationType::DARK:          return "DARK";
    case DFAgent::ObservationType::SKY_FLAT:      return "SKY_FLAT";
    case DFAgent::ObservationType::DOME_FLAT:     return "DOME_FLAT";
    case DFAgent::ObservationType::SPECTRAL_FLAT: return "SPECTRAL_FLAT";
    case DFAgent::ObservationType::ARC:           return "ARC";
    case DFAgent::ObservationType::SKY:           return "SKY";
    case DFAgent::ObservationType::OBJECTS:       return "OBJECTS";
    default:
      throw InvalidValue(
          (ACE_TEXT("%s [line=#%l]"), __PRETTY_FUNCTION__),
          "The given value is unknown for DFAgent::ObservationType data type."
      );
      return "UNKNOWN";
  }
}

//----------------------------------------------------------------------
// to_string
//----------------------------------------------------------------------
const char* to_string(const DFAgent::ObservationClass &obj)
{
  switch (obj)
  {
    case DFAgent::ObservationClass::SCIENCE: return "SCIENCE";
    case DFAgent::ObservationClass::CALIB:   return "CALIB";
    default:
      throw InvalidValue(
          (ACE_TEXT("%s [line=#%l]"), __PRETTY_FUNCTION__),
          "The given value is unknown for DFAgent::ObservationClass data type."
      );
      return "UNKNOWN";
  }
}

//----------------------------------------------------------------------
// from_string
//----------------------------------------------------------------------
void from_string(const std::string &str, DFAgent::ObservationType &obj)
{
  if      ( str == "BIAS"          ) { obj = DFAgent::ObservationType::BIAS          ; return; }
  else if ( str == "DARK"          ) { obj = DFAgent::ObservationType::DARK          ; return; }
  else if ( str == "SKY_FLAT"      ) { obj = DFAgent::ObservationType::SKY_FLAT      ; return; }
  else if ( str == "DOME_FLAT"     ) { obj = DFAgent::ObservationType::DOME_FLAT     ; return; }
  else if ( str == "SPECTRAL_FLAT" ) { obj = DFAgent::ObservationType::SPECTRAL_FLAT ; return; }
  else if ( str == "ARC"           ) { obj = DFAgent::ObservationType::ARC           ; return; }
  else if ( str == "SKY"           ) { obj = DFAgent::ObservationType::SKY           ; return; }
  else if ( str == "OBJECTS"       ) { obj = DFAgent::ObservationType::OBJECTS       ; return; }
  else
  {
    throw InvalidValue(
        (ACE_TEXT("%s [line=#%l]"), __PRETTY_FUNCTION__),
        "The given string doesn't match with a value of DFAgent::ObservationType data type."
    );
  }
}

//----------------------------------------------------------------------
// from_string
//----------------------------------------------------------------------
void from_string(const std::string &str, DFAgent::ObservationClass &obj)
{
  if      ( str == "SCIENCE" ) { obj = DFAgent::ObservationClass::SCIENCE ; return ; }
  else if ( str == "CALIB"   ) { obj = DFAgent::ObservationClass::CALIB   ; return ; }
  else
  {
    throw InvalidValue(
        (ACE_TEXT("%s [line=#%l]"), __PRETTY_FUNCTION__),
        "The given string doesn't match with a value of DFAgent::ObservationClass data type."
    );
  }
}

//----------------------------------------------------------------------
// operator << DFAgent::ObservationType
//----------------------------------------------------------------------
std::ostream& operator<<(std::ostream &out, const DFAgent::ObservationType &obj)
{
  out << to_string(obj);
  return out;
}

//----------------------------------------------------------------------
// operator << DFAgent::ObservationClass
//----------------------------------------------------------------------
std::ostream& operator<<(std::ostream &out, const DFAgent::ObservationClass &obj)
{
  out << to_string(obj);
  return out;
}
