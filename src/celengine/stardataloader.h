
#pragma once

#include "parser.h"
#include "dataloader.h"

class StarSTCDataLoader : public AstroDataLoader
{
 public:
    std::string resourcePath;
    StarSTCDataLoader() : AstroDataLoader(Content_CelestiaStarCatalog) {}
    virtual bool load(std::istream &);
    static void stcError(const Tokenizer& tok, const std::string& msg)
    {
        fmt::fprintf(cerr,  _("Error in .stc file (line %i): %s\n"), tok.getLineNumber(), msg);
    }
};
