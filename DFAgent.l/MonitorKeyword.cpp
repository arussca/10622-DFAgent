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
  //;
#endif
//</editor-fold>

/************************************************************************
*************************************************************************
**                                                                     **
**                              Constants                              **
**                                                                     **
*************************************************************************
************************************************************************/

const MonitorKeyword::AllowedMembers::FieldType MonitorKeyword::AllowedMembers::name         = "keyword";
const MonitorKeyword::AllowedMembers::FieldType MonitorKeyword::AllowedMembers::comment      = "comment";
const MonitorKeyword::AllowedMembers::FieldType MonitorKeyword::AllowedMembers::unit         = "unit";
const MonitorKeyword::AllowedMembers::FieldType MonitorKeyword::AllowedMembers::component    = "component";
const MonitorKeyword::AllowedMembers::FieldType MonitorKeyword::AllowedMembers::monitor      = "monitor";
const MonitorKeyword::AllowedMembers::FieldType MonitorKeyword::AllowedMembers::type         = "type";
const MonitorKeyword::AllowedMembers::FieldType MonitorKeyword::AllowedMembers::when         = "when";
const MonitorKeyword::AllowedMembers::FieldType MonitorKeyword::AllowedMembers::format       = "format";
const MonitorKeyword::AllowedMembers::FieldType MonitorKeyword::AllowedMembers::range        = "range";
const MonitorKeyword::AllowedMembers::FieldType MonitorKeyword::AllowedMembers::transposed   = "transposed";
const MonitorKeyword::AllowedMembers::FieldType MonitorKeyword::AllowedMembers::initialIndex = "initialIndex";
const MonitorKeyword::AllowedMembers::FieldType MonitorKeyword::AllowedMembers::labels       = "labels";

#if defined(__cplusplus) && (__cplusplus >= 201103L)

//----------------------------------------------------------------------
// Definition of MonitorKeyword::AllowedMembers::all static variable
// without initialization
//----------------------------------------------------------------------
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

#else

//----------------------------------------------------------------------
// Definition of MonitorKeyword::AllowedMembers::all static variable
// without initialization
//----------------------------------------------------------------------
MonitorKeyword::AllowedMembers::FieldsContainterType MonitorKeyword::AllowedMembers::all;

//----------------------------------------------------------------------
// Insert all allowed fields in all container.
//----------------------------------------------------------------------
void MonitorKeyword::AllowedMembers::insertAllAllowedFields()
{
  MonitorKeyword_TRACE
  if ( MonitorKeyword::AllowedMembers::all.empty() )
  {
    // Mandatory members related to FITS header keyword
    MonitorKeyword::AllowedMembers::all.insert(MonitorKeyword::AllowedMembers::name);
    MonitorKeyword::AllowedMembers::all.insert(MonitorKeyword::AllowedMembers::comment);
    MonitorKeyword::AllowedMembers::all.insert(MonitorKeyword::AllowedMembers::unit);
    // Mandatory members related to any monitor
    MonitorKeyword::AllowedMembers::all.insert(MonitorKeyword::AllowedMembers::component);
    MonitorKeyword::AllowedMembers::all.insert(MonitorKeyword::AllowedMembers::monitor);
    MonitorKeyword::AllowedMembers::all.insert(MonitorKeyword::AllowedMembers::type);
    // Optional members related to both types
    MonitorKeyword::AllowedMembers::all.insert(MonitorKeyword::AllowedMembers::when);
    // Optional members related to sampled monitor
    MonitorKeyword::AllowedMembers::all.insert(MonitorKeyword::AllowedMembers::format);
    MonitorKeyword::AllowedMembers::all.insert(MonitorKeyword::AllowedMembers::transposed);
    MonitorKeyword::AllowedMembers::all.insert(MonitorKeyword::AllowedMembers::range);
    MonitorKeyword::AllowedMembers::all.insert(MonitorKeyword::AllowedMembers::initialIndex);
    // Optional members related to discrete monitor
    MonitorKeyword::AllowedMembers::all.insert(MonitorKeyword::AllowedMembers::labels);
  }
}

#endif // defined(__cplusplus) && (__cplusplus >= 201103L)

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
    , monitorType(MonitorKeyword/*::MonitorType*/::MonitorType_NONE)
    , when(MonitorKeyword/*::When*/::When_NONE)
    , keyFormat(MonitorKeyword/*::KeyFormat*/::KeyFormat_NONE)
    , transposed(false)
    , keyInitialIndex(0)
    , arrayRange()
    , enumLabels()
    // protected
    , presentMembers_()
    , verbose_(0)
    , logAgent_(NULL)
    , trace_()
// private
{
  MonitorKeyword_TRACE

  #if defined(__cplusplus) && (__cplusplus >= 201103L)
  // Nothing to do. The MonitorKeyword::AllowedMembers::all was declared const
  // and initialized in above statements.
  #else
  MonitorKeyword::AllowedMembers::insertAllAllowedFields();
  #endif

  
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
    , monitorType( MonitorKeyword/*::MonitorType*/::MonitorType_NONE)
    , when(MonitorKeyword/*::When*/::When_NONE)
    , keyFormat(MonitorKeyword/*::KeyFormat*/::KeyFormat_NONE)
    , transposed(false)
    , keyInitialIndex(0)
    , arrayRange(std::make_pair(0, 0))
    , enumLabels()
    // protected
    , presentMembers_()
    , verbose_(verbose)
    , logAgent_(logAgent)
    , trace_(& trace)
// private
{
  MonitorKeyword_TRACE

  #if defined(__cplusplus) && (__cplusplus >= 201103L)
  // Nothing to do. The MonitorKeyword::AllowedMembers::all was declared const
  // and initialized in above statements.
  #else
  MonitorKeyword::AllowedMembers::insertAllAllowedFields();
  #endif //defined(__cplusplus) && (__cplusplus >= 201103L)

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
    , monitorType( MonitorKeyword/*::MonitorType*/::MonitorType_NONE)
    , when(MonitorKeyword/*::When*/::When_NONE)
    , keyFormat(MonitorKeyword/*::KeyFormat*/::KeyFormat_NONE)
    , transposed(false)
    , keyInitialIndex(0)
    , arrayRange(std::make_pair(0, 0))
    , enumLabels()
    // protected
    , presentMembers_()
    , verbose_(verbose)
    , logAgent_(logAgent)
    , trace_(& trace)
    // private
{
  MonitorKeyword_TRACE

  #if defined(__cplusplus) && (__cplusplus >= 201103L)
  // Nothing to do. The MonitorKeyword::AllowedMembers::all was declared const
  // and initialized in above statements.
  #else
  MonitorKeyword::AllowedMembers::insertAllAllowedFields();
  #endif

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
  other.logAgent_ = NULL;
  other.trace_ = NULL;
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
  other.logAgent_       = NULL;
  other.trace_          = NULL;
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
  logAgent_ = NULL;
  trace_ = NULL;
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
  for (MonitorKeyword::AllowedMembers::FieldsContainterType::iterator it = MonitorKeyword::AllowedMembers::all.begin();
       it != MonitorKeyword::AllowedMembers::all.end();
       ++it)
  {
    presentMembers_[*it] = false;
  }

  // # Auxiliary elements
  std::string str("");
  std::stringstream ss(line);

  // # Start parsing fields
  // ## Parsing KeyName field
  {
    ss >> keyName;
    keyName.erase(std::remove_if(keyName.begin(), keyName.end(), (int(*)(int)) std::isspace), keyName.end());
    if( keyName.empty() )
    {
      SyntaxError exception(__PRETTY_FUNCTION__);
      exception.addText("Missing keyword name.");
      throw exception;
    }
    presentMembers_[MonitorKeyword::AllowedMembers::name] = true;
  }
  // ## Parsing deviceName field
  {
    ss >> deviceName;
    deviceName.erase(std::remove_if(deviceName.begin(), deviceName.end(), (int(*)(int)) std::isspace), deviceName.end());
    if( deviceName.empty() )
    {
      SyntaxError exception(__PRETTY_FUNCTION__);
      exception.addText("Missing device name.");
      throw exception;
    }
    presentMembers_[MonitorKeyword::AllowedMembers::component] = true;
  }
  // ## Parsing monitorName field
  {
    ss >> monitorName;
    monitorName.erase(std::remove_if(monitorName.begin(), monitorName.end(), (int(*)(int)) std::isspace), monitorName.end());
    if( monitorName.empty() )
    {
      SyntaxError exception(__PRETTY_FUNCTION__);
      exception.addText("Missing monitor name.");
      throw exception;
    }
    presentMembers_[MonitorKeyword::AllowedMembers::monitor] = true;
  }
  // ## Parsing monitorType field
  {
    ss >> str;
    str.erase(std::remove_if(str.begin(), str.end(), (int(*)(int)) std::isspace), str.end());
    if( str.empty() )
    {
      SyntaxError exception(__PRETTY_FUNCTION__);
      exception.addText("Missing monitor type value.");
      throw exception;
    }
    from_string(monitorType, str);
    presentMembers_[MonitorKeyword::AllowedMembers::type] = true;
  }
  // ## Parsing keyFormat field
  {
    ss >> str;
    str.erase(std::remove_if(str.begin(), str.end(), (int(*)(int)) std::isspace), str.end());
    if( str.empty() )
    {
      SyntaxError exception(__PRETTY_FUNCTION__);
      exception.addText("Missing transformation value.");
      throw exception;
    }
    from_string(keyFormat, str);
    presentMembers_[MonitorKeyword::AllowedMembers::format] = true;
  }
  // ## Optional fields: must be tab separated!!! No spaces allowed!!
  while( ss.good() )
  {
    std::string cell;
    std::getline(ss, cell, ']');
    if( not cell.empty() )
    {
      std::string tag ( cell.substr(0, cell.find_first_of('=')) );
      tag.erase(std::remove_if(tag.begin(), tag.end(), (int(*)(int)) std::isspace), tag.end());
      std::transform(tag.begin(), tag.end(), tag.begin(), (int(*)(int)) std::tolower);
      if ( not tag.empty() )
      {
        if ( tag == MonitorKeyword::AllowedMembers::range )
        {
          str = cell.substr(cell.find_first_of('[') + 1);
          try
          {
            // Parse string with the format as follows; value1,value2
            from_string(arrayRange, str);
            presentMembers_[MonitorKeyword::AllowedMembers::range] = true;
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
          presentMembers_[MonitorKeyword::AllowedMembers::initialIndex] = true;
        }
        else if ( tag == MonitorKeyword::AllowedMembers::transposed )
        {
          transposed = true;
          presentMembers_[MonitorKeyword::AllowedMembers::transposed] = true;
        }
        else if ( tag == MonitorKeyword::AllowedMembers::labels )
        {
          str = cell.substr((cell.find_first_of('[') + 1));
          try
          {
            // Parse string with the format as follows; label1,label2,...,labelN
            from_string(enumLabels, str);
            presentMembers_[MonitorKeyword::AllowedMembers::labels] = true;
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
          presentMembers_[MonitorKeyword::AllowedMembers::comment] = true;
        }
        else if ( tag == MonitorKeyword::AllowedMembers::when )
        {
          // Parse string with the format as follows; [START|END]
          {
            str = cell.substr((cell.find_first_of('=') + 1));
            str = str.substr((str.find_first_of('[') + 1), (str.find_first_of(']') - 1));
            /// @todo: Remove // str.erase(std::remove_if(str.begin(), str.end(), isSpace), str.end());
            if( str.empty() )
            {
              SyntaxError exception(__PRETTY_FUNCTION__);
              exception.addText("Missing when value.");
              throw exception;
            }
          }
          from_string(when, str);
          presentMembers_[MonitorKeyword::AllowedMembers::when] = true;
        }
        else
        {
          SyntaxError exception(__PRETTY_FUNCTION__);
          exception.addText("No valid field have been found: '%s'.", tag.c_str());
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
  for (MonitorKeyword::AllowedMembers::FieldsContainterType::const_iterator it = MonitorKeyword::AllowedMembers::all.begin();
       it != MonitorKeyword::AllowedMembers::all.end();
       ++it)
  {
    presentMembers_[*it] = false;
  }

  // Check if all members of the given node belong to the set of allowed members
  {
    const Json::Value::Members & nodeMembers = node.getMemberNames();
    for (Json::Value::Members::const_iterator nodeMember = nodeMembers.begin();
         nodeMember != nodeMembers.end();
         ++nodeMember)
    {
      if ( MonitorKeyword::AllowedMembers::all.count(*nodeMember) > 0 )
      {
        presentMembers_[*nodeMember] = true;
      }
      else
      {
        // Throw exception if any not-allowed member is present
        SyntaxError exception (__PRETTY_FUNCTION__);
        exception.addText("The '%s' member is not an allowed.", nodeMember->c_str());
        throw exception;
      }
    }
  }

  // Define alias for each allowed member.
  const Json::Value
      &nodeName         (node[MonitorKeyword::AllowedMembers::name        ])
    , &nodeComment      (node[MonitorKeyword::AllowedMembers::comment     ])
    , &nodeUnit         (node[MonitorKeyword::AllowedMembers::unit        ])
    , &nodeComponent    (node[MonitorKeyword::AllowedMembers::component   ])
    , &nodeMonitor      (node[MonitorKeyword::AllowedMembers::monitor     ])
    , &nodeType         (node[MonitorKeyword::AllowedMembers::type        ])
    , &nodeFormat       (node[MonitorKeyword::AllowedMembers::format      ])
    , &nodeTransposed   (node[MonitorKeyword::AllowedMembers::transposed  ])
    , &nodeLabels       (node[MonitorKeyword::AllowedMembers::labels      ])
    , &nodeRange        (node[MonitorKeyword::AllowedMembers::range       ])
    , &nodeInitialIndex (node[MonitorKeyword::AllowedMembers::initialIndex])
    , &nodeWhen         (node[MonitorKeyword::AllowedMembers::when        ])
    ;

  // Check if the mandatory members are present or not.
  if ( not isMemberPresent(MonitorKeyword::AllowedMembers::name)
     // @note: comment should be a mandatory field, but meanwhile all thoses file do not contain this field,
     // then is will be an optional field.
     //or not isMemberPresent(MonitorKeyword::AllowedMembers::comment)
       or not isMemberPresent(MonitorKeyword::AllowedMembers::component)
       or not isMemberPresent(MonitorKeyword::AllowedMembers::monitor)
       or not isMemberPresent(MonitorKeyword::AllowedMembers::type)
     )
  {
    throw SyntaxError(__PRETTY_FUNCTION__, "At least a mandatory member is missing.");
  }

  // Check types of present mandatory members.
  if (  not nodeName.isString()
      // @note: comment should be a mandatory field, but meanwhile all thoses file do not contain this field,
      // then is will be an optional field.
      //or not nodeComment.isString()
        or not nodeComponent.isString()
        or not nodeMonitor.isString()
        or not nodeType.isString()
     )
  {
    throw SyntaxError(__PRETTY_FUNCTION__, "At least a mandatory member has an invalid type.");
  }

  // Check types of present optional members.
  if (    ( isMemberPresent(MonitorKeyword::AllowedMembers::unit)         and not nodeUnit.isString()         )
       // @note: comment should be a mandatory field, but meanwhile all thoses file do not contain this field,
       // then is will be an optional field.
       or ( isMemberPresent(MonitorKeyword::AllowedMembers::comment)      and not nodeComment.isString()      )
       or ( isMemberPresent(MonitorKeyword::AllowedMembers::format)       and not nodeFormat.isString()       )
       or ( isMemberPresent(MonitorKeyword::AllowedMembers::transposed)   and not nodeTransposed.isBool()     )
       or ( isMemberPresent(MonitorKeyword::AllowedMembers::labels)       and not nodeLabels.isArray()        )
       or ( isMemberPresent(MonitorKeyword::AllowedMembers::range)        and not nodeRange.isArray()         )
       or ( isMemberPresent(MonitorKeyword::AllowedMembers::initialIndex) and not (nodeInitialIndex.isInt() or nodeInitialIndex.isUInt() ) )
       or ( isMemberPresent(MonitorKeyword::AllowedMembers::when)         and not nodeWhen.isString()         )
      )
  {
    throw SyntaxError(__PRETTY_FUNCTION__, "At least an optional member has an invalid type.");
  }
  // labels member must have at least one string element
  if ( isMemberPresent(MonitorKeyword::AllowedMembers::labels) and nodeLabels.size() < 1 )
  {
    SyntaxError exception (__PRETTY_FUNCTION__);
    exception.addText("'%s' member must have at least one string item.", MonitorKeyword::AllowedMembers::labels);
    throw exception;
  }
  // range member must have two natural numbers
  #ifdef VXWORKS
  typedef Json::Value::ArrayIndex ArrayIndex;
  #elif LINUX
  typedef Json::ArrayIndex ArrayIndex;
  #endif
  static const ArrayIndex index_first(0), index_last(1);
  if ( isMemberPresent(MonitorKeyword::AllowedMembers::range)
       and ( nodeRange.size() not_eq 2
             or ( nodeRange[index_first].asInt() < 0 )
             or ( nodeRange[index_last].asInt()  < 0 )
           )
     )
  {
    SyntaxError exception (__PRETTY_FUNCTION__);
    exception.addText("'%s' member must have two natural numbers.", MonitorKeyword::AllowedMembers::range);
    throw exception;
  }

  // Assign values
  // - Mandatory members
  keyName                     = node[MonitorKeyword::AllowedMembers::name].asString();
  keyComment                  = node[MonitorKeyword::AllowedMembers::comment].asString();
  keyUnit                     = node[MonitorKeyword::AllowedMembers::unit].asString();
  deviceName                  = nodeComponent.asString();
  monitorName                 = nodeMonitor.asString();
  std::string monitorTypeStr  = nodeType.asString();
  from_string(monitorType, monitorTypeStr);
  // - Optional members
  std::string keyFormatStr    = isMemberPresent(MonitorKeyword::AllowedMembers::format) ? nodeFormat.asString() : "NONE" ;
  from_string(keyFormat, keyFormatStr);
  transposed = isMemberPresent(MonitorKeyword::AllowedMembers::transposed) and nodeTransposed.asBool();
  for ( ArrayIndex i (0) ; isMemberPresent(MonitorKeyword::AllowedMembers::labels) and i < nodeLabels.size() ; ++i )
  {
    if ( not nodeLabels[i].isString() )
    {
      SyntaxError exception (__PRETTY_FUNCTION__);
      exception.addText("The data type of member #%02d in '%s' must be a string.",
                        (i+1), MonitorKeyword::AllowedMembers::labels );
      throw exception;
    }
    enumLabels.push_back(nodeLabels[i].asString());
  }
  arrayRange =
      isMemberPresent(MonitorKeyword::AllowedMembers::range) ?
      std::make_pair(nodeRange[index_first].asUInt(), nodeRange[index_last].asUInt()) :
      arrayRange ;
  keyInitialIndex =
      isMemberPresent(MonitorKeyword::AllowedMembers::initialIndex) ?
      nodeInitialIndex.asUInt() :
      isMemberPresent(MonitorKeyword::AllowedMembers::range) ?
      arrayRange.first :
      keyInitialIndex ;
  std::string whenStr = isMemberPresent(MonitorKeyword::AllowedMembers::when) ? nodeWhen.asString() : "START" ;
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
  returnValue &= MonitorKeyword::AllowedMembers::all.count(member) > 0;
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
  MapPresentMembers::const_iterator it = presentMembers_.find(member);
  returnValue &= it not_eq presentMembers_.end();
  if ( returnValue )
  { // is member present ?
    returnValue &= it->second;
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
      // @note: comment should be a mandatory field, but meanwhile all thoses file do not contain this field,
      // then is will be an optional field.
      //  or keyComment.empty()
          or deviceName.empty()
          or monitorName.empty()
          or monitorType == MonitorKeyword::MonitorType_NONE
      )
  {
    SyntaxError exception (__PRETTY_FUNCTION__);
    exception.addText("The mandatory members can not be empty.");
    throw exception;
  }

  // Check types values
  switch (monitorType)
  {
    case  MonitorKeyword/*::MonitorType*/::SAMPLED:
    case  MonitorKeyword/*::MonitorType*/::DISCRETE:
      break;
    case  MonitorKeyword/*::MonitorType*/::MonitorType_NONE:
    default:
    {
      SyntaxError exception (__PRETTY_FUNCTION__);
      exception.addText("The allowed values of the '%s' member are {'SAMPLED','DISCRETE'}.", MonitorKeyword::AllowedMembers::type);
      throw exception;
    }
  }

  // Check format values
  if ( isMemberPresent(MonitorKeyword::AllowedMembers::format) )
  {
    switch (keyFormat)
    {
      case MonitorKeyword/*::KeyFormat*/::RAD_TO_HMS:
      case MonitorKeyword/*::KeyFormat*/::RAD_TO_DMS:
      case MonitorKeyword/*::KeyFormat*/::RAD_TO_DEG:
      case MonitorKeyword/*::KeyFormat*/::ARRAY_TO_SCALAR:
      case MonitorKeyword/*::KeyFormat*/::KeyFormat_NONE:
        break;
      default:
      {
        SyntaxError exception (__PRETTY_FUNCTION__);
        exception.addText("The allowed values of the '%s' member are {'RAD_TO_HMS','RAD_TO_DMS','RAD_TO_DEG','ARRAY_TO_SCALAR'}.", MonitorKeyword::AllowedMembers::format);
        throw exception;
      }
    }
  }

  // Soft check of range values
  if ( isMemberPresent(MonitorKeyword::AllowedMembers::range) and arrayRange.second < arrayRange.first )
  {
    SyntaxError exception (__PRETTY_FUNCTION__);
    exception.addText("'%s' member has invalid values: second index ('%d') must be greater than or equal to first index ('%d').",
                      MonitorKeyword::AllowedMembers::range, arrayRange.second, arrayRange.first);
    throw exception;
  }


  // Detects optional members, which are not allowed, depending on the value of the type member.
  detectOptionalMembersNotAllowed_();

  // Get reference of MonitorManager.
  MonitorHub *hub_ (NULL);
  MM::MonitorManager_ifce_var monitorManager_ (NULL);
  try
  {
    // Obtain pointer of instance of MonitorHub
    hub_ = (MonitorHubFactory::instance()->defaultMonitorHub());
    if ( hub_ == NULL )
      throw NoEntries(__PRETTY_FUNCTION__, "Unable to get reference to MonitorHubFactory instance");

    // Obtain CORBA reference of MonitorManager
    monitorManager_= (hub_->getMonitorManagerInstance());
    if ( monitorManager_.ptr() not_eq NULL )
      throw NoEntries(__PRETTY_FUNCTION__, "Unable to get reference to MonitorManager");

  }
  catch (CORBA::Exception& ex)
  {
    GCSException gcsex = DException::mapCORBAException(const_cast<CORBA::Exception&>(ex));
    gcsex.addToTrace(__PRETTY_FUNCTION__);
    gcsex.addText("Unable to obtain reference to MonitorManager service.");
    if ( trace_ not_eq NULL )
      trace_->err( gcsex.toString() );
    if ( logAgent_ not_eq NULL )
      logAgent_->logError( gcsex.toString() );
  }
  catch (GCSException& ex)
  {
    ex.addToTrace(__PRETTY_FUNCTION__);
    ex.addText("Unable to obtain reference to MonitorManager service.");
    if ( trace_ not_eq NULL )
      trace_->err( ex.toString() );
    if ( logAgent_ not_eq NULL )
      logAgent_->logError( ex.toString() );
  }
  catch (...)
  {
    if ( trace_ not_eq NULL )
      trace_->err("Unable to obtain reference to MonitorManager service.");
    if ( logAgent_ not_eq NULL )
      logAgent_->logError("Unable to obtain reference to MonitorManager service.");
  }

  // Check  whether the database description of this monitor/magnitude matches with the given information or not
  checkDataBaseDescription_( monitorManager_ );

  // Release all resources.
  if ( monitorManager_.ptr() not_eq NULL )
  {
    //delete monitorManager_;
    monitorManager_ = NULL;
  }
  if ( hub_ not_eq NULL )
  {
    //delete hub_;
    hub_ = NULL;
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
  monitorType     =  MonitorKeyword/*::MonitorType*/::MonitorType_NONE;
  when            = MonitorKeyword/*::When*/::When_NONE;
  keyFormat       = MonitorKeyword/*::KeyFormat*/::KeyFormat_NONE;
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
    case  MonitorKeyword/*::MonitorType*/::SAMPLED:
    {
      // labels member is present, but it is not allowed when type is SAMPLED
      if ( isMemberPresent(MonitorKeyword::AllowedMembers::labels) )
      {
        presentMembers_[MonitorKeyword::AllowedMembers::labels] = false;
        SyntaxError exception (__PRETTY_FUNCTION__);
        exception.addText("'%s' is an optional member reserved when '%s' member is DISCRETE.",
                          MonitorKeyword::AllowedMembers::labels, MonitorKeyword::AllowedMembers::type);
        throw exception;
      }

      // Check if any not-allowed optional member is present when type is SAMPLED and format is not present or its value is not ARRAY_TO_SCALAR
      if ( not isMemberPresent(MonitorKeyword::AllowedMembers::format)
           or keyFormat not_eq MonitorKeyword/*::KeyFormat*/::ARRAY_TO_SCALAR
          )
      {

        // transport member is present, but it is not allowed when type is SAMPLED and format is not present or its value is not ARRAY_TO_SCALAR
        if ( isMemberPresent(MonitorKeyword::AllowedMembers::transposed) )
        {
          presentMembers_[MonitorKeyword::AllowedMembers::transposed] = false;
          SyntaxError exception (__PRETTY_FUNCTION__);
          exception.addText("'%s' is an optional member reserved when '%s' member is ARRAY_TO_SCALAR.",
                            MonitorKeyword::AllowedMembers::transposed, MonitorKeyword::AllowedMembers::format);
          throw exception;
        }

        // initialIndex member is present, but it is not allowed when type is SAMPLED and format is not present or its value is not ARRAY_TO_SCALAR
        if ( isMemberPresent(MonitorKeyword::AllowedMembers::initialIndex) )
        {
          presentMembers_[MonitorKeyword::AllowedMembers::initialIndex] = false;
          SyntaxError exception (__PRETTY_FUNCTION__);
          exception.addText("'%s' is an optional member reserved when '%s' member is ARRAY_TO_SCALAR.",
                            MonitorKeyword::AllowedMembers::initialIndex, MonitorKeyword::AllowedMembers::format);
          throw exception;
        }
      }
      break;
    }
    //</editor-fold>

    //<editor-fold desc="Check if any not-allowed optional member is present when type is DISCRETE">
    case  MonitorKeyword/*::MonitorType*/::DISCRETE:
    {
      // If format member is present and it is not NONE, then it is not allowed when type is DISCRETE
      if ( isMemberPresent(MonitorKeyword::AllowedMembers::format) and
           MonitorKeyword::keyFormat not_eq MonitorKeyword/*::KeyFormat*/::KeyFormat_NONE )
      {
        presentMembers_[MonitorKeyword::AllowedMembers::format] = false;
        SyntaxError exception (__PRETTY_FUNCTION__);
        exception.addText("'%s' must be 'NONE' when '%s' member is DISCRETE.",
                          MonitorKeyword::AllowedMembers::format, MonitorKeyword::AllowedMembers::type);
        throw exception;
      }

      // transposed member is present, but it is not allowed when type is DISCRETE
      if ( isMemberPresent(MonitorKeyword::AllowedMembers::transposed) )
      {
        presentMembers_[MonitorKeyword::AllowedMembers::transposed] = false;
        SyntaxError exception (__PRETTY_FUNCTION__);
        exception.addText("'%s' is an optional member reserved when '%s' member is SAMPLED.",
                          MonitorKeyword::AllowedMembers::transposed, MonitorKeyword::AllowedMembers::type);
        throw exception;
      }

      // range member is present, but it is not allowed when type is DISCRETE
      if ( isMemberPresent(MonitorKeyword::AllowedMembers::range) )
      {
        presentMembers_[MonitorKeyword::AllowedMembers::range] = false;
        SyntaxError exception (__PRETTY_FUNCTION__);
        exception.addText("%s'' is an optional member reserved when '%s' member is SAMPLED.",
                          MonitorKeyword::AllowedMembers::range, MonitorKeyword::AllowedMembers::type);
        throw exception;
      }
      // initialIndex member is present, but it is not allowed when type is DISCRETE
      if ( isMemberPresent(MonitorKeyword::AllowedMembers::initialIndex) )
      {
        presentMembers_[MonitorKeyword::AllowedMembers::initialIndex] = false;
        SyntaxError exception (__PRETTY_FUNCTION__);
        exception.addText("'%s' is an optional member reserved when '%s' member is SAMPLED.",
                          MonitorKeyword::AllowedMembers::initialIndex, MonitorKeyword::AllowedMembers::type);
        throw exception;
      }
      break;
    }
    //</editor-fold>

    default:
    {
      SyntaxError exception (__PRETTY_FUNCTION__);
      exception.addText("The allowed values of the '%s' member are {'SAMPLED','DISCRETE'}.", MonitorKeyword::AllowedMembers::type);
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
  if ( monitorManager == NULL )
  {
    NoEntries exception(__PRETTY_FUNCTION__);
    exception.addText(
        "MonitorManager pointer is null. Unable to obtain a description of '%s.%s' monitor/magnitude.",
        deviceName.c_str(), monitorName.c_str()
    );
    if ( trace_ not_eq NULL )
      trace_->err( exception.text() );
    if ( logAgent_ not_eq NULL )
      logAgent_->logError( exception.text() );
    throw exception;
  }

  try
  {
    switch (monitorType)
    {
      //<editor-fold desc="Hard check data integrity for sampled monitor">
      case  MonitorKeyword/*::MonitorType*/::SAMPLED:
      {
        // Obtain information of the sampled monitor
        MM::Description_var monitorDescription(monitorManager->getMonitorDescription(deviceName.c_str(), monitorName.c_str()));
        if ( monitorDescription == NULL )
        {
          NoEntries exception (__PRETTY_FUNCTION__);
          exception.addText("MonitorManager returns null description of '%s.%s' monitor.", deviceName.c_str(), monitorName.c_str());
          if ( trace_ not_eq NULL )
            trace_->err( exception.text() );
          if ( logAgent_ not_eq NULL )
            logAgent_->logError( exception.text() );
          throw exception;
        }
        checkMonitorDescription_(monitorDescription);
        break;
      }// SAMPLED
      //</editor-fold>

      //<editor-fold desc="Hard check data integrity for discrete monitor or magnitude">
      case  MonitorKeyword/*::MonitorType*/::DISCRETE:
      {
        // Obtain information of the discrete monitor
        MM::MagnitudeDescription_var magnitudeDescription(monitorManager->getMagnitudeDescription(deviceName.c_str(), monitorName.c_str()));
        if ( magnitudeDescription == NULL )
        {
          NoEntries exception (__PRETTY_FUNCTION__);
          exception.addText("MonitorManager returns null description of the '%s.%s' magnitude.", deviceName.c_str(), monitorName.c_str());
          throw exception;
        }
        checkMagnitudeDescription_(magnitudeDescription);
        break;
      }// DISCRETE
      //</editor-fold>

      default:
      {
        SyntaxError exception (__PRETTY_FUNCTION__);
        exception.addText("The allowed values of the '%s' member are {'SAMPLED','DISCRETE'}.", MonitorKeyword::AllowedMembers::type);
        throw exception;
      }
    }
  }
  catch (CORBA::Exception& ex)
  {
    GCSException gcsex = DException::mapCORBAException(const_cast<CORBA::Exception&>(ex));
    gcsex.addToTrace(__PRETTY_FUNCTION__);
    gcsex.addText("Problems with '%s.%s' monitor.", deviceName.c_str(), monitorName.c_str() );
    if ( trace_ not_eq NULL )
      trace_->err( gcsex.toString() );
    if ( logAgent_ not_eq NULL )
      logAgent_->logError( gcsex.toString() );
    throw gcsex;
  }
  catch (GCSException& ex)
  {
    ex.addToTrace(__PRETTY_FUNCTION__);
    ex.addText("Problems with '%s.%s' monitor.", deviceName.c_str(), monitorName.c_str() );
    if ( trace_ not_eq NULL )
      trace_->err( ex.toString() );
    if ( logAgent_ not_eq NULL )
      logAgent_->logError( ex.toString() );
    throw ex;
  }
  catch (...)
  {
    if ( trace_ not_eq NULL )
      trace_->err("Problems with '%s.%s' monitor.", deviceName.c_str(), monitorName.c_str() );
    if ( logAgent_ not_eq NULL )
      logAgent_->logError("Problems with '%s.%s' monitor.", deviceName.c_str(), monitorName.c_str() );
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
      if ( isMemberPresent(MonitorKeyword::AllowedMembers::format) and keyFormat == MonitorKeyword/*::KeyFormat*/::ARRAY_TO_SCALAR )
      {
        InvalidValue exception(__PRETTY_FUNCTION__);
        exception.addText("The '%s' member can not be 'ARRAY_TO_SCALAR', because the monitor '%s.%s' is a scalar.",
                          MonitorKeyword::AllowedMembers::format, deviceName.c_str(), monitorName.c_str());
        throw exception;
      }
      break;
    }
    // The monitor is an array, if the format member is present, then it must be 'ARRAY_TO_SCALAR'
    // Array 1D
    case MM::TYPE_SAMPLE_DOUBLE_ARRAY1D_LIST:
    case MM::TYPE_SAMPLE_FLOAT_ARRAY1D_LIST:
    case MM::TYPE_SAMPLE_LONG_ARRAY1D_LIST:
    case MM::TYPE_SAMPLE_SHORT_ARRAY1D_LIST:
    case MM::TYPE_SAMPLE_OCTET_ARRAY1D_LIST:
    // Array 2D
    case MM::TYPE_SAMPLE_DOUBLE_ARRAY2D_LIST:
    /**
     * @bug Mismatch between declared value of TYPE_SAMPLE_DOUBLE_ARRAY2D_LIST (1) in MMCommon.idl and
     *      value assigned to column monitormanager.monitor_description.type for double array 2D (9).
     */
    case '9':

    case MM::TYPE_SAMPLE_FLOAT_ARRAY2D_LIST:
    /**
     * @bug Mismatch between declared value of TYPE_SAMPLE_FLOAT_ARRAY2D_LIST (2) in MMCommon.idl and
     *      value assigned to column monitormanager.monitor_description.type for float array 2D (8).
     */
    case '8':

    case MM::TYPE_SAMPLE_LONG_ARRAY2D_LIST:
    /**
     * @bug Mismatch between declared value of TYPE_SAMPLE_LONG_ARRAY2D_LIST (3) in MMCommon.idl and
     *      value assigned to column monitormanager.monitor_description.type for long array 2D (7).
     */
    case '7':

    case MM::TYPE_SAMPLE_SHORT_ARRAY2D_LIST:
    /**
     * @bug Mismatch between declared value of TYPE_SAMPLE_SHORT_ARRAY2D_LIST (4) in MMCommon.idl and
     *      value assigned to column monitormanager.monitor_description.type for short array 2D (6).
     */
    case '6':

    case MM::TYPE_SAMPLE_OCTET_ARRAY2D_LIST:
    {
      if (isMemberPresent(MonitorKeyword::AllowedMembers::format)
          and keyFormat not_eq MonitorKeyword/*::KeyFormat*/::ARRAY_TO_SCALAR
          and keyFormat not_eq MonitorKeyword/*::KeyFormat*/::KeyFormat_NONE)
      {
        InvalidValue exception(__PRETTY_FUNCTION__);
        exception.addText("The '%s' member must be 'ARRAY_TO_SCALAR' or 'NONE', because the monitor '%s.%s' is an array.",
                          MonitorKeyword::AllowedMembers::format, deviceName.c_str(), monitorName.c_str());
        throw exception;
      }
      break;
    }
      // Unknown type
    default:
    {
      InvalidValue exception(__PRETTY_FUNCTION__);
      exception.addText("Unknown type value ('%c') of the monitor '%s.%s'", type, deviceName.c_str(), monitorName.c_str());
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
  const CORBA::Long & x_or_height (monitorDescription->dimension_x);
  const CORBA::Long & y_or_width  (monitorDescription->dimension_y);
  const CORBA::Long dimensions (y_or_width * x_or_height - 1);
  if ( isMemberPresent(MonitorKeyword::AllowedMembers::range) )
  {
    // Check given range
    if ( arrayRange.second > dimensions )
    {
      InvalidValue exception(__PRETTY_FUNCTION__);
      exception.addText(
          "The value ('%d') of second element of '%s' must be less than or equal to the dimensions - 1 ('%d') of '%s.%s' monitor.",
          arrayRange.second,
          MonitorKeyword::AllowedMembers::range,
          dimensions,
          deviceName.c_str(),
          monitorName.c_str()
      );
      throw exception;
    }
  }
  else
  {
    // Assign range. No specified range means the whole range is applied
    arrayRange.first = 0;
    arrayRange.second = dimensions;
    presentMembers_[MonitorKeyword::AllowedMembers::range] = true;
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
  const CORBA::ULong & magnitudeDescriptionValuesSize = magnitudeDescription->values.length();
  if ( isMemberPresent(MonitorKeyword::AllowedMembers::labels) )
  {
    // Check if both sizes match.
    if ( enumLabels.size() not_eq magnitudeDescriptionValuesSize )
    {
      InvalidValue exception(__PRETTY_FUNCTION__);
      exception.addText(
          "Mismatch sizes of the specified '%s' member ('%d') and the retrieved information from database of the '%s.%s' monitor ('%d').",
          MonitorKeyword::AllowedMembers::labels, enumLabels.size(), deviceName.c_str(), monitorName.c_str(), magnitudeDescriptionValuesSize
      );
      throw exception;
    }
    // Check if all elements match
    for (unsigned int i (0); i < magnitudeDescriptionValuesSize; ++i)
    {
      const std::string& dataBaseElement =  magnitudeDescription->values[i].in();
      const std::string& specifiedLabel = enumLabels[i];
      if ( dataBaseElement not_eq specifiedLabel )
      {
        InvalidValue exception(__PRETTY_FUNCTION__);
        exception.addText(
            "Mismatch the specified information (%s[%d]='%s') and the retrieved information from database ('%s.%s'.values[%d]='%s').",
            MonitorKeyword::AllowedMembers::labels, i, specifiedLabel.c_str(), deviceName.c_str(), monitorName.c_str(), i, dataBaseElement.c_str()
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
    presentMembers_[MonitorKeyword::AllowedMembers::labels] = true;
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
bool operator==(const MonitorKeyword &lhs, const MonitorKeyword &rhs) /*noexcept*/
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
bool operator!=(const MonitorKeyword &lhs, const MonitorKeyword &rhs) /*noexcept*/
{
  MonitorKeyword_TRACE
  return not(rhs == lhs);
}

//----------------------------------------------------------------------
// MonitorKeyword  <  MonitorKeyword operator
//----------------------------------------------------------------------
bool operator<(const MonitorKeyword &lhs, const MonitorKeyword &rhs) /*noexcept*/
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
bool operator>(const MonitorKeyword &lhs, const MonitorKeyword &rhs) /*noexcept*/
{
  MonitorKeyword_TRACE
  return (rhs < lhs);
}

//----------------------------------------------------------------------
// MonitorKeyword  <=  MonitorKeyword operator
//----------------------------------------------------------------------
bool operator<=(const MonitorKeyword &lhs, const MonitorKeyword &rhs) /*noexcept*/
{
  MonitorKeyword_TRACE
  return not(rhs < lhs);
}

//----------------------------------------------------------------------
// MonitorKeyword  >=  MonitorKeyword operator
//----------------------------------------------------------------------
bool operator>=(const MonitorKeyword &lhs, const MonitorKeyword &rhs) /*noexcept*/
{
  MonitorKeyword_TRACE
  return not(lhs < rhs);
}

//----------------------------------------------------------------------
// std::ostream << MonitorKeyword 
//----------------------------------------------------------------------
std::ostream& operator<<(std::ostream &os, const MonitorKeyword &obj)
{
  os << "[MonitorKeyword ("<< (&obj) << ")"
     << " deviceName: '" << obj.deviceName << "'"
     << " monitorName: '" << obj.monitorName << "'"
     << " monitorType: '" << obj.monitorType << "'"
     << " id: '" << obj.id << "'"
     << " keyName: '" << obj.keyName << "'"
     << " keyComment: '" << obj.keyComment << "'"
     << " keyUnit: '" << obj.keyUnit << "'"
  ;
  if ( obj.isMemberPresent(MonitorKeyword::AllowedMembers::format ) )
    os << " keyFormat: '" << obj.keyFormat << "'";
  if ( obj.isMemberPresent(MonitorKeyword::AllowedMembers::when ) )
    os << " when: '" << obj.when << "'";
  if ( obj.isMemberPresent(MonitorKeyword::AllowedMembers::transposed ) )
    os << " transposed: '" << obj.transposed << "'";
  if ( obj.isMemberPresent(MonitorKeyword::AllowedMembers::initialIndex ) )
    os << " keyInitialIndex: '" << obj.keyInitialIndex << "'";
  if ( obj.isMemberPresent(MonitorKeyword::AllowedMembers::range ) )
    os << " arrayRange: [" << to_string(obj.arrayRange) << "]";
  if ( obj.isMemberPresent(MonitorKeyword::AllowedMembers::labels ) )
    os << " enumLabels: [" << to_string(obj.enumLabels) << "]";
  os << "]";
  return os;
}

//----------------------------------------------------------------------
// from_string MonitorKeyword::MonitorType
//----------------------------------------------------------------------
void from_string(MonitorKeyword::MonitorType &obj, std::string str)
{
  MonitorKeyword_TRACE
  std::transform(str.begin(), str.end(), str.begin(), (int(*)(int)) std::toupper);
  if      ( str == "NONE"     ) { obj = MonitorKeyword/*::MonitorType*/::MonitorType_NONE    ; }
  else if ( str == "SAMPLED"  ) { obj = MonitorKeyword/*::MonitorType*/::SAMPLED ; }
  else if ( str == "DISCRETE" ) { obj = MonitorKeyword/*::MonitorType*/::DISCRETE; }
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
  std::transform(str.begin(), str.end(), str.begin(), (int(*)(int)) std::toupper);
  if      ( str == "NONE"  ) { obj = MonitorKeyword/*::When*/::When_NONE  ; }
  else if ( str == "START" ) { obj = MonitorKeyword/*::When*/::START ; }
  else if ( str == "END"   ) { obj = MonitorKeyword/*::When*/::END   ; }
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
  std::transform(str.begin(), str.end(), str.begin(), (int(*)(int)) std::toupper);
  if      ( str == "NONE"            ) { obj = MonitorKeyword/*::KeyFormat*/::KeyFormat_NONE           ; }
  else if ( str == "RAD_TO_HMS"      ) { obj = MonitorKeyword/*::KeyFormat*/::RAD_TO_HMS     ; }
  else if ( str == "RAD_TO_DMS"      ) { obj = MonitorKeyword/*::KeyFormat*/::RAD_TO_DMS     ; }
  else if ( str == "RAD_TO_DEG"      ) { obj = MonitorKeyword/*::KeyFormat*/::RAD_TO_DEG     ; }
  else if ( str == "ARRAY_TO_SCALAR" ) { obj = MonitorKeyword/*::KeyFormat*/::ARRAY_TO_SCALAR; }
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
    obj.push_back(label);
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
    case MonitorKeyword/*::MonitorType*/::MonitorType_NONE:
      return "NONE";
    case MonitorKeyword/*::MonitorType*/::SAMPLED:
      return "SAMPLED";
    case MonitorKeyword/*::MonitorType*/::DISCRETE:
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
    case MonitorKeyword/*::When*/::When_NONE:
      return "NONE";
    case MonitorKeyword/*::When*/::START:
      return "START";
    case MonitorKeyword/*::When*/::END:
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
    case MonitorKeyword/*::KeyFormat*/::KeyFormat_NONE:
      return "NONE";
    case MonitorKeyword/*::KeyFormat*/::RAD_TO_HMS:
      return "RAD_TO_HMS";
    case MonitorKeyword/*::KeyFormat*/::RAD_TO_DMS:
      return "RAD_TO_DMS";
    case MonitorKeyword/*::KeyFormat*/::RAD_TO_DEG:
      return "RAD_TO_DEG";
    case MonitorKeyword/*::KeyFormat*/::ARRAY_TO_SCALAR:
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
  return oss.str().c_str();
}

//----------------------------------------------------------------------
// to_string MonitorKeyword::EnumLabels
//----------------------------------------------------------------------
const char *to_string(const MonitorKeyword::EnumLabels &obj)
{
  std::ostringstream oss;
  for (MonitorKeyword::EnumLabels::const_iterator it = obj.begin(); it != obj.end(); ++it)
  {
    oss << (*it) << ',';
  }
  oss.flush();
  std::string str (oss.str());
  str = str.substr(0, str.size()-1);
  return str.c_str();
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
