#ifndef _FITSHeaderInfo_h_
#define _FITSHeaderInfo_h_

#include <iostream>

#include <IDL_Adapters_DGT.h>
#include <vector>
#include <MonitorManagerC.h>
#include <MonitorAgent.h>
#include <DException.h>
#include <GCSTypes.h>
#include <Frame.h>
#include <CORBARef.h>
#include <slalib.h>
#include <slamac.h>

class DFAgent;

struct KeywordInfo
{
   KeywordInfo (string key, string dev, string mon, string type, string format, string range, string labels, string comment, string condition, string if_condition, string when)
    : keyName(key)
     ,monitorName(mon)
     ,deviceName(dev)
     ,monitorType(type)
     ,keyFormat(format)
     ,arrayRange(range)
     ,enumLabels(labels)
     ,keyComment(comment) {} 
   
   string keyName;
   string deviceName;
   string monitorName;  
   string monitorType;
   string keyFormat;
   string arrayRange;
   string enumLabels;
   string keyComment;
   string condition;
   string if_condition;
   string when;
};

class FITSHeaderInfo
{
  public:
    void writeHeaderToFile( const TimeValue& time, const TimeValue& expTime, MonitorAgent* dfMonitorAgent, fitsfile* rawFile);
    void parseKeywordsFile(const string& fileName);
    void gatherMonitorKeywords(const TimeValue& time, const TimeValue& expTime, MonitorAgent* dfMonitorAgent, DataDescription& frame );
    void forceSampleData(MonitorAgent* dfMonitorAgent);
    
  private:
    void gatherSpecialMonitorKeywords_(const TimeValue& time, const TimeValue& expTime, MonitorAgent* dfMonitorAgent, DataDescription& frame);
    std::string formatRadianToHms_(double rad);
    std::string formatRadianToDms_(double rad);
    double formatRadianToDeg_(double rad);
    std::pair<unsigned int, unsigned int> rangeFromString_( const string &range );
    string getLabelFromValue_(const string &sLabels, const long &val);

    template<class ListType>
    ostream& toStream_(ostream& ost, const ListType& a);
    
    template<class ListType, typename ValueType> 
    void addSampledMonitorKeyword_(const KeywordInfo& keyword, ListType& list, DataDescription& frame);
    
    template<typename C>
    C trimmedArray1D_(const C arrayIn, const std::string sRange);
    
    void gatherSampledMonitor_( const KeywordInfo& keyword, const TimeValue& requestedTime, const TimeValue& requestedExpTime,
                                MonitorAgent* dfMonitorAgent, DataDescription& frame );
    void gatherDiscreteMonitor_( const KeywordInfo& keyword, const TimeValue& requestedTime, 
                                MonitorAgent* dfMonitorAgent, DataDescription& frame );
    std::vector<KeywordInfo> monList_;
    std::map<string, bool> conditionList_;
    
};

#endif // _FITSHeaderInfo_h_
