#pragma once

#include <string>

namespace os
{

/**
 * Return current working directory.
 */
std::string getcwd();

/**
 * Return current user home directory.
 */
std::string getHomeDirectory();

/**
 * Return the home directory of the user passed as parameter.
 */
std::string getHomeDirectory(const std::string& username);

}