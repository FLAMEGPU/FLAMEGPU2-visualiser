#include "util/Resources.h"
#include "VisException.h"
// If earlier than VS 2019
#if defined(_MSC_VER) && _MSC_VER < 1920
#include <filesystem>
using std::tr2::sys::temp_directory_path;
using std::tr2::sys::exists;
using std::tr2::sys::path;
using std::tr2::sys::create_directory;
#else
// VS2019 requires this macro, as building pre c++17 cant use std::filesystem
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
using std::experimental::filesystem::v1::temp_directory_path;
using std::experimental::filesystem::v1::exists;
using std::experimental::filesystem::v1::path;
using std::experimental::filesystem::v1::create_directory;
#endif

#include <cmrc/cmrc.hpp>
#include <SDL.h>
CMRC_DECLARE(resources);

namespace {
    void recursive_create_dir(const path &dir) {
        if (::exists(dir)) {
            return;
        }
        path parent_dir = dir.parent_path();
        if (!::exists(parent_dir)) {
            recursive_create_dir(parent_dir);
        }
        create_directory(dir);
    }
    path getTMP() {
        static path result;
        if (result.empty()) {
            path tmp =  std::getenv("FLAMEGPU2_TMP_DIR") ? std::getenv("FLAMEGPU2_TMP_DIR") : temp_directory_path();
            // Create the $tmp/fgpu2/vis folder hierarchy
            if (!::exists(tmp) && !create_directory(tmp)) {
                THROW InvalidFilePath("Directory '%s' does not exist and cannot be created by visualisation.", tmp.generic_string().c_str());
            }
            if (!std::getenv("FLAMEGPU2_TMP_DIR")) {
                tmp /= "fgpu2";
                if (!::exists(tmp)) {
                    create_directory(tmp);
                }
            }
            tmp /= "vis";
            if (!::exists(tmp)) {
                create_directory(tmp);
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
    const path t_filename = path(std::to_string(hash_larson64(_path.c_str())).append("_").append(path(_path).filename().generic_string()));
    // File exists in working directory, use that
    {
        if (::exists(path(_path)))
            return _path;
    }
    path temp_dir_path = path(getTMP());
    temp_dir_path /= t_filename;
    // See if file exists in temp dir, use that
    {
        if (::exists(temp_dir_path)) {
            return temp_dir_path.string();
        } else {
            // file doesn't exist in temp dir, so create it there
            const auto fs = cmrc::resources::get_filesystem();
            if (fs.exists(_path)) {
                // file exists within internal resources
                const auto resource_file = fs.open(_path);
                // open a file to temp_dir_path
                // Check the output directory exists
                path output_dir = temp_dir_path;
                output_dir.remove_filename();
                recursive_create_dir(output_dir);
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
    const path t_filename = path(std::to_string(hash_larson64(_path.c_str())).append("_").append(path(_path).filename().generic_string()));
    path output_dir = getTMP();
    output_dir /= t_filename;
    const path output_path = output_dir;
    output_dir.remove_filename();
    recursive_create_dir(output_dir);
    return output_path.string();
}
