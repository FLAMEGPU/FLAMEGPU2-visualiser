include_guard(GLOBAL)
# Define a function which can be used to set common compiler options for a target
# We do not want to force these options on end users (although they should be used ideally), hence not just public properties on the library target
# Function to suppress compiler warnings for a given target
function(flamegpu_visualiser_common_compiler_settings)
    # Parse the expected arguments, prefixing variables.
    cmake_parse_arguments(
        CCS
        ""
        "TARGET"
        ""
        ${ARGN}
    )

    # Ensure that a target has been passed, and that it is a valid target.
    if(NOT CCS_TARGET)
        message( FATAL_ERROR "flamegpu_visualiser_common_compiler_settings: 'TARGET' argument required")
    elseif(NOT TARGET ${CCS_TARGET} )
        message( FATAL_ERROR "flamegpu_visualiser_common_compiler_settings: TARGET '${CCS_TARGET}' is not a valid target")
    endif()

    # Add device debugging symbols to device builds of CUDA objects
    target_compile_options(${CCS_TARGET} PRIVATE "$<$<AND:$<COMPILE_LANGUAGE:CUDA>,$<CONFIG:Debug>>:-G>")
    # Ensure DEBUG and _DEBUG are defined for Debug builds
    target_compile_definitions(${CCS_TARGET} PRIVATE $<$<AND:$<COMPILE_LANGUAGE:CUDA>,$<CONFIG:Debug>>:DEBUG>)
    target_compile_definitions(${CCS_TARGET} PRIVATE $<$<AND:$<COMPILE_LANGUAGE:CUDA>,$<CONFIG:Debug>>:_DEBUG>)
    # Enable -lineinfo for Release builds, for improved profiling output.
    target_compile_options(${CCS_TARGET} PRIVATE "$<$<AND:$<COMPILE_LANGUAGE:CUDA>,$<CONFIG:Release>>:-lineinfo>")

    # Set an NVCC flag which allows host constexpr to be used on the device.
    target_compile_options(${CCS_TARGET} PRIVATE "$<$<COMPILE_LANGUAGE:CUDA>:--expt-relaxed-constexpr>")

    # Prevent windows.h from defining max and min.
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_definitions(${CCS_TARGET} PRIVATE NOMINMAX)
    endif()

    # MSVC handling of SYSTEM for external includes.
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.10)
        # These flags don't currently have any effect on how CMake passes system-private includes to msvc (VS 2017+)
        set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "/external:I")
        set(CMAKE_INCLUDE_SYSTEM_FLAG_CUDA "/external:I")
        # VS 2017+
        target_compile_options(${CCS_TARGET} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:/experimental:external>")
    endif()

    # Enable parallel compilation
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${CCS_TARGET} PRIVATE "$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcompiler /MP>")
        target_compile_options(${CCS_TARGET} PRIVATE "$<$<COMPILE_LANGUAGE:C,CXX>:/MP>")
    endif()

    # If CUDA 11.2+, can build multiple architectures in parallel. 
    # Note this will be multiplicative against the number of threads launched for parallel cmake build, which may lead to processes being killed, or excessive memory being consumed.
    if(CMAKE_CUDA_COMPILER_VERSION VERSION_GREATER_EQUAL "11.2" AND USE_NVCC_THREADS AND DEFINED NVCC_THREADS AND NVCC_THREADS GREATER_EQUAL 0)
        target_compile_options(${CCS_TARGET} PRIVATE "$<$<COMPILE_LANGUAGE:CUDA>:SHELL:--threads ${NVCC_THREADS}>")
    endif()

endfunction()