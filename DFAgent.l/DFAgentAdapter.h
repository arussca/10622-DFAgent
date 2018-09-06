// Copyright (c) 2004 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved

// $Log$

#ifndef _DFAgentAdapter_h_
#define _DFAgentAdapter_h_

// Includes
#include <IDL_Adapters_DGT.h>
#include <DFAGENTS.h>
#include <DeviceAdapter.h>
#include <DFAgent.h>

class DFAgent;


                               	   	   	   	   		   
									


//This class implements the IDL interface DFAgent_ifce
class DFAgentAdapter : public virtual POA_DFAGENT::DFAgent_ifce,
									public virtual POA_DF::RecipeResultConsumer_ifce,
									public DeviceAdapter
{
  //--------------------------------------------------
  // public section 
  //--------------------------------------------------
  public:
  
  // Constructor
  DFAgentAdapter(DFAgent* device);

  // Destructor
  ~DFAgentAdapter();


  //----------------------------------------------------------------------
  // Device specific operations
  //----------------------------------------------------------------------
  void   receiveFrame         (const DDPKF::Frame &, int, int, 
                               const DGT::TinyVector &, const DGT::TinyVector &, 
	                       const DGT::TimeValue &, const DGT::TimeValue &, const char * );
  void   receiveRecipeResult (const DDPKF::RecipeResult& result);

  void   setPrincipalInvestigatorName            (const char* piName);
  void   setInstrumentMode    (const char* insMode);
  void   setObservationType   (DFAGENT::ObservationType obsType);
  void   setObservationClass   (DFAGENT::ObservationClass obsClass);  
  void   setObserverName      (const char* observerName);
  void   setObjectName        (const char* objectName);
  char*  getObjectName        ();  
  void   setProgramId         (const char* programId);
  void   setESOProgramId         (const char* esoProgramId);  
  void   setObservationBlockId           (const char* obId);
  void   setImageTitle        (const char* imageTitle);
  void   setObservationComment(const char* observationComment);
  void   setObsModeLowLevel   (CORBA::Short obsMode);
  void 	 setObservationMode   (const char * obsMode);
  void   setPropagateFrame    (CORBA::Boolean doesPropagate);
  void   setSaveFrame         (CORBA::Boolean doesSave);
  void   setPropagateFITS     (CORBA::Boolean doesPropagate);
  void   setSaveFITS          (CORBA::Boolean doesSave);
  void   createArchiveFits    ();
  void   importFitsFile       ();
  void   commitArchiveFits    ();
  void   setAutoReduce        (CORBA::Boolean doesAutoReduce);
  void   setWriteKeywords        (CORBA::Boolean doesWriteKeywords);  
  void   setReductionRecipe   ();
  void   commitOR             ();

  //----------------------------------------------------------------------
  // Monitorable attributes (device magnitudes) 
  //----------------------------------------------------------------------
  CORBA::Boolean       doesSaveFrame     ();
  CORBA::Boolean       doesPropagateFrame();
  CORBA::Boolean       doesSaveFITS      ();
  CORBA::Boolean       doesPropagateFITS ();
  CORBA::Boolean       doesAutoReduce ();
  CORBA::Boolean       doesWriteKeywords ();  
  
  //----------------------------------------------------------------------
  // Configurable attributes (device properties) 
  //----------------------------------------------------------------------
  char*                offlineImagesPath         ();
  void                 offlineImagesPath         (const char* value); 

  char*                dfBranchInstanceName      ();
  void                 dfBranchInstanceName      (const char* value); 
  char*		       dfManagerInstanceName	 ();
  void                 dfManagerInstanceName     (const char* value); 
  char*                dfAgentPartnerInstanceName      ();
  void                 dfAgentPartnerInstanceName      (const char* value);   
  CORBA::Short         propagateFrame            ();
  void                 propagateFrame            (CORBA::Short value); 
  CORBA::Short         saveFrame                 ();
  void                 saveFrame                 (CORBA::Short value); 
  CORBA::Short         propagateFITS             ();
  void                 propagateFITS             (CORBA::Short value); 
  CORBA::Short         saveFITS                  ();
  void                 saveFITS                  (CORBA::Short value); 
  CORBA::Short         autoReduce                  ();
  void                 autoReduce                  (CORBA::Short value);   
  CORBA::Short         writeKeywords                  ();
  void                 writeKeywords                  (CORBA::Short value);     
  CORBA::Short         flippedImage                  ();
  void                 flippedImage                  (CORBA::Short value);       
  CORBA::Short         testingMode                  ();
  void                 testingMode                  (CORBA::Short value);         
  char*                obsModeKeywordListPath    ();
  void                 obsModeKeywordListPath    (const char* value); 
  char*                recipeDictionaryPath      ();
  void                 recipeDictionaryPath      (const char* value); 
  char*                gcsComponent              ();
  void                 gcsComponent              (const char* value); 
  char*                cameraName                ();
  void                 cameraName                (const char* value); 
  DGT::DoubleArray*    referencePixel();
  void                 referencePixel(const DGT::DoubleArray& value);  

  
  //-----------------------------------------------------------------
  // protected section 
  //-----------------------------------------------------------------
  protected:

  //--------------------------------------------------
  // private section 
  //--------------------------------------------------
  private:

  DFAgent* device_;
};

#endif // _DFAgentAdapter_h_ 

