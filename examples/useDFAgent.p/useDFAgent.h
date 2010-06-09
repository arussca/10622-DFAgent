// Copyright (c) 2004 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved

// $Log$

// GCS includes
#include "CommandLine.h"
#include "GCSNameService.h"
#include "IDL_Adapters_DGT.h"
#include "DFAGENTC.h" // C++ Stubs for this Device

// VxWorks Includes
#include <stdio.h>
#include <stdlib.h>

#ifdef VXWORKS
#include <usrLib.h>
#include <tickLib.h>

/// This function can be called directly from the VxWorks shell
extern "C" int useDFAgent(char *devicename);
#else
int main(int argc, char** argv);
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
// Prototypes
void helpShow(void);

