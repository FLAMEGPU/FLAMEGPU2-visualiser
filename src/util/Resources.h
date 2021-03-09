#ifndef SRC_UTIL_RESOURCES_H_
#define SRC_UTIL_RESOURCES_H_
#include <string>
#include <cstdio>


class Resources {
 public:
    /**
     * fopen wrapper, this should be preferred for shaders/models
     * Attempts to load file from module directory or internal resources if path does not exist
     * @note Instead of returning null, throws exception on failure
     */
    static FILE *fopen(const char *filename, const char *mode);
    /**
     * Appends the path to module dir, and creates any missing directories
     */
    static std::string toTempDir(const std::string &path);
    /**
     * Returns the path to a file
     * If the file is only in resources, it will be extracted to module dir
     */
    static std::string locateFile(const std::string &path);
    /**
     * Returns true if path is a valid resource path
     */
    static bool exists(const std::string &path);
};

#endif  // SRC_UTIL_RESOURCES_H_
