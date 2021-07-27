# Font config is only required on unix
if(UNIX)
    find_package(Fontconfig)
    if (NOT Fontconfig_FOUND)
        message(FATAL_ERROR "Fontconfig is required for visualisation, install it via your package manager.\n"
                    "e.g. sudo apt install libfontconfig1-dev")
    endif()    
endif()
