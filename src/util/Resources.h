#ifndef SRC_UTIL_RESOURCES_H_
#define SRC_UTIL_RESOURCES_H_
#include <string>
#include <cstdio>


class Resources {
 public:
    /**
     * fopen wrapper, this should be preferred for shaders/models
     * Attempts to load file from module directory or internal resources if path does not exist
     */
    static FILE *fopen(const char *filename, const char *mode);
    /**
     * Apppends the path to module dir, and creates any missing directories
     */
    static std::string toModuleDir(const std::string &path);

 private:
    static std::string getModuleDir();
    static std::string Resources::locateFile(const std::string &path);
};

#endif  // SRC_UTIL_RESOURCES_H_
