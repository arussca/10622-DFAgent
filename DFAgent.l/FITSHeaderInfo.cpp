// Copyright (c) 2018 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved

// Includes
#include "FITSHeaderInfo.h"

#include <cstdio>                         // std::fprintf
#include <cstdlib>                        // std::atof
#include <fstream>                        // std::ifstream
#include <sstream>                        // std::ostringstream, std::stringstream
#include <iomanip>                        // std::setw, std::setfill
#include <algorithm>                      // std::remove_if
#include <GCSException.h>                 // GCSException, NotFound
#include <PropertyParser.h>               // SyntaxError
#include <MMCommonS.h>                    // MM::MonitorQuery
#include <ace/Trace.h>                    // ACE_Trace
#include <json/json.h>                    // Json::Value, Json::ArrayIndex
//#include <type_traits>                    // static_assert, std::is_same
#include <cassert>                        // assert
#include <slalib.h>                       // slaDr2tf, slaDr2af
#include <slamac.h>                       // DR2D (180/pi:  radians to degrees)
#include <Wrappers_All.h>                 // endsWith

/************************************************************************
*************************************************************************
**                                                                     **
**                               Macros                                **
**                                                                     **
*************************************************************************
************************************************************************/
//<editor-fold desc="Enable or disable debug trace via source code">
//#define FITSHeaderInfo_DEBUG // Enable debug trace
//#undef FITSHeaderInfo_DEBUG  // Disable debug trace
//</editor-fold>
//<editor-fold desc="Define FITSHeaderInfo_TRACE">
#if defined(FITSHeaderInfo_DEBUG)
#include <ace/Trace.h>                    // ACE_Trace
#define FITSHeaderInfo_TRACE \
  ACE_Trace ace_trace(__func__, __LINE__, __FILE__);\
  ace_trace.start_tracing();\
  ace_trace.set_nesting_indent(1);
#else
#define FITSHeaderInfo_TRACE \
  //;
#endif
//</editor-fold>

#ifndef _FITSHeaderInfo_cpp_
#define _FITSHeaderInfo_cpp_

#define log_out(args...) \
  if (logAgent_ not_eq NULL) { logAgent_->logInfo(args); }

#define log_err(args...) \
  if (logAgent_ not_eq NULL) { logAgent_->logError(args); }

#define trace_out(args...) \
  if (trace_ not_eq NULL) { trace_->out(args); } \
  else { std::fprintf(stdout, args); }

#define trace_err(args...) \
  if (trace_ not_eq NULL) { trace_->err(args); } \
  else { std::fprintf(stderr, args); }

#define log_trace_out(args...)  log_out(args) trace_out(args)
#define log_trace_err(args...)  log_err(args) trace_err(args)

#endif // _FITSHeaderInfo_cpp_

/************************************************************************
*************************************************************************
**                                                                     **
**                              Constants                              **
**                                                                     **
*************************************************************************
************************************************************************/

FITSHeaderInfo::AllowedFileNames FITSHeaderInfo::allowedFieldNamesForFixedKeywords;

/************************************************************************
*************************************************************************
**                                                                     **
**                 Constructors and destructor methods                 **
**                                                                     **
*************************************************************************
************************************************************************/

//----------------------------------------------------------------------
// Default constructor
//----------------------------------------------------------------------
FITSHeaderInfo::FITSHeaderInfo()
    : keywordNamesList_()
    , fixedKeywordList_()
    , monitorKeywordList_()
    , parsedKeywords_()
    , notParsedNodesID_()
    , notWrittenKeywords_()
    , discreteMonitorCounter_(0)
    , sampledMonitorCounter_(0)
    , monitorAgent_(NULL)
    , logAgent_(NULL)
    , trace_(NULL)
{
  FITSHeaderInfo_TRACE
}

//----------------------------------------------------------------------
// Customized constructor
//----------------------------------------------------------------------
FITSHeaderInfo::FITSHeaderInfo(MonitorAgent *&monitorAgent,
                               LogAgent *&logAgent,
                               Trace &trace)
    : keywordNamesList_()
    , fixedKeywordList_()
    , monitorKeywordList_()
    , parsedKeywords_()
    , notParsedNodesID_()
    , notWrittenKeywords_()
    , discreteMonitorCounter_(0)
    , sampledMonitorCounter_(0)
    , monitorAgent_(monitorAgent)
    , logAgent_(logAgent)
    , trace_(& trace)
{
  FITSHeaderInfo_TRACE
}

//----------------------------------------------------------------------
// Copy constructor
//----------------------------------------------------------------------
FITSHeaderInfo::FITSHeaderInfo(const FITSHeaderInfo &other)
    : keywordNamesList_(other.keywordNamesList_)
    , fixedKeywordList_()
    , monitorKeywordList_(other.monitorKeywordList_)
    , parsedKeywords_(other.parsedKeywords_)
    , notParsedNodesID_(other.notParsedNodesID_)
    , notWrittenKeywords_(other.notWrittenKeywords_)
    , discreteMonitorCounter_(other.discreteMonitorCounter_)
    , sampledMonitorCounter_(other.sampledMonitorCounter_)
    , monitorAgent_(other.monitorAgent_)
    , logAgent_(other.logAgent_)
    , trace_(other.trace_)
{
  FITSHeaderInfo_TRACE
  for (FixedKeywordContainer::const_iterator item = other.fixedKeywordList_.begin();
       item != other.fixedKeywordList_.end();
       ++item)
  {
    fixedKeywordList_.push_back((*item)->clone());
  }
}

//----------------------------------------------------------------------
// Move constructor
//----------------------------------------------------------------------
#if defined(__cplusplus) && (__cplusplus >= 201103L)
FITSHeaderInfo::FITSHeaderInfo(FITSHeaderInfo &&other)
    : keywordNamesList_(other.keywordNamesList_)
    , fixedKeywordList_(other.fixedKeywordList_)
    , monitorKeywordList_(other.monitorKeywordList_)
    , parsedKeywords_(other.parsedKeywords_)
    , notParsedNodesID_(other.notParsedNodesID_)
    , notWrittenKeywords_(other.notWrittenKeywords_)
    , discreteMonitorCounter_(other.discreteMonitorCounter_)
    , sampledMonitorCounter_(other.sampledMonitorCounter_)
    , monitorAgent_(other.monitorAgent_)
    , logAgent_(other.logAgent_)
    , trace_(other.trace_)
{
  FITSHeaderInfo_TRACE
  other.monitorAgent_ = NULL;
  other.logAgent_ = NULL;
  other.trace_ = NULL;
  other.fixedKeywordList_.clear();
}
#endif

//----------------------------------------------------------------------
// Copy assignment
//----------------------------------------------------------------------
FITSHeaderInfo& FITSHeaderInfo::operator=(const FITSHeaderInfo &other)
{
  FITSHeaderInfo_TRACE
  this->keywordNamesList_.clear();
  this->fixedKeywordList_.clear();
  this->monitorKeywordList_.clear();
  this->parsedKeywords_.clear();
  this->notParsedNodesID_.clear();
  this->notWrittenKeywords_.clear();
  this->discreteMonitorCounter_ = 0;
  this->sampledMonitorCounter_ = 0;

  this->keywordNamesList_ = other.keywordNamesList_;
  this->fixedKeywordList_ = other.fixedKeywordList_;
  this->monitorKeywordList_ = other.monitorKeywordList_;
  this->parsedKeywords_ = other.parsedKeywords_;
  this->notParsedNodesID_ = other.notParsedNodesID_;
  this->notWrittenKeywords_ = other.notWrittenKeywords_;
  this->discreteMonitorCounter_  = other.discreteMonitorCounter_ ;
  this->sampledMonitorCounter_  = other.sampledMonitorCounter_ ;

  this->monitorAgent_ = other.monitorAgent_;
  this->logAgent_ = other.logAgent_;
  this->trace_ = other.trace_;
  return *this;
}

//----------------------------------------------------------------------
// Move assignment
//----------------------------------------------------------------------
#if defined(__cplusplus) && (__cplusplus >= 201103L)
FITSHeaderInfo& FITSHeaderInfo::operator=(FITSHeaderInfo &&other)
{
  FITSHeaderInfo_TRACE
  this->keywordNamesList_.clear();
  this->fixedKeywordList_.clear();
  this->monitorKeywordList_.clear();
  this->parsedKeywords_.clear();
  this->notParsedNodesID_.clear();
  this->notWrittenKeywords_.clear();
  this->discreteMonitorCounter_ = 0;
  this->sampledMonitorCounter_ = 0;

  this->keywordNamesList_ = other.keywordNamesList_;
  this->fixedKeywordList_ = other.fixedKeywordList_;
  this->monitorKeywordList_ = other.monitorKeywordList_;
  this->parsedKeywords_ = other.parsedKeywords_;
  this->notParsedNodesID_ = other.notParsedNodesID_;
  this->notWrittenKeywords_ = other.notWrittenKeywords_;
  this->discreteMonitorCounter_  = other.discreteMonitorCounter_ ;
  this->sampledMonitorCounter_  = other.sampledMonitorCounter_ ;

  this->monitorAgent_ = other.monitorAgent_;
  this->logAgent_ = other.logAgent_;
  this->trace_ = other.trace_;

  other.monitorAgent_ = NULL;
  other.logAgent_ = NULL;
  other.trace_ = NULL;
  other.fixedKeywordList_.clear();

  return *this;
}
#endif

//----------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------
FITSHeaderInfo::~FITSHeaderInfo()
{
  FITSHeaderInfo_TRACE
  clearAllMembers_();
}

/************************************************************************
*************************************************************************
**                                                                     **
**                            public methods                           **
**                                                                     **
*************************************************************************
************************************************************************/

//----------------------------------------------------------------------
// setMonitorAgent
//----------------------------------------------------------------------
void FITSHeaderInfo::setMonitorAgent(MonitorAgent *monitorAgent)
{
  FITSHeaderInfo_TRACE
  monitorAgent_ = monitorAgent;
}

//----------------------------------------------------------------------
// setLogAgent
//----------------------------------------------------------------------
void FITSHeaderInfo::setLogAgent(LogAgent *logAgent)
{
  FITSHeaderInfo_TRACE
  logAgent_ = logAgent;
}

//----------------------------------------------------------------------
// setTrace
//----------------------------------------------------------------------
void FITSHeaderInfo::setTrace(Trace *trace)
{
  FITSHeaderInfo_TRACE
  trace_ = trace;
}

//----------------------------------------------------------------------
// parseFITSHeaderKeywordsFromFile
//----------------------------------------------------------------------
void FITSHeaderInfo::parseFITSHeaderKeywordsFromFile(const std::string &fileName)
{
  FITSHeaderInfo_TRACE
  clearAllMembers_();

  // Check if file can be opened
  {
    std::ifstream file(fileName.c_str());
    if ( not file.is_open() )
    {
      NotFound exception(__PRETTY_FUNCTION__);
      exception.addText("Unable to open file '%s'", fileName.c_str() );
      throw exception;
    }
    else
    {
      file.close();
    }
  }

  // Determine the file format by its extension
  if ( endsWith (fileName.c_str(), ".json") )
  {
    parseFITSHeaderKeywordsFromJSONFile_(fileName);
  }// JSON
  else
  {
    parseFITSHeaderKeywordsFromPlainTextFile_(fileName);
  }// PlainText

}

//----------------------------------------------------------------------
// parseFITSHeaderKeywordsFromInline
//----------------------------------------------------------------------
void FITSHeaderInfo::parseFITSHeaderKeywordsFromInline(const Json::Value &root)
{
  FITSHeaderInfo_TRACE
  clearAllMembers_();

  // Check type of root node
  if ( not root.isArray() )
    throw SyntaxError(__PRETTY_FUNCTION__, "The root JSON node must be an array.");

  // Parse all keywords from subnodes of root.
  #ifdef VXWORKS
  typedef Json::Value::ArrayIndex ArrayIndex;
  #elif LINUX
  typedef Json::ArrayIndex ArrayIndex;
  #endif
  unsigned int id;
  for (ArrayIndex i = 0; i < root.size(); ++i )
  {
    try
    {
      id = i+1;
      const unsigned int verbose = 0;
      trace_out("Parsing node #%03d", id)
      // Is a fixed keyword?
      if ( root[i].isMember("value") )
      {
        parseFixedKeywords_(id, root[i]);
      }
        // Is a monitor keyword?
      else if ( root[i].isMember("monitor") )
      {
        parseMonitorKeywords_(id, root[i]);
      }
      else
      {
        SyntaxError exception(__PRETTY_FUNCTION__);
        exception.addText("One of following fields should be provided: 'value' or 'monitor'.");
        log_trace_err(exception.text())
        throw exception;
      }
    }
    catch (GCSException &exception)
    {
      notParsedNodesID_.insert( id );
      exception.addToTrace(__PRETTY_FUNCTION__);
      exception.addText("Unable to parse the node #%03d. Check syntax or allowed fields or theirs values.", id );
      log_trace_err(exception.text())
      throw;
    }
    catch (...)
    {
      notParsedNodesID_.insert( id );
      UnknownException exception(__PRETTY_FUNCTION__);
      exception.addText("Unable to parse the node #%03d. Check syntax or allowed fields or theirs values.", id );
      log_trace_err(exception.text())
      throw;
    }
  }
}

//----------------------------------------------------------------------
// merge
//----------------------------------------------------------------------
void FITSHeaderInfo::merge(const FITSHeaderInfo &other)
{
  FITSHeaderInfo_TRACE
  //<editor-fold desc="Merge trace, logAgent and monitorManager">
  // Assign trace
  if ( this->trace_ == NULL and other.trace_ not_eq NULL )
  {
    this->trace_ = other.trace_;
  }
  // Assign logAgent
  if ( this->logAgent_ == NULL and other.logAgent_ not_eq NULL )
  {
    this->logAgent_ = other.logAgent_;
  }
  // Assign monitorAgent
  if ( this->monitorAgent_ == NULL and other.monitorAgent_ not_eq NULL )
  {
    this->monitorAgent_ = other.monitorAgent_;
  }
  //</editor-fold>

  //<editor-fold desc="Merge non-repeated keywords">
  // Add keywords contained in other, which doesn't exist in this object.
  unsigned int repeatedKeywordsCounter(0);

  //<editor-fold desc="Merge fixed-Keywords">
  for (FixedKeywordContainer::const_iterator item = other.fixedKeywordList_.begin();
       item != other.fixedKeywordList_.end();
       ++item)
  {

    // Discard keyword if it's repeated in this object.
    if (keywordNamesList_.count((*item)->name()) > 0)
    {
      // item exists in 'this' list of monitor keyword -> discard item.
      log_trace_err("Duplicated fixed-keyword '%s', which will be discarded.", (*item)->name().c_str() )
      ++repeatedKeywordsCounter;
      continue; // Ignore this keyword definition.
    }
    // Add keyword
    this->keywordNamesList_.insert((*item)->name());
    this->parsedKeywords_.insert((*item)->name());
    this->fixedKeywordList_.push_back((*item));
  }
  //</editor-fold>

  //<editor-fold desc="Merge monitor-Keywords">
  for (MonitorKeywordContainer::const_iterator item = other.monitorKeywordList_.begin();
       item != other.monitorKeywordList_.end();
       ++item)
  {
    // Discard keyword if it's repeated in this object.
    if (keywordNamesList_.count(item->keyName) > 0)
    {
      // item exists in 'this' list of monitor keyword -> discard item.
      log_trace_err("Duplicated monitor-keyword '%s', which will be discarded.", item->keyName.c_str() )
      ++repeatedKeywordsCounter;
      continue; // @note: Ignore keyword definition.
    }
    // Add keyword
    this->keywordNamesList_.insert(item->keyName);
    this->parsedKeywords_.insert(item->keyName);
    this->monitorKeywordList_.insert(*item);
    // Increase counters
    switch (item->monitorType)
    {
      case MonitorKeyword/*::MonitorType*/::SAMPLED:
        ++sampledMonitorCounter_;
        break;
      case MonitorKeyword/*::MonitorType*/::DISCRETE:
        ++discreteMonitorCounter_;
        break;
      default:
        break;
    }
  }
  //</editor-fold>

  //<editor-fold desc="Throw exception, if there is any repeated keyword">
  if ( repeatedKeywordsCounter > 0 )
  {
    CannotProceed exception(__PRETTY_FUNCTION__);
    static const std::string singularOrPlural (repeatedKeywordsCounter == 1? "is" : "are");
    exception.addText("There %s '%ld' repeated keyword(s).", singularOrPlural.c_str(), repeatedKeywordsCounter);
    throw exception;
  }
  //</editor-fold>
  //</editor-fold>

  // Nothing to do for not parsed nodes ID
  // Nothing to do for not written keywords
}

//----------------------------------------------------------------------
// gatherFixedKeywords
//----------------------------------------------------------------------
bool FITSHeaderInfo::gatherFixedKeywords(DataDescription &header)
{
  FITSHeaderInfo_TRACE
  bool everythingOK (true);
  for (FixedKeywordContainer::const_iterator item = fixedKeywordList_.begin();
       item != fixedKeywordList_.end();
       ++item)
  {
    try
    {
      header.addKeyword (*(*item));
    }
    catch (GCSException & exception)
    {
      exception.addToTrace(__PRETTY_FUNCTION__);
      exception.addText("Problem to add keyword '%s' in header.", (*item)->name().c_str() );
      log_trace_err(exception.text())
      everythingOK = false;
    }
    catch (...)
    {
      UnknownException exception(__PRETTY_FUNCTION__);
      exception.addText("Problem to add keyword '%s' in header.", (*item)->name().c_str() );
      log_trace_err(exception.text())
      everythingOK = false;
    }
  }
  return everythingOK;
}

//----------------------------------------------------------------------
// gatherMonitorKeywords
//----------------------------------------------------------------------
bool FITSHeaderInfo::gatherMonitorKeywords(const TimeValue &time,
                                           const TimeValue &expTime,
                                           DataDescription &header)
{
  FITSHeaderInfo_TRACE
  if ( monitorAgent_ not_eq NULL )
    return gatherMonitorKeywords(time, expTime, monitorAgent_, header);
  else
    throw CannotProceed(__PRETTY_FUNCTION__, "The reference to MonitorAgent is null.");
}

//----------------------------------------------------------------------
// gatherMonitorKeywords
//----------------------------------------------------------------------
bool FITSHeaderInfo::gatherMonitorKeywords(const TimeValue &startExpTime,
                                           const TimeValue &durationExpTime,
                                           MonitorAgent *monitorAgent,
                                           DataDescription &header)
{
  FITSHeaderInfo_TRACE
  if (monitorAgent == NULL)
  {
    throw CannotProceed(__PRETTY_FUNCTION__, "The reference to MonitorAgent is null.");
  }
  bool returnValue (true);
  notWrittenKeywords_.clear();
  DGT::TimeValue idlStartTime = toIDL(startExpTime);
  DGT::TimeValue idlEndTime   = toIDL(startExpTime + durationExpTime);

  //<editor-fold desc="Group all queries by their type: sampled and discrete">
  int indexSampled (-1), indexDiscrete (-1);
  MM::MonitorQueryList sampledQueryList, discreteQueryList;
  sampledQueryList.length( sampledMonitorCounter_ );
  discreteQueryList.length( discreteMonitorCounter_ );
  for (MonitorKeywordContainer::iterator monitorKeyword = monitorKeywordList_.begin();
       monitorKeyword != monitorKeywordList_.end();
       ++monitorKeyword)
  {
    try
    {
      MM::MonitorQuery query;
      query.gcs_component = CORBA::string_dup( monitorKeyword->deviceName.c_str() );
      query.magnitude     = CORBA::string_dup( monitorKeyword->monitorName.c_str());
      // Set startExpTime stamp of the query
      if ( monitorKeyword->when not_eq MonitorKeyword/*::When*/::END )
        query.time_stamp = idlStartTime;
      else
        query.time_stamp = idlEndTime;
      // Add query to its respective group
      if (monitorKeyword->monitorType == MonitorKeyword/*::MonitorType*/::SAMPLED)
      {
        sampledQueryList[++indexSampled] = query;
      }
      else if (monitorKeyword->monitorType == MonitorKeyword/*::MonitorType*/::DISCRETE)
      {
        discreteQueryList[++indexDiscrete] = query;
      }
      else
      {
        InvalidValue exception(__PRETTY_FUNCTION__);
        exception.addText("Unknown type was found for monitor '%s.%s'.",
                          monitorKeyword->deviceName.c_str(),
                          monitorKeyword->monitorName.c_str());
        throw exception;
      }
    }
    catch (...)
    {
      UnknownException exception(__PRETTY_FUNCTION__);
      exception.addText("Unable to create the query for the keyword '%s'", monitorKeyword->keyName.c_str() );
      log_trace_err(exception.text())
      notWrittenKeywords_.insert(monitorKeyword->keyName);
      returnValue = false;
    }
  }
  //</editor-fold>

  //<editor-fold desc="Retrieve all SAMPLED monitors">
  MM::DataBlockList retrievedMonitors;
  try
  {
    if ( sampledQueryList.length() > 0 )
    {
      retrievedMonitors = *(monitorAgent->retrieveLastDataSet(sampledQueryList));
    }
  }
  catch (const CORBA::Exception &corbaException)
  {
    GCSException exception = DException::mapCORBAException(const_cast<CORBA::Exception&>(corbaException));
    exception.addToTrace(__PRETTY_FUNCTION__);
    exception.addText("Unable to retrieve all SAMPLED monitors");
    log_trace_err(exception.text())
    returnValue = false;
  }
  catch (GCSException &exception)
  {
    exception.addToTrace(__PRETTY_FUNCTION__);
    exception.addText("Unable to retrieve all SAMPLED monitors");
    log_trace_err(exception.text())
    returnValue = false;
  }
  catch (...)
  {
    UnknownException exception(__PRETTY_FUNCTION__);
    exception.addText("Unable to retrieve all SAMPLED monitors");
    log_trace_err(exception.text())
    returnValue = false;
  }
  //</editor-fold>

  //<editor-fold desc="Retrieve all DISCRETE monitors">
  MM::MagnitudeChangeList retrievedMagnitudes;
  try
  {
    if ( discreteQueryList.length() > 0)
    {
      retrievedMagnitudes = monitorAgent->retrieveLastMagnitudeSet(discreteQueryList);
    }
  }
  catch (const CORBA::Exception &corbaException)
  {
    GCSException exception = DException::mapCORBAException(const_cast<CORBA::Exception&>(corbaException));
    exception.addToTrace(__PRETTY_FUNCTION__);
    exception.addText("Unable to retrieve all DISCRETE monitors");
    log_trace_err(exception.text())
    returnValue = false;
  }
  catch (GCSException &exception)
  {
    exception.addToTrace(__PRETTY_FUNCTION__);
    exception.addText("Unable to retrieve all DISCRETE monitors");
    log_trace_err(exception.text())
    returnValue = false;
  }
  catch (...)
  {
    UnknownException exception(__PRETTY_FUNCTION__);
    exception.addText("Unable to retrieve all DISCRETE monitors");
    log_trace_err(exception.text())
    returnValue = false;
  }
  //</editor-fold>

  // Did all queries retrieve successfully?
  NumberOfKeywords retrievedMonitorsSize ( retrievedMonitors.length() );
  NumberOfKeywords retrievedMagnitudesSize ( retrievedMagnitudes.length() );
  bool retrievedAllMonitors ( retrievedMonitorsSize == sampledMonitorCounter_ );
  bool retrievedAllMagnitudes ( retrievedMagnitudesSize == discreteMonitorCounter_ );
  returnValue &= retrievedAllMonitors and retrievedAllMagnitudes;

  //<editor-fold desc="Write retrieved monitors and magnitudes on header">
  notWrittenKeywords_.clear();
  indexSampled  = -1;
  indexDiscrete = -1;
  for (MonitorKeywordContainer::iterator monitorKeyword = monitorKeywordList_.begin();
       monitorKeyword != monitorKeywordList_.end();
       ++monitorKeyword)
  {
    try
    {
      switch (monitorKeyword->monitorType)
      {

        //<editor-fold desc="SAMPLED">
        case MonitorKeyword/*::MonitorType*/::SAMPLED:
        {
          if ( not retrievedAllMonitors )
          {
            //Search dataBlock that matches the keyword info within DataBlockList
            signed int i (-1);
            for(; ++i < retrievedMonitorsSize;)
            {
              if ( monitorKeyword->deviceName ==  std::string ( retrievedMonitors[i].gcs_component.in() )
                   and monitorKeyword->monitorName ==  std::string ( retrievedMonitors[i].magnitude.in() )
                  )
              {
                gatherSampledMonitor_(*monitorKeyword, retrievedMonitors[i], header);
                break;
              }
            }
            if ( i == retrievedMonitorsSize )
            {
              NotFound exception( __PRETTY_FUNCTION__ );
              exception.addText("The keyword '%s' could not be retrieved from database.", monitorKeyword->keyName.c_str() );
              throw exception;
            }
          }
          else
          {
            gatherSampledMonitor_(*monitorKeyword, retrievedMonitors[++indexSampled], header);
          }
          break;
        }// SAMPLED
        //</editor-fold>

        //<editor-fold desc="DISCRETE">
        case MonitorKeyword/*::MonitorType*/::DISCRETE:
        {
          if ( not retrievedAllMagnitudes )
          {
            //Search magnitudeChange that matches the keyword info within MagnitudeChangeList
            signed int i (-1);
            for( ; ++i < retrievedMagnitudesSize; )
            {
              if ( monitorKeyword->deviceName ==  std::string ( retrievedMagnitudes[i].gcs_component.in() )
                   and monitorKeyword->monitorName ==  std::string ( retrievedMagnitudes[i].magnitude.in() )
                  )
              {
                gatherDiscreteMonitor_(*monitorKeyword, retrievedMagnitudes[i], header);
                break;
              }
            }
            if ( i == retrievedMonitorsSize )
            {
              NotFound exception( __PRETTY_FUNCTION__ );
              exception.addText("The keyword '%s' could not be retrieved from database.", monitorKeyword->keyName.c_str() );
              throw exception;
            }
          }
          else
          {
            gatherDiscreteMonitor_(*monitorKeyword, retrievedMagnitudes[++indexDiscrete], header);
          }
          break;
        }// DISCRETE
        //</editor-fold>
      }
    }
    catch (const CORBA::Exception &corbaException)
    {
      notWrittenKeywords_.insert(monitorKeyword->keyName);
      returnValue = false;
      GCSException exception = DException::mapCORBAException(const_cast<CORBA::Exception&>(corbaException));
      exception.addToTrace(__PRETTY_FUNCTION__);
      exception.addText("Error writing header keyword '%s'.", monitorKeyword->keyName.c_str() );
      log_trace_err(exception.text())
      //throw ; // @note: Don't re-throw corbaException to prevent break loop
    }
    catch (GCSException &exception)
    {
      notWrittenKeywords_.insert(monitorKeyword->keyName);
      returnValue = false;
      exception.addToTrace(__PRETTY_FUNCTION__);
      exception.addText("Error writing header keyword '%s'.", monitorKeyword->keyName.c_str() );
      log_trace_err(exception.text())
      //throw ; // @note: Don't re-throw exception to prevent break loop
    }
    catch (...)
    {
      notWrittenKeywords_.insert(monitorKeyword->keyName);
      returnValue = false;
      UnknownException exception(__PRETTY_FUNCTION__);
      exception.addText("Error writing header keyword '%s'.", monitorKeyword->keyName.c_str() );
      log_trace_err(exception.text())
      //throw ; // @note: Don't re-throw exception to prevent break loop
    }
  }
  //</editor-fold>

  return returnValue;
}

//----------------------------------------------------------------------
// writeHeaderToFile
//----------------------------------------------------------------------
void FITSHeaderInfo::writeHeaderToFile(const TimeValue &time,
                                       const TimeValue &expTime,
                                       fitsfile *rawFile)
{
  FITSHeaderInfo_TRACE
  if ( monitorAgent_ not_eq NULL )
    writeHeaderToFile(time, expTime, monitorAgent_, rawFile);
  else
    throw CannotProceed(__PRETTY_FUNCTION__, "The reference to MonitorAgent is null.");
}

//----------------------------------------------------------------------
// writeHeaderToFile
//----------------------------------------------------------------------
void FITSHeaderInfo::writeHeaderToFile(const TimeValue &time,
                                       const TimeValue &expTime,
                                       MonitorAgent *monitorAgent,
                                       fitsfile *rawFile)
{
  FITSHeaderInfo_TRACE
  if ( monitorAgent == NULL )
  {
    throw CannotProceed(__PRETTY_FUNCTION__, "The reference to MonitorAgent is null.");
  }

  //first of all we gather monitor keywrods into the dataDescription object
  DataDescription newKeywords;
  gatherFixedKeywords (newKeywords);
  gatherMonitorKeywords (time, expTime, monitorAgent, newKeywords);

  int status_=0;
  //We replace fits_write_key with fits_update_key funtion
  //obtained from HDU::write, the routine there is not suitable
  //for our needs, but nearly
  try
  {
    for (DataDescription::RecordConstIterator it = newKeywords.begin(); it != newKeywords.end(); it++)
    {
      status_ = 0;
      if (!FITS::initialKeyword((*it)->name()))
      {
        (*it)->toFits (rawFile, &status_);
      }
    }
  }
  catch(GCSException &exception)
  {
    exception.addToTrace(__PRETTY_FUNCTION__);
    exception.addText("ERROR While adding header to FITS file");
    log_trace_err(exception.text())
    //throw ;
  }
  catch(...)
  {
    UnknownException exception(__PRETTY_FUNCTION__);
    exception.addText("ERROR While adding header to FITS file");
    log_trace_err(exception.text())
    //throw ;
  }

}

//----------------------------------------------------------------------
// forceSampleData
//----------------------------------------------------------------------
void FITSHeaderInfo::forceSampleData() const
{
  FITSHeaderInfo_TRACE
  if ( monitorAgent_ not_eq NULL )
  {
    forceSampleData(monitorAgent_);
  }
  else
  {
    throw CannotProceed(__PRETTY_FUNCTION__, "The reference to MonitorAgent is null.");
  }
}

//----------------------------------------------------------------------
// forceSampleData
//----------------------------------------------------------------------
void FITSHeaderInfo::forceSampleData(MonitorAgent *monitorAgent) const
{
  FITSHeaderInfo_TRACE
  if (monitorAgent == NULL)
  {
    throw CannotProceed(__PRETTY_FUNCTION__, "The reference to MonitorAgent is null.");
  }

  if(monitorKeywordList_.size() == 0)
  {
    throw InvalidValue(__PRETTY_FUNCTION__, "No monitor-keywords found.");
  }

  for (MonitorKeywordContainer::iterator item = monitorKeywordList_.begin();
       item != monitorKeywordList_.end();
       ++item)
  {
    try
    {
      monitorAgent->forceSampleData(item->deviceName, item->monitorName);
    }
    catch (const CORBA::Exception &corbaException)
    {
      GCSException exception = DException::mapCORBAException(const_cast<CORBA::Exception&>(corbaException));
      exception.addToTrace(__PRETTY_FUNCTION__);
      exception.addText("ERROR forcing sample data: %s", item->keyName.c_str() );
      log_trace_err(exception.text())
      //throw; // @note: Don't re-throw corbaException
    }
    catch (GCSException &exception)
    {
      exception.addToTrace(__PRETTY_FUNCTION__);
      exception.addText("ERROR forcing sample data: %s", item->keyName.c_str() );
      log_trace_err(exception.text())
      //throw; // @note: Don't re-throw exception
    }
    catch (...)
    {
      UnknownException exception(__PRETTY_FUNCTION__);
      exception.addText("ERROR forcing sample data: %s", item->keyName.c_str() );
      log_trace_err(exception.text())
      //throw; // @note: Don't re-throw exception
    }
  }
}

//----------------------------------------------------------------------
// getKeywordNameList
//----------------------------------------------------------------------
const FITSHeaderInfo::KeywordNamesContainer &FITSHeaderInfo::getKeywordNameList() const
{
  FITSHeaderInfo_TRACE
  return keywordNamesList_;
}

//----------------------------------------------------------------------
// getMonitorKeywordList
//----------------------------------------------------------------------
const FITSHeaderInfo::FixedKeywordContainer& FITSHeaderInfo::getFixedKeywordList() const
{
  FITSHeaderInfo_TRACE
  return fixedKeywordList_;
}

//----------------------------------------------------------------------
// getMonitorKeywordList
//----------------------------------------------------------------------
const FITSHeaderInfo::MonitorKeywordContainer& FITSHeaderInfo::getMonitorKeywordList() const
{
  FITSHeaderInfo_TRACE
  return monitorKeywordList_;
}

//----------------------------------------------------------------------
// getParsedKeywords
//----------------------------------------------------------------------
const FITSHeaderInfo::ParsedKeywordsContainer& FITSHeaderInfo::getParsedKeywords() const
{
  FITSHeaderInfo_TRACE
  return parsedKeywords_;
}

//----------------------------------------------------------------------
// getNotParsedNodesID
//----------------------------------------------------------------------
const FITSHeaderInfo::NotParsedNodesIDContainer& FITSHeaderInfo::getNotParsedNodesID() const
{
  FITSHeaderInfo_TRACE
  return notParsedNodesID_;
}

//----------------------------------------------------------------------
// getNotWrittenKeywords
//----------------------------------------------------------------------
const FITSHeaderInfo::NotWrittenKeywordsContainer& FITSHeaderInfo::getNotWrittenKeywords() const
{
  FITSHeaderInfo_TRACE
  return notWrittenKeywords_;
}

//----------------------------------------------------------------------
// getNumberOfDiscreteKeywords
//----------------------------------------------------------------------
const FITSHeaderInfo::NumberOfKeywords& FITSHeaderInfo::getNumberOfDiscreteKeywords() const
{
  FITSHeaderInfo_TRACE
  return discreteMonitorCounter_;
}

//----------------------------------------------------------------------
// getNumberOfSampledKeywords
//----------------------------------------------------------------------
const FITSHeaderInfo::NumberOfKeywords& FITSHeaderInfo::getNumberOfSampledKeywords() const
{
  FITSHeaderInfo_TRACE
  return sampledMonitorCounter_;
}

/************************************************************************
*************************************************************************
**                                                                     **
**                          protected methods                          **
**                                                                     **
*************************************************************************
************************************************************************/

//----------------------------------------------------------------------
// clearAllMembers_
//----------------------------------------------------------------------
void FITSHeaderInfo::clearAllMembers_()
{
  FITSHeaderInfo_TRACE
  monitorAgent_ = NULL;
  logAgent_ = NULL;
  trace_ = NULL;

  keywordNamesList_.clear();

  for (int i (-1); ++i < fixedKeywordList_.size(); )
  {
    if ( fixedKeywordList_[i] not_eq NULL )
    {
      //delete (fixedKeywordList_[i]); // #ToDo: release allocate memory.
      fixedKeywordList_[i] = NULL;
    }
  }
  fixedKeywordList_.erase(fixedKeywordList_.begin(), fixedKeywordList_.end());
  fixedKeywordList_.clear();
  monitorKeywordList_.clear();
  parsedKeywords_.clear();
  notParsedNodesID_.clear();
  notWrittenKeywords_.clear();
  discreteMonitorCounter_ = 0;
  sampledMonitorCounter_ = 0;
}

//----------------------------------------------------------------------
// parseFITSHeaderKeywordsFromPlainTextFile_
//----------------------------------------------------------------------
void FITSHeaderInfo::parseFITSHeaderKeywordsFromPlainTextFile_(const std::string &fileName)
{
  FITSHeaderInfo_TRACE
  clearAllMembers_();
  std::ifstream file(fileName.c_str());
  std::string line;
  unsigned int lineNumber(0);
  const unsigned int verbose = 0;
  if ( file.is_open() )
  {
    while( not file.eof() )
    {
      getline(file,line);
      if ( line.empty() )
      {
        break;
      }
      ++lineNumber;
      //trace_out("Parsing line #%03d.\n", lineNumber)
      try
      {
        MonitorKeyword keyMon (line, lineNumber, logAgent_, *trace_);
        if ( parsedKeywords_.count(keyMon.keyName) > 0 )
        {
          // keyword duplicated
          notParsedNodesID_.insert( lineNumber );
          log_trace_err("FITSHeaderInfo::%s(...) Keyword name '%s' already exists.\n", __func__, keyMon.keyName.c_str())
          continue;
        }
        // Add keyword
        keywordNamesList_.insert(keyMon.keyName);
        parsedKeywords_.insert(keyMon.keyName);
        if ( monitorKeywordList_.count(keyMon) > 0 )
        {
          notParsedNodesID_.insert( lineNumber );
          log_trace_err("FITSHeaderInfo::%s(...) Keyword in line #%d already exists.\n", __func__, lineNumber)
          continue;
        }
        monitorKeywordList_.insert(keyMon);
        // Increase counters
        switch (keyMon.monitorType)
        {
          case MonitorKeyword/*::MonitorType*/::SAMPLED:
            ++sampledMonitorCounter_;
            break;
          case MonitorKeyword/*::MonitorType*/::DISCRETE:
            ++discreteMonitorCounter_;
            break;
        }
      }
      catch (const CORBA::Exception &corbaException)
      {
        notParsedNodesID_.insert( lineNumber );
        GCSException exception = DException::mapCORBAException(const_cast<CORBA::Exception&>(corbaException));
        exception.addToTrace(__PRETTY_FUNCTION__);
        exception.addText("Unable to parse the line #%02d", lineNumber );
        log_trace_err(exception.text())
        throw ;
      }
      catch (GCSException &exception)
      {
        notParsedNodesID_.insert( lineNumber );
        exception.addToTrace(__PRETTY_FUNCTION__);
        exception.addText("Unable to parse the line #%02d", lineNumber );
        log_trace_err(exception.text())
        throw ;
      }
      catch (...)
      {
        notParsedNodesID_.insert( lineNumber );
        UnknownException exception(__PRETTY_FUNCTION__);
        exception.addText("Unable to parse the line #%d", lineNumber );
        log_trace_err(exception.text())
        throw ;
      }
    }
    file.close();
  }
}

//----------------------------------------------------------------------
// parseFITSHeaderKeywordsFromJSONFile_
//----------------------------------------------------------------------
void FITSHeaderInfo::parseFITSHeaderKeywordsFromJSONFile_(const std::string &fileName)
{
  FITSHeaderInfo_TRACE
  clearAllMembers_();
  std::ifstream file(fileName.c_str());

  // Parse JSON file
  Json::Value root;
  try
  {
    file >> root;
  }
  catch (...)
  {
    SyntaxError exception(__PRETTY_FUNCTION__);
    exception.addText("Error when parsing JSON file ('%s'). Suggestion: Check JSON syntax.", fileName.c_str() );
    log_trace_err(exception.text())
    throw exception;
  }
  file.close();

  parseFITSHeaderKeywordsFromInline(root);
}

//----------------------------------------------------------------------
// parseFixedKeywords_
//----------------------------------------------------------------------
void FITSHeaderInfo::parseFixedKeywords_(const unsigned int &nodeNumber, const Json::Value &node)
{
  FITSHeaderInfo_TRACE
  // Define allowed fields values
  static const char
      * keywordFieldName (MonitorKeyword::AllowedMembers::name),
      * valueFieldName   ("value"),
      * commentFieldName (MonitorKeyword::AllowedMembers::comment),
      * unitsFieldName   (MonitorKeyword::AllowedMembers::unit)
  ;
  {
    AllowedFileNames::iterator it = allowedFieldNamesForFixedKeywords.begin();
    allowedFieldNamesForFixedKeywords.insert(it, keywordFieldName); ++it;
    allowedFieldNamesForFixedKeywords.insert(it, valueFieldName);   ++it;
    allowedFieldNamesForFixedKeywords.insert(it, commentFieldName); ++it;
    allowedFieldNamesForFixedKeywords.insert(it, unitsFieldName);   ++it;
  }

  // Check if all mandatory fields are present or not in this node.
  for (AllowedFileNames::const_iterator it = allowedFieldNamesForFixedKeywords.begin();
       it != allowedFieldNamesForFixedKeywords.end();
       ++it)
  {
    if ( not node.isMember(*it) )
    {
      SyntaxError exception(__PRETTY_FUNCTION__);
      exception.addText("The node number '%03d' must have defined the field called '%s'.", nodeNumber, *it);
      log_trace_err(exception.text())
      throw exception;
    }
  }

  // Define alias for each allowed member.
  const Json::Value
      &nodeKeyword(node[keywordFieldName]),
      &nodeComment(node[commentFieldName]),
      &nodeUnit(node[unitsFieldName]),
      &nodeValue(node[valueFieldName]);

  // Check types of values
  try
  {
    // Check type of "keywordFieldName" field.
    if ( not nodeKeyword.isString() )
    {
      SyntaxError exception(__PRETTY_FUNCTION__);
      exception.addText("The type of field '%s' must be a string.", keywordFieldName);
      log_trace_err(exception.text())
      throw exception;
    }
    // Check type of commentFieldName"" field.
    if ( not nodeComment.isString() )
    {
      SyntaxError exception(__PRETTY_FUNCTION__);
      exception.addText("The type of field '%s' must be a string.", commentFieldName);
      log_trace_err(exception.text())
      throw exception;
    }
    // Check type of "unitsFieldName" field.
    if ( not nodeUnit.isString() )
    {
      SyntaxError exception(__PRETTY_FUNCTION__);
      exception.addText("The type of field '%s' must be a string.", unitsFieldName);
      log_trace_err(exception.text())
      throw exception;
    }
    // Check type of "valueFieldName" field.
    //if ( not nodeValue.isString() and not nodeValue.isBool() and not nodeValue.isNumeric()) {
    if (nodeValue.isNull() or nodeValue.isObject() or nodeValue.isArray())
    {
      SyntaxError exception(__PRETTY_FUNCTION__);
      exception.addText("The type of field '%s' can be a string, bool, long, double.", valueFieldName);
      log_trace_err(exception.text())
      throw exception;
    }
  }
  catch (...)
  {
    UnknownException exception(__PRETTY_FUNCTION__);
    exception.addText("Unable to parse the node #%d. Check syntax or allowed fields or theirs values.", nodeNumber );
    log_trace_err(exception.text())
    throw ;
  }


  // Define variables and assign the content of fields values.
  std::string
      keywordFieldValue(nodeKeyword.asString()),
      commentFieldValue(nodeComment.asString()),
      unitsFieldValue(nodeUnit.asString())
  ;

  FixedKeywordContainer::value_type keywordPtr (NULL);
  try
  {
    switch (nodeValue.type())
    {
      // bool
      case Json/*::ValueType*/::booleanValue:
      {
        keywordPtr = new (std::nothrow) DPKF::Keyword<bool>(keywordFieldValue, nodeValue.asBool(), commentFieldValue, unitsFieldValue);
        break;
      }
      // string
      case Json/*::ValueType*/::stringValue:
      {
        keywordPtr = new (std::nothrow) DPKF::Keyword<std::string>(keywordFieldValue, nodeValue.asString(), commentFieldValue, unitsFieldValue);
        break;
      }
        // double
      case Json/*::ValueType*/::realValue:
      {
        keywordPtr = new (std::nothrow) DPKF::Keyword<double>(keywordFieldValue, nodeValue.asDouble(), commentFieldValue, unitsFieldValue);
        break;
      }
        // long
      case Json/*::ValueType*/::intValue:
      case Json/*::ValueType*/::uintValue:
      {
        keywordPtr = new (std::nothrow) DPKF::Keyword<long>(keywordFieldValue, nodeValue.asInt(), commentFieldValue, unitsFieldValue);
        break;
      }
        // Save-guard!
      case Json/*::ValueType*/::nullValue:
      case Json/*::ValueType*/::arrayValue:
      case Json/*::ValueType*/::objectValue:
      default:
      {
        SyntaxError exception(__PRETTY_FUNCTION__);
        exception.addText("The type of field '%s' can be a string, bool, double, long.", valueFieldName);
        log_trace_err(exception.text())
        throw exception;
      }
    }

    if ( keywordPtr == NULL )
    {
      CannotProceed exception(__PRETTY_FUNCTION__);
      exception.addText("Unable to allocate memory.");
      log_trace_err(exception.text())
      throw exception;
    }

    if ( parsedKeywords_.count(keywordFieldValue) > 0 )
    {
      // keyword repeated
      notParsedNodesID_.insert( nodeNumber );
      log_trace_err("FITSHeaderInfo::%s(...) Keyword '%s' already exists", __func__, keywordFieldValue.c_str())
      return;
    }

    // Add keyword
    keywordNamesList_.insert(keywordFieldValue);
    parsedKeywords_.insert(keywordFieldValue);
    fixedKeywordList_.push_back(keywordPtr);
  }
  catch (...)
  {
    UnknownException exception(__PRETTY_FUNCTION__);
    exception.addText("Problems to parse fixed keyword of the node #%03d.", nodeNumber );
    log_trace_err(exception.text())
    throw ;
  }
}

//----------------------------------------------------------------------
// parseMonitorKeywords_
//----------------------------------------------------------------------
void FITSHeaderInfo::parseMonitorKeywords_(const unsigned int &nodeNumber, const Json::Value &node)
{
  FITSHeaderInfo_TRACE
  try
  {
    MonitorKeyword keyMon (node, nodeNumber, logAgent_, *trace_);
    if ( parsedKeywords_.count(keyMon.keyName) > 0 )
    {
      // keyword repeated
      notParsedNodesID_.insert( nodeNumber );
      AlreadyExists exception(__PRETTY_FUNCTION__);
      exception.addText(
          "%s[%ld] Keyword '%s' already exists",
          __FILE__, __LINE__, keyMon.keyName.c_str()
      );
      log_trace_err(exception.text())
      throw exception;
    }
    // Add keyword
    keywordNamesList_.insert(keyMon.keyName);
    parsedKeywords_.insert(keyMon.keyName);
    monitorKeywordList_.insert(keyMon);
    // Increase counters
    switch (keyMon.monitorType)
    {
      case MonitorKeyword/*::MonitorType*/::SAMPLED:
      {
        ++sampledMonitorCounter_;
        break;
      }
      case MonitorKeyword/*::MonitorType*/::DISCRETE:
      {
        ++discreteMonitorCounter_;
        break;
      }
    }
  }
  catch (const CORBA::Exception &corbaException)
  {
    notParsedNodesID_.insert( nodeNumber );
    GCSException exception = DException::mapCORBAException(const_cast<CORBA::Exception&>(corbaException));
    exception.addToTrace(__PRETTY_FUNCTION__);
    exception.addText("Unable to parse a JSON node #%03d.", nodeNumber );
    log_trace_err(exception.text())
    throw ;
  }
  catch (GCSException &exception)
  {
    notParsedNodesID_.insert( nodeNumber );
    exception.addToTrace(__PRETTY_FUNCTION__);
    exception.addText("Unable to parse a JSON node #%03d.", nodeNumber );
    log_trace_err(exception.text())
    throw ;
  }
  catch (...)
  {
    notParsedNodesID_.insert( nodeNumber );
    UnknownException exception(__PRETTY_FUNCTION__);
    exception.addText("Unable to parse the node #%03d.", nodeNumber );
    log_trace_err( exception.text() )
    throw ;
  }
}

//----------------------------------------------------------------------
// gatherSampledMonitor_
//----------------------------------------------------------------------
void FITSHeaderInfo::gatherSampledMonitor_(const MonitorKeyword &keyword,
                                           const MM::DataBlock &mons,
                                           DataDescription &header)
{
  FITSHeaderInfo_TRACE
  try
  {
    switch ( mons.samples._d() )
    {
      case MM::TYPE_SAMPLE_DOUBLE_LIST:
      {
        addSampledMonitorKeyword_<MM::SampleDoubleList, double>(keyword, mons.samples.d(), header);
        break;
      }
      case MM::TYPE_SAMPLE_FLOAT_LIST:
      {
        addSampledMonitorKeyword_<MM::SampleFloatList, double>(keyword, mons.samples.f(), header);
        break;
      }
      case MM::TYPE_SAMPLE_LONG_LIST:
      {
        addSampledMonitorKeyword_<MM::SampleLongList, long>(keyword, mons.samples.l(), header);
        break;
      }
      case MM::TYPE_SAMPLE_SHORT_LIST:
      {
        addSampledMonitorKeyword_<MM::SampleShortList, long>(keyword, mons.samples.s(), header);
        break;
      }
      case MM::TYPE_SAMPLE_OCTET_LIST:
      {
        addSampledMonitorKeyword_<MM::SampleOctetList, long>(keyword, mons.samples.o(), header);
        break;
      }
      case MM::TYPE_SAMPLE_DOUBLE_ARRAY1D_LIST:
      case MM::TYPE_SAMPLE_DOUBLE_ARRAY2D_LIST:
      {
        addArrayKeywords_<MM::SampleDoubleArrayList, MM::SampleDoubleArray, double>(keyword, mons.samples.array_d(), header);
        break;
      }
      case MM::TYPE_SAMPLE_FLOAT_ARRAY1D_LIST:
      case MM::TYPE_SAMPLE_FLOAT_ARRAY2D_LIST:
      {
        addArrayKeywords_<MM::SampleFloatArrayList, MM::SampleFloatArray, double>(keyword, mons.samples.array_f(), header);
        break;
      }
      case MM::TYPE_SAMPLE_LONG_ARRAY1D_LIST:
      case MM::TYPE_SAMPLE_LONG_ARRAY2D_LIST:
      {
        addArrayKeywords_<MM::SampleLongArrayList, MM::SampleLongArray, long>(keyword, mons.samples.array_l(), header);
        break;
      }
      case MM::TYPE_SAMPLE_SHORT_ARRAY1D_LIST:
      case MM::TYPE_SAMPLE_SHORT_ARRAY2D_LIST:
      {
        addArrayKeywords_<MM::SampleShortArrayList, MM::SampleShortArray, long>(keyword, mons.samples.array_s(), header);
        break;
      }
      case MM::TYPE_SAMPLE_OCTET_ARRAY1D_LIST:
      case MM::TYPE_SAMPLE_OCTET_ARRAY2D_LIST:
      {
        addArrayKeywords_<MM::SampleOctetArrayList, MM::SampleOctetArray, long>(keyword, mons.samples.array_o(), header);
        break;
      }
      default:
      {
        throw InvalidValue(__PRETTY_FUNCTION__,"No type identified.");
      }
    }
  }
  catch (const CORBA::Exception &corbaException)
  {
    GCSException exception = DException::mapCORBAException(const_cast<CORBA::Exception&>(corbaException));
    exception.addToTrace(__PRETTY_FUNCTION__);
    exception.addText(
      "Unable to gather sampled-monitor for keyword '%s'.",
      keyword.keyName.c_str()
    );
    log_trace_err(exception.text())
    throw exception;
  }
  catch (GCSException &exception)
  {
    exception.addToTrace(__PRETTY_FUNCTION__);
    exception.addText(
      "Unable to gather sampled-monitor for keyword '%s'.",
      keyword.keyName.c_str()
    );
    throw;
  }
  catch (...)
  {
    UnknownException exception(__PRETTY_FUNCTION__);
    exception.addText(
      "Unable to gather sampled-monitor for keyword '%s'.",
      keyword.keyName.c_str()
    );
    log_trace_err(exception.text() );
    throw;
  }

}

//----------------------------------------------------------------------
// gatherDiscreteMonitor_
//----------------------------------------------------------------------
void FITSHeaderInfo::gatherDiscreteMonitor_(const MonitorKeyword &keyword,
                                            const MM::MagnitudeChange &mons,
                                            DataDescription &header)
{
  FITSHeaderInfo_TRACE
  try
  {
    const MonitorKeyword::EnumLabels::size_type &size ( keyword.enumLabels.size() );
    if ( size <= 0 or mons.new_value > size )
    {
      header.addKeyword(DPKF::Keyword<long>(keyword.keyName, mons.new_value, keyword.keyComment, keyword.keyUnit));
    }
    else
    {
      header.addKeyword(DPKF::Keyword<std::string>(keyword.keyName, keyword.enumLabels[mons.new_value], keyword.keyComment, keyword.keyUnit));
    }
  }
  catch (const CORBA::Exception &corbaException)
  {
    GCSException exception = DException::mapCORBAException(const_cast<CORBA::Exception&>(corbaException));
    exception.addToTrace(__PRETTY_FUNCTION__);
    exception.addText("Unable to gather discrete-monitor.");
    log_trace_err(exception.text())
    throw exception;
  }
  catch (GCSException &exception)
  {
    exception.addToTrace(__PRETTY_FUNCTION__);
    exception.addText("Unable to gather discrete-monitor.");
    log_trace_err(exception.text())
    throw;
  }
  catch (...)
  {
    UnknownException exception(__PRETTY_FUNCTION__);
    exception.addText("Unable to gather discrete-monitor.");
    log_trace_err(exception.text())
    throw ;
  }
}

//----------------------------------------------------------------------
// addSampledMonitorKeyword_
//----------------------------------------------------------------------
template<typename ArrayListType, typename ValueType>
void FITSHeaderInfo::addSampledMonitorKeyword_(const MonitorKeyword &keyword,
                                               const ArrayListType &arrayList,
                                               DataDescription &header)
{
  FITSHeaderInfo_TRACE

  #if defined(__cplusplus) && (__cplusplus >= 201103L)
  //<editor-fold desc="Check typenames in compile time">
  // Check in compile time if ArrayListType has a valid type or not.
  static_assert(
      (
          std::is_same<MM::SampleDoubleList, ArrayListType>::value or
          std::is_same<MM::SampleFloatList,  ArrayListType>::value or
          std::is_same<MM::SampleLongList,   ArrayListType>::value or
          std::is_same<MM::SampleShortList,  ArrayListType>::value or
          std::is_same<MM::SampleOctetList,  ArrayListType>::value
      ),
      "Invalid given type for ArrayListType typename."
  );
  // Check in compile time if ValueType has a valid type or not.
  static_assert(
      (
          std::is_same<double, ValueType>::value or
          std::is_same<long,   ValueType>::value
      ),
      "Invalid given type for ValueType typename."
  );
  static_assert(
      (
          std::is_same<double, ValueType>::value and
          (
              std::is_same<MM::SampleDoubleList, ArrayListType>::value or
              std::is_same<MM::SampleFloatList,  ArrayListType>::value
          )
      )
      or
      (
          std::is_same<long, ValueType>::value and
          (
              std::is_same<MM::SampleLongList,  ArrayListType>::value or
              std::is_same<MM::SampleShortList, ArrayListType>::value or
              std::is_same<MM::SampleOctetList, ArrayListType>::value
          )
      ),
      "Invalid given combination of typename ArrayListType and typename ValueType."
  );
  //</editor-fold>
  #endif

  if (arrayList.length() < 1)
  {
    throw NotFound(__PRETTY_FUNCTION__, "Data length is 0.");
  }

  std::string unitComment = keyword.keyUnit;
  if (keyword.keyFormat == MonitorKeyword/*::KeyFormat*/::RAD_TO_HMS)
  {
    unitComment = "h:m:s";
    header.addKeyword(DPKF::Keyword<std::string>(keyword.keyName, formatRadianToHms_((double) arrayList[0].value), keyword.keyComment, unitComment ));
  }
  else if (keyword.keyFormat == MonitorKeyword/*::KeyFormat*/::RAD_TO_DMS)
  {
    unitComment = " d:m:s";
    header.addKeyword(DPKF::Keyword<std::string>(keyword.keyName, formatRadianToDms_((double) arrayList[0].value), keyword.keyComment, unitComment ));
  }
  else if (keyword.keyFormat == MonitorKeyword/*::KeyFormat*/::RAD_TO_DEG)
  {
    unitComment = "degrees";
    header.addKeyword(DPKF::Keyword<double>(keyword.keyName, formatRadianToDeg_((double) arrayList[0].value), keyword.keyComment, unitComment ));
  }
  else
  {
    header.addKeyword (DPKF::Keyword<ValueType>(keyword.keyName, (ValueType) arrayList[0].value, keyword.keyComment, unitComment));
  }
}

//----------------------------------------------------------------------
// addArrayKeywords_
//----------------------------------------------------------------------
template<typename SampledArrayListType, typename SampledArrayType, typename ValueType>
void FITSHeaderInfo::addArrayKeywords_(const MonitorKeyword &keyword,
                                       const SampledArrayListType &arrayList,
                                       DataDescription &header)
{
  #if defined(__cplusplus) && (__cplusplus >= 201103L)
  //<editor-fold desc="Check typenames at compile time">
  // Check in compile time if SampledArrayListType has a valid type or not.
  static_assert(
      (
          std::is_same<MM::SampleDoubleArrayList, SampledArrayListType>::value or
          std::is_same<MM::SampleFloatArrayList,  SampledArrayListType>::value or
          std::is_same<MM::SampleLongArrayList,   SampledArrayListType>::value or
          std::is_same<MM::SampleShortArrayList,  SampledArrayListType>::value or
          std::is_same<MM::SampleOctetArrayList,  SampledArrayListType>::value
      ),
      "Invalid given type for SampledArrayListType typename."
  );
  // Check in compile time if SampledArrayType has a valid type or not.
  static_assert(
      (
          std::is_same<MM::SampleDoubleArray, SampledArrayType>::value or
          std::is_same<MM::SampleFloatArray,  SampledArrayType>::value or
          std::is_same<MM::SampleLongArray,   SampledArrayType>::value or
          std::is_same<MM::SampleShortArray,  SampledArrayType>::value or
          std::is_same<MM::SampleOctetArray,  SampledArrayType>::value
      ),
      "Invalid given type for SampledArrayType typename."
  );
  // Check in compile time if ValueType has a valid type or not.
  static_assert(
      (
          std::is_same<double, ValueType>::value or
          std::is_same<long,   ValueType>::value
      ),
      "Invalid given type for ValueType typename."
  );
  static_assert(
      (
          std::is_same<double, ValueType>::value and
          (
              std::is_same<MM::SampleDoubleArrayList, SampledArrayListType>::value or
              std::is_same<MM::SampleFloatArrayList,  SampledArrayListType>::value
          )
      )
      or
      (
          std::is_same<long, ValueType>::value and
          (
              (
                  std::is_same<MM::SampleLongArrayList,  SampledArrayListType>::value and
                  std::is_same<MM::SampleLongArray,  SampledArrayType>::value
              ) or
              (
                  std::is_same<MM::SampleShortArrayList, SampledArrayListType>::value and
                  std::is_same<MM::SampleShortArray, SampledArrayType>::value
              ) or
              (
                  std::is_same<MM::SampleOctetArrayList, SampledArrayListType>::value and
                  std::is_same<MM::SampleOctetArray, SampledArrayType>::value
              )
          )
      ),
      "Invalid given combination of SampledArrayType, SampledArrayListType and ValueType typenames."
  );
  //</editor-fold desc="Check typenames at compile time">
  #endif
  if(keyword.keyFormat == MonitorKeyword/*::KeyFormat*/::ARRAY_TO_SCALAR)
  {
    addArrayToScalarKeywords_<SampledArrayListType, ValueType>(keyword, arrayList, header);
    return;
  }
  std::ostringstream formatbuf;
  toStream_<SampledArrayType>(formatbuf, trimmedArray1D_(keyword, arrayList[0] ));
  const std::string trimmed_array_1D (formatbuf.str());
  if (keyword.keyFormat == MonitorKeyword/*::KeyFormat*/::KeyFormat_NONE)
  {
    header.addKeyword (DPKF::Keyword<std::string>(keyword.keyName, trimmed_array_1D, keyword.keyComment));
    return;
  }
  const double radian (std::atof(trimmed_array_1D.c_str()));
  if (keyword.keyFormat == MonitorKeyword/*::KeyFormat*/::RAD_TO_HMS)
  {
    const std::string unit ("H:M:S");
    const std::string hms (formatRadianToHms_(radian));
    header.addKeyword(DPKF::Keyword<std::string>(keyword.keyName, hms, keyword.keyComment, unit));
    return;
  }
  if (keyword.keyFormat == MonitorKeyword/*::KeyFormat*/::RAD_TO_DMS)
  {
    const string unit ("D:M:S");
    const std::string dms (formatRadianToDms_(radian));
    header.addKeyword(DPKF::Keyword<std::string>(keyword.keyName, dms, keyword.keyComment, unit));
    return;
  }
  if (keyword.keyFormat == MonitorKeyword/*::KeyFormat*/::RAD_TO_DEG)
  {
    const string unit ("degree");
    const double degree (formatRadianToDeg_(radian));
    header.addKeyword(DPKF::Keyword<double>(keyword.keyName, degree, keyword.keyComment, unit));
    return;
  }
  InvalidValue exception(__PRETTY_FUNCTION__);
  exception.addText(
      "[%s:%ld] Invalid value provided for keyword format ('%s').",
      __FILE__, __LINE__, to_string(keyword.keyFormat)
  );
  throw exception;
}

//----------------------------------------------------------------------
// addArrayToScalarKeywords_
//----------------------------------------------------------------------
template<typename ArrayListType, typename ValueType>
void FITSHeaderInfo::addArrayToScalarKeywords_(const MonitorKeyword &keyword,
                                               const ArrayListType &arrayList,
                                               DataDescription &header)
{
  FITSHeaderInfo_TRACE

  #if defined(__cplusplus) && (__cplusplus >= 201103L)
  //<editor-fold desc="Check typenames in compile time">
  // Check in compile time if ArrayListType has a valid type or not.
  static_assert(
      (
          std::is_same<MM::SampleDoubleArrayList, ArrayListType>::value or
          std::is_same<MM::SampleFloatArrayList,  ArrayListType>::value or
          std::is_same<MM::SampleLongArrayList,   ArrayListType>::value or
          std::is_same<MM::SampleShortArrayList,  ArrayListType>::value or
          std::is_same<MM::SampleOctetArrayList,  ArrayListType>::value
      ),
      "Invalid given type for ArrayListType typename."
  );
  // Check in compile time if ValueType has a valid type or not.
  static_assert(
      (
          std::is_same<double, ValueType>::value or
          std::is_same<long,   ValueType>::value
      ),
      "Invalid given type for ValueType typename."
  );
  static_assert(
      (
          std::is_same<double, ValueType>::value and
          (
              std::is_same<MM::SampleDoubleArrayList, ArrayListType>::value or
              std::is_same<MM::SampleFloatArrayList,  ArrayListType>::value
          )
      )
      or
      (
          std::is_same<long, ValueType>::value and
          (
              std::is_same<MM::SampleLongArrayList,  ArrayListType>::value or
              std::is_same<MM::SampleShortArrayList, ArrayListType>::value or
              std::is_same<MM::SampleOctetArrayList, ArrayListType>::value
          )
      ),
      "Invalid given combination of typename ArrayListType and typename ValueType."
  );
  //</editor-fold>
  #endif

  if (arrayList.length() < 1)
  {
    throw NotFound(__PRETTY_FUNCTION__, "Data length is 0.");
  }

  unsigned int keySufix;
  std::ostringstream keySufix_ostr;

  if ( not keyword.isMemberPresent(MonitorKeyword::AllowedMembers::initialIndex) )
  {
    keySufix = keyword.arrayRange.first;
  }
  else
  {
    keySufix = keyword.keyInitialIndex;
  }

  if ( not keyword.transposed )
  {
    for(unsigned int j = keyword.arrayRange.first; j <= keyword.arrayRange.second; ++j, ++keySufix)
    {
      keySufix_ostr.clear();
      keySufix_ostr.seekp(0);
      keySufix_ostr << keySufix;
      header.addKeyword(DPKF::Keyword<ValueType>(keyword.keyName + keySufix_ostr.str(),
                                                 (ValueType) arrayList[0].value.data[j],
                                                 keyword.keyComment,
                                                 keyword.keyUnit));
    }
  }
  else
  {
    const unsigned int &width  ( arrayList[0].value.dimension.width  );
    const unsigned int &height ( arrayList[0].value.dimension.height );
    //</editor-fold>
    for (unsigned int h(0); h < height; h++)
    {
      for (unsigned int w(0); w < width; w++)
      {
        const unsigned int &index (w * height + h);
        //</editor-fold>
        if (    index >= keyword.arrayRange.first
            and index <= keyword.arrayRange.second
            )
        {
          keySufix_ostr.clear();
          keySufix_ostr.seekp(0);
          keySufix_ostr << keySufix;
          header.addKeyword(
              DPKF::Keyword<ValueType>(
                  keyword.keyName + keySufix_ostr.str(),      // name
                  (ValueType) arrayList[0].value.data[index], // value
                  keyword.keyComment,                         // comment
                  keyword.keyUnit                             // unit
              )
          );
          keySufix++;
        }
      }
    }
  }
}

//----------------------------------------------------------------------
// trimmedArray1D_
//----------------------------------------------------------------------
template<typename ArrayType>
ArrayType FITSHeaderInfo::trimmedArray1D_(const MonitorKeyword &keyword,
                                          const ArrayType &arrayIn)
{
  FITSHeaderInfo_TRACE

  #if defined(__cplusplus) && (__cplusplus >= 201103L)
  //<editor-fold desc="Check typenames in compile time">
  // Check in compile time if ArrayType has a valid type or not.
  static_assert(
      (
          std::is_same<MM::SampleDoubleArray, ArrayType>::value or
          std::is_same<MM::SampleFloatArray,  ArrayType>::value or
          std::is_same<MM::SampleLongArray,   ArrayType>::value or
          std::is_same<MM::SampleShortArray,  ArrayType>::value or
          std::is_same<MM::SampleOctetArray,  ArrayType>::value
      ),
      "Invalid given type for ArrayType typename."
  );
  //</editor-fold>
  #endif

  const std::size_t size ( (keyword.arrayRange.second - keyword.arrayRange.first) + 1 );
  ArrayType arrayOut;
  arrayOut.time_stamp = arrayIn.time_stamp;
  arrayOut.value.dimension.width = 1;
  arrayOut.value.dimension.height = size;
  arrayOut.value.data.length( size );
  for (unsigned int arrayOut_index( 0 ), arrayIn_index ( keyword.arrayRange.first ), arrayIn_upper_limit ( keyword.arrayRange.second + 1 );
       arrayIn_index < arrayIn_upper_limit ;
       ++arrayIn_index, ++arrayOut_index)
  {
    arrayOut.value.data[arrayOut_index] = arrayIn.value.data[arrayIn_index];
  }
  return arrayOut;
}

//----------------------------------------------------------------------
// toStream_
//----------------------------------------------------------------------
template<typename ArrayType>
std::ostream& FITSHeaderInfo::toStream_(std::ostream &ost,
                                        const ArrayType &array)
{
  FITSHeaderInfo_TRACE

  #if defined(__cplusplus) && (__cplusplus >= 201103L)
  //<editor-fold desc="Check typenames in compile time">
  // Check in compile time if ArrayType has a valid type or not.
  static_assert(
      (std::is_same<MM::SampleDoubleArray, ArrayType>::value or
          std::is_same<MM::SampleFloatArray,  ArrayType>::value or
          std::is_same<MM::SampleLongArray,   ArrayType>::value or
          std::is_same<MM::SampleShortArray,  ArrayType>::value or
          std::is_same<MM::SampleOctetArray,  ArrayType>::value),
      "Invalid given type for ArrayType typename."
  );
  //</editor-fold>
  #endif

  unsigned i,j;
  if (array.value.dimension.height != 1 or array.value.dimension.width != 1)
  {
    ost << "[ ";
  }
  for(i=0; i< array.value.dimension.height; ++i)
  {
    for(j=0; j< array.value.dimension.width; ++j)
    {
      ost << array.value.data[ array.value.dimension.width  *i + j] << " ";
    }
    if (i < (array.value.dimension.height -1))
    {
      ost << "; ";
    }
    if ( (array.value.dimension.width >= 5) and (i < (array.value.dimension.height -1)) )
    {
      ost << "\\" << endl;
    }
  }
  if (array.value.dimension.height != 1 or array.value.dimension.width != 1)
  {
    ost << "]";
  }
  return ost;
}

//----------------------------------------------------------------------
// formatRadianToHms_
//----------------------------------------------------------------------
std::string FITSHeaderInfo::formatRadianToHms_(const double &rad) const
{
  FITSHeaderInfo_TRACE
  char sign;
  int ihmsf[4];
  std::ostringstream strbuf;

  slaDr2tf (3, rad, &sign, ihmsf);
  if(sign not_eq '+')
  { //adds sign when negative
    strbuf << sign;
  }
  strbuf << std::setw(2) << std::setfill('0') << ihmsf[0] << ":"
         << std::setw(2) << std::setfill('0') << ihmsf[1] << ":"
         << std::setw(2) << std::setfill('0') << ihmsf[2] << "."
         << std::setw(3) << std::setfill('0') << ihmsf[3];
  return strbuf.str();
}

//----------------------------------------------------------------------
// formatRadianToDms_
//----------------------------------------------------------------------
std::string FITSHeaderInfo::formatRadianToDms_(const double &rad) const
{
  FITSHeaderInfo_TRACE
  char sign;
  int ihmsf[4];
  std::ostringstream strbuf;
  slaDr2af (3, rad, &sign, ihmsf);
  if(sign != '+')
  { //adds sign when negative
    strbuf << sign;
  }
  strbuf << std::setw(2) << std::setfill('0') << ihmsf[0] << ":"
         << std::setw(2) << std::setfill('0') << ihmsf[1] << ":"
         << std::setw(2) << std::setfill('0') << ihmsf[2] << "."
         << std::setw(3) << std::setfill('0') << ihmsf[3];
  return strbuf.str();
}

//----------------------------------------------------------------------
// formatRadianToDeg_
//----------------------------------------------------------------------
double
FITSHeaderInfo::formatRadianToDeg_(const double &rad) const
{
  FITSHeaderInfo_TRACE
  return rad*DR2D;
}

/************************************************************************
*************************************************************************
**                                                                     **
**                           private methods                           **
**                                                                     **
*************************************************************************
************************************************************************/

//----------------------------------------------------------------------
//----------------------------------------------------------------------
