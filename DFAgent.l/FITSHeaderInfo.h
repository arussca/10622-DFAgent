// Copyright (c) 2018 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved

#ifndef _FITSHeaderInfo_h_
#define _FITSHeaderInfo_h_

#include "MonitorKeyword.h"               // MonitorKeyword
#include <iostream>                       // std::ostream
#include <string>                         // std::string
#include <set>                            // std::set
#include <vector>                         // std::vector
#include <MonitorAgent.h>                 // MonitorAgent
#include <LogAgent.h>                     // LogAgent
#include <Trace.h>                        // Trace
#include <TimeValue.h>                    // TimeValue
#include <DataDescription.h>              // DataDescription
#include <MMCommonS.h>                    // MM::DataBlock, MM::MagnitudeChange
#include <Keyword.h>                      // DPKF::Keyword, DPKF::KeywordBase
#include <cfitsio/fitsio.h>               // fitsfile
#include <json/json.h>                    // Json::Value, Json::ArrayIndex

/**
 * FITSHeaderInfo contains the relation between a recipe and its keywords, whose values
 * comes from the monitors of the GCS components.
 *
 * @related MonitorKeyword
 * @related DFAgent
 */
class FITSHeaderInfo
{

 //--------------------------------------------------
 // public section
 //--------------------------------------------------
 public:

  //############################ PUBLIC TYPES ############################

  typedef std::set<std::string>            KeywordNamesContainer;
  typedef std::vector<DPKF::KeywordBase *> FixedKeywordContainer;
  typedef std::set<MonitorKeyword>         MonitorKeywordContainer;
  typedef std::set<std::string>            ParsedKeywordsContainer;
  typedef std::set<unsigned int>           NotParsedNodesIDContainer;
  typedef std::set<std::string>            NotWrittenKeywordsContainer;
  typedef std::size_t                      NumberOfKeywords;
  typedef std::set<const char *>           AllowedFileNames;

  //########################## PUBLIC CONSTANTS ##########################

  //############# CONSTRUCTORS, DESTRUCTOR & SPECIAL METHODS #############
  // References:
  // -----------
  // - [C++ Special methods][1]
  //
  // ---
  // [1]:<http://www.cplusplus.com/doc/tutorial/classes2/> (online at 2018/07/21)

  /**
   * Default constructor
   */
  /*explicit*/ FITSHeaderInfo();

  /**
   * Customized constructor
   *
   * @param[in] monitorAgent MonitorAgent pointer, which will be used to
   *                         retrieve the values of all specified monitors.
   * @param[in] logAgent     LogAgent pointer, which is related to the
   *                         object that invokes this constructor.
   * @param[in] trace        Trace pointer, which is related to the object
   *                         that invokes this constructor.
   */
  /*explicit*/ FITSHeaderInfo(MonitorAgent *& monitorAgent,
                              LogAgent *& logAgent,
                              Trace & trace);

  /**
   * Copy constructor.
   *
   * @param[in] other Other instance of FITSHeaderInfo class,
   *                  used as reference to construct this object.
   */
  FITSHeaderInfo(const FITSHeaderInfo & other);

  #if defined(__cplusplus) && (__cplusplus >= 201103L)
  /**
   * Move constructor
   *
   * @param[in] other Other instance of FITSHeaderInfo class,
   *                  used as reference to construct this object.
   */
  FITSHeaderInfo(FITSHeaderInfo &&other);
  #endif

  /**
   * Copy assignment
   *
   * @param[in] other Other instance of FITSHeaderInfo class,
   *                  used as reference to construct this object.
   * @return          A reference of itself.
   */
  FITSHeaderInfo & operator=(const FITSHeaderInfo & other);

  #if defined(__cplusplus) && (__cplusplus >= 201103L)
  /**
   * Move assignment
   *
   * @param[in] other Other instance of FITSHeaderInfo class,
   *                  used as reference to construct this object.
   * @return          A reference of itself.
   */
  FITSHeaderInfo& operator=(FITSHeaderInfo &&other);
  #endif

  /**
   * Destructor
   */
  virtual ~FITSHeaderInfo();

  //########################## PUBLIC METHODS ############################

  /**
   * Assigns a new value to the internal monitorAgent_ member.
   *
   * @related monitorAgent_
   *
   * @param[in] monitorAgent Pointer to MonitorAgent instance.
   */
  void setMonitorAgent(MonitorAgent *& monitorAgent);

  /**
   * Assigns a new value to the internal logAgent_ member.
   *
   * @related logAgent_
   *
   * @param[in] logAgent Pointer to LogAgent instance.
   */
  void setLogAgent(LogAgent *& logAgent);

  /**
   * Assigns a new value to the internal trace_ member.
   *
   * @related trace_
   *
   * @param[in] trace Pointer to Trace instance.
   */
  void setTrace(Trace *& trace);

  /**
   * Parses a file to extract all the desired keywords related to a monitor.
   *
   * @param[in] fileName File name, which contains all desired keywords related to a monitor.
   */
  virtual void parseFITSHeaderKeywordsFromFile(const std::string & fileName);

  /**
   * Parses a JSON node to extract all the desired keywords related to a monitor.
   *
   * @param[in] root JSON node where all set of keywords are defined.
   */
  virtual void parseFITSHeaderKeywordsFromInline(const Json::Value & root);

  /**
   * It merges in this object the information contained in a given object.
   *
   * @param[in] other Other FITSHeaderInfo object to merge.
   */
  virtual void merge(const FITSHeaderInfo & other);

  /**
   * Writes the keywords with fixed values into the given FITS Header instance.
   *
   * @param[out] header  FITS Header instance where all keywords with fixed
   *                     values must be written.
   * @return             Indicates if all keywords could be written in the
   *                     header or not.
   */
  virtual bool gatherFixedKeywords(DataDescription & header);

  /**
   * Obtains the monitor data from the database of each specified keyword
   * and writes them into the given FITS Header instance.
   *
   * @param[in]  time    Start time of the exposure.
   * @param[in]  expTime Duration time of the exposure.
   * @param[out] header  FITS Header instance where all specified keywords
   *                     must be written.
   * @return             Indicates if all keywords could be written in the
   *                     header or not.
   */
  virtual bool gatherMonitorKeywords(const TimeValue & time,
                                     const TimeValue & expTime,
                                     DataDescription & header);

  /**
   * Obtains the monitor data from the database of each specified keyword
   * and writes them into the given FITS Header instance. To do this, the
   * given MonitorAgent pointer is used to query the MonitorManager service.
   *
   * @param[in]  startExpTime     Start time of the exposure.
   * @param[in]  durationExpTime  Duration time of the exposure.
   * @param[in]  monitorAgent     Pointer to the instance of MonitorAgent, which
   *                              is used to query the MonitorManager service.
   * @param[out] header           FITS Header instance where all specified keywords
   *                              must be written.
   * @return                      Indicates if all keywords could be written in the
   *                              header or not.
   */
  virtual bool gatherMonitorKeywords(const TimeValue & startExpTime,
                                     const TimeValue & durationExpTime,
                                     MonitorAgent * monitorAgent,
                                     DataDescription & header);

  /**
   * Writes all specified FITS header keyword at an existing FITS file.
   *
   * @param[in]  time    Start time of the exposure.
   * @param[in]  expTime Duration time of the exposure.
   * @param[out] rawFile File name of the existing FITS file.
   */
  virtual void writeHeaderToFile(const TimeValue & time,
                                 const TimeValue & expTime,
                                 fitsfile * rawFile);

  /**
   * Writes all specified FITS header keyword at an existing FITS file.
   *
   * @param[in]  time         Start time of the exposure.
   * @param[in]  expTime      Duration time of the exposure.
   * @param[in]  monitorAgent Pointer to the instance of MonitorAgent, which
   *                          is used to query the MonitorManager service.
   * @param[out] rawFile      File name of the existing FITS file.
   */
  virtual void writeHeaderToFile(const TimeValue & time,
                                 const TimeValue & expTime,
                                 MonitorAgent * monitorAgent,
                                 fitsfile * rawFile);

  /**
   * Force the sample operation of all specified monitors.
   */
  virtual void forceSampleData() const;

  /**
   * Force the sample operation of all specified monitors, using the given
   * pointer of the MonitorAgent.
   *
   * @param[in] monitorAgent Pointer to the instance of MonitorAgent, which
   *                         is used to force the sampling of the specified
   *                         monitors.
   */
  virtual void forceSampleData(MonitorAgent * monitorAgent) const;

  /**
   * Getter method that returns the list of all keyword names,
   * including monitor-keywords and fixed-keywords.
   *
   * @related keywordNamesList_
   *
   * @return List of all keyword names.
   */
  virtual const KeywordNamesContainer & getKeywordNameList() const;

  /**
   * Getter method that returns the list of all keywords instances with
   * fixed values, which were successfully parsed from file.
   *
   * @related fixedKeywordList_
   *
   * @return List of keyword instances with fixed values, which were
   *         successfully parsed from file.
   *         successfully parsed from file.
   */
  virtual const FixedKeywordContainer & getFixedKeywordList() const;

  /**
   * Getter method that returns the list of all MonitorKeyword instances,
   * which were successfully parsed from file.
   *
   * @related monitorKeywordList_
   *
   * @return List of MonitorKeyword instances, which were successfully
   *         parsed from file.
   */
  virtual const MonitorKeywordContainer & getMonitorKeywordList() const;

  /**
   * Getter method that returns the list of the keyword names,
   * which were successfully parsed form file.
   *
   * @related parsedKeywords_
   *
   * @return List of keyword names, which were successfully
   *         parsed from file.
   */
  virtual const ParsedKeywordsContainer & getParsedKeywords() const;

  /**
   * Getter method that returns the set of node identifiers,
   * which could not be parsed.
   *
   * @related notParsedNodesID_
   *
   * @return List of the node identifiers, which could not
   *         be parsed.
   */
  virtual const NotParsedNodesIDContainer & getNotParsedNodesID() const;

  /**
   * Getter method that returns the set of keyword names,
   * which could not be written at header file.
   *
   * @related notWrittenKeywords_
   *
   * @return List of keyword names, which could not be
   *         written at header file.
   */
  virtual const NotWrittenKeywordsContainer & getNotWrittenKeywords() const;

  /**
   * Getter method that returns the number of the discrete monitors,
   * which were successfully parsed from file.
   *
   * @related discreteMonitorCounter_
   *
   * @return Number of the discrete monitors, which were successfully
   *         parsed from file.
   */
  virtual const NumberOfKeywords & getNumberOfDiscreteKeywords() const;

  /**
   * Getter method that returns the number of the sampled monitors,
   * which were successfully parsed from file.
   *
   * @related sampledMonitorCounter_
   *
   * @return Number of the sampled monitors, which were successfully
   *         parsed from file.
   */
  virtual const NumberOfKeywords & getNumberOfSampledKeywords() const;

  //########################## PUBLIC MEMBERS ############################

  static AllowedFileNames allowedFieldNamesForFixedKeywords;

 //--------------------------------------------------
 // protected section
 //--------------------------------------------------
 protected:

  //########################### PROTECTED TYPES ##########################


  //######################### PROTECTED CONSTANTS ########################


  //########################## PROTECTED METHODS #########################

  /**
   * Sets all internal members to default values.
   */
  virtual void clearAllMembers_();

  /**
   * Parses a file to extract all the desired keywords related to a monitor.
   * The file must use format defined by GTC.
   *
   * @param[in] fileName File that contains all desired keywords related to
   *                     a monitor, using an custom format.
   */
  virtual void parseFITSHeaderKeywordsFromPlainTextFile_(const std::string & fileName);

  /**
   * Parses a file to extract all the desired keywords related to a monitor.
   * The file must use JSON (JavaScript Object Notation) as format.
   *
   * @param[in] fileName File that contains all desired keywords related to
   *                     a monitor, using JSON as format.
   */
  virtual void parseFITSHeaderKeywordsFromJSONFile_(const std::string & fileName);

  /**
   * @brief Parses a node from JSON and appends it to the list of
   *        fixed keywords.
   * @param nodeNumber Number of the node in the array.
   * @param node JSON node that contain the information of a fixed keyword.
   *
   * @related fixedKeywordList_
   */
  virtual void parseFixedKeywords_(const unsigned int & nodeNumber, const Json::Value & node);

  /**
   * @brief Parses a node from JSON and appends it to the list of
   *        monitor keywords.
   * @param nodeNumber Number of the node in the array.
   * @param node JSON node that contain the information of a fixed keyword.
   *
   * @related monitorKeywordList_
   */
  virtual void parseMonitorKeywords_(const unsigned int & nodeNumber, const Json::Value & node);

  /**
   * Extracts the data of a sampled monitor (scalar or array) related to a
   * concrete keyword and writes it into the given header.
   *
   * @param[in]  keyword Keyword information.
   * @param[in]  mons    Result of the query to the database of a sampled monitor.
   * @param[out] header  Header where the keyword will be written.
   */
  virtual void gatherSampledMonitor_(const MonitorKeyword & keyword,
                                     const MM::DataBlock & mons,
                                     DataDescription & header);

  /**
   * Extracts the data of a discrete monitor (enum or boolean) related to a
   * concrete keyword and writes it into the given header.
   *
   * @param[in]  keyword Keyword information.
   * @param[in]  mons    Result of the query to the database of a scalar monitor.
   * @param[out] header  Header where the keyword will be written.
   */
  virtual void gatherDiscreteMonitor_(const MonitorKeyword & keyword,
                                      const MM::MagnitudeChange & mons,
                                      DataDescription & header);

  /**
   * Extracts the data of a scalar monitor (double, float, long, short and
   * octect) related to a concrete keyword and writes it into the given header.
   *
   * @tparam     ArrayListType Database data type related to this monitor.
   * @tparam     ValueType     Keyword data type related to this monitor.
   * @param[in]  keyword       Keyword information.
   * @param[in]  arrayList     IDL sequence related to ArrayListType,
   *                           where the value of the monitor is contains.
   * @param[out] header        Header where the keyword will be written.
   */
  template<typename ArrayListType, typename ValueType>
  void addSampledMonitorKeyword_(const MonitorKeyword & keyword,
                                 const ArrayListType & arrayList,
                                 DataDescription & header);

  /**
   * Extracts the data of a array monitor (double, float, long, short and
   * octect) related to a concrete keyword and writes it into the given header.
   *
   * @tparam     ArrayListType Database data type related to this monitor.
   * @tparam     ValueType     Keyword data type related to this monitor.
   * @param[in]  keyword       Keyword information.
   * @param[in]  arrayList     IDL sequence related to ArrayListType,
   *                           where the value of the monitor is contains.
   * @param[out] header        Header where the keyword will be written.
   */
  template<typename ArrayListType, typename ValueType>
  void addArrayToScalarKeywords_(const MonitorKeyword & keyword,
                                 const ArrayListType & arrayList,
                                 DataDescription & header);

  /**
   * Trims a array, using the range information of the keyword.
   *
   * @tparam    ArrayType  Database data type related to this monitor.
   * @param[in] keyword    Keyword information.
   * @param[in] arrayIn    Array to be trimmed.
   * @return               Trimmed array.
   */
  template<typename ArrayType>
  ArrayType trimmedArray1D_(const MonitorKeyword & keyword,
                            const ArrayType & arrayIn);

  /**
   * Represents a array into a output stream object.
   *
   * @tparam        ArrayType    Database data type related to this monitor.
   * @param[in,out] ost          Output stream object, which will contain
   *                             the array string representation.
   * @param[in]     array        Array to be represented.
   * @return                     Output stream objet with the string array
   *                             representation.
   */
  template<typename ArrayType>
  std::ostream & toStream_(std::ostream & ost,
                           const ArrayType & array);

  /**
   * Transforms an angle in radians to its string representation in
   * hours(H), minutes(m), seconds(s).
   *
   * @param[in] rad Value of an angle in radians.
   * @return        String representation in hours(H), minutes(m),
   *                seconds(s) of an angle in radians.
   */
  virtual std::string formatRadianToHms_(const double & rad) const;

  /**
   * Transforms an angle in radians to its string representation in
   * degrees(D), arcminutes(m), arcseconds(s).
   *
   * @param[in] rad Value of an angle in radians.
   * @return        String representation in degrees(D), arcminutes(m),
   *                arcseconds(s) of an angle in radians.
   */
  virtual std::string formatRadianToDms_(const double & rad) const;

  /**
   * Transforms an angle in radians to degrees(Deg).
   *
   * @param rad Value of an angle in radians.
   * @return    Value of an angle in degrees.
   */
  virtual double formatRadianToDeg_(const double & rad) const;

  //########################## PROTECTED MEMBERS #########################

  /**
   * List of all keyword names for a specific recipe.
   */
  KeywordNamesContainer keywordNamesList_;

  /**
   * List of parsed keyword information for a specific recipe.
   */
  FixedKeywordContainer fixedKeywordList_;

  /**
   * List of parsed keyword information for a specific recipe.
   */
  MonitorKeywordContainer monitorKeywordList_;

  /**
   * List of keyword names, which have been parsed successfully.
   */
  ParsedKeywordsContainer parsedKeywords_;

  /**
   * Set of number of node identifier, which could not be parsed successfully.
   */
  NotParsedNodesIDContainer notParsedNodesID_;

  /**
   * Set of the name of keywords that could not be written
   * because an problem occurs.
   */
  NotWrittenKeywordsContainer notWrittenKeywords_;

  /**
   * Number of discrete monitors (enum or bool) parsed successfully.
   */
  NumberOfKeywords discreteMonitorCounter_;

  /**
   * Number of sampled monitors (scalar or array) parsed successfully.
   */
  NumberOfKeywords sampledMonitorCounter_;

  /**
   * Pointer of a MonitorAgent instance,
   * which will be used to retrieve the values of all monitors.
   */
  MonitorAgent * monitorAgent_;

  /**
   * Pointer of a LogAgent instance,
   * which is related to the invoke the constructor of this object.
   */
  LogAgent * logAgent_;

  /**
   * Pointer of a Trace instance,
   * which is related to the invoke the constructor of this object.
   */
  Trace * trace_;


  //--------------------------------------------------
  // private section
  //--------------------------------------------------
 private:

  //############################ PRIVATE TYPES ###########################

  /**
   * Forward class declaration, which will be reserved to
   * perform the unit test.
   */
  friend class FITSHeaderInfoTest;

  //########################## PRIVATE CONSTANTS #########################

  //########################### PRIVATE METHODS ##########################

  //########################### PRIVATE MEMBERS ##########################

  //############################ OTHER METHODS ###########################

  /**
   * Relational operator, which determines if lhs is equal to rhs or not.
   *
   * @param[in] lhs Left-hand side operand.
   * @param[in] rhs Right-hand side operand.
   * @return True, if lhs is equal to rhs. False, otherwise.
   */
  friend bool operator==(const FITSHeaderInfo & lhs, const FITSHeaderInfo & rhs) /*noexcept*/;

  /**
   * Relational operator, which determines if lhs is not equal to rhs or not.
   *
   * @param[in] lhs Left-hand side operand.
   * @param[in] rhs Right-hand side operand.
   * @return True, if lhs is not equal to rhs. False, otherwise.
   */
  friend bool operator!=(const FITSHeaderInfo & lhs, const FITSHeaderInfo & rhs) /*noexcept*/;

  /**
   * Relational operator, which determines if lhs is less than rhs or not.
   *
   * @param[in] lhs Left-hand side operand.
   * @param[in] rhs Right-hand side operand.
   * @return True, if lhs is less than rhs. False, otherwise.
   */
  friend bool operator<(const FITSHeaderInfo & lhs, const FITSHeaderInfo & rhs) /*noexcept*/;

};

#endif // _FITSHeaderInfo_h_
