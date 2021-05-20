#include "os.hpp"

#include <pwd.h>
#include <sys/types.h>
#include <vector>
#include <unistd.h>

namespace os
{

std::string getcwd()
{
    std::vector<char> buffer;
    buffer.resize(128);
    auto result = ::getcwd(buffer.data(), buffer.size());
    while (result == nullptr)
    {
        buffer.resize(static_cast<size_t>(buffer.size() * 1.5));
        result = ::getcwd(buffer.data(), buffer.size());
    }

    return std::string (buffer.data());
}

std::string getHomeDirectory()
{
    std::string userhome;
    char* sEnv = getenv("HOME");

    if (sEnv == nullptr)
    {
        passwd pwd;
        passwd* ppwd;
        std::vector<char> buffer;
        buffer.resize(512);

        int error = getpwuid_r(getuid(), &pwd, buffer.data(), buffer.size(), &ppwd);
        while (error == ERANGE)
        {
            buffer.resize(static_cast<size_t>(buffer.size() * 1.5));
            int error = getpwuid_r(getuid(), &pwd, buffer.data(), buffer.size(), &ppwd);
        }

        if (error || ppwd == nullptr)
            return "";
        
        userhome = pwd.pw_dir;
    }
    else
    {
        userhome = sEnv;
    }

    return userhome;
}

std::string getHomeDirectory(const std::string& username)
{
    if (username.empty())
        return getHomeDirectory();

    std::string userhome;
    passwd pwd;
    passwd* ppwd;
    std::vector<char> buffer;
    buffer.resize(512);

    int error = getpwnam_r(username.c_str(), &pwd, buffer.data(), buffer.size(), &ppwd);
    while (error == ERANGE)
    {
        buffer.resize(static_cast<size_t>(buffer.size() * 1.5));
        int error = getpwnam_r(username.c_str(), &pwd, buffer.data(), buffer.size(), &ppwd);
    }
    if (error || ppwd == nullptr)
            return "";
        
    userhome = pwd.pw_dir;
    return userhome;
}

}