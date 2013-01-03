// Copyright (c) 2004 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved

// $Log$

static const char rcsid[]="@(#) $Id$";


// Includes
#include "useDFAgent.h"

#ifdef VXWORKS
int useDFAgent(char *devicename)
{
  if(devicename==NULL)
  {
    cerr << "Usage:  useDFAgent(devicename)" << endl;
    return(1);
  } // Default
#else
int main(int argc, char** argv)
{
  CommandLine::current(argc, argv);
  CommandLine cl(argc, argv);
  char *devicename = cl.getOption("-name","Test/DFAgent_1");
#endif
  CORBA::Object* obj = 0;
  DFAGENT::DFAgent_ifce* device = 0;
  int quit=FALSE;
  int option, result=0;
  
  printf("-------------------------------------------------------------\n");
  printf("DFAgent:\n");
  try
  {
    ACE::init();
    printf(" - ORB_CONFIG_DIR=%s\n",getenv("ORB_CONFIG_DIR"));
    printf(" - looking for %s in the GCS Name Service\n",devicename);
    obj = GCSNameService::resolve(devicename);
    if (CORBA::is_nil(obj))
    {
      cerr << "null reference to Device" << endl;
      throw 0;
    }
    printf(" - narrowing interface\n");
    try
    {
      device = DFAGENT::DFAgent_ifce::_narrow(obj);
    }
    catch(const CORBA::Exception &ex)
    {
      cerr << "can't specialise Device interface:" << endl;
      cerr << ex << endl;
      throw 0;
    }
    if (CORBA::is_nil(device))
    {
      cerr << "Reference is not a DFAGENT::DFAgent_ifce" << endl;
      throw 0;
    }
    printf(" - ping DFAgent\n");
    device->ping();
    printf(" - device found\n");
    State state = ::fromIDL(*device->state());
    cout << "The device " << devicename << " is in state=" << state << endl;
    
    helpShow();
    do
    {
      try
      {
        printf("DFAgent-> ");
        scanf("%d", &option);
        switch(option)
        {
          case 0: // Help
              helpShow();
              break;

          case 1: // Start
              device->start();
              break;

          case 2: // Init
              device->init();
              break;

          case 3: // Report
//              cerr << device->report(0);
              break;

          case 4: // Enable/disable
              if(device->isDisabled()) device->enable();
              else device->disable();
              break;

          case 5: // Halt
              device->halt();
              break;

          case 6: // Shutdown
              device->shutdown();
              break;

          case 99: // Quit program
              quit = TRUE;
              break;

          default:
              cerr << "Unknown option" << endl;
              break;

        }
      }
      catch(const CORBA::Exception &ex)
      {
        cerr << "CORBA Exception: " << endl;
        cerr << ex << endl;
      }
      printf("\n");
    } while(quit == FALSE);
  }
  catch (...)
  {
    return(1); 
  }
  cerr << "Bye" << endl;
  return(result);  // On error return 1; On success return 0
}


void helpShow(void)
{
  printf("\n");
  printf(" -------Select Option----------------------------------- \n");
  printf("|                           |                           |\n");
  printf("| 0- Show this menu         |                           |\n");
  printf("| 1- Start device           |                           |\n");
  printf("| 2- Init device            |                           |\n");
  printf("| 3- Report device          |                           |\n");
  printf("| 4- Enable/Disable device  |                           |\n");
  printf("| 5- Halt device            |                           |\n");
  printf("| 6- Shutdown device        |                           |\n");
  printf("| 7-                        |                           |\n");
  printf("| 8-                        |                           |\n");
  printf("| 9-                        | 99- Quit Test Program     |\n");
  printf("|                           |                           |\n");
  printf(" ------------------------------------------------------- \n");
}
