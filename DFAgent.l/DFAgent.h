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
#include <fstream>
#include <vector>
#include <map>
#include <Frame.h>
#include <FrameC.h>
#include <DictionaryMap.h>
#include <DFBranchC.h>
#include <FITSHeaderInfo.h>
#include <MonitorManagerC.h>
#include <DFDataCollectorC.h>
#include <DFAGENTC.h>

#include "DFObservationStrategy.h"

//@epc

class FITSHeaderInfo;
class DFAgentTest;

/**
 *
 */
class DFAgent : public Device
{
 //--------------------------------------------------
 // public section
 //--------------------------------------------------
 public:

  //############################ PULBIC TYPES ############################

  /**
   *  Current Observation Type
   */
  enum ObservationType
  {
    BIAS, DARK, SKY_FLAT, DOME_FLAT, SPECTRAL_FLAT, ARC, SKY, OBJECTS
  };

  /**
   *  Current Observation Class
   */
  enum ObservationClass
  {
    SCIENCE, CALIB
  };

  /**
   * Alias declaration for the data type used as a result of obtaining a hash.
   */
  typedef long unsigned int Hash_t;

  //####################### CONSTRUCTOR & DESTRUCTOR #####################

  /** Device Constructor */
  DFAgent(std::string name);

  /** Destructor */
  virtual ~DFAgent();

  //########################## DEVICE METHODS ############################

  /** Connects the device with a CORBA and other GCS Services*/
  virtual void connect(std::string orbname, unsigned threads, long priority,
                       bool mode = Device::INTEGRATED,
                       bool update = Device::DO_NOT_UPDATE);

  /** Starting procedure */
  virtual void start();

  /** Initialization procedure */
  virtual void init();

  /** Halting procedure */
  virtual void halt();

  /** Shutdown procedure */
  virtual void shutdown();

  /** Reset procedure */
  virtual void reset();

  /** Restore procedure */
  virtual void restore();

  /** Test procedure */
  virtual void test();

  /** Stops all commands and goes to fault */
  virtual void abort();

  /** Report device info */
  virtual char * report(short level);

  /** Reports device version */
  const char * version();

  //####################### SPECIFIC METHODS #############################

  /**
   * Starts the process when a frame is received, it could be
   * redefined in the inherited class
   */
  virtual void receiveFrame(const DDPKF::Frame & idlNoKeywordsFrame, int currentWindow,
                            int numWindows, const DGT::TinyVector & idlTopLeft,
                            const DGT::TinyVector & idlBottomRight, const DGT::TimeValue & timestamp,
                            const DGT::TimeValue & exposureTime, const char * cameraId);

  virtual void receiveRecipeResult(std::string observingMode,
                                   unsigned long id,
                                   unsigned long obsResultId,
                                   DF::RecipeQualityControlType qc,
                                   const DDPKF::RecipeResult & result);


  /**
   * Sets the Principal Investigator name
   */
  virtual void setPrincipalInvestigatorName(const char * piName);

  /**
   * Sets Observation Type [BIAS, DARK, SKY_FLAT, DOME_FLAT, SPECTRAL_FLAT, ARC, SKY, OBJECTS]
   */
  virtual void setObservationType(ObservationType obsType);

  /**
   * Sets Observation Class [SCIENCE|CALIB]
   */
  virtual void setObservationClass(ObservationClass obsClass);

  /**
   * Sets the instrument mode
   */
  virtual void setInstrumentMode(const char * insMode);

  /**
   * Sets the observer name
   */
  virtual void setObserverName(const char * observerName);

  /**
   * Sets the name of the Object to be observed
   */
  virtual void setObjectName(const char * objectName);

  /**
   * Gets the name of the Object to be observed
   */
  std::string getObjectName();

  /**
   * Sets Program Identifier
   */
  virtual void setProgramId(const char * programId);

  /**
   * Sets ESO Program Identifier
   */
  virtual void setESOProgramId(const char * esoProgramId);

  /**
   * Sets Observation Block identifier
   */
  virtual void setObservationBlockId(const char * obId);

  /**
   * Sets Image Title
   */
  virtual void setImageTitle(const char * imageTitle);

  /**
   * Sets Observation Comment
   */
  virtual void setObservationComment(const char * observationComment);

  /**
   * Sets up observation mode
   * @param:obsMode Short associated with observation mode
   */
  void setObsModeLowLevel(short obsMode);

  /**
   * Sets up observation mode
   * @param:obsMode observation mode
   */
  virtual void setObservationMode(const char * obsMode);

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

  //####################### MONITORS #####################################

  /**
   * Current state of saveFrame flag
   * @magnitude:
   * @description:Save Frame flag
   */
  virtual bool doesSaveFrame();

  /**
   * Current state of propagateFrame flag
   * @magnitude:
   * @description:propagate Frame flag
   */
  virtual bool doesPropagateFrame();

  /**
   * Current state of saveFITS flag
   * @magnitude:
   * @description:Save FITS flag
   */
  virtual bool doesSaveFITS();

  /**
   * Current state of propagateFITS flag
   * @magnitude:
   * @description:propagate FITS flag
   */
  virtual bool doesPropagateFITS();

  /**
   * Current state of autoReduce flag
   * @magnitude:
   * @description:Auto Reduce flag
   */
  virtual bool doesAutoReduce();

  /**
   * Current state of write keywords flag
   * @magnitude:
   * @description:Write Keywords flag
   */
  virtual bool doesWriteKeywords();

  //####################### PROPERTIES ##################################

  /**
   * Path to directory where instrument places the image
   * @property:
   * @description:Path to directory where instrument places the image
   * @default:offlineDirectory
   */
  virtual const char * offlineImagesPath();
  virtual void offlineImagesPath(const char * new_value);

  /**
   * Instance name of the DF branch
   * @property:
   * @description:Instance name of the DF branch
   * @default:DF/DFBranch
   */
  virtual const char * dfBranchInstanceName();
  virtual void dfBranchInstanceName(const char * new_value);

  /**
   * Instance name of the DF Manager
   * @property:
   * @description:Instance name of the DF Manager
   * @default:DF/DFBranch
   */
  virtual const char * dfManagerInstanceName();
  virtual void dfManagerInstanceName(const char * new_value);

  /**
   * dfAgentPartnerInstanceName
   * @property:
   * @description: DFAgent partner instance Name
   * @default: DF/SFSNB_DFAgentPartner
   */
  virtual const char * dfAgentPartnerInstanceName();
  virtual void dfAgentPartnerInstanceName(const char * new_value);

  /**
   * Set propagateFrame to a default value
   * @property:
   * @description:State of propagate frame by default
   * @minimum:0
   * @maximum:1
   * @default:1
   */
  virtual short propagateFrame();
  virtual void propagateFrame(short new_value);

  /**
   * Set saveFrame to a default value
   * @property:
   * @description:State of save Frame by default
   * @minimum:0
   * @maximum:1
   * @default:1
   */
  virtual short saveFrame();
  virtual void saveFrame(short new_value);

  /**
   * Set propagateFITS to a default value
   * @property:
   * @description:State of propagate FITS by default
   * @minimum:0
   * @maximum:1
   * @default:1
   */
  virtual short propagateFITS();
  virtual void propagateFITS(short new_value);

  /**
   * Set saveFITS to a default value
   * @property:
   * @description:State of save FITS by default
   * @minimum:0
   * @maximum:1
   * @default:1
   */
  virtual short saveFITS();
  virtual void saveFITS(short new_value);

  /**
   * Set autoReduce to a default value
   * @property:
   * @description:State of autoReduce by default
   * @minimum:0
   * @maximum:1
   * @default:1
   */
  virtual short autoReduce();
  virtual void autoReduce(short new_value);

  /**
   * Set flippedImage to a default value
   * @property:
   * @description:State of flippedImage by default
   * @minimum:0
   * @maximum:1
   * @default:1
   */
  virtual short flippedImage();
  virtual void flippedImage(short new_value);

  /**
   * Set testingMode to a default value
   * @property:
   * @description:State of testingMode by default
   * @minimum:0
   * @maximum:1
   * @default:0
   */
  virtual short testingMode();
  virtual void testingMode(short new_value);

  /**
   * Set writeKeywords to a default value
   * @property:
   * @description:State of writeKeywords by default
   * @minimum:0
   * @maximum:1
   * @default:1
   */
  virtual short writeKeywords();
  virtual void writeKeywords(short new_value);

  /**
   * Array with paths to keyword list for each obs mode
   * @property:
   * @description:Array with paths to keyword list for each obs mode
   * @default:Path
   */
  virtual const char * obsModeKeywordListPath();
  virtual void obsModeKeywordListPath(const char * new_value);

  /**
   * Path to recipe dictionary
   * @property:
   * @description:Path to element dictionary between recipes
   * @default:dictionary
   */
  virtual const char * recipeDictionaryPath();
  virtual void recipeDictionaryPath(const char * new_value);

  /**
   * GCS Component name
   * @property:
   * @description:GCS Component name
   * @default:gcsComponent
   */
  virtual const char * gcsComponent();
  virtual void gcsComponent(const char * new_value);

  /**
   * cameraName
   * @property:
   * @description:Camera name
   * @default:cameraName
   */
  virtual const char * cameraName();
  virtual void cameraName(const char * new_value);

  /**
   * Reference Pixel for WCSParams
   * @property:
   * @description: Reference Pixel for WCSParams
   * @default:[512.0, 512.0]
   * @maximum:[1000.0, 1000.0]
   * @minimum:[1.0, 1.0]
   * @width:2
   */
  virtual const DoubleArray1D referencePixel();
  virtual void referencePixel(const DoubleArray1D & new_value);

 //-----------------------------------------------------------------
 // protected section
 //-----------------------------------------------------------------
 protected:

  //########################### PROTECTED TYPES ##########################

  /**
   * Alias declaration to use the ArrayIndex data type, which has different
   * declarations in jsoncpp lib for Linux and VxWorks.
   */
  #ifdef VXWORKS
  typedef Json::Value::ArrayIndex ArrayIndex;
  #elif LINUX
  typedef Json::ArrayIndex ArrayIndex;
  #endif

  /**
   * Alias declaration for data type used to make a relation between the hash
   * of observation mode and FITSHeaderInfo class.
   *
   * @related obsModeKeywordList_
   */
  typedef std::map<Hash_t, FITSHeaderInfo> ObsModeKeywordList_t;

  /**
   * Alias declaration for data type used to make a relation between the name
   * of observation mode and its hash code.
   *
   * @related obsModeHash_
   */
  typedef std::map<std::string, Hash_t> ObsModeHash_t;

  /**
   * Alias declaration for the data type used to define the fixed keywords that
   * must contain the file (in JSON), where all FITS keyowrds are defined for
   * each observation mode.
   */
  typedef std::set<std::string> JsonKeywordsList_t;

  /**
   * Alias declaration for associating each observation mode hash with
   * its name, type or class.
   * @related obsModeName_
   * @related obsTypeName_
   * @related obsClassName_
   */
  typedef map<Hash_t, std::string> ObsModeHashName_t;

  /**
   * Alias declaration for associating each observation mode hash with
   * its settings. Rigth now the number of elements is 6 and this
   * is their meaning:
   * - Element 0 represents the value applied for saveFrame property.
   * - Element 1 represents the value applied for saveFits property.
   * - Element 2 represents the value applied for propagateFrame property.
   * - Element 3 represents the value applied for propagateFits property.
   * - Element 4 represents the value applied for autoReduce property.
   * - Element 5 represents the value applied for writeKeywords property.
   *
   * @related obsModeSettings_
   */
  typedef map<Hash_t, std::vector<bool> > ObsModeSettings_t;

  /**
   * Represents the fixed keywords of 1st level, which must be in file (in JSON)
   * used to specify FITS Header keywords for each observation mode.
   */
  static
  #if defined(__cplusplus) && (__cplusplus >= 201103L)
         const
  #endif
               JsonKeywordsList_t jsonFixedKeywordLevel1_;

  /**
   * Represents the fixed keywords of 2nd level, which must be in file (in JSON)
   * used to specify FITS Header keywords for each observation mode.
   */
  static
  #if defined(__cplusplus) && (__cplusplus >= 201103L)
         const
  #endif
               JsonKeywordsList_t jsonFixedKeywordLevel2_;

  //################# PROTECTED STATIC FUNCTIONS MEMBERS #################

  /**
   * Computes the hash for the given string value.
   *
   * @param[in] obj Given value for computing its hash.
   * @return Computed hash of the given value.
   *
   * @attention Non-template function member.
   */
  static const Hash_t hash(const std::string & obj) /*noexcept*/;

  #if defined(__cplusplus) && (__cplusplus >= 201103L)
  /**
   * Computes the hash for the given value of different data types.
   *
   * @tparam T Data type to compute a hash for the given value.
   * @param[in] obj Given value for computing its hash.
   * @return Computed hash of the given value.
   *
   * @attention The allowed data type used as template are:
   *   - any enum (only available in C++11)
   *   - int short (only available in C++11)
   *   - int (only available in C++11)
   *   - int long (only available in C++11)
   *   - int long long (only available in C++11)
   *   - unsigned int short (only available in C++11)
   *   - unsigned int (only available in C++11)
   *   - unsigned int long (only available in C++11)
   *   - unsigned int long long (only available in C++11)
   */
  template<typename T>
  static const Hash_t hash(const T& obj) /*noexcept*/;
  #endif // __cplusplus >= 201103L

  //####################### PROTECTED DATA MEMBERS #######################

  // Monitorable attributes (magnitudes)
  bool doesSaveFrame_     ; ///< Current state of saveFrame flag
  bool doesPropagateFrame_; ///< Current state of propagateFrame flag
  bool doesSaveFITS_      ; ///< Current state of saveFITS flag
  bool doesPropagateFITS_ ; ///< Current state of propagateFITS flag
  bool doesAutoReduce_    ; ///< Current state of autoReduce flag
  bool doesWriteKeywords_ ; ///< Current state of writeKeywords flag

  // Configurable attributes (properties)
  String offlineImagesPath_         ; ///<Path to directory where instrument places the image
  String dfBranchInstanceName_      ; ///<Instance name of the DF branch
  String dfManagerInstanceName_     ; ///<Instance name of the DF Manager
  String obsModeKeywordListPath_    ; ///<Array with paths to keyword list for each obs mode
  String recipeDictionaryPath_      ; ///<Path to recipe dictionary
  String gcsComponent_              ; ///<GCS Component name
  String cameraName_                ; ///<cameraName
  String dfAgentPartnerInstanceName_; ///<Instance name of the agent partner
  bool flippedImage_                ; ///<State of flipped image flag
  bool testingMode_                 ; ///<State of testing Mode flag

  // Methods to create, start and stop monitors
  void createMonitors_();


  // Method to create Devices
  void createDevices_(bool update);

  //@ipc
  //custom member variables
  std::string imageTitle_               ; ///< Image Title
  std::string objectName_               ; /// <Observed Object Name
  std::string observationComment_       ; ///< Comment about observation
  std::string observationBlockId_       ; ///< Observation Block Id
  std::string principalInvestigatorName_; ///< Principal investigator Name
  std::string instrumentMode_           ; ///< Instrument Mode
  std::string observerName_             ; ///< Observer name
  std::string programId_                ; ///< Program Id
  std::string esoProgramId_             ; ///< ESO Program Id
  ObservationType obsType_              ; ///< Current Observation Type
  ObservationClass obsClass_            ; ///< Current Observation Class
  TinyVector<double, 2> referencePixel_ ; ///< Reference Pixel for WCS params

  /**
   * Short variable with the running observation mode.
   *
   * For those derived DFAgents, which use DFAgent::setObsModeLowLevel (old style)
   * method instead of DFAgent::setObservationMode (new style), then the value of
   * this member belongs to range of numbers defined by the Observation mode enumeration.
   *
   * For those derived DFAgents, which use DFAgent::setObservationMode (new style)
   * method instead of DFAgent::setObsModeLowLevel (old style), then the value of
   * this member will be the hash of the current observation mode name.
   *
   * @related dfSessionId_
   *
   * @note
   * - This member must change when the member dfSessionId_ changes.
   */
  Hash_t shortObsMode_;
  Frame<double, 2> * rawFrame_                          ; ///< raw frame acquired by the instrument
  DF::Branch_ifce_var dfBranchInstance_                 ; ///< Pointer to data factory branch instance in corba
  DFAGENT::DFAgent_ifce_var dfAgentPartnerInstance_     ; ///< Pointer to an auxiliar agent, which helps in coxplex tasks
  DF::DataAcquisitionManager_ifce_var dfManagerInstance_; ///< Pointer to data factory manager instance in corba
  DFAGENT::DFAgent_ifce * consumerId_                   ; ///<

  ObsModeHashName_t obsModeName_          ; ///< Map with string associated with each obsMode
  // [[deprecated]] #ToDo:
  //  - This member can be removed safely only if derived classes don't use it.
  ObsModeHashName_t obsTypeName_          ; ///< Map with string associated with each obsType [BIAS, DARK, SKY_FLAT, DOME_FLAT, SPECTRAL_FLAT, ARC, SKY, OBJECTS]
  // [[deprecated]] #ToDo:
  //  - This member can be removed safely only if derived classes don't use it.
  ObsModeHashName_t obsClassName_         ; ///< Map with string associated with each obsClass [SCIENCE|CALIB]
  ObsModeSettings_t obsModeSettings_      ; ///< Map with settings for each obsMode
  long dfSessionId_                       ; ///< Long number with the session id currently opened.
  vector<std::string> recipeElementNames_ ; ///< List with element names used in a recipe.
  DictionaryMap recipeDictionary_         ; ///< Equivalences dictionary between recipe elements and standard agent elements
  ObsModeKeywordList_t obsModeKeywordList_; ///< Keyword info list associated with every ObsMode
  bool isSessionRunning_                  ; ///< The flag show if there is a session running, if so close before open a new one
  bool multiFrameRecipe_                  ; ///< indicates whether the recipe requires a single or multiple frame when 'receiveFrame'

  //receiveFrame function arguments
  int currentWindow_              ; ///< current window number
  int numWindows_                 ; ///< Number of windows
  DGT::TinyVector idlTopLeft_     ; ///< Tiny vector with the top left corner coordenates
  DGT::TinyVector idlBottomRight_ ; ///< Tiny vector with the bottom right corner coordenates
  DGT::TimeValue timestamp_       ; ///< Time with the timestamp value
  DGT::TimeValue exposureTime_    ; ///< Time with the exposure time value
  std::string cameraId_           ; ///< string which carries the cameraID

  //custom member function
  virtual DDPKF::DataElement * getWithFrameRecipeElement_(const string & elementName);///< create DataElement object from the name of the element
  virtual DDPKF::DataElement * getBeforeFrameRecipeElement_(const string & elementName);///< create DataElement object from the name of the element
  // formerly dataElementFromElementName_
  void sendElement_(string elementRecipeName, bool withFrame);  ///< Send the element to DataFactory
  void sendElement_(string elementRecipeName);  ///< Send the element to DataFactory
  void initDFBranchInstance_(bool init_obsmode_lowlevel = true);    ///< initilize the DFBranchInstance object, connect to the Branch
  void initDFManagerInstance_();    ///< initilize the DFManagerInstance object, connect to the DFManager
  void initDFAgentPartnerInstance_();  ///< Initialize pointer and connect to DFAgentPartner
  virtual void writeStandardKeywords_(DataDescription & frame);  ///< Write standar keywords in Frame object
  virtual void applyObsModeSettings_(const std::vector<bool> & settings);  ///< Stores settings values in the local variables
  virtual void writeWCSKeywords_(DataDescription & frame, TinyVector<unsigned long, 2> & frameSize);  ///< Write WCS keywords in Frame object
  virtual void writeAdditionalKeywords_(DataDescription & frame);  ///< Write additional keywords in Frame object
  virtual void fetchORInfo_();  ///< Fetch Observation result info, and element names for current observation mode
  bool belongsToRecipe_(const string & elementName);  ///< Indicates if the element name is used in the recipe currently used

  void receiveFrameObsStrategy(const DDPKF::Frame & idlFrame,
                               int currentWindow,
                               int numWindows,
                               const DGT::TinyVector & idlTopLeft,
                               const DGT::TinyVector & idlBottomRight,
                               const DGT::TimeValue & timestamp,
                               const DGT::TimeValue & exposureTime,
                               bool propagate,
                               bool save,
                               bool raw_frame);

  DF::DFObservationStrategyList observation_strategies_;
  std::vector<std::string> getAvailableObsMode_();
  void unsubscribeToReductionResult_(const char * recipe_name);
  void subscribeToReductionResult_(const char * recipe_name);
  void configureObsMode_(const char * obsMode, long observation_strategy_session);

  /**
   * Parse the file where all the keywords for each observation mode are defined
   * and fill properly the internal members.
   *
   * @pre The properties must have been loaded before.
   *
   * @param[in] filePath File where all the keywords for each observation
   *                     mode are defined. An empty value means the value of
   *                     obsModeKeywordListPath_ member will be used as parameter.
   *
   * @related obsModeKeywordListPath_
   *
   * @note
   * This method invokes:
   *   DFAgent::parseFITSHeaderKeywordsFromPlainTextFile_(const std::string) method or
   *   DFAgent::parseFITSHeaderKeywordsFromJSONFile_(const std::string) method,
   * depending on the file name extension.
   *
   * @attention
   * This method must be invoked by derived classes because it is the new responsible
   * for filling those internal members, which will be used to collect all keywords for
   * each observation mode, instead of void DFAgent::obsModeKeywordListPath(const char*)
   * method.
   *
   */
  virtual void parseFITSHeaderKeywordsFromFile_(std::string filePath = "");

  /**
   * Parse the file (whose format is plain text - old format) where all the keywords
   * for each observation mode are defined and fill properly the internal members.
   *
   * @pre The properties must have been loaded before.
   * @pre The member obsModeName_ must be correctly filled in by those derived classes,
   *      which use DFAgent::setObsModeLowLevel (old DFAgents) method instead of
   *      DFAgent::setObservationMode (new DFAgents).
   *
   * @param[in] filePath File where all the keywords for each observation
   *                     mode are defined, following the legacy format.
   *
   * @related obsModeKeywordList_
   * @related obsModeHash_
   * @related obsModeName_
   * @related DFAgent::setObsModeLowLevel
   * @related DFAgent::setObservationMode
   *
   * @attention
   * The old format assumes that:
   * - The line number must match with the hash of each observation mode.
   * - Each line in the given file path contains a path, which points to the
   *   desired file where all keywords are defined.
   */
  virtual void parseFITSHeaderKeywordsFromPlainTextFile_(const std::string & filePath);

  /**
   * Parse the file (whose format must be JSON) where all the keywords for
   * each observation mode are defined and fill properly the internal members.
   *
   * @pre The properties must have been loaded before.
   * @pre The connection to DFBranch must be feasible.
   *
   * @param[in] filePath File where all the keywords for each observation
   *                     mode are defined, following the new format (JSON).
   *
   * @related obsModeKeywordList_
   * @related obsModeHash_
   * @related obsModeName_
   *
   */
  virtual void parseFITSHeaderKeywordsFromJSONFile_(const std::string & filePath);

  /**
   * Dedicated mutex to protect a critical region related to the management of the data
   * structure where all nested recipes are contained.
   *
   * @attention
   *  - The mutex, called deviceGuardMutex_, which is defined at Device.h  shouldn't be used
   *    for protecting critical regions, which are related to the management of the data structure,
   *    because:
   *    - This mutex is commonly used by developers to protect the critical region of each command.
   *    - If the deviceGuardMutex_ mutex is used in DFAgent::receiveFrameObsStrategy
   *      method - for instance - then an dead-lock situation will happen when an derived DFAgent,
   *      which also uses the deviceGuardMutex_ mutex to avoid others pre-processing process can be
   *      performed and this invokes the DFAgent::receiveFrameObsStrategy method.
   *
   * @related Device::deviceGuardMutex_
   * @related DFAgent::setObservationMode
   * @related DFAgent::receiveFrameObsStrategy
   * @related DFAgent::receiveRecipeResult
   */
  ACE_Thread_Mutex dfagentGuardMutex_;

  /**
   * Associative container that contains the hash for each observation mode name.
   *
   * It is used to assign a valid value to shortObsMode_ member based on
   * observation mode name when a derived DFAgent class invokes
   * DFAgent::setObservationMode (new DFAgents) instead of
   * DFAgent::setObsModeLowLevel (old DFAgents).
   *
   * @related shortObsMode_
   * @related DFAgent::setObservationMode
   * @related DFAgent::setObsModeLowLevel
   */
  ObsModeHash_t obsModeHash_;

  /**
   * Obtains the monitor data from the database of each specified keyword
   * and writes them into the given FITS Header instance.
   *
   * @param[in]  time    Start time of the exposure.
   * @param[in]  expTime Duration time of the exposure.
   * @param[out] header  FITS Header instance where all specified keywords
   *                     must be written.
   * @return             Indicates if all keywords could be written in the
   *                     header or not.
   */
  virtual bool gatherKeywords_(const TimeValue & time,
                               const TimeValue & expTime,
                               DataDescription & header);

  /**
   * Writes the keywords with fixed values into the given FITS Header instance.
   *
   * @param[out] header  FITS Header instance where all keywords with fixed
   *                     values must be written.
   * @return             Indicates if all keywords could be written in the
   *                     header or not.
   */
  virtual bool gatherFixedKeywords_(DataDescription & header);

  /**
   * Obtains the monitor data from the database of each specified keyword
   * and writes them into the given FITS Header instance.
   *
   * @param[in]  time    Start time of the exposure.
   * @param[in]  expTime Duration time of the exposure.
   * @param[out] header  FITS Header instance where all specified keywords
   *                     must be written.
   * @return             Indicates if all keywords could be written in the
   *                     header or not.
   */
  virtual bool gatherMonitorKeywords_(const TimeValue & time,
                                      const TimeValue & expTime,
                                      DataDescription & header);

  /**
   * Find the name of the first leaf node (observation mode), which belongs to
   * the first observation strategy that is not completed for acquisition yet.
   *
   * @throws NoEntries This exception is thrown when ...
   *
   *
   * @return Name of the first leaf node of first incomplete
   *         observation strategy.
   */
  const char * getNameOf1stAcquiringObsStrategyNode_();

  /**
   * Find the hash of the first leaf node (observation mode), which belongs to
   * the first observation strategy that is not completed for acquisition yet.
   *
   * @throws NoEntries This exception is thrown when ...
   *
   * @return Hash number of the first leaf node of first incomplete
   *         observation strategy.
   */
  Hash_t getHashOf1stAcquiringObsStrategyNode_();

 //-----------------------------------------------------------------
 // private section
 //-----------------------------------------------------------------
 //##### very important change, all private variables are turned into protected!!
 // in order to inherit them in the derived class
 private:

  //############################ PRIVATE TYPES ###########################

  /**
   * Forward class declaration, which will be reserved to
   * perform the unit test.
   */
  friend class DFAgentTest;

//@epc
};

/**
 * It returns the string representation of given value of DFAgent::ObservationType
 * enumeration type if it exists or it throws an exception if the given value is not
 * allowed.
 *
 * @param[in] obj Value of DFAgent::ObservationType enumeration type to be
 *                returned as string.
 * @return String representation of the given value of DFAgent::ObservationType
 *         enumeration type
 */
const char * to_string(const DFAgent::ObservationType & obj);

/**
 * It returns the string representation of given value of DFAgent::ObservationClass
 * enumeration type if it exists or it throws an exception if the given value is not
 * allowed.
 *
 * @param[in] obj Value of DFAgent::ObservationClass enumeration type to be
 *                returned as string.
 * @return String representation of the given value of DFAgent::ObservationClass
 *         enumeration type
 */
const char * to_string(const DFAgent::ObservationClass & obj);

/**
 * Sets a value of DFAgent::ObservationType enumeration type,
 * which matches with the given string, if it exists or it throws an
 * exception if the given string doesn't match.
 *
 * @param[in]  str The string representation of a value of
 *                 DFAgent::ObservationType enumeration type.
 * @param[out] obj DFAgent::ObservationType enumeration type,
 *                 where the string representation is set.
 */
void to_string(const std::string & str, DFAgent::ObservationType & obj);

/**
 * Sets a value of DFAgent::ObservationClass enumeration type,
 * which matches with the given string, if it exists or it throws an
 * exception if the given string doesn't match.
 *
 * @param[in]  str The string representation of a value of
 *                 DFAgent::ObservationClass enumeration type.
 * @param[out] obj DFAgent::ObservationClass enumeration type,
 *                 where the string representation is set.
 */
void to_string(const std::string & str, DFAgent::ObservationClass & obj);

/**
 * It appends to the output stream object the representation of given value of
 * DFAgent::ObservationType enumeration type.
 *
 * @param[in,out] out Out stream object where the representation of the given
 *                    value of DFAgent::ObservationType enumeration type will be
 *                    appended.
 * @param[in] obj     Value of DFAgent::ObservationType enumeration type to be
 *                    appened in the out stream object.
 * @return Out stream object where the representation of the given
 *         value of DFAgent::ObservationType enumeration type was appended.
 */
std::ostream & operator<<(std::ostream & out, const DFAgent::ObservationType & obj);

/**
 * It appends to the output stream object the representation of given value of
 * DFAgent::ObservationClass enumeration type.
 *
 * @param[in,out] out Out stream object where the representation of the given
 *                    value of DFAgent::ObservationClass enumeration type will be
 *                    appended.
 * @param[in] obj     Value of DFAgent::ObservationClass enumeration type to be
 *                    appened in the out stream object.
 * @return Out stream object where the representation of the given
 *         value of DFAgent::ObservationClass enumeration type was appended.
 */
std::ostream & operator<<(std::ostream & out, const DFAgent::ObservationClass & obj);

#endif // _DFAgent_h_

