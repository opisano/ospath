#include <vector>
#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "ospath.hpp"

TEST_CASE("baseName returns final path component", "[basename]") {
    REQUIRE(os::path::baseName("/foo/bar") == "bar");
    REQUIRE(os::path::baseName("/") == "");
    REQUIRE(os::path::baseName("foo") == "foo");
    REQUIRE(os::path::baseName("////foo") == "foo");
    REQUIRE(os::path::baseName("//foo//bar") == "bar");
}

TEST_CASE("dirName returns directory component", "[dirname]") {
    REQUIRE(os::path::dirName("/foo/bar") == "/foo");
    REQUIRE(os::path::dirName("/") == "/");
    REQUIRE(os::path::dirName("foo") == "");
    REQUIRE(os::path::dirName("////foo") == "////");
    REQUIRE(os::path::dirName("//foo//bar") == "//foo");
}

TEST_CASE("isAbs returns whether a path is absolute", "[isabs]") {
    REQUIRE(os::path::isAbs("") == false);
    REQUIRE(os::path::isAbs("/") == true);
    REQUIRE(os::path::isAbs("/foo") == true);
    REQUIRE(os::path::isAbs("/foo/bar") == true);
    REQUIRE(os::path::isAbs("foo/bar") == false);
}

TEST_CASE("split splits path components", "[split]") {
    REQUIRE(os::path::split("/foo/bar").first == "/foo");
    REQUIRE(os::path::split("/foo/bar").second == "bar");
    REQUIRE(os::path::split("/").first == "/");
    REQUIRE(os::path::split("/").second.empty());
    REQUIRE(os::path::split("foo").first.empty());
    REQUIRE(os::path::split("foo").second == "foo");
    REQUIRE(os::path::split("////foo").first == "////");
    REQUIRE(os::path::split("////foo").second == "foo");
    REQUIRE(os::path::split("//foo//bar").first == "//foo");
    REQUIRE(os::path::split("//foo//bar").second == "bar");
}

TEST_CASE("commonPrefix ", "[commonPrefix]") {
    std::vector<std::string> paths;
    REQUIRE(os::path::commonPrefix(paths) == "");
    paths.push_back("/usr/bin/ls");
    REQUIRE(os::path::commonPrefix(paths) == "/usr/bin/ls");
    paths.push_back("/usr/bin");
    REQUIRE(os::path::commonPrefix(paths) == "/usr/bin");
    paths.push_back("/usr");
    REQUIRE(os::path::commonPrefix(paths) == "/usr");
    paths.push_back("");
    REQUIRE(os::path::commonPrefix(paths) == "");

    std::vector<std::string> ps = { "aaa", "aabbb", "aaaaab" };
    REQUIRE(os::path::commonPrefix(ps) == "aa");
}

TEST_CASE("commonPath", "[commonPath]") {
    std::vector<std::string> paths = { "/usr/local" };
    REQUIRE(os::path::commonPath(paths) == "/usr/local");
    paths = { "/usr/local", "/usr/local" };
    REQUIRE(os::path::commonPath(paths) == "/usr/local");
    paths = { "/usr/local/", "/usr/local" };
    REQUIRE(os::path::commonPath(paths) == "/usr/local");
    paths = { "/usr/local/", "/usr/local/" };
    REQUIRE(os::path::commonPath(paths) == "/usr/local");
    paths = { "/usr//local/", "//usr/local/" };
    REQUIRE(os::path::commonPath(paths) == "/usr/local");
    paths = { "/usr/./local/", "/./usr/local/" };
    REQUIRE(os::path::commonPath(paths) == "/usr/local");
    paths = { "/", "/dev" };
    REQUIRE(os::path::commonPath(paths) == "/");
    paths = { "/usr", "/dev" };
    REQUIRE(os::path::commonPath(paths) == "/");
    paths = { "/usr/lib", "/usr/lib/python3" };
    REQUIRE(os::path::commonPath(paths) == "/usr/lib");
    paths = { "/usr/lib/", "/usr/lib64/" };
    REQUIRE(os::path::commonPath(paths) == "/usr");
    paths = { "/usr/lib", "/usr/lib64/" };
    REQUIRE(os::path::commonPath(paths) == "/usr");
    paths = { "/usr/lib/", "/usr/lib64" };
    REQUIRE(os::path::commonPath(paths) == "/usr");
    paths = { "spam"};
    REQUIRE(os::path::commonPath(paths) == "spam");
    paths = { "spam"};
    REQUIRE(os::path::commonPath(paths) == "spam");
    paths = { "spam", "spam"};
    REQUIRE(os::path::commonPath(paths) == "spam");
    paths = { "spam", "alot"};
    REQUIRE(os::path::commonPath(paths) == "");
    paths = { "and/jam", "and/spam"};
    REQUIRE(os::path::commonPath(paths) == "and");
    paths = { "and//jam", "and/spam//"};
    REQUIRE(os::path::commonPath(paths) == "and");
    paths = { "and/./jam", "./and/spam"};
    REQUIRE(os::path::commonPath(paths) == "and");
    paths = { "and/jam", "/and/spam", "alot"};
    REQUIRE(os::path::commonPath(paths) == "");
    paths = { "and/jam", "and/spam", "and"};
    REQUIRE(os::path::commonPath(paths) == "and");
    paths.clear();
    REQUIRE(os::path::commonPath(paths) == "");
}

TEST_CASE("join to join path elements", "[join]") {
    REQUIRE(os::path::join("/foo", "bar", "/bar", "baz") == "/bar/baz");
    REQUIRE(os::path::join("/foo", "bar", "baz") == "/foo/bar/baz");
    REQUIRE(os::path::join("/foo/", "bar/", "baz/") == "/foo/bar/baz/");
}

TEST_CASE("expandUser for current user", "[expandUser]") {
    REQUIRE(os::path::normPath(os::path::expandUser("~/..")) == "/home");
    REQUIRE(os::path::expandUser("~root/Documents") == "/root/Documents");
}
