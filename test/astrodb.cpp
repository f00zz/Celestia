
#undef NDEBUG

#include <iostream>
#include <cassert>
#include <celengine/astrodb.h>
#include <celengine/stardataloader.h>

using namespace std;

int main()
{
    SetDebugVerbosity(10);
    clog << "AstroDatabase test\n";
    AstroDatabase adb;
    StcDataLoader loader(&adb);
    bool ret = loader.load(string("data/nearstars.stc"));
    cout << "Data loaded with status: " << ret << endl;
    return 0;
}
