#include "util.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/thread.hpp>

static boost::once_flag debugPrintInitFlag = BOOST_ONCE_INIT;
static FILE* fileout = NULL;
static boost::mutex* mutexDebugLog = NULL;

int64_t GetTime()
{
    return time(NULL);
}

string DateTimeStrFormat(const char* pszFormat, int64_t nTime)
{
    // std::locale takes ownership of the pointer
    locale loc(std::locale::classic(), new boost::posix_time::time_facet(pszFormat));
    stringstream ss;
    ss.imbue(loc);
    ss << boost::posix_time::from_time_t(nTime);
    return ss.str();
}

static void DebugPrintInit()
{
    assert(fileout == NULL);
    assert(mutexDebugLog == NULL);

    boost::filesystem::path pathDebug = "./debug.log";
    fileout = fopen(pathDebug.string().c_str(), "a");
    if (fileout) setbuf(fileout, NULL);

    mutexDebugLog = new boost::mutex();
}

int LogPrintStr(const std::string &str)
{   
	int ret = 0;
    static bool fStartedNewLine = true;
    boost::call_once(&DebugPrintInit, debugPrintInitFlag);
    
    if (fileout == NULL)
    	return ret;
    
    boost::mutex::scoped_lock scoped_lock(*mutexDebugLog);
    
    // Debug print useful for profiling
	if (fStartedNewLine)
		ret += fprintf(fileout, "%s ", DateTimeStrFormat("%Y-%m-%d %H:%M:%S", GetTime()).c_str());
    if (!str.empty() && str[str.size()-1] == '\n')
        fStartedNewLine = true; 
    else
        fStartedNewLine = false;
    
    ret = fwrite(str.data(), 1, str.size(), fileout);
    
    return ret;
}
