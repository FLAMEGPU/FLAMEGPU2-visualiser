#include "Resources.h"
// If earlier than VS 2019
#if defined(_MSC_VER) && _MSC_VER < 1920
#include <filesystem>
using std::tr2::sys::exists;
using std::tr2::sys::path;
using std::tr2::sys::create_directory;
#else
// VS2019 requires this macro, as building pre c++17 cant use std::filesystem
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
using std::experimental::filesystem::v1::exists;
using std::experimental::filesystem::v1::path;
using std::experimental::filesystem::v1::create_directory;
#endif

#include <cmrc/cmrc.hpp>
#include <SDL.h>
CMRC_DECLARE(resources);

namespace {
    void recursive_create_dir(const path &dir) {
        if (exists(dir)) {
            return;
        }
        path parent_dir = dir.parent_path();
        if (!exists(parent_dir)) {
            recursive_create_dir(parent_dir);
        }
        create_directory(dir);
    }
}  // namespace

FILE *Resources::fopen(const char * filename, const char *mode) {
    return ::fopen(locateFile(filename).c_str(), mode);
}

std::string Resources::locateFile(const std::string &_path) {
    // File exists in working directory, use that
    {
        if (exists(path(_path)))
            return _path;
    }
    path module_dir_path = path(getModuleDir());
    module_dir_path += _path;
    // See if file exists in module dir, use that
    {
        if (exists(module_dir_path)) {
            return module_dir_path.string();
        } else {
            // file doesn't exist in module dir, so create it there
            auto fs = cmrc::resources::get_filesystem();
            if (fs.exists(_path)) {
                // file exists within internal resources
                auto resource_file = fs.open(_path);
                // open a file to module_dir_path
                // Check the output directory exists
                path output_dir = module_dir_path;
                output_dir.remove_filename();
                recursive_create_dir(output_dir);
                // we will extract the file to here
                FILE *out_file = ::fopen(module_dir_path.string().c_str(), "wb");
                fwrite(resource_file.begin(), resource_file.size(), 1, out_file);
                fclose(out_file);
                // Reopen the file we just created and return handle to user
                return module_dir_path.string();
            } else {
                // Unable to locate file
                throw std::runtime_error("Unable to open file!");  // TODO: Replace with FGPU exception
            }
        }
    }
}

std::string Resources::getModuleDir() {
    char *t = SDL_GetBasePath();
    std::string rtn = t;
    SDL_free(t);
    return rtn;
// #ifdef _MSC_VER
//    // #include <windows.h> is required
//    // When NULL is passed to GetModuleHandle, the handle of the exe itself is returned
//    HMODULE hModule = GetModuleHandle(nullptr);
//    if (hModule) {
//        char out_path[MAX_PATH];
//        GetModuleFileName(hModule, out_path, sizeof(out_path));
//        std::experimental::filesystem::path module_path = std::experimental::filesystem::path(out_path);
//        module_path.remove_filename();
//        return module_path.string();
//    } else {
//        fprintf(stderr, "Unable to locate module handle!\n");
//        return std::string(".");  // sensible default, reason why it would fail is unclear
//    }
// #else
//
// #endif
}

std::string Resources::toModuleDir(const std::string &_path) {
    path output_dir = getModuleDir();
    output_dir += _path;
    const path output_path = output_dir;
    output_dir.remove_filename();
    recursive_create_dir(output_dir);
    return output_path.string();
}
