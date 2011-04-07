// Copyright (c) 2004 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved

// $Log$

#ifndef _DFAgent_h_
#define _DFAgent_h_

// Includes
#include <IDL_Adapters_DGT.h>
#include <Device.h>
#include <ConcreteArrays.h>

//@ipc
#include <vector>
#include <map>
#include <Frame.h>
#include <FrameC.h>
#include <DictionaryMap.h>
#include <DFBranchC.h>
#include <FITSHeaderInfo.h>
#include <MONITORManagerC.h>  // for MONITOR::Consumer_ifce
#include <MONITORC.h>
#include <DFDataCollectorC.h>
#include <DFAGENTC.h>
//@epc

/**
* 
*/
class DFAgent : public Device
{
  //--------------------------------------------------
  // public section 
  //--------------------------------------------------
  public:

  /**
   *  Current Observation Mode
   */
  enum ObservationType
  {
    SCIENCE, CALIB
  };


  //####################### CONSTRUCTOR & DESTRUCTOR #####################
  
  /** Device Constructor */
  DFAgent(string name);

  /** Destructor */
  virtual ~DFAgent();

  //########################## DEVICE METHODS ############################
  
  /** Connects the device with a CORBA and other GCS Services*/
  void  connect(string orbname, unsigned threads,long priority,
                bool mode  =Device::INTEGRATED,
                bool update=Device::DO_NOT_UPDATE);
  
  /** Starting procedure */
  void  start();
  
  /** Initialization procedure */
  void  init();
  
  /** Halting procedure */
  void  halt();
  
  /** Shutdown procedure */
  void  shutdown();
  
  /** Reset procedure */
  void  reset();
  
  /** Restore procedure */
  void  restore();
  
  /** Test procedure */
  void  test();
  
  /** Stops all commands and goes to fault */
  void  abort();
  
  /** Report device info */
  char* report(short level);
  
  /** Reports device version */
  const char* version();
  
  //####################### SPECIFIC METHODS #############################

  /**
  * Starts the process when a frame is received, it could be
  * redefined in the inherited class
  */ 
  virtual void receiveFrame( const DDPKF::Frame& idlNoKeywordsFrame, int currentWindow, 
                             int numWindows, const DGT::TinyVector& idlTopLeft, 
                             const DGT::TinyVector& idlBottomRight, const DGT::TimeValue& timestamp, 
                             const DGT::TimeValue& exposureTime, const char* cameraId );

  /**
  * Sets the Principal Investigator name
  */
  void setPrincipalInvestigatorName(const char* piName);

  /**
  * Sets Observation Type [SCIENCE|CALIB]
  */
  void setObservationType(ObservationType obsType);

  /**
  * Sets the instrument mode
  */
  void setInstrumentMode(const char* insMode);

  /**
  * Sets the observer name
  */
  void setObserverName(const char* observerName);

  /**
  * Sets the name of the Object to be observed
  */
  void setObjectName(const char* objectName);

  /**
  * Sets Program Identifier
  */
  void setProgramId(const char* programId);

  /**
  * Sets Observation Block identifier
  */
  void setObservationBlockId(const char* obId);

  /**
  * Sets Image Title
  */
  void setImageTitle(const char* imageTitle);

  /**
  * Sets Observation Comment
  */
  void setObservationComment(const char* observationComment);

  /**
  * Sets up observation mode
  * @param:obsMode Short associated with observation mode
  */
  void setObsModeLowLevel(short obsMode);

  /**
  * Sets frame propagate flag state
  */
  void setPropagateFrame(bool doesPropagate);

  /**
  * Sets frame save flag state
  */
  void setSaveFrame(bool doesSave);

  /**
  * Sets FITS propagate flag state
  */
  void setPropagateFITS(bool doesPropagate);

  /**
  * Sets FITS save flag state
  */
  void setSaveFITS(bool doesSave);

  /**
  * Sets auto reduce flag state
  */
  void setAutoReduce(bool doesAuto);

  /**
  * Sets write keywords flag state
  */
  void setWriteKeywords(bool doesWrite);

  /**
  * Creates archive fits
  */
  void createArchiveFits();

  /**
  * Imports fits file
  */
  void importFitsFile();

  /**
  * Commits Archive file
  */
  void commitArchiveFits();

  /**
  * Sets reduction recipe
  */
  void setReductionRecipe();

  /**
  * Commits Observation Result
  */
  void commitOR();

  /**
  * Receive Data Blocks (from MONITOR::Consumer_ifce)
  */
  void receiveDataBlocks (const MONITOR::DataBlockList &msgs);

  /**
  * Receive State Changes (from MONITOR::Consumer_ifce)
  */
  void receiveStateChanges (const MONITOR::StateChangeList &msgs);

  /**
  * Receive Magnitude Changes (from MONITOR::Consumer_ifce)
  */
  void receiveMagnitudeChanges (const MONITOR::MagnitudeChangeList &msgs);
  
  //####################### MONITORS #####################################

  /**
  * Current state of saveFrame flag
  * @magnitude:
  * @description:Save Frame flag
  */
  bool doesSaveFrame();

  /**
  * Current state of propagateFrame flag
  * @magnitude:
  * @description:propagate Frame flag
  */
  bool doesPropagateFrame();

  /**
  * Current state of saveFITS flag
  * @magnitude:
  * @description:Save FITS flag
  */
  bool doesSaveFITS();

  /**
  * Current state of propagateFITS flag
  * @magnitude:
  * @description:propagate FITS flag
  */
  bool doesPropagateFITS();
  
  /**
  * Current state of autoReduce flag
  * @magnitude:
  * @description:Auto Reduce flag
  */
  bool doesAutoReduce();

  /**
  * Current state of write keywords flag
  * @magnitude:
  * @description:Write Keywords flag
  */
  bool doesWriteKeywords();
  
  //####################### PROPERTIES ##################################

  /**
  * Path to directory where instrument places the image
  * @property:
  * @description:Path to directory where instrument places the image
  * @default:offlineDirectory
  */
  const char* offlineImagesPath();
  void offlineImagesPath(const char* new_value); 

  /**
  * Instance name of the Monitor Manager
  * @property:
  * @description:Instance name of the Monitor Manager
  * @default:MonitorManager
  */
  const char* monitorManagerInstanceName();
  void monitorManagerInstanceName(const char* new_value); 

  /**
  * Instance name of the DF branch
  * @property:
  * @description:Instance name of the DF branch
  * @default:DF/DFBranch
  */
  const char* dfBranchInstanceName();
  void dfBranchInstanceName(const char* new_value); 
  
  /**
  * dfAgentPartnerInstanceName
  * @property:
  * @description: DFAgent partner instance Name
  * @default: DF/SFSNB_DFAgentPartner
  */
  const char* dfAgentPartnerInstanceName();
  void dfAgentPartnerInstanceName(const char* new_value); 
  
  /**
  * Set propagateFrame to a default value
  * @property:
  * @description:State of propagate frame by default
  * @minimum:0
  * @maximum:1
  * @default:1
  */
  short propagateFrame();
  void propagateFrame(short new_value); 

  /**
  * Set saveFrame to a default value
  * @property:
  * @description:State of save Frame by default
  * @minimum:0
  * @maximum:1
  * @default:1
  */
  short saveFrame();
  void saveFrame(short new_value); 

  /**
  * Set propagateFITS to a default value
  * @property:
  * @description:State of propagate FITS by default
  * @minimum:0
  * @maximum:1
  * @default:1
  */
  short propagateFITS();
  void propagateFITS(short new_value); 

  /**
  * Set saveFITS to a default value
  * @property:
  * @description:State of save FITS by default
  * @minimum:0
  * @maximum:1
  * @default:1
  */
  short saveFITS();
  void saveFITS(short new_value); 

  /**
  * Set autoReduce to a default value
  * @property:
  * @description:State of autoReduce by default
  * @minimum:0
  * @maximum:1
  * @default:1
  */
  short autoReduce();
  void autoReduce(short new_value); 

  /**
  * Set writeKeywords to a default value
  * @property:
  * @description:State of writeKeywords by default
  * @minimum:0
  * @maximum:1
  * @default:1
  */
  short writeKeywords();
  void writeKeywords(short new_value); 

  /**
  * Array with paths to keyword list for each obs mode
  * @property:
  * @description:Array with paths to keyword list for each obs mode
  * @default:Path
  */
  const char* obsModeKeywordListPath();
  void obsModeKeywordListPath(const char* new_value); 

  /**
  * Path to recipe dictionary
  * @property:
  * @description:Path to element dictionary between recipes
  * @default:dictionary
  */
  const char* recipeDictionaryPath();
  void recipeDictionaryPath(const char* new_value); 

  /**
  * GCS Component name
  * @property:
  * @description:GCS Component name
  * @default:gcsComponent
  */
  const char* gcsComponent();
  void gcsComponent(const char* new_value); 

  /**
  * cameraName
  * @property:
  * @description:Camera name
  * @default:cameraName
  */
  const char* cameraName();
  void cameraName(const char* new_value); 
  
  //-----------------------------------------------------------------
  // protected section 
  //-----------------------------------------------------------------
  protected:
  
  //-----------------------------------------------------------------
  // private section 
  //-----------------------------------------------------------------
  //##### very important change, all private variables are turned into protected!!
  // in order to inherit them in the derived class
//  private:
  
  // Monitorable attributes (magnitudes)
  bool doesSaveFrame_     ; ///< Current state of saveFrame flag
  bool doesPropagateFrame_; ///< Current state of propagateFrame flag
  bool doesSaveFITS_      ; ///< Current state of saveFITS flag
  bool doesPropagateFITS_ ; ///< Current state of propagateFITS flag
  bool doesAutoReduce_    ; ///< Current state of autoReduce flag
  bool doesWriteKeywords_ ; ///< Current state of writeKeywords flag
  
  // Configurable attributes (properties)
  String offlineImagesPath_         ; ///<Path to directory where instrument places the image
  String monitorManagerInstanceName_; ///<Instance name of the Monitor Manager
  String dfBranchInstanceName_      ; ///<Instance name of the DF branch
  String obsModeKeywordListPath_    ; ///<Array with paths to keyword list for each obs mode
  String recipeDictionaryPath_      ; ///<Path to recipe dictionary
  String gcsComponent_              ; ///<GCS Component name
  String cameraName_                ; ///<cameraName
  String dfAgentPartnerInstanceName_; ///<Instance name of the agent partner
  
  // MonitorAgent
  MonitorAgent<DFAgent>* DFAgentMonitorAgent_;
  
  // Methods to create, start and stop monitors
  void createMonitors_();
  void startMonitors_();
  void stopMonitors_();
  
  // Method to create Devices
  void createDevices_(bool update);
  
  //@ipc
  //custom member variables
  string imageTitle_; ///< Image Title
  string objectName_;  /// <Observed Object Name
  string observationComment_;  ///< Comment about observation
  string observationBlockId_;   ///< Observation Block Id
  string principalInvestigatorName_; ///< Principal investigator Name
  string instrumentMode_; ///< Instrument Mode
  string observerName_; ///< Observer name
  string programId_; ///< Program Id
  ObservationType obsType_; ///< Current Observation Type
  unsigned char monitorManagerSubscriptionID_;  ///< identifier for monitor manager subscription session
  
  short shortObsMode_;   ///< Short variable with the running obs mode, 
  Frame<double,2> *rawFrame_;   ///< raw frame acquired by the instrument
  DF::Branch_ifce_var dfBranchInstance_;     ///< Pointer to data factory branch instance in corba
  DFAGENT::DFAgent_ifce_var dfAgentPartnerInstance_;  ///< Pointer to an auxiliar agent, which helps in coxplex tasks
  MONITOR::Manager_ifce_var monitorManagerInstance_;   ///< Pointer to Obs engine instance in corba  
  map<short, std::string> obsModeName_; ///< Map with string associated with each obsMode
  map<short, std::string> obsTypeName_; ///< Map with string associated with each obsType [SCIENCE|CALIB]
  map<short, std::vector<bool> > obsModeSettings_; ///< Map with settings for each obsMode
  long dfSessionId_; ///< Long number with the session id currently opened.
  vector<string> recipeElementNames_; ///< List with element names used in a recipe.
  DictionaryMap recipeDictionary_;  ///< Equivalences dictionary between recipe elements and standard agent elements
  vector<FITSHeaderInfo> obsModeKeywordList_; ///< Keyword info list associated with every ObsMode
  bool isSessionRunning_;  ///< The flag show if there is a session running, if so close before open a new one
  bool multiFrameRecipe_; ///< indicates whether the recipe requires a single or multiple frame when 'receiveFrame'

  //receiveFrame function arguments
  int currentWindow_;      ///< current window number
  int numWindows_;         ///< Number of windows
  DGT::TinyVector idlTopLeft_;  ///< Tiny vector with the top left corner coordenates
  DGT::TinyVector idlBottomRight_; ///< Tiny vector with the bottom right corner coordenates
  DGT::TimeValue timestamp_;    ///< Time with the timestamp value
  DGT::TimeValue exposureTime_;  ///< Time with the exposure time value
  string cameraId_;         ///< string which carries the cameraID
			     
  //custom member function
  virtual DDPKF::DataElement *dataElementFromElementName_(const string & elementName);///< create DataElement object from the name of the element
  void scanDirectory_(vector<pair<time_t, string> > &timeFileList); ///< Function that search the directory looking for fits files
  void sendElement_(string elementRecipeName);  ///< Send the element to DataFactory
  void initDFBranchInstance_();    ///< initilize the DFBranchInstance object, connect to the Branch
  void initDFAgentPartnerInstance_();  ///< Initialize pointer and connect to DFAgentPartner
  void initMonitorManagerInstance_();  ///< Initilize pointer and connect to MonitorManager
  void writeStandardKeywords_(Frame<double,2>& frame);  ///< Write standar keywords in Frame object
  virtual void applyObsModeSettings_(const std::vector<bool> & settings);  ///< Stores settings values in the local variables
  virtual void writeAdditionalKeywords_(Frame<double,2>& frame);  ///< Write additional keywords in Frame object
  virtual void fetchORInfo_();  ///< Fetch Observation result info, and element names for current observation mode
  bool belongsToRecipe_(const string& elementName);  ///< Indicates if the element name is used in the recipe currently used
  //@epc
};

#endif // _DFAgent_h_ 

