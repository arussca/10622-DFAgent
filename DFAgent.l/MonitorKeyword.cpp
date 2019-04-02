// Copyright (c) 2018 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved

// Includes

#include "MonitorKeyword.h"               // MonitorKeyword

#include <sstream>                        // std::stringstream, std::istringstream
#include <cctype>                         // std::toupper, std::tolower, std::isspace
#include <cstdio>                         // std::snprintf
#include <algorithm>                      // std::transform, std::remove_if
#include <ace/Trace.h>                    // ACE_Trace
#include <ace/Log_Msg.h>                  // ACE_DEBUG
#include <DException.h>                   // DException
#include <GCSException.h>                 // GCSException
#include <PropertyParser.h>               // SyntaxError
#include <MonitorHub.h>                   // MonitorHub
#include <MonitorHubFactory.h>            // MonitorHubFactory::instance()
#include <MonitorManagerS.h>              // MM::MonitorManager_ifce_var
#include <MMCommonS.h>                    // MM::Description, MM::MagnitudeDescription, MM::TYPE_*

/************************************************************************
*************************************************************************
**                                                                     **
**                                Macros                               **
**                                                                     **
*************************************************************************
************************************************************************/
//<editor-fold desc="Enable or disable debug trace via source code">
//#define MonitorKeyword_DEBUG // Enable debug trace
//#undef MonitorKeyword_DEBUG  // Disable debug trace
//</editor-fold>
//<editor-fold desc="Define MonitorKeyword_TRACE">
#if defined(MonitorKeyword_DEBUG)
#include <ace/Trace.h>                    // ACE_Trace
#define MonitorKeyword_TRACE \
  ACE_Trace ace_trace(__func__, __LINE__, __FILE__);\
  ace_trace.start_tracing();\
  ace_trace.set_nesting_indent(1);
#else
#define MonitorKeyword_TRACE \
  ;
#endif
//</editor-fold>

/************************************************************************
*************************************************************************
**                                                                     **
**                              Constants                              **
**                                                                     **
*************************************************************************
************************************************************************/

const MonitorKeyword::AllowedMembers::FieldsContainterType MonitorKeyword::AllowedMembers::all =
    MonitorKeyword::AllowedMembers::FieldsContainterType({
        // Mandatory members related to FITS header keyword
        name, comment, unit,
        // Mandatory members related to any monitor
        component, monitor, type,
        // Optional members related to both types
        when,
        // Optional members related to sampled monitor
        format, transposed, range, initialIndex,
        // Optional members related to discrete monitor
        labels,
    });

/************************************************************************
*************************************************************************
**                                                                     **
**                     Constructors and destructor                     **
**                                                                     **
*************************************************************************
************************************************************************/

//----------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------
MonitorKeyword::MonitorKeyword()
    : // public
      id(0)
    , keyName("")
    , keyComment("")
    , keyUnit("")
    , deviceName("")
    , monitorName("")
    , monitorType(MonitorKeyword::MonitorType::NONE)
    , when(MonitorKeyword::When::NONE)
    , keyFormat(MonitorKeyword::KeyFormat::NONE)
    , transposed(false)
    , keyInitialIndex(0)
    , arrayRange()
    , enumLabels()
    // protected
    , presentMembers_()
    , verbose_(0)
    , logAgent_(nullptr)
    , trace_()
// private
{
  MonitorKeyword_TRACE
}

//----------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------
MonitorKeyword::MonitorKeyword(const std::string &line,
                               const unsigned int &lineNumber,
                               LogAgent *logAgent,
                               Trace &trace,
                               const unsigned short &verbose)
    : // public
      id(lineNumber)
    , keyName("")
    , keyComment("")
    , keyUnit("")
    , deviceName("")
    , monitorName("")
    , monitorType(MonitorType::NONE)
    , when(When::NONE)
    , keyFormat(KeyFormat::NONE)
    , transposed(false)
    , keyInitialIndex(0)
    , arrayRange(std::make_pair(0, 0))
    , enumLabels({})
    // protected
    , presentMembers_()
    , verbose_(verbose)
    , logAgent_(logAgent)
    , trace_(& trace)
// private
{
  MonitorKeyword_TRACE
  clearAllMembers_();
  verbose_ = verbose;
  parseFromPlainText(line, lineNumber, verbose);
}

//----------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------
MonitorKeyword::MonitorKeyword(const Json::Value &node,
                               const unsigned int &nodeID,
                               LogAgent *logAgent,
                               Trace &trace,
                               const unsigned short &verbose)
    : // public
      id(nodeID)
    , keyName("")
    , keyComment("")
    , keyUnit("")
    , deviceName("")
    , monitorName("")
    , monitorType(MonitorType::NONE)
    , when(When::NONE)
    , keyFormat(KeyFormat::NONE)
    , transposed(false)
    , keyInitialIndex(0)
    , arrayRange(std::make_pair(0, 0))
    , enumLabels({})
    // protected
    , presentMembers_()
    , verbose_(verbose)
    , logAgent_(logAgent)
    , trace_(& trace)
    // private
{
  MonitorKeyword_TRACE
  parseFromJSON(node, nodeID, verbose);
}

//----------------------------------------------------------------------
// Copy constructor
//----------------------------------------------------------------------
/*
MonitorKeyword::MonitorKeyword(const MonitorKeyword &other)
    : // public
      id(other.id)
    , keyName(other.keyName)
    , keyComment(other.keyComment)
    , keyUnit(other.keyUnit)
    , deviceName(other.deviceName)
    , monitorName(other.monitorName)
    , monitorType(other.monitorType)
    , when(other.when)
    , keyFormat(other.keyFormat)
    , transposed(other.transposed)
    , keyInitialIndex(other.keyInitialIndex)
    , arrayRange(other.arrayRange)
    , enumLabels(other.enumLabels)
    // protected
    , presentMembers_(other.presentMembers_)
    , verbose_(other.verbose_)
    , logAgent_(other.logAgent_)
    , trace_(other.trace_)
    // private
{}
//*/

//----------------------------------------------------------------------
// Move constructor
//----------------------------------------------------------------------
/*
MonitorKeyword::MonitorKeyword(MonitorKeyword &&other)
    : // public
      id(other.id)
    , keyName(other.keyName)
    , keyComment(other.keyComment)
    , keyUnit(other.keyUnit)
    , deviceName(other.deviceName)
    , monitorName(other.monitorName)
    , monitorType(other.monitorType)
    , when(other.when)
    , keyFormat(other.keyFormat)
    , transposed(other.transposed)
    , keyInitialIndex(other.keyInitialIndex)
    , arrayRange(other.arrayRange)
    , enumLabels(other.enumLabels)
    // protected
    , presentMembers_(other.presentMembers_)
    , verbose_(other.verbose_)
    , logAgent_(other.logAgent_)
    , trace_(other.trace_)
    // private
{
  MonitorKeyword_TRACE
  other.logAgent_ = nullptr;
  other.trace_ = nullptr;
}
//*/

//----------------------------------------------------------------------
// Copy assignment
//----------------------------------------------------------------------
/*
MonitorKeyword& MonitorKeyword::operator=(const MonitorKeyword &other)
{
  MonitorKeyword_TRACE
  this->id              = other.id              ;
  this->keyName         = other.keyName         ;
  this->keyComment      = other.keyComment      ;
  this->keyUnit         = other.keyUnit         ;
  this->deviceName      = other.deviceName      ;
  this->monitorName     = other.monitorName     ;
  this->monitorType     = other.monitorType     ;
  this->when            = other.when            ;
  this->keyFormat       = other.keyFormat       ;
  this->transposed      = other.transposed      ;
  this->keyInitialIndex = other.keyInitialIndex ;
  this->arrayRange      = other.arrayRange      ;
  this->enumLabels      = other.enumLabels      ;
  this->presentMembers_ = other.presentMembers_ ;
  this->verbose_        = other.verbose_        ;
  this->logAgent_       = other.logAgent_       ;
  this->trace_          = other.trace_          ;
  return *this;
}
//*/

//----------------------------------------------------------------------
// Move assignment
//----------------------------------------------------------------------
/*
MonitorKeyword& MonitorKeyword::operator=(MonitorKeyword &&other)
{
  MonitorKeyword_TRACE
  this->id              = other.id              ;
  this->keyName         = other.keyName         ;
  this->keyComment      = other.keyComment      ;
  this->keyUnit         = other.keyUnit         ;
  this->deviceName      = other.deviceName      ;
  this->monitorName     = other.monitorName     ;
  this->monitorType     = other.monitorType     ;
  this->when            = other.when            ;
  this->keyFormat       = other.keyFormat       ;
  this->transposed      = other.transposed      ;
  this->keyInitialIndex = other.keyInitialIndex ;
  this->arrayRange      = other.arrayRange      ;
  this->enumLabels      = other.enumLabels      ;
  this->presentMembers_ = other.presentMembers_ ;
  this->verbose_        = other.verbose_        ;
  this->logAgent_       = other.logAgent_       ;
  this->trace_          = other.trace_          ;
  other.logAgent_       = nullptr;
  other.trace_          = nullptr;
  return *this;
}
//*/

//----------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------
MonitorKeyword::~MonitorKeyword()
{
  MonitorKeyword_TRACE
  clearAllMembers_();
  logAgent_ = nullptr;
  trace_ = nullptr;
}

/************************************************************************
*************************************************************************
**                                                                     **
**                           Public methods                            **
**                                                                     **
*************************************************************************
************************************************************************/

//----------------------------------------------------------------------
// parseFromPlainText
//----------------------------------------------------------------------
void MonitorKeyword::parseFromPlainText(const std::string &line,
                                        const unsigned int &lineNumber,
                                        const unsigned short &verbose)
{
  MonitorKeyword_TRACE
  clearAllMembers_();
  verbose_ = verbose;
  id = lineNumber;

  // Initialize the present members flags
  presentMembers_.clear();
  for (const auto& allowedMember: AllowedMembers::all )
  {
    presentMembers_.emplace(allowedMember, false);
  }

  // # Auxiliary elements
  std::string str("");
  std::stringstream ss(line);
  auto isSpace = [](unsigned char c) { return std::isspace(c); };
  auto toLower = [](unsigned char c) { return std::tolower(c); };

  // # Start parsing fields
  // ## Parsing KeyName field
  {
    ss >> keyName;
    keyName.erase(std::remove_if(keyName.begin(), keyName.end(), isSpace), keyName.end());
    if( keyName.empty() )
    {
      SyntaxError exception(__PRETTY_FUNCTION__);
      exception.addText("Missing keyword name.");
      throw exception;
    }
    presentMembers_[AllowedMembers::name] = true;
  }
  // ## Parsing deviceName field
  {
    ss >> deviceName;
    deviceName.erase(std::remove_if(deviceName.begin(), deviceName.end(), isSpace), deviceName.end());
    if( deviceName.empty() )
    {
      SyntaxError exception(__PRETTY_FUNCTION__);
      exception.addText("Missing device name.");
      throw exception;
    }
    presentMembers_[AllowedMembers::component] = true;
  }
  // ## Parsing monitorName field
  {
    ss >> monitorName;
    monitorName.erase(std::remove_if(monitorName.begin(), monitorName.end(), isSpace), monitorName.end());
    if( monitorName.empty() )
    {
      SyntaxError exception(__PRETTY_FUNCTION__);
      exception.addText("Missing monitor name.");
      throw exception;
    }
    presentMembers_[AllowedMembers::monitor] = true;
  }
  // ## Parsing monitorType field
  {
    ss >> str;
    str.erase(std::remove_if(str.begin(), str.end(), isSpace), str.end());
    if( str.empty() )
    {
      SyntaxError exception(__PRETTY_FUNCTION__);
      exception.addText("Missing monitor type value.");
      throw exception;
    }
    from_string(monitorType, str);
    presentMembers_[AllowedMembers::type] = true;
  }
  // ## Parsing keyFormat field
  {
    ss >> str;
    str.erase(std::remove_if(str.begin(), str.end(), isSpace), str.end());
    if( str.empty() )
    {
      SyntaxError exception(__PRETTY_FUNCTION__);
      exception.addText("Missing transformation value.");
      throw exception;
    }
    from_string(keyFormat, str);
    presentMembers_[AllowedMembers::format] = true;
  }
  // ## Optional fields: must be tab separated!!! No spaces allowed!!
  while( ss.good() )
  {
    std::string cell;
    std::getline(ss, cell, ']');
    if( not cell.empty() )
    {
      std::string tag ( cell.substr(0, cell.find_first_of('=')) );
      tag.erase(std::remove_if(tag.begin(), tag.end(), isSpace), tag.end());
      std::transform(tag.begin(), tag.end(), tag.begin(), toLower);
      if ( not tag.empty() )
      {
        if ( tag == MonitorKeyword::AllowedMembers::range )
        {
          str = cell.substr(cell.find_first_of('[') + 1);
          try
          { // Parse string with the format as follows; value1,value2
            from_string(arrayRange, str);
            presentMembers_[AllowedMembers::range] = true;
          }
          catch (GCSException& exception)
          {
            exception.addToTrace(__PRETTY_FUNCTION__);
            exception.addText("Unable to get a numbers from string [tag='%s'].", MonitorKeyword::AllowedMembers::range);
            throw;
          }
        }
        else if ( tag == MonitorKeyword::AllowedMembers::initialIndex )
        {
          unsigned int value;
          // Parse string with the format as follows; [value]
          {
            str = cell.substr(cell.find_first_of('=') + 1);
            str = str.substr(str.find_first_of('[') + 1, str.find_first_of(']') - 1);
            std::istringstream iss(str);
            if( ( iss >> std::dec >> value).fail() )
            {
              SyntaxError exception(__PRETTY_FUNCTION__);
              exception.addText("Unable to get a number from string [tag='%s'].", MonitorKeyword::AllowedMembers::initialIndex);
              throw exception;
            }
          }
          keyInitialIndex = value;
          presentMembers_[AllowedMembers::initialIndex] = true;
        }
        else if ( tag == MonitorKeyword::AllowedMembers::transposed )
        {
          transposed = true;
          presentMembers_[AllowedMembers::transposed] = true;
        }
        else if ( tag == MonitorKeyword::AllowedMembers::labels )
        {
          str = cell.substr((cell.find_first_of('[') + 1));
          try
          { // Parse string with the format as follows; label1,label2,...,labelN
            from_string(enumLabels, str);
            presentMembers_[AllowedMembers::labels] = true;
          }
          catch (GCSException& exception)
          {
            exception.addToTrace(__PRETTY_FUNCTION__);
            exception.addText("Unable to get labels from string [tag='%s'].", MonitorKeyword::AllowedMembers::labels);
            throw;
          }
        }
        else if ( tag == MonitorKeyword::AllowedMembers::comment )
        {
          // Parse string with the format as follows; [.+]
          {
            str = cell.substr((cell.find_first_of('=') + 1));
            str = str.substr((str.find_first_of('[') + 1), (str.find_first_of(']') - 1));
          }
          keyComment = str;
          presentMembers_[AllowedMembers::comment] = true;
        }
        else if ( tag == MonitorKeyword::AllowedMembers::when )
        {
          // Parse string with the format as follows; [START|END]
          {
            str = cell.substr((cell.find_first_of('=') + 1));
            str = str.substr((str.find_first_of('[') + 1), (str.find_first_of(']') - 1));
            str.erase(std::remove_if(str.begin(), str.end(), isSpace), str.end());
            if( str.empty() )
            {
              SyntaxError exception(__PRETTY_FUNCTION__);
              exception.addText("Missing when value.");
              throw exception;
            }
          }
          from_string(when, str);
          presentMembers_[AllowedMembers::when] = true;
        }
        else
        {
          SyntaxError exception(__PRETTY_FUNCTION__);
          exception.addText("No valid field have been found: '%s'.", tag.data());
          throw exception;
        }
      }
    }
  }

  // Check data integrity
  checkDataIntegrity();
}

//----------------------------------------------------------------------
// parseFromJSON
//----------------------------------------------------------------------
void MonitorKeyword::parseFromJSON(const Json::Value &node,
                                   const unsigned int &nodeID,
                                   const unsigned short &verbose)
{
  MonitorKeyword_TRACE
  clearAllMembers_();
  verbose_ = verbose;
  id = nodeID;
  if ( not node.isObject() )
  {
    throw SyntaxError(__PRETTY_FUNCTION__, "The type of JSON node must be object.");
  }

  // Initialize the present members flags
  presentMembers_.clear();
  for (const auto& allowedMember: AllowedMembers::all )
  {
    presentMembers_.emplace(allowedMember, false);
  }

  // Check if all members of the given node belong to the set of allowed members
  for (const auto& nodeMember: node.getMemberNames() )
  {
    if ( AllowedMembers::all.count(nodeMember) > 0 )
      presentMembers_[nodeMember] = true;
    else
    {
      // Throw exception if any not-allowed member is present
      SyntaxError exception (__PRETTY_FUNCTION__);
      exception.addText("The '%s' member is not an allowed.", nodeMember.data());
      throw exception;
    }
  }

  // Define alias for each allowed member.
  const Json::Value
      &nodeName         (node[AllowedMembers::name        ])
    , &nodeComment      (node[AllowedMembers::comment     ])
    , &nodeUnit         (node[AllowedMembers::unit        ])
    , &nodeComponent    (node[AllowedMembers::component   ])
    , &nodeMonitor      (node[AllowedMembers::monitor     ])
    , &nodeType         (node[AllowedMembers::type        ])
    , &nodeFormat       (node[AllowedMembers::format      ])
    , &nodeTransposed   (node[AllowedMembers::transposed  ])
    , &nodeLabels       (node[AllowedMembers::labels      ])
    , &nodeRange        (node[AllowedMembers::range       ])
    , &nodeInitialIndex (node[AllowedMembers::initialIndex])
    , &nodeWhen         (node[AllowedMembers::when        ])
    ;

  // Check if the mandatory members are present or not.
  if ( not isMemberPresent(AllowedMembers::name)
       or not isMemberPresent(AllowedMembers::comment)
       or not isMemberPresent(AllowedMembers::component)
       or not isMemberPresent(AllowedMembers::monitor)
       or not isMemberPresent(AllowedMembers::type)
     )
  {
    throw SyntaxError(__PRETTY_FUNCTION__, "At least a mandatory member is missing.");
  }

  // Check types of present mandatory members.
  if (  not nodeName.isString()
        or not nodeComment.isString()
      //or not nodeComponent.isString()
      //or not nodeMonitor.isString()
      //or not nodeType.isString()
     )
  {
    throw SyntaxError(__PRETTY_FUNCTION__, "At least a mandatory member has an invalid type.");
  }

  // Check types of present optional members.
  if (    ( isMemberPresent(AllowedMembers::unit)         and not nodeUnit.isString()         )
       or ( isMemberPresent(AllowedMembers::format)       and not nodeFormat.isString()       )
       or ( isMemberPresent(AllowedMembers::transposed)   and not nodeTransposed.isBool()     )
       or ( isMemberPresent(AllowedMembers::labels)       and not nodeLabels.isArray()        )
       or ( isMemberPresent(AllowedMembers::range)        and not nodeRange.isArray()         )
       or ( isMemberPresent(AllowedMembers::initialIndex) and not nodeInitialIndex.isUInt()   )
       or ( isMemberPresent(AllowedMembers::when)         and not nodeWhen.isString()         )
      )
  {
    throw SyntaxError(__PRETTY_FUNCTION__, "At least an optional member has an invalid type.");
  }
  // labels member must have at least one string element
  if ( isMemberPresent(AllowedMembers::labels) and nodeLabels.size() < 1 )
  {
    SyntaxError exception (__PRETTY_FUNCTION__);
    exception.addText("'%s' member must have at least one string item.", AllowedMembers::labels);
    throw exception;
  }
  // range member must have two natural numbers
  if ( isMemberPresent(AllowedMembers::range)
       and ( nodeRange.size() not_eq 2
             or not nodeRange[0].isUInt()
             or not nodeRange[1].isUInt()
           )
     )
  {
    SyntaxError exception (__PRETTY_FUNCTION__);
    exception.addText("'%s' member must have two natural numbers.", AllowedMembers::range);
    throw exception;
  }

  // Assign values
  // - Mandatory members
  keyName                     = node[AllowedMembers::name].asString();
  keyComment                  = node[AllowedMembers::comment].asString();
  keyUnit                     = node[AllowedMembers::unit].asString();
  deviceName                  = nodeComponent.asString();
  monitorName                 = nodeMonitor.asString();
  std::string monitorTypeStr  = nodeType.asString();
  from_string(monitorType, monitorTypeStr);
  // - Optional members
  std::string keyFormatStr    = isMemberPresent(AllowedMembers::format) ? nodeFormat.asString() : "NONE" ;
  from_string(keyFormat, keyFormatStr);
  transposed =
      isMemberPresent(AllowedMembers::transposed) ?
      nodeTransposed.asBool() :
      false ;
  for ( Json::ArrayIndex i (-1) ; isMemberPresent(AllowedMembers::labels) and ++i < nodeLabels.size() ; )
  {
    if ( not nodeLabels[i].isString() )
    {
      SyntaxError exception (__PRETTY_FUNCTION__);
      exception.addText("The data type of member #%02d in '%s' must be a string.",
                        (i+1), AllowedMembers::labels );
      throw exception;
    }
    enumLabels.push_back(nodeLabels[i].asString());
  }
  arrayRange =
      isMemberPresent(AllowedMembers::range) ?
      std::make_pair(nodeRange[0].asUInt(), nodeRange[1].asUInt()) :
      arrayRange ;
  keyInitialIndex =
      isMemberPresent(AllowedMembers::initialIndex) ?
      nodeInitialIndex.asUInt() :
      isMemberPresent(AllowedMembers::range) ?
      arrayRange.first :
      keyInitialIndex ;
  std::string whenStr = isMemberPresent(AllowedMembers::when) ? nodeWhen.asString() : "START" ;
  from_string(when, whenStr);

  // Check data integrity
  checkDataIntegrity();
}

//----------------------------------------------------------------------
// setLogAgent
//----------------------------------------------------------------------
void MonitorKeyword::setLogAgent(LogAgent *logAgent)
{
  MonitorKeyword_TRACE
  logAgent_ = logAgent;
}

//----------------------------------------------------------------------
// setTrace
//----------------------------------------------------------------------
void MonitorKeyword::setTrace(Trace *&trace)
{
  MonitorKeyword_TRACE
  trace_ = trace;
}

//----------------------------------------------------------------------
// isMemberAllowed
//----------------------------------------------------------------------
bool MonitorKeyword::isMemberAllowed(const std::string &member) const
{
  //MonitorKeyword_TRACE
  bool returnValue (true);
  // is an allowed member ?
  returnValue &= AllowedMembers::all.count(member) > 0;
  return returnValue;
}

//----------------------------------------------------------------------
// isMemberPresent
//----------------------------------------------------------------------
bool MonitorKeyword::isMemberPresent(const std::string& member) const
{
  //MonitorKeyword_TRACE
  bool returnValue (true);
  // is an allowed member ?
  returnValue &= isMemberAllowed(member);
  // is in range ?
  returnValue &= presentMembers_.count(member) > 0;
  if ( returnValue )
  { // is member present ?
    returnValue &= presentMembers_.at(member);
  }
  return returnValue;
}

/************************************************************************
*************************************************************************
**                                                                     **
**                         Protected methods                           **
**                                                                     **
*************************************************************************
************************************************************************/

//----------------------------------------------------------------------
// checkDataIntegrity
//----------------------------------------------------------------------
void MonitorKeyword::checkDataIntegrity()
{
  MonitorKeyword_TRACE
  // keyword and comment members can not be empty
  if (    keyName.empty()
          or keyComment.empty()
          or deviceName.empty()
          or monitorName.empty()
          or monitorType == MonitorType::NONE
      )
  {
    SyntaxError exception (__PRETTY_FUNCTION__);
    exception.addText("The mandatory members can not be empty.");
    throw exception;
  }

  // Check types values
  switch (monitorType)
  {
    case MonitorType::SAMPLED:
    case MonitorType::DISCRETE:
      break;
    case MonitorType::NONE:
    default:
    {
      SyntaxError exception (__PRETTY_FUNCTION__);
      exception.addText("The allowed values of the '%s' member are {'SAMPLED','DISCRETE'}.", AllowedMembers::type);
      throw exception;
    }
  }

  // Check format values
  if ( isMemberPresent(AllowedMembers::format) )
  {
    switch (keyFormat)
    {
      case KeyFormat::RAD_TO_HMS:
      case KeyFormat::RAD_TO_DMS:
      case KeyFormat::RAD_TO_DEG:
      case KeyFormat::ARRAY_TO_SCALAR:
      case KeyFormat::NONE:
        break;
      default:
      {
        SyntaxError exception (__PRETTY_FUNCTION__);
        exception.addText("The allowed values of the '%s' member are {'RAD_TO_HMS','RAD_TO_DMS','RAD_TO_DEG','ARRAY_TO_SCALAR'}.", AllowedMembers::format);
        throw exception;
      }
    }
  }

  // Soft check of range values
  if ( isMemberPresent(AllowedMembers::range) and arrayRange.second < arrayRange.first )
  {
    SyntaxError exception (__PRETTY_FUNCTION__);
    exception.addText("'%s' member has invalid values: second index ('%d') must be greater than or equal to first index ('%d').",
                      AllowedMembers::range, arrayRange.second, arrayRange.first);
    throw exception;
  }


  // Detects optional members, which are not allowed, depending on the value of the type member.
  detectOptionalMembersNotAllowed_();

  // Get reference of MonitorManager.
  MonitorHub *hub_ (nullptr);
  MM::MonitorManager_ifce_var monitorManager_ (nullptr);
  try
  {
    // Obtain pointer of instance of MonitorHub
    hub_ = (MonitorHubFactory::instance()->defaultMonitorHub());
    if ( hub_ == nullptr )
      throw NoEntries(__PRETTY_FUNCTION__, "Unable to get reference to MonitorHubFactory instance");

    // Obtain CORBA reference of MonitorManager
    monitorManager_= (hub_->getMonitorManagerInstance());
    if ( monitorManager_ == nullptr )
      throw NoEntries(__PRETTY_FUNCTION__, "Unable to get reference to MonitorManager");

  }
  catch (CORBA::Exception& ex)
  {
    GCSException gcsex = DException::mapCORBAException(const_cast<CORBA::Exception&>(ex));
    gcsex.addToTrace(__PRETTY_FUNCTION__);
    gcsex.addText("Unable to obtain reference to MonitorManager service.");
    if ( trace_ not_eq nullptr )
      trace_->err( gcsex.toString() );
    if ( logAgent_ not_eq nullptr )
      logAgent_->logError( gcsex.toString() );
  }
  catch (GCSException& ex)
  {
    ex.addToTrace(__PRETTY_FUNCTION__);
    ex.addText("Unable to obtain reference to MonitorManager service.");
    if ( trace_ not_eq nullptr )
      trace_->err( ex.toString() );
    if ( logAgent_ not_eq nullptr )
      logAgent_->logError( ex.toString() );
  }
  catch (...)
  {
    if ( trace_ not_eq nullptr )
      trace_->err("Unable to obtain reference to MonitorManager service.");
    if ( logAgent_ not_eq nullptr )
      logAgent_->logError("Unable to obtain reference to MonitorManager service.");
  }

  // Check  whether the database description of this monitor/magnitude matches with the given information or not
  checkDataBaseDescription_( monitorManager_ );

  // Release all resources.
  if ( monitorManager_ not_eq nullptr )
  {
    //delete monitorManager_;
    monitorManager_ = nullptr;
  }
  if ( hub_ not_eq nullptr )
  {
    //delete hub_;
    hub_ = nullptr;
  }

}

//----------------------------------------------------------------------
// clearAllMembers_
//----------------------------------------------------------------------
void MonitorKeyword::clearAllMembers_()
{
  MonitorKeyword_TRACE
  // public members
  id              = 0;
  keyName         = "";
  keyComment      = "";
  keyUnit         = "";
  deviceName      = "";
  monitorName     = "";
  monitorType     = MonitorType::NONE;
  when            = When::NONE;
  keyFormat       = KeyFormat::NONE;
  transposed      = false;
  keyInitialIndex = 0;
  arrayRange      = ArrayRange(0,0);
  enumLabels.clear();
  // protected members
  presentMembers_.clear();
  verbose_ = 0;
  // private members
}

//----------------------------------------------------------------------
// detectOptionalMembersNotAllowed_
//----------------------------------------------------------------------
void MonitorKeyword::detectOptionalMembersNotAllowed_()
{
  MonitorKeyword_TRACE
  switch (monitorType)
  {
    //<editor-fold desc="Check if any not-allowed optional member is present when type is SAMPLED">
    case MonitorType::SAMPLED:
    {
      // labels member is present, but it is not allowed when type is SAMPLED
      if ( isMemberPresent(AllowedMembers::labels) )
      {
        presentMembers_[AllowedMembers::labels] = false;
        SyntaxError exception (__PRETTY_FUNCTION__);
        exception.addText("'%s' is an optional member reserved when '%s' member is DISCRETE.",
                          AllowedMembers::labels, AllowedMembers::type);
        throw exception;
      }

      // Check if any not-allowed optional member is present when type is SAMPLED and format is not present or its value is not ARRAY_TO_SCALAR
      if ( not isMemberPresent(AllowedMembers::format)
           or keyFormat not_eq KeyFormat::ARRAY_TO_SCALAR
          )
      {

        // transport member is present, but it is not allowed when type is SAMPLED and format is not present or its value is not ARRAY_TO_SCALAR
        if ( isMemberPresent(AllowedMembers::transposed) )
        {
          presentMembers_[AllowedMembers::transposed] = false;
          SyntaxError exception (__PRETTY_FUNCTION__);
          exception.addText("'%s' is an optional member reserved when '%s' member is ARRAY_TO_SCALAR.",
                            AllowedMembers::transposed, AllowedMembers::format);
          throw exception;
        }

        // initialIndex member is present, but it is not allowed when type is SAMPLED and format is not present or its value is not ARRAY_TO_SCALAR
        if ( isMemberPresent(AllowedMembers::initialIndex) )
        {
          presentMembers_[AllowedMembers::initialIndex] = false;
          SyntaxError exception (__PRETTY_FUNCTION__);
          exception.addText("'%s' is an optional member reserved when '%s' member is ARRAY_TO_SCALAR.",
                            AllowedMembers::initialIndex, AllowedMembers::format);
          throw exception;
        }
      }
      break;
    }
    //</editor-fold>

    //<editor-fold desc="Check if any not-allowed optional member is present when type is DISCRETE">
    case MonitorType::DISCRETE:
    {
      // format member is present, but it is not allowed when type is DISCRETE
      if ( isMemberPresent(AllowedMembers::format) )
      {
        presentMembers_[AllowedMembers::format] = false;
        SyntaxError exception (__PRETTY_FUNCTION__);
        exception.addText("'%s' is an optional member reserved when '%s' member is SAMPLED.",
                          AllowedMembers::format, AllowedMembers::type);
        throw exception;
      }

      // transposed member is present, but it is not allowed when type is DISCRETE
      if ( isMemberPresent(AllowedMembers::transposed) )
      {
        presentMembers_[AllowedMembers::transposed] = false;
        SyntaxError exception (__PRETTY_FUNCTION__);
        exception.addText("'%s' is an optional member reserved when '%s' member is SAMPLED.",
                          AllowedMembers::transposed, AllowedMembers::type);
        throw exception;
      }

      // range member is present, but it is not allowed when type is DISCRETE
      if ( isMemberPresent(AllowedMembers::range) )
      {
        presentMembers_[AllowedMembers::range] = false;
        SyntaxError exception (__PRETTY_FUNCTION__);
        exception.addText("%s'' is an optional member reserved when '%s' member is SAMPLED.",
                          AllowedMembers::range, AllowedMembers::type);
        throw exception;
      }
      // initialIndex member is present, but it is not allowed when type is DISCRETE
      if ( isMemberPresent(AllowedMembers::initialIndex) )
      {
        presentMembers_[AllowedMembers::initialIndex] = false;
        SyntaxError exception (__PRETTY_FUNCTION__);
        exception.addText("'%s' is an optional member reserved when '%s' member is SAMPLED.",
                          AllowedMembers::initialIndex, AllowedMembers::type);
        throw exception;
      }
      break;
    }
    //</editor-fold>

    default:
    {
      SyntaxError exception (__PRETTY_FUNCTION__);
      exception.addText("The allowed values of the '%s' member are {'SAMPLED','DISCRETE'}.", AllowedMembers::type);
      throw exception;
    }
  }

}

//----------------------------------------------------------------------
// checkDataBaseDescription_
//----------------------------------------------------------------------
void MonitorKeyword::checkDataBaseDescription_(const MM::MonitorManager_ifce_var& monitorManager)
{
  MonitorKeyword_TRACE
  if ( monitorManager == nullptr )
  {
    NoEntries exception(__PRETTY_FUNCTION__);
    exception.addText(
        "MonitorManager pointer is null. Unable to obtain a description of '%s.%s' monitor/magnitude.",
        deviceName.data(), monitorName.data()
    );
    if ( trace_ not_eq nullptr )
      trace_->err( exception.text() );
    if ( logAgent_ not_eq nullptr )
      logAgent_->logError( exception.text() );
    throw exception;
  }

  try
  {
    switch (monitorType)
    {
      //<editor-fold desc="Hard check data integrity for sampled monitor">
      case MonitorType::SAMPLED:
      {
        // Obtain information of the sampled monitor
        MM::Description_var monitorDescription(monitorManager->getMonitorDescription(deviceName.data(), monitorName.data()));
        if ( monitorDescription == nullptr )
        {
          NoEntries exception (__PRETTY_FUNCTION__);
          exception.addText("MonitorManager returns null description of '%s.%s' monitor.", deviceName.data(), monitorName.data());
          if ( trace_ not_eq nullptr )
            trace_->err( exception.text() );
          if ( logAgent_ not_eq nullptr )
            logAgent_->logError( exception.text() );
          throw exception;
        }
        checkMonitorDescription_(monitorDescription);
        break;
      }// SAMPLED
      //</editor-fold>

      //<editor-fold desc="Hard check data integrity for discrete monitor or magnitude">
      case MonitorType::DISCRETE:
      {
        // Obtain information of the discrete monitor
        MM::MagnitudeDescription_var magnitudeDescription(monitorManager->getMagnitudeDescription(deviceName.data(), monitorName.data()));
        if ( magnitudeDescription == nullptr )
        {
          NoEntries exception (__PRETTY_FUNCTION__);
          exception.addText("MonitorManager returns null description of the '%s.%s' magnitude.", deviceName.data(), monitorName.data());
          throw exception;
        }
        checkMagnitudeDescription_(magnitudeDescription);
        break;
      }// DISCRETE
      //</editor-fold>

      default:
      {
        SyntaxError exception (__PRETTY_FUNCTION__);
        exception.addText("The allowed values of the '%s' member are {'SAMPLED','DISCRETE'}.", AllowedMembers::type);
        throw exception;
      }
    }
  }
  catch (CORBA::Exception& ex)
  {
    GCSException gcsex = DException::mapCORBAException(const_cast<CORBA::Exception&>(ex));
    gcsex.addToTrace(__PRETTY_FUNCTION__);
    gcsex.addText("Problems with '%s.%s' monitor.", deviceName.data(), monitorName.data() );
    if ( trace_ not_eq nullptr )
      trace_->err( gcsex.toString() );
    if ( logAgent_ not_eq nullptr )
      logAgent_->logError( gcsex.toString() );
    throw gcsex;
  }
  catch (GCSException& ex)
  {
    ex.addToTrace(__PRETTY_FUNCTION__);
    ex.addText("Problems with '%s.%s' monitor.", deviceName.data(), monitorName.data() );
    if ( trace_ not_eq nullptr )
      trace_->err( ex.toString() );
    if ( logAgent_ not_eq nullptr )
      logAgent_->logError( ex.toString() );
    throw ex;
  }
  catch (...)
  {
    if ( trace_ not_eq nullptr )
      trace_->err("Problems with '%s.%s' monitor.", deviceName.data(), monitorName.data() );
    if ( logAgent_ not_eq nullptr )
      logAgent_->logError("Problems with '%s.%s' monitor.", deviceName.data(), monitorName.data() );
    throw;
  }
  return;
}

//----------------------------------------------------------------------
// checkMonitorDescription_
//----------------------------------------------------------------------
void MonitorKeyword::checkMonitorDescription_(const MM::Description_var &monitorDescription)
{
  MonitorKeyword_TRACE
  // Check if format information is valid or not
  char type = monitorDescription->type_id;
  switch (type)
  {
    // The monitor is a scalar, if the format member is present, then it must not be 'ARRAY_TO_SCALAR'
    case MM::TYPE_SAMPLE_DOUBLE_LIST:
    case MM::TYPE_SAMPLE_FLOAT_LIST:
    case MM::TYPE_SAMPLE_LONG_LIST:
    case MM::TYPE_SAMPLE_SHORT_LIST:
    case MM::TYPE_SAMPLE_OCTET_LIST:
    {
      if ( isMemberPresent(AllowedMembers::format) and keyFormat == KeyFormat::ARRAY_TO_SCALAR )
      {
        InvalidValue exception(__PRETTY_FUNCTION__);
        exception.addText("The '%s' member can not be 'ARRAY_TO_SCALAR', because the monitor '%s.%s' is a scalar.",
                          AllowedMembers::format, deviceName.data(), monitorName.data());
        throw exception;
      }
      break;
    }
    // The monitor is an array, if the format member is present, then it must be 'ARRAY_TO_SCALAR'
    case MM::TYPE_SAMPLE_DOUBLE_ARRAY1D_LIST:
    case MM::TYPE_SAMPLE_FLOAT_ARRAY1D_LIST:
    case MM::TYPE_SAMPLE_LONG_ARRAY1D_LIST:
    case MM::TYPE_SAMPLE_SHORT_ARRAY1D_LIST:
    case MM::TYPE_SAMPLE_OCTET_ARRAY1D_LIST:
    // Idem
    case MM::TYPE_SAMPLE_DOUBLE_ARRAY2D_LIST:
    case MM::TYPE_SAMPLE_FLOAT_ARRAY2D_LIST:
    case MM::TYPE_SAMPLE_LONG_ARRAY2D_LIST:
    case MM::TYPE_SAMPLE_SHORT_ARRAY2D_LIST:
    case MM::TYPE_SAMPLE_OCTET_ARRAY2D_LIST:
    {
      if (isMemberPresent(AllowedMembers::format)
          and keyFormat not_eq KeyFormat::ARRAY_TO_SCALAR
          and keyFormat not_eq KeyFormat::NONE)
      {
        InvalidValue exception(__PRETTY_FUNCTION__);
        exception.addText("The '%s' member must be 'ARRAY_TO_SCALAR' or 'NONE', because the monitor '%s.%s' is an array.",
                          AllowedMembers::format, deviceName.data(), monitorName.data());
        throw exception;
      }
      break;
    }
      // Unknown type
    default:
    {
      InvalidValue exception(__PRETTY_FUNCTION__);
      exception.addText("Unknown type value ('%c') of the monitor '%s.%s'", type, deviceName.data(), monitorName.data());
      throw exception;
    }
  }

  //<editor-fold desc="Check if range has valid values or assign values">
  /**
   *  @note
   *   - The allowed values are [i,j], such as: 0 <= i <= j <= (N-1) and N=(width * height) of the array monitor.
   *   - If it is not present in the file, then the range will be [0,(width * height - 1)].
   *
   *  @see arrayRange
   */
  const auto & x_or_height (monitorDescription->dimension_x);
  const auto & y_or_width  (monitorDescription->dimension_y);
  const auto dimensions (y_or_width * x_or_height - 1);
  if ( isMemberPresent(AllowedMembers::range) )
  {
    // Check given range
    if ( arrayRange.second > dimensions )
    {
      InvalidValue exception(__PRETTY_FUNCTION__);
      exception.addText(
          "The value ('%d') of second element of '%s' must be less than or equal to the dimensions - 1 ('%d') of '%s.%s' monitor.",
          arrayRange.second,
          AllowedMembers::range,
          dimensions,
          deviceName.data(),
          monitorName.data()
      );
      throw exception;
    }
  }
  else
  {
    // Assign range. No specified range means the whole range is applied
    arrayRange.first = 0;
    arrayRange.second = dimensions;
    presentMembers_[AllowedMembers::range] = true;
  }
  //</editor-fold>
}

//----------------------------------------------------------------------
// checkMagnitudeDescription_
//----------------------------------------------------------------------
void MonitorKeyword::checkMagnitudeDescription_(const MM::MagnitudeDescription_var &magnitudeDescription)
{
  MonitorKeyword_TRACE
  // Check if elements of labels have valid values or assign values, which are retrieved from database, to labels member.
  const auto & magnitudeDescriptionValuesSize = magnitudeDescription->values.length();
  if ( isMemberPresent(AllowedMembers::labels) )
  {
    // Check if both sizes match.
    if ( enumLabels.size() not_eq magnitudeDescriptionValuesSize )
    {
      InvalidValue exception(__PRETTY_FUNCTION__);
      exception.addText(
          "Mismatch sizes of the specified '%s' member ('%d') and the retrieved information from database of the '%s.%s' monitor ('%d').",
          AllowedMembers::labels, enumLabels.size(), deviceName.data(), monitorName.data(), magnitudeDescriptionValuesSize
      );
      throw exception;
    }
    // Check if all elements match
    for (unsigned int i (-1); ++i < magnitudeDescriptionValuesSize; )
    {
      const std::string& dataBaseElement =  magnitudeDescription->values[i].in();
      const std::string& specifiedLabel = enumLabels[i];
      if ( dataBaseElement not_eq specifiedLabel )
      {
        InvalidValue exception(__PRETTY_FUNCTION__);
        exception.addText(
            "Mismatch the specified information (%s[%d]='%s') and the retrieved information from database ('%s.%s'.values[%d]='%s').",
            AllowedMembers::labels, i, specifiedLabel.data(), deviceName.data(), monitorName.data(), i, dataBaseElement.data()
        );
        throw exception;
      }
    }
  }
  else
  {
    // No specified labels means the database labels are applied
    enumLabels.clear();
    for (unsigned int i (0); i < magnitudeDescriptionValuesSize; ++i)
    {
      enumLabels.push_back( magnitudeDescription->values[i].in() );
    }
    presentMembers_[AllowedMembers::labels] = true;
  }
}

/************************************************************************
*************************************************************************
**                                                                     **
**                           Private methods                           **
**                                                                     **
*************************************************************************
************************************************************************/

/************************************************************************
*************************************************************************
**                                                                     **
**                        Other related methods                        **
**                                                                     **
*************************************************************************
************************************************************************/

//----------------------------------------------------------------------
// MonitorKeyword  ==  MonitorKeyword operator
//----------------------------------------------------------------------
bool operator==(const MonitorKeyword &lhs, const MonitorKeyword &rhs) noexcept
{
  MonitorKeyword_TRACE
  return (
      lhs.deviceName == rhs.deviceName and
      lhs.monitorName == rhs.monitorName
  );
}

//----------------------------------------------------------------------
// MonitorKeyword  !=  MonitorKeyword operator
//----------------------------------------------------------------------
bool operator!=(const MonitorKeyword &lhs, const MonitorKeyword &rhs) noexcept
{
  MonitorKeyword_TRACE
  return not(rhs == lhs);
}

//----------------------------------------------------------------------
// MonitorKeyword  <  MonitorKeyword operator
//----------------------------------------------------------------------
bool operator<(const MonitorKeyword &lhs, const MonitorKeyword &rhs) noexcept
{
  MonitorKeyword_TRACE
  if (lhs.deviceName < rhs.deviceName)
    return true;
  if (rhs.deviceName < lhs.deviceName)
    return false;
  return lhs.monitorName < rhs.monitorName;
}

//----------------------------------------------------------------------
// MonitorKeyword  >  MonitorKeyword operator
//----------------------------------------------------------------------
bool operator>(const MonitorKeyword &lhs, const MonitorKeyword &rhs) noexcept
{
  MonitorKeyword_TRACE
  return (rhs < lhs);
}

//----------------------------------------------------------------------
// MonitorKeyword  <=  MonitorKeyword operator
//----------------------------------------------------------------------
bool operator<=(const MonitorKeyword &lhs, const MonitorKeyword &rhs) noexcept
{
  MonitorKeyword_TRACE
  return not(rhs < lhs);
}

//----------------------------------------------------------------------
// MonitorKeyword  >=  MonitorKeyword operator
//----------------------------------------------------------------------
bool operator>=(const MonitorKeyword &lhs, const MonitorKeyword &rhs) noexcept
{
  MonitorKeyword_TRACE
  return not(lhs < rhs);
}

//----------------------------------------------------------------------
// from_string MonitorKeyword::MonitorType
//----------------------------------------------------------------------
void from_string(MonitorKeyword::MonitorType &obj, std::string str)
{
  MonitorKeyword_TRACE
  auto ToUpper = [](unsigned char c) { return std::toupper(c); };
  std::transform(str.begin(), str.end(), str.begin(), ToUpper);
  if      ( str == "NONE"     ) { obj = MonitorKeyword::MonitorType::NONE    ; }
  else if ( str == "SAMPLED"  ) { obj = MonitorKeyword::MonitorType::SAMPLED ; }
  else if ( str == "DISCRETE" ) { obj = MonitorKeyword::MonitorType::DISCRETE; }
  else {
     throw InvalidValue(
         __PRETTY_FUNCTION__,
         "The given value is unknown for MonitorKeyword::MonitorType data type."
     );
  }
}

//----------------------------------------------------------------------
// from_string MonitorKeyword::When
//----------------------------------------------------------------------
void from_string(MonitorKeyword::When &obj, std::string str)
{
  MonitorKeyword_TRACE
  auto ToUpper = [](unsigned char c) { return std::toupper(c); };
  std::transform(str.begin(), str.end(), str.begin(), ToUpper);
  if      ( str == "NONE"  ) { obj = MonitorKeyword::When::NONE  ; }
  else if ( str == "START" ) { obj = MonitorKeyword::When::START ; }
  else if ( str == "END"   ) { obj = MonitorKeyword::When::END   ; }
  else {
    throw InvalidValue(
        __PRETTY_FUNCTION__,
        "The given value is unknown for MonitorKeyword::When data type."
    );
  }
}

//----------------------------------------------------------------------
// from_string MonitorKeyword::KeyFormat
//----------------------------------------------------------------------
void from_string(MonitorKeyword::KeyFormat &obj, std::string str)
{
  MonitorKeyword_TRACE
  auto ToUpper = [](unsigned char c) { return std::toupper(c); };
  std::transform(str.begin(), str.end(), str.begin(), ToUpper);
  if      ( str == "NONE"            ) { obj = MonitorKeyword::KeyFormat::NONE           ; }
  else if ( str == "RAD_TO_HMS"      ) { obj = MonitorKeyword::KeyFormat::RAD_TO_HMS     ; }
  else if ( str == "RAD_TO_DMS"      ) { obj = MonitorKeyword::KeyFormat::RAD_TO_DMS     ; }
  else if ( str == "RAD_TO_DEG"      ) { obj = MonitorKeyword::KeyFormat::RAD_TO_DEG     ; }
  else if ( str == "ARRAY_TO_SCALAR" ) { obj = MonitorKeyword::KeyFormat::ARRAY_TO_SCALAR; }
  else {
    throw InvalidValue(
        __PRETTY_FUNCTION__,
        "The given value is unknown for MonitorKeyword::KeyFormat data type."
    );
  }
}

//----------------------------------------------------------------------
// from_string
//----------------------------------------------------------------------
void from_string(MonitorKeyword::ArrayRange &obj, std::string &str)
{
  MonitorKeyword_TRACE
  std::string::size_type index ( str.find_first_of(',') );
  std::string vX1 (str.substr (0, index));
  std::string vX2 (str.substr (index + 1, std::string::npos));
  std::istringstream issvX1(vX1), issvX2(vX2);
  if( (issvX1 >> std::dec >> obj.first).fail() or (issvX2 >> std::dec >> obj.second).fail() )
  {
    SyntaxError exception(__PRETTY_FUNCTION__);
    exception.addText("Unable to get a number from string.");
    throw exception;
  }
}

//----------------------------------------------------------------------
// from_string
//----------------------------------------------------------------------
void from_string(MonitorKeyword::EnumLabels &obj, std::string &str)
{
  MonitorKeyword_TRACE
  obj.erase(obj.begin(), obj.end());
  obj.clear();
  std::string label("");
  std::stringstream ss(str);
  while( ss.good() )
  {
    std::getline(ss, label, ',');
    obj.emplace_back(label);
  }
}

//----------------------------------------------------------------------
// to_string MonitorKeyword::MonitorType
//----------------------------------------------------------------------
const char* to_string(const MonitorKeyword::MonitorType &obj)
{
  MonitorKeyword_TRACE
  switch(obj)
  {
    case MonitorKeyword::MonitorType::NONE:
      return "NONE";
    case MonitorKeyword::MonitorType::SAMPLED:
      return "SAMPLED";
    case MonitorKeyword::MonitorType::DISCRETE:
      return "DISCRETE";
    default:
      throw InvalidValue(
          __PRETTY_FUNCTION__,
          "The given value is unknown for MonitorKeyword::MonitorType data type."
      );
      return "UNKNOWN";
  }
}

//----------------------------------------------------------------------
// to_string MonitorKeyword::When
//----------------------------------------------------------------------
const char* to_string(const MonitorKeyword::When &obj)
{
  MonitorKeyword_TRACE
  switch(obj)
  {
    case MonitorKeyword::When::NONE:
      return "NONE";
    case MonitorKeyword::When::START:
      return "START";
    case MonitorKeyword::When::END:
      return "END";
    default:
      throw InvalidValue(
          __PRETTY_FUNCTION__,
          "The given value is unknown for MonitorKeyword::When data type."
      );
      return "UNKNOWN";
  }
}

//----------------------------------------------------------------------
// to_string MonitorKeyword::KeyFormat
//----------------------------------------------------------------------
const char* to_string(const MonitorKeyword::KeyFormat &obj)
{
  MonitorKeyword_TRACE
  switch(obj)
  {
    case MonitorKeyword::KeyFormat::NONE:
      return "NONE";
    case MonitorKeyword::KeyFormat::RAD_TO_HMS:
      return "RAD_TO_HMS";
    case MonitorKeyword::KeyFormat::RAD_TO_DMS:
      return "RAD_TO_DMS";
    case MonitorKeyword::KeyFormat::RAD_TO_DEG:
      return "RAD_TO_DEG";
    case MonitorKeyword::KeyFormat::ARRAY_TO_SCALAR:
      return "ARRAY_TO_SCALAR";
    default:
      throw InvalidValue(
          __PRETTY_FUNCTION__,
          "The given value is unknown for MonitorKeyword::KeyFormat data type."
      );
      return "UNKNOWN";
  }
}

//----------------------------------------------------------------------
// to_string MonitorKeyword::ArrayRange
//----------------------------------------------------------------------
const char *to_string(const MonitorKeyword::ArrayRange &obj)
{
  MonitorKeyword_TRACE
  std::ostringstream oss;
  oss << obj.first << ','  << obj.second;
  oss.flush();
  return oss.str().data();
}

//----------------------------------------------------------------------
// to_string MonitorKeyword::EnumLabels
//----------------------------------------------------------------------
const char *to_string(const MonitorKeyword::EnumLabels &obj)
{
  std::ostringstream oss;
  for (const auto &item : obj)
  {
    oss << item << ',';
  }
  oss.flush();
  std::string str (oss.str());
  str = str.substr(0, str.size()-1);
  return str.data();
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------
std::ostream &operator<<(std::ostream &out, const MonitorKeyword::MonitorType &obj)
{
  MonitorKeyword_TRACE
  out << to_string(obj);
  return out;
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------
std::ostream &operator<<(std::ostream &out, const MonitorKeyword::When &obj)
{
  MonitorKeyword_TRACE
  out << to_string(obj);
  return out;
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------
std::ostream &operator<<(std::ostream &out, const MonitorKeyword::KeyFormat &obj)
{
  MonitorKeyword_TRACE
  out << to_string(obj);
  return out;
}
