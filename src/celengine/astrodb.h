#pragma once

#include <unordered_map>
#include "selection.h"
#include "name.h"
#include "astrocat.h"

class AstroDatabase {
public:
    class Entry : public Selection{
        AstroCatalog::IndexNumber m_celNumber;
    public:
        AstroCatalog::IndexNumber getIndex() const { return m_celNumber; }
    };
    typedef std::unordered_map<AstroCatalog::IndexNumber, AstroCatalog::IndexNumber> CrossIndex;
    
    enum Catalog
    {
        Celestia     = 0,
        HenryDrapper = 1,
        Gliese       = 2,
        SAO          = 3,
        Hipparcos    = 4,
        Tycho        = 5,
        MaxBuiltinCatalog = 6
    };
    
    static const char *CatalogPrefix[MaxBuiltinCatalog];
    
    Entry getObject(AstroCatalog::IndexNumber nr) { return m_mainIndex[nr]; };
    size_t size() const { return m_mainIndex.size(); };
    
    AstroCatalog::IndexNumber findCatalogNumberByName(const std::string&) const;
    
    AstroCatalog::IndexNumber searchCrossIndexForCatalogNumber(int, AstroCatalog::IndexNumber) const;
    AstroCatalog::IndexNumber crossIndex(int, AstroCatalog::IndexNumber) const;
    bool isInCrossIndex(int, AstroCatalog::IndexNumber) const;
    
    std::string catalogNumberToString(AstroCatalog::IndexNumber) const;
    std::string catalogNumberToString(int, AstroCatalog::IndexNumber) const;
    
    std::string getObjectName(AstroCatalog::IndexNumber, bool = false) const;
    std::string getObjectNameList(AstroCatalog::IndexNumber, int) const;
    
    std::vector<std::string> getCompletion(const std::string&name) const
    {
        return m_nameDB.getCompletion(name);
    }
    
    bool addAstroCatalog(int, AstroCatalog*);
    bool addCatalogNumber(AstroCatalog::IndexNumber, int, AstroCatalog::IndexNumber);
    
protected:
    std::unordered_map<AstroCatalog::IndexNumber, Entry> m_mainIndex;
    std::unordered_map<int, AstroCatalog*> m_catalogs;
    std::unordered_map<const char *, Catalog> m_prefixCatalog;
    std::unordered_map<int, CrossIndex*> m_catxindex;
    std::unordered_map<int, CrossIndex*> m_celxindex;
    NameDatabase m_nameDB;
    
    void createBuildinCatalogs();
};
