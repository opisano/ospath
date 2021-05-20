#pragma once


#include <exception>
#include <string>
#include <utility>
#include <vector>

namespace os::path
{

constexpr char SEP = '/';

/**
 * Return a normalized absolutized version of the pathname path.
 */
std::string absPath(const std::string& path);

/**
 * Normalize a pathname by collapsing redundant separators and up-level
 * references so that A//B, A/B/, A/./B and A/foo/../B all become A/B. 
 * This string manipulation may change the meaning of a path that 
 * contains symbolic links.
 */
std::string normPath(const std::string& path);

/**
 * Return the base name of pathname path. This is the second element of
 * the pair returned by passing path to the function split(). Note that 
 * the result of this function is different from the Unix basename program; 
 * where basename for '/foo/bar/' returns 'bar', the basename() function 
 * returns an empty string ("").
 */
std::string baseName(const std::string& pathname);

/**
 * Return the longest common sub-path of each pathname in the sequence paths. 
 * Return empty string if paths contain both absolute and relative pathnames, 
 * the paths are on the different drives or if paths is empty. Unlike 
 * commonprefix(), this returns a valid path.
 */
std::string commonPath(const std::vector<std::string>& paths);

/**
 * Return the longest path prefix (taken character-by-character) that is a 
 * prefix of all paths in list. If list is empty, return the empty string ('').
 *
 * Note:
 *     This function may return invalid paths because it works a character at a 
 *     time. To obtain a valid path, see commonpath().
 */
std::string commonPrefix(const std::vector<std::string>& paths);

/**
 * Return the directory name of pathname path. This is the first element of the
 * pair returned by passing path to the function split().
 */
std::string dirName(const std::string& pathname);

/**
 * Returns true if a path exists, false if the path does not exist or if is a 
 * broken symbolic link. 
 */
bool exists(const std::string& pathname);

/**
 * Test whether a path exists.  Returns true for broken symbolic links.
 */
bool lexists(const std::string& pathname);

/**
 * Return the argument with an initial component of ~ or ~user replaced by that
 * user’s home directory.
 * 
 * On Unix, an initial ~ is replaced by the environment variable HOME if it is
 * set; otherwise the current user’s home directory is looked up in the password 
 * directory. An initial ~user is looked up directly in the password directory.
 */
std::string expandUser(const std::string& path);

/**
 * Return whether a path is absolute.
 */
bool isAbs(const std::string& pathname);

/**
 * Return true if path is an existing regular file. This follows symbolic 
 * links, so both isLink() and isFile() can be true for the same path.
 */
bool isFile(const std::string& pathname);

/**
 * Return True if path is an existing directory. This follows symbolic 
 * links, so both isLink() and isDir() can be true for the same path.
 */
bool isDir(const std::string& pathname);

/**
 * Return whether a path is a symbolic link.
 */
bool isLink(const std::string& pathname);

/**
 * Return true if pathname path is a mount point: a point in a file system 
 * where a different file system has been mounted. On POSIX, the function checks
 * whether path’s parent, path/.., is on a different device than path, or 
 * whether path/.. and path point to the same i-node on the same device — this 
 * should detect mount points for all Unix and POSIX variants. It is not able to
 * reliably detect bind mounts on the same filesystem. 
 */
bool isMount(const std::string& pathname);


template <typename ... Args>
std::string join(std::string& a, const std::string& b, Args ... args)
{
    if (isAbs(b))
    {
        a = b;
    }
    else 
    {
        if (a.size() && a.back() != SEP)
            a.push_back(SEP);
        a.append(b);
    }
    
    if constexpr(sizeof...(Args) > 0)
        return join(a, args...);
    else
        return a;
}

/**
 * Join two or more path components intelligently. The return value is the
 * concatenation of path and any members with exactly one directory 
 * separator following each non-empty part except the last, meaning 
 * that the result will only end in a separator if the last part is empty. 
 * If a component is an absolute path, all previous components are thrown 
 * away and joining continues from the absolute path component.
 */
template <typename ... Args>
std::string join(const std::string& a, const std::string& b, Args&& ... args)
{
    std::string result;
    return join(result, a, b, std::forward<Args>(args) ...);
}

/**
 * Return the canonical path of the specified filename, eliminating any 
 * symbolic links encountered in the path
 */
std::string realPath(const std::string& path);


/**
 * Split the pathname path into a pair, (head, tail) where tail is 
 * the last pathname component and head is everything leading up to that. 
 */
std::pair<std::string, std::string> split(const std::string& pathname);


}