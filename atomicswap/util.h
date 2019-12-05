#include <string>
#include <stdint.h>

using namespace std;

int64_t GetTime();
int LogPrintStr(const string &str);
string DateTimeStrFormat(const char* pszFormat, int64_t nTime);

static inline bool AtoError(const char* format)
{
    LogPrintStr(string("ERROR: ") + format + "\n");
    return false;
}
