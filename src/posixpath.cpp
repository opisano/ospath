#include "ospath.hpp"

#include <algorithm>
#include <cstring>
#include <cerrno>
#include <climits>
#include <numeric>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "os.hpp"

namespace
{
    std::vector<std::string> splitString(const std::string& text, char delim)
    {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(text);
        while (std::getline(tokenStream, token, delim))
        {
            tokens.push_back(token);
        }
        return tokens;
    }

    bool startsWith(const std::string& haystack, const std::string& needle)
    {
        return (haystack.rfind(needle, 0) == 0);
    }

    template<typename InIt>
    std::string joinString(char sep, InIt first, InIt end)
    {
        std::string result;
        if (first == end)
        {
            return result;
        }

        // reserve memory to avoid multiple reallocs
        size_t needed = std::accumulate(first, end, 0, 
                [](size_t a, const std::string& b)
                {
                    return a + 1 + b.size();
                });
        result.reserve(needed);

        auto it = first;
        for (; it != end-1; ++it)
        {
            result.append(*it);
            result.push_back(sep);
        }
        result.append(*it);
        return result;
    }
}


namespace os::path
{

std::string absPath(const std::string& path)
{
    if (!isAbs(path))
    {
        auto cwd = os::getcwd();
        return normPath(join(cwd, path));
    }
    else
    {
        return normPath(path);
    }
}

std::string normPath(const std::string& path)
{
    if (path.empty())
        return ".";

    int initialSlashes = (path[0] == SEP);
    if (initialSlashes &&
            startsWith(path, "//") && !startsWith(path, "///"))
    {
        initialSlashes = 2;
    }

    std::vector<std::string> comps = splitString(path, SEP);
    comps.erase(std::remove_if(std::begin(comps), std::end(comps), 
                               [](const std::string& s)
                               {
                                   return s.empty() || s == ".";
                               }),
                std::end(comps));
    
    std::vector<std::string> newComps;
    for (const auto& comp : comps)
    {
        if (comp != ".." || (initialSlashes == 0 && newComps.empty()) 
                || (!newComps.empty() && newComps.back() == ".."))
            newComps.push_back(comp);
        else if (!newComps.empty())
        {
            newComps.pop_back();
        }
    }

    comps = std::move(newComps);
    auto result = joinString(SEP, std::cbegin(comps), std::cend(comps));
    if (initialSlashes)
        result = std::string(initialSlashes, SEP) + result;
    return result.empty() ? "." : result;
}

std::string baseName(const std::string& pathname)
{
    size_t index = pathname.rfind(SEP);
    if (index == std::string::npos)
        return pathname;
    return pathname.substr(index+1);
}

std::string commonPrefix(const std::vector<std::string>& paths)
{
    if (paths.empty())
        return "";
    
    auto pair = std::minmax_element(std::cbegin(paths), std::cend(paths));
    auto& s1 = pair.first;
    auto& s2 = pair.second;
    auto diffpair = std::mismatch(std::cbegin(*s1), std::cend(*s1), std::cbegin(*s2));
    return std::string(std::cbegin(*s1), diffpair.first);
}

std::string commonPath(const std::vector<std::string>& paths)
{
    if (paths.empty())
    {
        return "";
    }

    // check that we have either relative or absolute paths, but not a mix 
    // of each
    if (!std::equal(std::cbegin(paths) + 1, std::cend(paths), 
                    std::cbegin(paths), 
                    [](const std::string& a, const std::string& b) { 
                        return isAbs(a) == isAbs(b); 
                    }))
    {
        return "";
    }
        
    // split each path by the path separator 
    std::vector<std::vector<std::string>> split_paths;
    std::transform(std::cbegin(paths), std::cend(paths),
                   std::back_inserter(split_paths), 
                   [](const std::string& s) { return splitString(s, SEP); });
    
    // filter out empty paths or '.' 
    for (auto& s : split_paths)
    {
        s.erase(std::remove_if(std::begin(s), std::end(s), 
                               [](const std::string& c)
                               {
                                   return c.empty() || c == ".";
                               }),
                std::end(s));
    }
    
    auto pair = std::minmax_element(std::cbegin(split_paths), std::cend(split_paths));
    auto& s1 = pair.first;
    auto& s2 = pair.second;
    auto diffpair = std::mismatch(std::cbegin(*s1), std::cend(*s1), std::cbegin(*s2));
    std::string prefix = isAbs(paths[0]) ? "/" : "";
    std::string result = joinString(SEP, std::cbegin(*s1), diffpair.first);
    return prefix + result;
}

std::string dirName(const std::string& pathname)
{
    size_t index = pathname.rfind(SEP) + 1;
    auto head = pathname.substr(0, index);
    if (!head.empty() && head != std::string(head.size(), SEP))
        while (head.back() == SEP)
            head.pop_back();
    return head;
}

bool exists(const std::string& pathname)
{
    struct stat s;
    int error = stat(pathname.c_str(), &s);
    return error == 0;
}

bool lexists(const std::string& pathname)
{
    struct stat s;
    int error = lstat(pathname.c_str(), &s);
    return error == 0;
}

std::string expandUser(const std::string& path)
{
    const std::string tilde = "~";
    std::string userhome;

    if (!startsWith(path, tilde))
        return path;
    auto i = path.find(SEP, 1);
    if (i == std::string::npos)
        i = path.size();
    
    if (i == 1) // if path starts with "~/" -> find current user home dir
    {
        userhome = os::getHomeDirectory();
    }
    else // find some other user home dir
    {
        auto name = path.substr(1, i - 1);
        userhome = os::getHomeDirectory(name);
    }

    if (!userhome.empty() &&userhome.back() == SEP)
        userhome.pop_back();
    return userhome + path.substr(i);
}

bool isAbs(const std::string& pathname)
{
    return !pathname.empty() && pathname[0] == SEP;
}

bool isFile(const std::string& pathname)
{
    struct stat s;
    int error = stat(pathname.c_str(), &s);
    if (error)
        return false;
    return S_ISREG(s.st_mode) != 0;
}

bool isDir(const std::string& pathname)
{
    struct stat s;
    int error = stat(pathname.c_str(), &s);
    if (error)
        return false;
    return S_ISDIR(s.st_mode) != 0;
}

bool isLink(const std::string& pathname)
{
    struct stat s;
    int error = stat(pathname.c_str(), &s);
    if (error)
        return false;
    return S_ISLNK(s.st_mode) != 0;
}

bool isMount(const std::string& pathname)
{
    struct stat s1;
    int error = stat(pathname.c_str(), &s1);
    if (error)
        return false;
    if (S_ISLNK(s1.st_mode))
        return false;
    
    auto parent = realPath(join(pathname, ".."));
    struct stat s2;
    error = stat(parent.c_str(), &s2);
    if (error)
        return false;

    return s1.st_dev != s2.st_dev || s1.st_ino != s2.st_ino;
}

std::string realPath(const std::string& path)
{
    std::vector<char> buffer;
    buffer.resize(PATH_MAX);
    char* result = realpath(path.c_str(), buffer.data());
    if (!result)
    {
        return "";
    }
    return std::string(buffer.data());
}

std::pair<std::string, std::string> split(const std::string& pathname)
{
    size_t index = pathname.rfind(SEP) + 1;
    auto head = pathname.substr(0, index);
    auto tail = pathname.substr(index);
    if (!head.empty() && head != std::string(head.size(), SEP))
        while (head.back() == SEP)
            head.pop_back();
    return std::make_pair(head, tail);
}

}