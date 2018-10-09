// Copyright (c) 2004 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved

// $Log$

#ifndef _DFAgentConfigAgent_h_
#define _DFAgentConfigAgent_h_

// Includes
#include <IDL_Adapters_DGT.h>
#include <DFAgent.h>
#include <Trace.h>
#include <ConfigAgent.h>

class DFAgent;

// Class used to configure DFAgentConfigAgent objects
class DFAgentConfigAgent : public ConfigAgent
{
	//--------------------------------------------------
	// public section
	//--------------------------------------------------
public:

	// Constructor
	DFAgentConfigAgent(DFAgent* device, const int mode, TimerEngine* timer);

	// Destructor
	virtual ~DFAgentConfigAgent();

	// This method applies the stored configuration to the Device
	virtual void applyProperties();

	// This method obtains the properties from the Device
	virtual int readProperties();

	// Converts the Device properties to a Configuration
	virtual void toConfiguration(CONFIG::Configuration& c);

	// Converts a Configuration to the Device format
	virtual void fromConfiguration(const CONFIG::Configuration& c);


	//-----------------------------------------------------------------
	// protected section
	//-----------------------------------------------------------------

protected:
	Trace trace_;

	DFAgent* device_;

	//
	// Configurable attributes (properties)
	//
	String               offlineImagesPath_  ;
	String               dfBranchInstanceName_ ;
	String               dfAgentPartnerInstanceName_ ;
	String 	       dfManagerInstanceName_;
	short                propagateFrame_     ;
	short                saveFrame_          ;
	short                propagateFITS_      ;
	short                saveFITS_           ;
	short                autoReduce_           ;
	short                writeKeywords_           ;
	short                flippedImage_           ;
	short                testingMode_           ;
	String               obsModeKeywordListPath_ ;
	String               recipeDictionaryPath_ ;
	String               gcsComponent_       ;
	String               cameraName_         ;
	DoubleArray1D        referencePixel_     ;
};

#endif // _DFAgentConfigAgent_h_
