#include "Resources.h"
#include <filesystem>
#ifdef _MSC_VER
#define filesystem tr2::sys
#endif

#include <cmrc/cmrc.hpp>
#include <SDL.h>
CMRC_DECLARE(resources);

FILE *Resources::fopen(const char * filename, const char *mode) {
    return ::fopen(locateFile(filename).c_str(), mode);
}

std::string Resources::locateFile(const std::string &path) {
    // File exists in working directory, use that
    {
        if (std::filesystem::exists(std::filesystem::path(path)))
            return path;
    }
    std::filesystem::path module_dir_path = std::filesystem::path(getModuleDir());
    module_dir_path += path;
    // See if file exists in module dir, use that
    {
        if (std::filesystem::exists(module_dir_path)) {
            return path;
        } else {
            // file doesn't exist in module dir, so create it there
            auto fs = cmrc::resources::get_filesystem();
            if (fs.exists(path)) {
                // file exists within internal resources
                auto resource_file = fs.open(path);
                // open a file to module_dir_path
                // we will extract the file to here
                FILE *out_file = ::fopen(module_dir_path.string().c_str(), "wb");
                fwrite(resource_file.begin(), resource_file.size(), 1, out_file);
                fclose(out_file);
                // Reopen the file we just created and return handle to user
                return path;
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
//        std::filesystem::path module_path = std::filesystem::path(out_path);
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
