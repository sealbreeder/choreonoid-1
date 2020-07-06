/*!
  @file
  @author Shin'ichiro Nakaoka
*/

#include "ExecutablePath.h"
#include "FileUtil.h"
#include "UTF8.h"

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __linux__
#include <sys/utsname.h>
#include <unistd.h>
#include <dlfcn.h>
#include <cstring>
#endif

#ifndef MACOSX
#if defined(__APPLE__) && defined(__MACH__)
#define	MACOSX
#endif
#endif

#ifdef MACOSX
#include <mach-o/dyld.h>
#endif

using namespace std;

namespace {
string executableFile_;
string executableDir_;
string executableTopDir_;
string pluginDir_;
string shareDir_;
string executableBasename_;
}


namespace cnoid {

namespace filesystem = stdx::filesystem;

void detectExecutableFile()
{
#ifdef __linux__

    Dl_info dlInfo;
    if(dladdr((void*)(&detectExecutableFile), &dlInfo) == 0){
        throw std::runtime_error("The execution of the dladdr function to get the executable path failed.");
    }
    filesystem::path path(dlInfo.dli_fname);
    
    utsname info;
    if(uname(&info) == 0){
        if(strncmp(info.sysname, "Linux", 6) == 0){
            static const int BUFSIZE = 1024;
            char buf[BUFSIZE];
            int n = readlink("/proc/self/exe", buf, BUFSIZE - 1);
            buf[n] = 0;
            executableFile_ = buf;
        }
    }

#elif defined(_WIN32)

    constexpr int BufSize = MAX_PATH + 64;
    char execFilePath[BufSize] = "";
    if(GetModuleFileName(NULL, execFilePath, BufSize)){
        executableFile_ = execFilePath;
    }
    filesystem::path path(executableFile_);
    
#elif defined(MACOSX)
    
    char buf[1024];
    uint32_t n = sizeof(buf);
    if(_NSGetExecutablePath(buf, &n) == 0){
        executableFile_ = buf;
    }
        
    filesystem::path path;
    // remove dot from a path like bin/./choreonoid
    makePathCompact(filesystem::path(executableFile_), path);
    //filesystem::path path = filesystem::canonical(filesystem::path(executableFile_));
    
#endif

    if(path.empty()){
        throw std::runtime_error("The executable path cannot be detected.");
    }

    executableFile_ = toUTF8(executableFile_);

    executableDir_ = toUTF8(path.parent_path().string());
    
    filesystem::path topPath = path.parent_path().parent_path();
    executableTopDir_ = toUTF8(topPath.string());

    filesystem::path pluginPath = topPath / CNOID_PLUGIN_SUBDIR;
    pluginDir_ = toUTF8(pluginPath.make_preferred().string());
        
    filesystem::path sharePath = topPath / CNOID_SHARE_SUBDIR;
    if(filesystem::is_directory(sharePath)){
        shareDir_ = toUTF8(sharePath.make_preferred().string());

    } else if(filesystem::is_directory(sharePath.parent_path())){
        shareDir_ = toUTF8(sharePath.parent_path().make_preferred().string());

    } else if(topPath.has_parent_path()){ // case of a sub build directory
        sharePath = topPath.parent_path() / "share";
        if(filesystem::is_directory(sharePath)){
            shareDir_ = toUTF8(sharePath.make_preferred().string());
        }
    }

#ifdef _WIN32
    if(path.extension() == ".exe"){
        executableBasename_ = getBasename(path);
    } else {
        executableBasename_ = getFilename(path);
    }
#else
    executableBasename_ = getFilename(path);
#endif
}

const std::string& executableFile()
{
    if(executableFile_.empty()){
        detectExecutableFile();
    }
    return executableFile_;
}


const std::string& executableBasename()
{
    if(executableFile_.empty()){
        detectExecutableFile();
    }
    return executableBasename_;
}


const std::string& executableDir()
{
    if(executableFile_.empty()){
        detectExecutableFile();
    }
    return executableDir_;
}

const std::string& executableTopDir()
{
    if(executableFile_.empty()){
        detectExecutableFile();
    }
    return executableTopDir_;
}

stdx::filesystem::path executableTopDirPath()
{
    return fromUTF8(executableTopDir());
}

const std::string& pluginDir()
{
    if(executableFile_.empty()){
        detectExecutableFile();
    }
    return pluginDir_;
}

stdx::filesystem::path pluginDirPath()
{
    return fromUTF8(pluginDir());
}

const std::string& shareDir()
{
    if(executableFile_.empty()){
        detectExecutableFile();
    }
    return shareDir_;
}

stdx::filesystem::path shareDirPath()
{
    return fromUTF8(shareDir());
}

}
