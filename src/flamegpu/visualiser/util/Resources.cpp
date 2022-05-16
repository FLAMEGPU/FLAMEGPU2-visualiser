#include "flamegpu/visualiser/util/Resources.h"
#include "VisException.h"
#include <filesystem>

#include <cmrc/cmrc.hpp>
#include <SDL.h>
CMRC_DECLARE(resources);

namespace flamegpu {
namespace visualiser {

namespace {
    std::filesystem::path getTMP() {
        static std::filesystem::path result;
        if (result.empty()) {
            std::filesystem::path tmp =  std::getenv("FLAMEGPU2_TMP_DIR") ? std::getenv("FLAMEGPU2_TMP_DIR") : std::filesystem::temp_directory_path();
            // Create the $tmp/flamegpu/vis folder hierarchy
            if (!std::filesystem::exists(tmp) && !std::filesystem::create_directories(tmp)) {
                THROW InvalidFilePath("Directory '%s' does not exist and cannot be created by visualisation.", tmp.generic_string().c_str());
            }
            if (!std::getenv("FLAMEGPU2_TMP_DIR")) {
                tmp /= "flamegpu";
                if (!std::filesystem::exists(tmp)) {
                    std::filesystem::create_directories(tmp);
                }
            }
            tmp /= "vis";
            if (!std::filesystem::exists(tmp)) {
                std::filesystem::create_directories(tmp);
            }
            result = tmp;
        }
        return result;
    }
    inline uint64_t hash_larson64(const char* s,
        uint64_t seed = 0) {
        uint64_t hash = seed;
        while (*s) {
            hash = hash * 101 + *s++;
        }
        return hash;
    }
}  // namespace

FILE *Resources::fopen(const char * filename, const char *mode) {
    return ::fopen(locateFile(filename).c_str(), mode);
}

std::string Resources::locateFile(const std::string &_path) {
    // Convert the path into '<hash(_path)>_filename.file_ext'
    const std::filesystem::path t_filename = std::filesystem::path(std::to_string(hash_larson64(_path.c_str())).append("_").append(std::filesystem::path(_path).filename().generic_string()));
    // File exists in working directory, use that
    {
        if (std::filesystem::exists(std::filesystem::path(_path)))
            return _path;
    }
    std::filesystem::path temp_dir_path = std::filesystem::path(getTMP());
    temp_dir_path /= t_filename;
    // See if file exists in temp dir, use that
    {
        if (std::filesystem::exists(temp_dir_path)) {
            return temp_dir_path.string();
        } else {
            // file doesn't exist in temp dir, so create it there
            const auto fs = cmrc::resources::get_filesystem();
            if (fs.exists(_path)) {
                // file exists within internal resources
                const auto resource_file = fs.open(_path);
                // open a file to temp_dir_path
                // Check the output directory exists
                std::filesystem::path output_dir = temp_dir_path;
                output_dir.remove_filename();
                std::filesystem::create_directories(output_dir);
                // we will extract the file to here
                FILE *out_file = ::fopen(temp_dir_path.string().c_str(), "wb");
                fwrite(resource_file.begin(), resource_file.size(), 1, out_file);
                fclose(out_file);
                // Reopen the file we just created and return handle to user
                return temp_dir_path.string();
            } else {
                // Unable to locate file
                THROW ResourceError("Resources::locateFile(): File '%s' could not be found!", _path.c_str());
            }
        }
    }
}

bool Resources::exists(const std::string &path) {
    const auto fs = cmrc::resources::get_filesystem();
    return fs.exists(path);
}

std::string Resources::toTempDir(const std::string &_path) {
    // Convert the path into '<hash(_path)>_filename.file_ext'
    const std::filesystem::path t_filename = std::filesystem::path(std::to_string(hash_larson64(_path.c_str())).append("_").append(std::filesystem::path(_path).filename().generic_string()));
    std::filesystem::path output_dir = getTMP();
    output_dir /= t_filename;
    const std::filesystem::path output_path = output_dir;
    output_dir.remove_filename();
    std::filesystem::create_directories(output_dir);
    return output_path.string();
}

}  // namespace visualiser
}  // namespace flamegpu
