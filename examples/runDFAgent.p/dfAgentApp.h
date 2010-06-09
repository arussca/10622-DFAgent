// Copyright (c) 2004 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved

// $Log$

#ifndef _dfAgentApp_h_
#define _dfAgentApp_h_

// GCS Includes
#include "Application.h"

// Forward declarations
class DFAgent;

/** This class manages a group of related Device instances */
class dfAgentApp : public Application
{
 public:

  /** Constructor */
  dfAgentApp(char* name);

  /** Destructor */
  virtual ~dfAgentApp();

  /** Creates devices */
  int run(int update=0);

  /** Destroy devices */
  int quit();

 protected:

 private:

  DFAgent* dfAgent_;
};

#endif  // _dfAgentApp_h_
