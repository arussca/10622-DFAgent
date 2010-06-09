// Copyright (c) 2004 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved

// $Log$

static const char rcsid[]="@(#) $Id$";


// Includes
#include "testDFAgent.h"
#include "ConcreteArrays.h"

#ifdef VXWORKS
/// This function can be called directly from the VxWorks shell
int testDFAgent(char* devicename,int updateProfile,int mode,long priority,unsigned threads)
#else
int main(int argc, char** argv)
#endif
{
  DFAgent *device;
  bool quit=FALSE;
  int option;
  int result=0;

  // Init ACE
  ACE::init();

#ifdef VXWORKS
  if(priority==0) priority=128; if(threads==0) threads=4; // Default
#else
  char* devicename;
  int updateProfile=0;
  int mode=0;
  long priority=128;
  unsigned threads=4;
  CommandLine::current(argc, argv);
  CommandLine cl(argc, argv);
  updateProfile = cl.getFlag("-update");
  mode = cl.getFlag("-integrated");
  priority = atoi(cl.getOption("-p", "128"));
  threads = atoi(cl.getOption("-t", "4"));
  devicename = cl.getOption("-name", "Test/DFAgent_1");
#endif

  // Start test
  printf("---------------------------------------------------------------------\n");
  printf("MyDevice TEST: \n");
  printf("Instance = %s\n",devicename);
  if(mode==1)
  {
    printf("Mode     = INTEGRATED  ");
    if(updateProfile) printf("*** Update profile requested");
  }
  else if(mode==0) 
  {
    printf("Mode     = STANDALONE  ");
    if(updateProfile) printf("*** Update option ignored in this mode");
    updateProfile=0;
  }
  printf("\n");
  printf("Priority = %d \n",priority);
  printf("Threads  = %d \n",threads);

  helpShow();
  
  do
  {
    //taskDelay(20);
    printf("DFAgent-> ");
    scanf("%d", &option);
    switch(option)
    {
      case 0: // Help
              helpShow();
              break;

      case 1: // Create new device
              try
              {
                device = new DFAgent(devicename);
                device->connect("orb",threads,priority,mode,updateProfile);
              }
              catch(GCSException& ex)
              {
                ex.addToTrace("testDFAgent::constructor");
                cerr << ex;
                result = 1;
                quit   = TRUE;
              }
              break;

      case 2: // Start
              try
              {
                device->start();
              }
              catch(GCSException& ex)
              {
                ex.addToTrace("testDFAgent::start");
                cerr << ex;
              }
              break;

      case 3: // Init
              try
              {
                device->init();
              }
              catch(GCSException& ex)
              {
                ex.addToTrace("testDFAgent::init");
                cerr << ex;
              }
              break;

      case 4: // State
              try
              {
                if(device!=NULL) cerr << "State = " << device->state();
                else cerr << "Device is not created" << endl;
              }
              catch(GCSException& ex)
              {
                ex.addToTrace("testDFAgent::state");
                cerr << ex;
              }
              break;

      case 5: // Report
              try
              {
                cerr << device->report(0);
              }
              catch(GCSException& ex)
              {
                ex.addToTrace("testDFAgent::report");
                cerr << ex;
              }
              break;

      case 6: // Enable/disable
              try
              {
                if(device->isDisabled()) device->enable();
                else device->disable();
              }
              catch(GCSException& ex)
              {
                ex.addToTrace("testDFAgent::enable/disable");
                cerr << ex;
              }
              break;

      case 7: // Halt
              try
              {
                device->halt();
              }
              catch(GCSException& ex)
              {
                ex.addToTrace("testDFAgent::halt");
                cerr << ex;
              }
              break;

      case 8: // Shutdown
              try
              {
                device->shutdown();
              }
              catch(GCSException& ex)
              {
                ex.addToTrace("testDFAgent::shutdown");
                cerr << ex;
              }
              break;

      case 9: // Destroy
              try
              {
                if (device!=NULL)
                {
                  device->disconnect();
                  delete device;
                  device=NULL;
                }
              }
              catch(GCSException& ex )
              {
                ex.addToTrace("testDFAgent::destructor");
                cerr << ex;
              }
              break;

      case 10: // Call specific methods
              try
              {
                if (device!=NULL)
                {
                  //## call specific methods of your device
                  cerr << "Add your tests here" << endl;
                }
              }
              catch(GCSException& ex )
              {
                ex.addToTrace("testDFAgent::specific methods");
                cerr << ex;
              }
              break;

      case 99: // Quit program
              quit = TRUE;
              break;

      default:
              cerr << "Unknown option" << endl;
              break;

    }
    printf("\n");
  } while(quit == FALSE);
  cerr << "Bye" << endl;
  return(result);  // On error return 1; On success return 0
}


void helpShow(void)
{
  printf("\n");
  printf("--------Select Option------------------------------------\n");
  printf("|                           |                           |\n");
  printf("| 0- Show this menu         | 10- Specific methods      |\n");
  printf("| 1- Create device          |                           |\n");
  printf("| 2- Start device           |                           |\n");
  printf("| 3- Init device            |                           |\n");
  printf("| 4- Get device state       |                           |\n");
  printf("| 5- Report device          |                           |\n");
  printf("| 6- Enable/Disable device  |                           |\n");
  printf("| 7- Halt device            |                           |\n");
  printf("| 8- Shutdown device        |                           |\n");
  printf("| 9- Destroy device         | 99- Quit Test Program     |\n");
  printf("|                           |                           |\n");
  printf("---------------------------------------------------------\n");
}
