// Copyright (c) 2018 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved

#ifndef _MonitorKeyword_h_
#define _MonitorKeyword_h_

#include <string>                         // std::string
#include <utility>                        // std::pair, std::make_pair
#include <set>                            // std::set
#include <vector>                         // std::vector
#include <map>                            // std::map
#include <LogAgent.h>                     // LogAgent
#include <Trace.h>                        // Trace
#include <MMCommonS.h>                    // MM::MonitorQuery, MM::Description, MM::MagnitudeDescription
#include <MonitorManagerS.h>              // MM::MonitorManager_ifce
#include <json/json.h>                    // Json::Value

/**
 * Represents a FITS header keyword, whose information comes from
 * the database through a monitor.
 *
 * The information will be loaded from a file. Now, the following
 * files are supported:
 * - JavaScript Object Notation (JSON).
 * - Custom GTC style (old version).
 *
 * @related FITSHeaderInfo
 *
 */
class MonitorKeyword
{
  //--------------------------------------------------
  // public section
  //--------------------------------------------------
  public:

  //############################ PUBLIC TYPES ############################

  /**
   * Data type to group all allowed string members in the FITS header keyword
   * specification file, regardless of their syntax.
   */
  struct AllowedMembers
  {
    using FieldType = const char *const;


    /**
     * Represents the keyword reserved for the name of the FITS header.
     *
     * @related keyName
     */
    static constexpr FieldType name = "keyword";

    /**
     * Represents the keyword reserved for the comment of the FITS header.
     *
     * @related keyComment
     */
    static constexpr FieldType comment = "comment";

    /**
     * Represents the keyword reserved for the unit of the FITS header.
     *
     * @related keyUnit
     */
    static constexpr FieldType unit = "unit";

    /**
     * Represents the keyword reserved for the GCS component, which declares
     * and updates the monitor.
     *
     * @related deviceName
     */
    static constexpr FieldType component = "component";

    /**
     * Represents the keyword reserved for the monitor name.
     *
     * @related monitorName
     */
    static constexpr FieldType monitor = "monitor";

    /**
     * Represents the keyword reserved for the type of the monitor: DISCRETE
     * or SAMPLED.
     *
     * @related monitorType
     */
    static constexpr FieldType type = "type";

    /**
     * Represents the keyword reserved to indicate whether the monitor will be
     * retrieved at the beginning or end of the exposition time.
     *
     * @related when
     */
    static constexpr FieldType when = "when";

    /**
     * Represents the keyword reserved for the transformation, which will be
     * applied when the monitor type is SAMPLED.
     *
     * @related KeyFormat
     * @related keyFormat
     */
    static constexpr FieldType format = "format";

    /**
     * Represents the keyword reserved to indicate the specific indexes, which
     * will be used when the monitor is an array.
     *
     * @related arrayRange
     */
    static constexpr FieldType range = "range";

    /**
     * Represents the keyword reserved to indicate that there will be an FITS
     * header keyword for each index of the monitor, when it is an array.
     *
     * @related transposed
     */
    static constexpr FieldType transposed = "transposed";

    /**
     * Represents the keyword reserved to indicate the beginning index used as
     * suffix when the monitor is an array and the option transposed is true.
     *
     * @related keyInitialIndex
     */
    static constexpr FieldType initialIndex = "initialIndex";

    /**
     * Represents the keyword reserved to indicate the set of labels when the
     * monitor type is DISCRETE.
     *
     * @related enumLabels
     */
    static constexpr FieldType labels = "labels";

    using FieldsContainterType = std::set <std::string>;

    /**
     * Represent the set of all allowed fields/members
     * in the FITS header keyword specification file.
     *
     */
    static const FieldsContainterType all;

  };

  /**
   * Enumeration that determines the two types of existing monitors:
   * SAMPLED or  DISCRETE.
   *
   * @related AllowedMembers::type
   * @related monitorType
   *
   */
  enum class MonitorType : unsigned char
  {
    NONE     = 'N', ///< Indicates an invalid or uninitialized value.
    SAMPLED  = 'S', ///< Indicates that the monitor type is a number o an array of numbers.
    DISCRETE = 'D', ///< Indicates that the monitor type is an enumeration o boolean.
  };

  /**
   * Enumeration that determines all allowed values of the time to retrieved the monitor.
   * The allowed values are: START, END
   *
   * @related AllowedMembers::when
   * @related when
   */
  enum class When : unsigned char
  {
    NONE  = 'N', ///< Indicates an invalid or uninitialized value.
    START = 'S', ///< Indicates that the timestamp used to retrieved the monitor is before the exposure.
    END   = 'E', ///< Indicates that the timestamp used to retrieved the monitor is after the exposure.
  };

  /**
   * Enumeration that determines all available transformations to apply to SAMPLED monitor.
   *
   * @related AllowedMembers::format
   * @related keyFormat
   */
  enum class KeyFormat : signed short
  {
    NONE            = -1, ///< Indicates that no conversions will be applied.
    RAD_TO_HMS      =  1, ///< It can only be used for scalar monitor to convert an angle in radians to hours, minutes, seconds.
    RAD_TO_DMS      =  2, ///< It can only be used for scalar monitor to convert an angle in radians to degrees, arcminutes, arcseconds.
    RAD_TO_DEG      =  3, ///< It can only be used for scalar monitor to convert an angle in radians to degrees.
    ARRAY_TO_SCALAR =  4, ///< It can only be used for array monitor.
  };

  using ArrayRange  = std::pair<unsigned int, unsigned int>;
  using EnumLabels  = std::vector<std::string>;

  //########################## PUBLIC CONSTANTS ##########################

  //############# CONSTRUCTORS, DESTRUCTOR & SPECIAL METHODS #############

  /**
   * Default constructor
   */
  explicit MonitorKeyword();

  /**
   * Customized constructor: fills all its internal members with information
   * from plain text line.
   *
   * @param[in] line       String line that contains the needed information
   *                       to fill all internal members.
   * @param[in] nodeID Number of line on file.
   * @param[in] logAgent   LogAgent pointer, which is related to the object
   *                       that invokes this constructor.
   * @param[in] trace      Trace pointer, which is related to the object
   *                       that invokes this constructor.
   * @param[in] verbose    Level for traces and logs.
   */
  explicit MonitorKeyword(const std::string &line,
                          const unsigned int &nodeID,
                          LogAgent *logAgent,
                          Trace &trace,
                          const unsigned short &verbose = 0);

  /**
   * Customized constructor: fills all its internal members with information
   * from the JSON node.
   *
   * @param[in] node     JSON node.
   * @param[in] nodeID   Identifier number of the JSON node in the file.
   * @param[in] logAgent LogAgent pointer, which is related to the object
   *                     that invokes this constructor.
   * @param[in] trace    Trace pointer, which is related to the object
   *                     that invokes this constructor.
   * @param[in] verbose  Level for traces and logs.
   */
  explicit MonitorKeyword(const Json::Value &node,
                          const unsigned int &nodeID,
                          LogAgent *logAgent,
                          Trace &trace,
                          const unsigned short &verbose = 0);

  /**
   * Copy constructor.
   *
   * @param[in] other Other instance of MonitorKeyword class,
   *                  used as reference to construct this object.
   */
  MonitorKeyword(const MonitorKeyword &other) = default;

  /**
   * Move constructor
   *
   * @param[in] other Other instance of MonitorKeyword class,
   *                  used as reference to construct this object.
   */
  MonitorKeyword(MonitorKeyword &&other) = default;

  /**
   * Copy assignment
   *
   * @param[in] other Other instance of MonitorKeyword class,
   *                  used as reference to construct this object.
   * @return          A reference of itself.
   */
  MonitorKeyword& operator=(const MonitorKeyword &other) = default;

  /**
   * Move assignment
   *
   * @param[in] other Other instance of MonitorKeyword class,
   *                  used as reference to construct this object.
   * @return          A reference of itself.
   */
  MonitorKeyword& operator=(MonitorKeyword &&other) = default;

  /**
   * Destructor
   */
  virtual ~MonitorKeyword();

  //########################## PUBLIC METHODS ############################

  /**
   * Parses the given string line and extracts its information to fill all
   * internal members.
   *
   * @param[in] line       String line that contains the needed information
   *                       to fill all internal members.
   * @param[in] lineNumber Number of line on file.
   * @param[in] verbose    Level for traces and logs.
   */
  virtual void parseFromPlainText(const std::string &line,
                                  const unsigned int &lineNumber,
                                  const unsigned short &verbose);

  /**
   * Parses the JSON node and extracts its information to fill all internal members.
   *
   * @param[in] node    JSON node.
   * @param[in] nodeID  Identifier number of the JSON node in the file.
   * @param[in] verbose Level for traces and logs.
   */
  virtual void parseFromJSON(const Json::Value &node,
                             const unsigned int &nodeID,
                             const unsigned short &verbose);

  /**
   * Assigns a new value to the internal logAgent_ member.
   *
   * @related logAgent_
   *
   * @param[in] logAgent Pointer to LogAgent instance.
   */
  virtual void setLogAgent(LogAgent *logAgent);

  /**
   * Assigns a new value to the internal trace_ member.
   *
   * @related trace_
   *
   * @param[in] trace Pointer to Trace instance.
   */
  virtual void setTrace(Trace *&trace);

  /**
   * Indicates whether the given member is allowed or not.
   *
   * @param[in] member String of the member to check.
   * @return true, if the member is allowed. false, otherwise.
   */
  virtual bool isMemberAllowed(const std::string &member) const;

  /**
   * Indicates whether the member given member is present or not.
   *
   * @param[in] member String of the member to check.
   * @return true, if the member is allowed and is present. false, otherwise.
   */
  virtual bool isMemberPresent(const std::string &member) const;

  //########################## PUBLIC MEMBERS ############################

  /**
   * Identifier number of this node in the file.
   */
  unsigned int id;

  /**
   * Name of the keyword.
   *
   * @related AllowedMembers::name
   *
   * @note:
   * - List of constraints:
   *   - It is an mandatory member.
   *   - It value can not be empty.
   */
  std::string keyName;

  /**
   * Comment of the keyword.
   *
   * @related AllowedMembers::comment
   *
   * @note:
   * - List of constraints:
   *   - It is an mandatory member.
   *   - It value can not be empty.
   */
  std::string keyComment;

  /**
   * Unit of the keyword.
   *
   * @related AllowedMembers::unit
   *
   * @note:
   * - List of constraints:
   *   - It is an optional member.
   */
  std::string keyUnit;

  /**
   * GCS component instance name, which declares and updates the monitor.
   *
   * @related AllowedMembers::component
   *
   * @note:
   * - List of constraints:
   *   - It is an mandatory member.
   *   - It value can not be empty.
   */
  std::string deviceName;

  /**
   * Monitor name used as data source to write the keyword in the FITS header.
   *
   * @related AllowedMembers::monitor
   *
   * @note:
   * - List of constraints:
   *   - It is an mandatory member.
   *   - It value can not be empty.
   */
  std::string monitorName;

  /**
   * Type of the monitor.
   *
   * @related AllowedMembers::type
   * @related MonitorType
   *
   * @note:
   * - List of constraints:
   *   - It is an mandatory member.
   *   - It value can not be empty.
   *   - The allowed values are: "SAMPLED" or "DISCRETE"
   * - Meaning of values:
   *   - "SAMPLED" value is reserved for scalar or array monitor.
   *   - "DISCRETE" value is reserved for enum or boolean monitor.
   */
  MonitorType monitorType;

  /**
   * Time to retrieved the monitor.
   *
   * @related AllowedMembers::when
   * @related When
   *
   * @note:
   * - List of constraints:
   *   - It is an optional member.
   *   - The allowed values are: "START" or "END".
   * - Meaning of values:
   *   - "START" value is used to retrieve the monitor at beginning of the exposure time.
   *   - "END" value is used to retrieve the monitor at end of the exposure time.
   */
  When when;

  /**
   * Transformation to apply to the monitor.
   *
   * @related AllowedMembers::format
   * @related AllowedMembers::type
   * @related KeyFormat
   * @related monitorType
   *
   * @note:
   * - List of constraints:
   *   - It is an optional member.
   *   - It can only be present when the monitorType is "SAMPLED".
   *   - The allowed values are: "RAD_TO_HMS", "RAD_TO_DMS", "RAD_TO_DEG", "ARRAY_TO_SCALAR".
   * - Meaning of values:
   *   - "RAD_TO_HMS" can only be used for scalar monitor to convert an angle in radians to hours, minutes, seconds.
   *   - "RAD_TO_DMS" can only be used for scalar monitor to convert an angle in radians to degrees, arcminutes, arcseconds.
   *   - "RAD_TO_DEG" can only be used for scalar monitor to convert an angle in radians to degrees.
   *   - "ARRAY_TO_SCALAR" can only be used for array monitor.
   */
  KeyFormat keyFormat;

  /**
   * Indicator to generate a keyword for each index of the monitor.
   *
   * @related AllowedMembers::transposed
   * @related AllowedMembers::type
   * @related monitorType
   * @related AllowedMembers::format
   * @related keyFormat
   * @related AllowedMembers::initialIndex
   * @related keyInitialIndex
   * @related AllowedMembers::range
   * @related arrayRange
   *
   * @note:
   * - List of constraints:
   *   - It is an optional member.
   *   - It can only be present when the monitorType is "SAMPLED"
   *     and keyFormat is "ARRAY_TO_SCALAR".
   *   - The allowed values are: true or false.
   * - Meaning of values:
   *   - true is used to generate a keyword for each value of valid range of the array monitor.
   *   - false (default) is used to write all values as string into one keyword.
   */
  bool transposed;

  /**
   * Beginning counter used as suffix for each generated keyword.
   *
   * @related AllowedMembers::initialIndex
   * @related AllowedMembers::type
   * @related monitorType
   * @related AllowedMembers::format
   * @related keyFormat
   * @related AllowedMembers::transposed
   * @related transposed
   *
   * @note:
   * - List of constraints:
   *   - It is an optional member.
   *   - The allowed values are the natural numbers.
   *   - It can only be present when the monitorType is "SAMPLED"
   *     and keyFormat is "ARRAY_TO_SCALAR" and transposed is true.
   */
  unsigned int keyInitialIndex;

  /**
   * Specific valid range of indexes for the array monitor.
   *
   * @related AllowedMembers::range
   * @related AllowedMembers::type
   * @related monitorType
   * @related AllowedMembers::format
   * @related keyFormat
   *
   * @note:
   * - List of constraints:
   *   - It is an optional member.
   *   - It can only be present when the monitorType is "SAMPLED"
   *     and keyFormat is "ARRAY_TO_SCALAR".
   *   - The allowed values are [i,j], such as: 0 <= i <= j <= (N-1) and N=(width * height) of the array monitor.
   *   - If it is not present in the file, then the range will be [0,((width * height) - 1)].
   */
  ArrayRange arrayRange;

  /**
   * List of labels for an enum or boolean monitor.
   *
   * @related AllowedMembers::labels
   * @related AllowedMembers::type
   * @related monitorType
   *
   * @note:
   * - List of constraints:
   *   - It is an optional member.
   *   - All its values must be strings.
   *   - All its values will be compared with the description of the database.
   *     If any values mismatches, then an exception will be raised.
   *   - It can only be present when the monitorType is "DISCRETE".
   */
  EnumLabels enumLabels;

  //--------------------------------------------------
  // protected section
  //--------------------------------------------------
  protected:

  //########################### PROTECTED TYPES ##########################

  /**
   * Data type to represent if an allowed member is present or not.
   */
  using MapPresentMembers = std::map<std::string, bool>;


  //######################### PROTECTED CONSTANTS ########################


  //########################## PROTECTED METHODS #########################

  /**
   * Checks whether the information comes from the FITS header keyword
   * specification file is consistent and matches with the information
   * of the database.
   *
   * @pre
   * - The FITS header keywords specification file had to be loaded first.
   *
   * @note:
   * - If the given information is not consistent, then an exception
   *   will be raised.
   * - If the given information and the monitor description of the database
   *   do not match, then an exception will be raised.
   */
  virtual void checkDataIntegrity();

  /**
   * Sets all internal members to default values.
   */
  virtual void clearAllMembers_();

  /**
   * Detects whether there are optional members present, which could be allowed
   * or not, depending on the value of the type member.
   *
   * @note
   * - An exception is raise, when an not-allowed optional member is detected.
   */
  virtual void detectOptionalMembersNotAllowed_();

  /**
   * Obtains the monitor description of the database and contrasts the given information
   * with the database information.
   *
   * @note
   * - An exception is raised if there is an inconsistency in the information.
   *
   * @param[in] monitorManager Reference to the MonitorManager service.
   */
  virtual void checkDataBaseDescription_(const MM::MonitorManager_ifce_var &monitorManager);

  /**
   * Contrasts the given information with the description of the scalar or array monitor,
   * which comes from the database.
   *
   * @note
   * - An exception is raised if there is an inconsistency in the information.
   *
   * @param[in] monitorDescription Reference to the monitor description.
   */
  virtual void checkMonitorDescription_(const MM::Description_var &monitorDescription);

  /**
   * Contrasts the given information with the description of the enum or boolean monitor,
   * which comes from the database.
   *
   * @note
   * - An exception is raised if there is an inconsistency in the information.
   *
   * @param[in] magnitudeDescription Reference to the magnitude description.
   */
  virtual void checkMagnitudeDescription_(const MM::MagnitudeDescription_var &magnitudeDescription);

  //########################## PROTECTED MEMBERS #########################

  /**
   * Map to indicate if a member is present or not.
   */
  MapPresentMembers presentMembers_;

  /**
   * Indicates the level of verbosity.
   */
  unsigned short verbose_;

  /**
   * Pointer of a LogAgent instance,
   * which is related to the invoke the constructor of this object.
   */
  LogAgent *logAgent_;

  /**
   * Pointer of a Trace instance,
   * which is related to the invoke the constructor of this object.
   */
  Trace *trace_;

 //--------------------------------------------------
 // private section
 //--------------------------------------------------
 private:

  //############################ PRIVATE TYPES ###########################

  /**
   * Forward class declaration, which will be reserved to
   * perform the unit test.
   */
  friend class MonitorKeywordTest;

  //########################## PRIVATE CONSTANTS #########################

  //########################### PRIVATE METHODS ##########################

  //########################### PRIVATE MEMBERS ##########################

  //############################ OTHER METHODS ###########################

  //<editor-fold desc="Equality operators">
  /**
   * Equality operator, which determines if lhs is equal to rhs or not.
   *
   * @param[in] lhs Left-hand side operand.
   * @param[in] rhs Right-hand side operand.
   * @return True, if lhs is equal to rhs. False, otherwise.
   */
  friend bool operator==(const MonitorKeyword &lhs, const MonitorKeyword &rhs) noexcept;

  /**
   * Equality operator, which determines if lhs is not equal to rhs or not.
   *
   * @param[in] lhs Left-hand side operand.
   * @param[in] rhs Right-hand side operand.
   * @return True, if lhs is not equal to rhs. False, otherwise.
   */
  friend bool operator!=(const MonitorKeyword &lhs, const MonitorKeyword &rhs) noexcept;
  //</editor-fold>

  //<editor-fold desc="Relational operators">
  /**
   * Relational operator, which determines if lhs is less than rhs or not.
   *
   * @param[in] lhs Left-hand side operand.
   * @param[in] rhs Right-hand side operand.
   * @return True, if lhs is less than rhs. False, otherwise.
   */
  friend bool operator<(const MonitorKeyword &lhs, const MonitorKeyword &rhs) noexcept;

  /**
   * Relational operator, which determines if lhs is greater than rhs or not.
   *
   * @param[in] lhs Left-hand side operand.
   * @param[in] rhs Right-hand side operand.
   * @return True, if lhs is greater than rhs. False, otherwise.
   */
  friend bool operator>(const MonitorKeyword &lhs, const MonitorKeyword &rhs) noexcept;

  /**
   * Relational operator, which determines if lhs is less than or equal to rhs or not.
   *
   * @param[in] lhs Left-hand side operand.
   * @param[in] rhs Right-hand side operand.
   * @return True, if lhs is less than or equal to rhs. False, otherwise.
   */
  friend bool operator<=(const MonitorKeyword &lhs, const MonitorKeyword &rhs) noexcept;

  /**
   * Relational operator, which determines if lhs is greater than or equal to rhs or not.
   *
   * @param[in] lhs Left-hand side operand.
   * @param[in] rhs Right-hand side operand.
   * @return True, if lhs is greater than or equal to rhs. False, otherwise.
   */
  friend bool operator>=(const MonitorKeyword &lhs, const MonitorKeyword &rhs) noexcept;
  //</editor-fold>

};

//<editor-fold desc="from_string">
/**
 * Convert a string to a valid value of MonitorKeyword::MonitorType
 * if it exist, or throw an exception if it doesn't exist.
 *
 * @param[out] obj MonitorKeyword::MonitorType variable to setup.
 * @param[in]  str String representation of MonitorKeyword::MonitorType.
 *
 * @exception InvalidValue
 * This exception is thrown if the given string value doesn't match
 * with a valid value of MonitorKeyword::MonitorType.
 *
 * @code{.cpp}
 * MonitorKeyword::MonitorType type;
 * from_string(type, "NONE");
 * from_string(type, "SAMPLED");
 * from_string(type, "DISCRETE");
 * @endcode
 *
 */
void from_string(MonitorKeyword::MonitorType &obj, std::string str);

/**
 * Convert a string to a valid value of MonitorKeyword::When
 * if it exist, or throw an exception if it doesn't exist.
 *
 * @param[out] obj MonitorKeyword::When variable to setup.
 * @param[in]  str String representation of MonitorKeyword::When.
 *
 * @exception InvalidValue
 * This exception is thrown if the given string value doesn't match
 * with a valid value of MonitorKeyword::When.
 *
 * @code{.cpp}
 * MonitorKeyword::When when;
 * from_string(when, "NONE");
 * from_string(when, "START");
 * from_string(when, "END");
 * @endcode
 *
 */
void from_string(MonitorKeyword::When &obj, std::string str);

/**
 * Convert a string to a valid value of MonitorKeyword::KeyFormat
 * if it exist, or throw an exception if it doesn't exist.
 *
 * @param[out] obj MonitorKeyword::KeyFormat variable to setup.
 * @param[in]  str String representation of MonitorKeyword::KeyFormat.
 *
 * @exception InvalidValue
 * This exception is thrown if the given string value doesn't match
 * with a valid value of MonitorKeyword::KeyFormat.
 *
 * @code{.cpp}
 * MonitorKeyword::KeyFormat format;
 * from_string(format, "NONE");
 * from_string(format, "RAD_TO_HMS");
 * from_string(format, "RAD_TO_DMS");
 * from_string(format, "RAD_TO_DEG");
 * from_string(format, "ARRAY_TO_SCALAR");
 * @endcode
 *
 */
void from_string(MonitorKeyword::KeyFormat &obj, std::string str);

/**
 * Convert a string to a valid value of MonitorKeyword::ArrayRange,
 * or throw an exception if it is invalid.
 *
 * @param[out] obj MonitorKeyword::ArrayRange variable to setup.
 * @param[in]  str String range that contains the needed information
 *                 to fill the container. The expected syntax is:
 *                 "<uint>,<uint>", where <uint> is an
 *                 unsigned int value.
 *
 * @exception SyntaxError
 * This exception is thrown if the given string has an invalid format.
 *
 * @code{.cpp}
 * MonitorKeyword::ArrayRange range;
 * from_string(range, "1,2");
 * @endcode
 */
void from_string(MonitorKeyword::ArrayRange &obj, std::string &str);

/**
 * Convert a string to a valid value of MonitorKeyword::EnumLabels,
 * or throw an exception if it is invalid.
 *
 * @param[out] obj MonitorKeyword::EnumLabels variable to setup.
 * @param[in]  str String labels that contains the needed information
 *                 to fill the container.The expected syntax is:
 *                 "label1,label2,....,labelN".
 *
 * @exception SyntaxError
 * This exception is thrown if the given string has an invalid format.
 *
 * @code{.cpp}
 * MonitorKeyword::EnumLabels labels;
 * from_string(labels, "IN,OUT");
 * @endcode
 */
void from_string(MonitorKeyword::EnumLabels &obj, std::string &str);
//</editor-fold>

//<editor-fold desc="to_string">
/**
 * Convert a value of MonitorKeyword::MonitorType to its string representation.
 *
 * @param[in] obj Variable to convert to string.
 * @return Valid string representation of MonitorKeyword::MonitorType.
 *
 * @exception InvalidValue
 * This exception is thrown if the given value of the given variable doesn't
 * match with a declared value of MonitorKeyword::MonitorType.
 */
const char* to_string(const MonitorKeyword::MonitorType &obj);

/**
 * Convert a value of MonitorKeyword::When to its string representation.
 *
 * @param[in] obj Variable to convert to string.
 * @return Valid string representation of MonitorKeyword::When.
 *
 * @exception InvalidValue
 * This exception is thrown if the given value of the given variable doesn't
 * match with a declared value of MonitorKeyword::When.
 */
const char* to_string(const MonitorKeyword::When &obj);

/**
 * Convert a value of MonitorKeyword::KeyFormat to its string representation.
 *
 * @param[in] obj Variable to convert to string.
 * @return Valid string representation of MonitorKeyword::KeyFormat.
 *
 * @exception InvalidValue
 * This exception is thrown if the given value of the given variable doesn't
 * match with a declared value of MonitorKeyword::KeyFormat.
 */
const char* to_string(const MonitorKeyword::KeyFormat &obj);

/**
 * Convert a value of MonitorKeyword::ArrayRange to its string representation.
 *
 * @param[in] obj Variable to convert to string.
 * @return Valid string representation of MonitorKeyword::ArrayRange.
 *
 * @exception InvalidValue
 * This exception is thrown if the given value of the given variable doesn't
 * match with a declared value of MonitorKeyword::ArrayRange.
 *
 * @code{.cpp}
 * MonitorKeyword::ArrayRange range (1,2);
 * std::string strRange (to_string(range)); "1,2"
 * @endcode
 */
const char* to_string(const MonitorKeyword::ArrayRange &obj);

/**
 * Convert a value of MonitorKeyword::EnumLabels to its string representation.
 *
 * @param[in] obj Variable to convert to string.
 * @return Valid string representation of MonitorKeyword::EnumLabels.
 *
 * @exception InvalidValue
 * This exception is thrown if the given value of the given variable doesn't
 * match with a declared value of MonitorKeyword::EnumLabels.
 *
 * @code{.cpp}
 * MonitorKeyword::EnumLabels labels ({"IN","OUT"});
 * std::string strLabels (to_string(labels)); // "IN,OUT"
 * @endcode
 */
const char* to_string(const MonitorKeyword::EnumLabels &obj);
//</editor-fold>

//<editor-fold desc="operator<<">
/**
 * Insert formatted output of MonitorKeyword::MonitorType variable to an output stream.
 * @param[in,out] out Output stream where the given object will be inserted.
 * @param[in]     obj Object to insert to the output stream.
 * @return Output stream with the given object inserted.
 */
std::ostream& operator<<(std::ostream& out, const MonitorKeyword::MonitorType &obj);

/**
 * Insert formatted output of MonitorKeyword::When variable to an output stream.
 * @param[in,out] out Output stream where the given object will be inserted.
 * @param[in]     obj Object to insert to the output stream.
 * @return Output stream with the given object inserted.
 */
std::ostream& operator<<(std::ostream& out, const MonitorKeyword::When &obj);

/**
 * Insert formatted output of MonitorKeyword::KeyFormat variable to an output stream.
 * @param[in,out] out Output stream where the given object will be inserted.
 * @param[in]     obj Object to insert to the output stream.
 * @return Output stream with the given object inserted.
 */
std::ostream& operator<<(std::ostream& out, const MonitorKeyword::KeyFormat &obj);
//</editor-fold>

#endif // _MonitorKeyword_h_
