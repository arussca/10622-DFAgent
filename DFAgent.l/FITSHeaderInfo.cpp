#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>
#include <FITSHeaderInfo.h>
#include <DCFC.h>

using namespace std;

string FITSHeaderInfo::formatRadianToHms_(double rad)
{
   char sign;
   int ihmsf[4];
   ostringstream strbuf;
   
   slaDr2tf (3, rad, &sign, ihmsf);
   strbuf<<sign<<ihmsf[0]<<":"<<ihmsf[1]<<":"<<ihmsf[2]<<"."<<ihmsf[3];
   
   return strbuf.str();
}

string FITSHeaderInfo::formatRadianToDms_(double rad)
{
   char sign;
   int ihmsf[4];
   ostringstream strbuf;
   
   slaDr2af (3, rad, &sign, ihmsf);
   strbuf<<sign<<ihmsf[0]<<":"<<ihmsf[1]<<":"<<ihmsf[2]<<"."<<ihmsf[3];
   
   return strbuf.str();
}

double FITSHeaderInfo::formatRadianToDeg_(double rad)
{
   return rad*DR2D;
}

template<class ListType, typename ValueType>
void FITSHeaderInfo::addSampledMonitorKeyword_(const KeywordInfo& keyword, ListType& list, Frame<double,2>& frame)
{
   if (list.length() < 1)
      throw NotFound ("addSampledMonitorKeyword_()", "Data length is 0");
   
   if (keyword.keyFormat == "RAD_TO_HMS")
      frame.addKeyword(Keyword<string>(keyword.keyName, formatRadianToHms_((double) list[0]), "h:m:s"));   
   else if (keyword.keyFormat == "RAD_TO_DMS")
      frame.addKeyword(Keyword<string>(keyword.keyName, formatRadianToDms_((double) list[0]), "d:m:s"));   
   else if (keyword.keyFormat == "RAD_TO_DEG")
      frame.addKeyword(Keyword<double>(keyword.keyName, formatRadianToDeg_((double) list[0]), "degrees"));      
   else   
      frame.addKeyword (Keyword<ValueType>(keyword.keyName, (ValueType) list[0], ""));   
}


void FITSHeaderInfo::parseKeywordsFile(const string& fileName)
{
  ifstream f(fileName.c_str());
  string devName, monName, keyName, keyType, keyFormat, arrayRange, specLine;
  if (f.is_open())
  {
    while(!f.eof())
    { //this keyword is not mandatory, so it needs to be cleaned from previous value
      arrayRange = "";
      
      getline(f,specLine);
      stringstream ss(specLine);
      
      ss >> keyName;
      ss >> devName;
      ss >> monName;
      ss >> keyType;
      ss >> keyFormat;
      if(ss.good())
      { //this keyword is optional in order to keep it compatible with previuos versions
        ss >> arrayRange;
      }

      monList_.push_back(KeywordInfo(keyName, devName, monName, keyType, keyFormat, arrayRange));
    }
    f.close();
  }
}

void FITSHeaderInfo::subscribeToMonitors(MONITOR::Manager_ifce_var &manager, unsigned char monitorManagerSubscriptionID)
{
  vector<KeywordInfo>::const_iterator it, endIt;

  it = monList_.begin();
  endIt = monList_.end();

  while (it != endIt)
  {
    try
    { 
      DCF::Device_ifce_var deviceInstance;
      CORBARef<DCF::Device_ifce> dev_ref((it->deviceName).c_str(), 0);
      dev_ref.roundtripTimeout( TimeValue(1, 0) );
      deviceInstance = dev_ref.typedRef();
     
      MONITOR::MagnitudeMonitor_ifce_var magnitudeMonitorInstance = deviceInstance->magnitudeMonitor((it->monitorName).c_str());
      magnitudeMonitorInstance->start(monitorManagerSubscriptionID);        
    }
    catch (GCSException& ex)
    {
/*    
      cerr << "ERROR subscribing to monitor: " << it->keyName << endl;
      ex.addToTrace("subscribeToMonitors()");      
//      logError_ ("Error subscribing to monitors: %s", ex.toString());
      cerr << ex << endl;   
*/      
    }
    catch (const CORBA::Exception &ex)
    {
/*    
      GCSException gcsex = DException::mapCORBAException(const_cast<CORBA::Exception&>(ex));
      cerr << "CORBA ERROR subscribing to monitors" << it->keyName << endl;
//      logError_ ("CORBA Error subscribing to monitors: %s", gcsex.toString());
      cerr << gcsex << endl;   
*/      
    }   
    //increment position
    ++it;
  }
  
}

void FITSHeaderInfo::gatherMonitorKeywords(const TimeValue& time, MONITOR::Manager_ifce_var &manager, Frame<double,2>& frame)
{
  vector<KeywordInfo>::const_iterator it, endIt;
  MONITOR::MagnitudeChangeRepo_ifce_var changeRepo = manager->magnitudeChangeRepo();

  it = monList_.begin();
  endIt = monList_.end();
  
  while (it != endIt)
  {
    try
    {
      if (it->monitorType == "SAMPLED")
      {
        gatherSampledMonitor_(*it, time, manager, frame);
      }
      else if (it->monitorType == "DISCRETE")
      {
        gatherDiscreteMonitor_(*it, time, changeRepo, frame);
      }
      else
      {
        cout << "ERROR: invalid key format : " << it->keyName << " " << it->keyFormat << endl;
      }
        
    }
    catch (GCSException& ex)
    {
/*    
      cerr<<"ERROR gathering header keyword: "<<it->keyName<<endl;
      ex.addToTrace ("gatherMonitorKeywords_()");   
//      logError_ ("Error gathering header keyword: %s", ex.toString());
      cerr<<ex<<endl;
*/      
    }
    catch (const CORBA::Exception &ex)
    {
/*    
      cerr << "CORBA ERROR gathering header keyword" << endl;
//      logError_ ("CORBA Error gathering header keyword: %s", ex.toString());     
      cerr<<ex<<endl;
*/      
    }   
    //increment position
    ++it;
  }
}

std::pair<unsigned int, unsigned int> FITSHeaderInfo::rangeFromString_( const string &sRange )
{
  //parse string with the format as follows; [valueX1,valueX2]
  unsigned int startIndex, endIndex;
  string vX1(sRange.substr(sRange.find_first_of("[") + 1, sRange.find_first_of(",")));
  string vX2(sRange.substr(sRange.find_first_of(",") + 1, sRange.find_last_of("]")));
  
  istringstream issvX1(vX1), issvX2(vX2);
  if( (issvX1 >> dec >> startIndex).fail() || (issvX2 >> dec >> endIndex).fail() )
  {
    throw InvalidValue("FITSHeaderInfo::rangeFromString_(string)","Cannot get a number from string");
  }

  return std::pair<unsigned int, unsigned int>(startIndex, endIndex);
}

template<typename C>
C FITSHeaderInfo::trimmedArray1D_(C arrayIn, const std::string sRange)
{
  std::pair<unsigned int, unsigned int> range;
  if(sRange == "")
  { //no specified range means the whole range is applied
    range = make_pair(0, (unsigned int)(arrayIn.dimension.width - 1));
  }
  else
  {
    range = rangeFromString_(sRange);
  }
  
  C arrayOut;
  arrayOut.dimension.width = (range.second - range.first) + 1;
  if( arrayIn.dimension.height != 1 )
  {
    throw InvalidValue("FITSHeaderInfo::trimmedArray1D_(C, string)","Only one dimentional array supported");
  }
  if( range.second < range.first )
  {
    throw InvalidValue("FITSHeaderInfo::trimmedArray1D_(C, string)","Last index lower than first");
  }
  if( range.first > (unsigned int)(arrayIn.dimension.width - 1) || range.second > (unsigned int)(arrayIn.dimension.width - 1) )
  {
    throw InvalidValue("FITSHeaderInfo::trimmedArray1D_(C, string)","Range out of bounds");
  }
  
  arrayOut.dimension.height = 1;
  arrayOut.data.length(arrayOut.dimension.width);
  
  unsigned int i(0);
  for(unsigned int j=range.first; j<=range.second; ++j)
  {
    arrayOut.data[i] = arrayIn.data[j];
    ++i;
  }
  
  return arrayOut;
}

void FITSHeaderInfo::gatherSampledMonitor_(const KeywordInfo& keyword, const TimeValue& requestedTime, MONITOR::Manager_ifce_var &manager, Frame<double,2>& frame)
{
   DGT::TimeInterval ti;
   TimeValue startTime = requestedTime - TimeValue(1,0);
   
   try
   {  
      ti.begin = toIDL(startTime);
      ti.end = toIDL(requestedTime);
      
      MONITOR::DataBlockList_var mons = manager->retrieveData (keyword.deviceName.c_str(), keyword.monitorName.c_str(), ti);
      if (mons->length() < 1)
         throw NotFound ("", "Data not found");
      
      switch ( (*mons)[0].samples._d() )
      {
         case DGT::TYPE_DOUBLE:
           addSampledMonitorKeyword_<DGT::DoubleList, double>(keyword, (*mons)[0].samples.d(), frame);
           break;
         case DGT::TYPE_FLOAT:
           addSampledMonitorKeyword_<DGT::FloatList, double>(keyword, (*mons)[0].samples.f(), frame);
           break;
         case DGT::TYPE_LONG:
           addSampledMonitorKeyword_<DGT::LongList, long>(keyword, (*mons)[0].samples.l(), frame);
           break;
         case DGT::TYPE_SHORT:
           addSampledMonitorKeyword_<DGT::ShortList, long>(keyword, (*mons)[0].samples.s(), frame);
           break;
         case DGT::TYPE_OCTET:
           addSampledMonitorKeyword_<DGT::OctetList, long>(keyword, (*mons)[0].samples.o(), frame);
           break;
         case DGT::TYPE_DOUBLE_ARRAY1D:
         {
	        ostringstream formatbuf;
	        toStream(formatbuf, trimmedArray1D_((*mons)[0].samples.array_d()[0], keyword.arrayRange), 1);
	        frame.addKeyword (Keyword<string>(keyword.keyName, formatbuf.str(), "")); 
         }   
	      break;
         case DGT::TYPE_FLOAT_ARRAY1D:
         {
	        ostringstream formatbuf;
	        toStream(formatbuf, trimmedArray1D_((*mons)[0].samples.array_f()[0], keyword.arrayRange), 1);
	        frame.addKeyword (Keyword<string>(keyword.keyName, formatbuf.str(), "")); 
         }   
	      break;         
         case DGT::TYPE_LONG_ARRAY1D:
         {
	        ostringstream formatbuf;
	        toStream(formatbuf, trimmedArray1D_((*mons)[0].samples.array_l()[0], keyword.arrayRange), 1);
	        frame.addKeyword (Keyword<string>(keyword.keyName, formatbuf.str(), "")); 
         }
	      break;
         case DGT::TYPE_SHORT_ARRAY1D:
         {
	        ostringstream formatbuf;
	        toStream(formatbuf, trimmedArray1D_((*mons)[0].samples.array_s()[0], keyword.arrayRange), 1);
	        frame.addKeyword (Keyword<string>(keyword.keyName, formatbuf.str(), "")); 
         }   
	      break;
         case DGT::TYPE_OCTET_ARRAY1D:
         {
	        ostringstream formatbuf;
	        toStream(formatbuf, trimmedArray1D_((*mons)[0].samples.array_o()[0], keyword.arrayRange), 1);
	        frame.addKeyword (Keyword<string>(keyword.keyName, formatbuf.str(), "")); 
         }   
	      break;
      }
   }
   catch (GCSException& ex)
   {
      ex.addToTrace ("gatherSampledMonitor()");
      throw;
   }
}

void FITSHeaderInfo::gatherDiscreteMonitor_(const KeywordInfo& keyword, const TimeValue& requestedTime, MONITOR::MagnitudeChangeRepo_ifce_var &manager, Frame<double,2>& frame)
{
   DGT::TimeInterval ti;
   TimeValue startTime = requestedTime - TimeValue(1,0);

   try
   {
      ti.begin = toIDL(startTime);
      ti.end = toIDL(requestedTime);
      
      MONITOR::MagnitudeChangeFilter filter;
      
      filter.gcs_component = keyword.deviceName.c_str();
      filter.magnitude_name = keyword.monitorName.c_str();
      filter.time_stamp_range = ti;
      filter.value = DGT::NUM_DOESNT_CARE;
      
      MONITOR::MagnitudeChangeList_var mons = manager->select (filter);
      
      if (mons->length() < 1)
         throw NotFound ("", "Data not found");
      
      frame.addKeyword(Keyword<long>(keyword.keyName, mons[0].new_value, ""));   
   }
   catch (GCSException& ex)
   {
      ex.addToTrace ("gatherDiscreteMonitor()");
      throw;
   }   
}

void FITSHeaderInfo::writeHeaderToFile( const TimeValue& time, MONITOR::Manager_ifce_var &manager, fitsfile* rawFile)
{
  //first of all we gather monitor keywrods into the dataDescription object
  Frame<double,2> nFrame;
  gatherMonitorKeywords (time, manager, nFrame);
  DataDescription newKeywords(nFrame.dataDescription());
  
  int status_=0;
  //We replace fits_write_key with fits_update_key funtion
  //obtained from HDU::write, the routine there is not suitable
  //for our needs, but nearly
  try
  {
    typedef std::map<string, Keyword<string> >::const_iterator MIS;
    for(MIS it = newKeywords.stringBegin();it!= newKeywords.stringEnd();++it)
    {
      if(!FITS::initialKeyword((*it).first))
      {
        fits_update_key(rawFile, TSTRING, const_cast<char *>((*it).first.c_str()),
		     const_cast<char *>((*it).second.value().c_str()),
		     const_cast<char *>((*it).second.comment().c_str()), &status_);
        fits_write_key_unit(rawFile,const_cast<char *>((*it).first.c_str()),
           const_cast<char *>((*it).second.unit().c_str()),&status_);
      }
    }
    typedef std::map<string, Keyword<double> >::const_iterator MID;
    for(MID it = newKeywords.doubleBegin();it!= newKeywords.doubleEnd();++it)
    {
      if(!FITS::initialKeyword((*it).first))
      {
        fits_update_key(rawFile, TDOUBLE, const_cast<char *>((*it).first.c_str()),
		     const_cast<double *>(&((*it).second.value())),
		     const_cast<char *>((*it).second.comment().c_str()), &status_);
        fits_write_key_unit(rawFile,const_cast<char *>((*it).first.c_str()),
           const_cast<char *>((*it).second.unit().c_str()),&status_);
      }
    }
    typedef std::map<string, Keyword<long> >::const_iterator MIL;
    for(MIL it = newKeywords.longBegin();it!= newKeywords.longEnd();++it)
    {
      if(!FITS::initialKeyword((*it).first))
      {
    	  fits_update_key(rawFile, TLONG, const_cast<char *>((*it).first.c_str()),
          const_cast<long *>(&((*it).second.value())),
          const_cast<char *>((*it).second.comment().c_str()), &status_);
        fits_write_key_unit(rawFile,const_cast<char *>((*it).first.c_str()),
          const_cast<char *>((*it).second.unit().c_str()),&status_);
      }
    }
    typedef std::map<string, Keyword<bool> >::const_iterator MIB;
    for(MIB it = newKeywords.boolBegin();it!= newKeywords.boolEnd();++it)
    {
      if(!FITS::initialKeyword((*it).first))
      {
        fits_update_key(rawFile, TLOGICAL, const_cast<char *>((*it).first.c_str()),
		    const_cast<bool *>(&((*it).second.value())),
		    const_cast<char *>((*it).second.comment().c_str()), &status_);
        fits_write_key_unit(rawFile,const_cast<char *>((*it).first.c_str()),
          const_cast<char *>((*it).second.unit().c_str()),&status_);
      }
    }
    typedef std::vector<string>::const_iterator VI;
    for(VI  it = newKeywords.history().begin(); it != newKeywords.history().end(); ++it)
      fits_write_history(rawFile, const_cast<char *>((*it).c_str()), &status_);
    for(VI  it = newKeywords.comments().begin(); it != newKeywords.comments().end(); ++it)
      fits_write_comment(rawFile, const_cast<char *>((*it).c_str()), &status_);
  }
  catch(GCSException &ex)
  {
    cerr<<"ERROR While adding header to FITS file"<<endl;
    ex.addToTrace ("ERROR While adding header to FITS file");
    cerr << ex << endl;
  }    

}
