#ifndef _FITSHeaderInfo_h_
#define _FITSHeaderInfo_h_

#include <iostream.h>

#include <IDL_Adapters_DGT.h>
#include <vector>
#include <MONITORManagerC.h>
#include <MONITORC.h>
#include <DException.h>
#include <GCSTypes.h>
#include <Frame.h>
#include <CORBARef.h>
#include <slalib.h>
#include <slamac.h>


struct KeywordInfo
{
   KeywordInfo (string key, string dev, string mon, string type, string format, string range)
    : keyName(key)
     ,monitorName(mon)
     ,deviceName(dev)
     ,monitorType(type)
     ,keyFormat(format)
     ,arrayRange(range) {} 
   
   string keyName;
   string deviceName;
   string monitorName;  
   string monitorType;
   string keyFormat;
   string arrayRange;
};

class FITSHeaderInfo
{
  public:
    void writeHeaderToFile( const TimeValue& time, MONITOR::Manager_ifce_var &manager, fitsfile* rawFile);
    void parseKeywordsFile(const string& fileName);
    void gatherMonitorKeywords(const TimeValue& time, MONITOR::Manager_ifce_var &manager, Frame<double,2>& frame );
    void subscribeToMonitors(MONITOR::Manager_ifce_var &manager, unsigned char monitorManagerSubscriptionID);
  private:
    std::string formatRadianToHms_(double rad);
    std::string formatRadianToDms_(double rad);
    double formatRadianToDeg_(double rad);
    std::pair<unsigned int, unsigned int> rangeFromString_( const string &range );
    
    template<class ListType, typename ValueType> 
    void addSampledMonitorKeyword_(const KeywordInfo& keyword, ListType& list, Frame<double,2>& frame);
    
    template<typename C>
    C trimmedArray1D_(const C arrayIn, const std::string sRange);

    void gatherSampledMonitor_( const KeywordInfo& keyword, const TimeValue& requestedTime, 
                                MONITOR::Manager_ifce_var &manager, Frame<double,2>& frame );
    void gatherDiscreteMonitor_( const KeywordInfo& keyword, const TimeValue& requestedTime, 
                                 MONITOR::MagnitudeChangeRepo_ifce_var &manager, Frame<double,2>& frame );
    std::vector<KeywordInfo> monList_;
    
};

#endif // _FITSHeaderInfo_h_
