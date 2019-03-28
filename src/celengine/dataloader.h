
#pragma once

#include <string>
#include <fstream>
#include <celutil/filetype.h>

class AstroDatabase;

class AstroDataLoader
{
 protected:
    AstroDatabase *m_db;
    const ContentType m_CType;
 public:
    AstroDataLoader(ContentType type) : m_db(nullptr), m_CType(type) {}
    AstroDataLoader(AstroDatabase *db, ContentType type) : m_db(db), m_CType(type) {}
    void setDatabase(AstroDatabase *db) { m_db = db; }
    bool load(const std::string&, bool = true);
    virtual bool load(std::istream&) = 0;
    ContentType getSupportedContentType() const { return m_CType; }
};
