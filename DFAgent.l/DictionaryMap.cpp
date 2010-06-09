#include "DictionaryMap.h"

inline DictionaryMap::DictionaryMap() :
       PersistentHashMap< WordSet >(std::string("ASGRecipeDictionary")) {}


void DictionaryMap::fromPortableObject( const PortableObject& object,
                                          WordSet& wordSet )
{
  std::string attribute;
      
  attribute = object.getAttribute("rawFrame");
  if(attribute == "")
  {
    std::string msg = "Attribute '" + attribute +
      	      	      "' not found in portable object";
    throw NotFound("DictionaryMap::fromPortableObjecct()", msg.c_str());
  }    
  try
  {
    wordSet.rawFrame = attribute;
  }
  catch(GCSException& exception)
  {
    exception.addToTrace("DictionaryMap::fromPortableObject()");
    throw;
  }

      
  attribute = object.getAttribute("exposureTime");
  if(attribute == "")
  {
    std::string msg = "Attribute '" + attribute +
      	      	      "' not found in portable object";
    throw NotFound("DictionaryMap::fromPortableObjecct()", msg.c_str());
  }    
  try
  {
    wordSet.exposureTime = attribute;
  }
  catch(GCSException& exception)
  {
    exception.addToTrace("DictionaryMap::fromPortableObject()");
    throw;
  }

}

inline std::string DictionaryMap::fetchTranslation(const std::string& word, const std::string& wordSetID)
{
  try
  { 
    if(word == "rawFrame")
    {
      return this->entry(wordSetID).rawFrame;
    }
    else if(word == "exposureTime")
    {
      return this->entry(wordSetID).exposureTime;
    }
    else
    {
      // no translation found
      return word;
    }
  }
  catch(GCSException& exception)
  {
    exception.addToTrace("DictionaryMap::fetchTranslation()"),
    throw;
  }
}

inline std::string DictionaryMap::fetchOriginal(const std::string& word, const std::string& wordSetID)
{
  try
  { 
    if(this->entry(wordSetID).rawFrame == word)
    {
      return std::string("rawFrame");
    }
    else if(this->entry(wordSetID).exposureTime == word)
    {
      return std::string("exposureTime");
    }
    else
    {
      //no original found
      return word;
    }
  }
  catch(GCSException& exception)
  {
    exception.addToTrace("DictionaryMap::fetchOriginal()"),
    throw;
  }
}

inline size_t DictionaryMap::size() const
{
  return PersistentHashMap< WordSet >::size();
}
