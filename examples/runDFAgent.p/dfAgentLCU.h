// Copyright (c) 2004 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved

// $Log$

#ifndef _dfAgentLCU_h_
#define _dfAgentLCU_h_

// GCS Includes
#include "LCU.h"
#include "CommandLine.h"

// Forward declarations
class dfAgentApp;

/** Implements the LCU interface */
class dfAgentLCU : public LCU
{
 public:

  /** Constructor */
  dfAgentLCU(char* name);

  /** Destructor */
  virtual ~dfAgentLCU();

  /** Start LCU services and applications */
  int start(int update=0);

  /** Stop LCU services and applications */
  int stop();

 protected:

 private:

  dfAgentApp* dfAgentApp_;
};

#endif  // _dfAgentLCU_h_
