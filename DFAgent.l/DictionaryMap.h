#ifndef _DictionaryMap_h_
#define _DictionaryMap_h_

#include <iostream>

#include "PersistentHashMap.h"

struct WordSet
{
  std::string rawFrame;
  std::string exposureTime;
  std::string theExcludedLensletIDsCophasing;
};

class DictionaryMap : public PersistentHashMap< WordSet >
{
  public:
    DictionaryMap();
    std::string fetchTranslation(const std::string& word, const std::string& wordSetID);
    std::string fetchOriginal(const std::string& word, const std::string& wordSetID);
    inline size_t size() const;

  private:
    void fromPortableObject(const PortableObject& object, WordSet& wordSet);
};

#endif   //_DictionaryMap_h_
