
#include <celutil/debug.h>
#include "dataloader.h"

using namespace std;

bool AstroDataLoader::load(const string& fname, bool cType)
{
    if (cType)
    {
        if (getSupportedContentType() != Content_Unknown && DetermineFileType(fname) != getSupportedContentType())
        {
            DPRINTF("Error while loading content from \"%s\": wrong file content type.\n", fname.c_str());
            return false;
        }
    }

    fstream stream(fname, ios::in);
    if (!stream.good()) {
        DPRINTF("Error while loading content from \"%s\": cannot open file\n.");
        return false;
    }

    return load(stream);
}
