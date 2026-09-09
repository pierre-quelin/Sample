cmake_minimum_required(VERSION 3.25)

project(sample VERSION 1.0.0)

include(${CMAKE_SOURCE_DIR}/tools/SampleDist.cmake)

if(NOT CMAKE_CONFIGURATION_TYPES)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Debug, Release, RelWithDebInfo, MinSizeRel" FORCE)
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/lib/Foundation/CMakeLists.txt")
    message(FATAL_ERROR "lib/Foundation missing — run Build.bat fetch")
endif()

add_subdirectory(lib/Foundation)

set(MAIN_SRCS
    src/main.cpp
)
set(_factory_link_cpp "${CMAKE_CURRENT_BINARY_DIR}/generated/FactoryLink.cpp")
list(APPEND MAIN_SRCS "${_factory_link_cpp}")

set(_link_scan_srcs)
if(TARGET foundation)
    get_target_property(_found_srcs foundation SOURCES)
    if(_found_srcs)
        list(APPEND _link_scan_srcs ${_found_srcs})
    endif()
endif()

include(${CMAKE_SOURCE_DIR}/lib/Foundation/tools/GenerateFactoryLink.cmake)
foundation_generate_factory_link(
    OUTPUT "${_factory_link_cpp}"
    SOURCES ${_link_scan_srcs}
)

if ($ENV{BUILD_TARGET} MATCHES "msvc[0-9]+")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    add_definitions(-D_CRTDBG_MAP_ALLOC -D_BIND_TO_CURRENT_CRT_VERSION -DNOMINMAX -DNOGDI)
endif()

add_executable(sample ${MAIN_SRCS})
set(sample_GUID_CMAKE "8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942" CACHE INTERNAL "Stored GUID")

if ($ENV{BUILD_TARGET} MATCHES "msvc[0-9]+")
    if(NOT (MSVC_VERSION LESS 1910))
        target_compile_options(sample PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/std:c++17 /permissive- /Zc:__cplusplus /EHsc /W4>
        )
    endif()
endif()

include(${CMAKE_SOURCE_DIR}/lib/Foundation/tools/FoundationLib.cmake)
foundation_link_application(sample)

if ($ENV{BUILD_TARGET} MATCHES "msvc[0-9]+" OR $ENV{BUILD_TARGET} STREQUAL "mingw64")
    target_link_libraries(sample PRIVATE ws2_32)
endif()

if ($ENV{BUILD_TARGET} MATCHES "msvc[0-9]+")
    set(SEARCH_DIRS "")
    if(TARGET libusb::libusb)
        get_target_property(_LIBUSB_DLL libusb::libusb IMPORTED_LOCATION_RELEASE)
        if(NOT _LIBUSB_DLL OR _LIBUSB_DLL MATCHES "NOTFOUND")
            get_target_property(_LIBUSB_DLL libusb::libusb IMPORTED_LOCATION)
        endif()
        if(_LIBUSB_DLL AND NOT _LIBUSB_DLL MATCHES "NOTFOUND")
            get_filename_component(_libusb_dll_dir "${_LIBUSB_DLL}" DIRECTORY)
            list(APPEND SEARCH_DIRS "${_libusb_dll_dir}")
        endif()
    endif()
    set(COPY_DEPS_DIRS_FILE "${CMAKE_CURRENT_BINARY_DIR}/copy_deps_dirs.txt")
    file(WRITE "${COPY_DEPS_DIRS_FILE}" "${SEARCH_DIRS}")
    add_custom_command(TARGET sample POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            -D TARGET_FILE=$<TARGET_FILE:sample>
            -D TARGET_DIR=$<TARGET_FILE_DIR:sample>
            -D SEARCH_DIRS_FILE="${COPY_DEPS_DIRS_FILE}"
            -P ${CMAKE_SOURCE_DIR}/lib/Foundation/tools/copy_deps.cmake)
endif()

# Same binary, two deploy folders — same main.json, different main.ini (platformName).
function(sample_install_variant _subdir)
    install(TARGETS sample
            RUNTIME DESTINATION ${SAMPLE_DIST_ROOT}/${_subdir})
    install(FILES ${CMAKE_SOURCE_DIR}/cfg/${_subdir}/main.ini
            DESTINATION ${SAMPLE_DIST_ROOT}/${_subdir}
            COMPONENT CONFIGS)
    install(FILES ${CMAKE_SOURCE_DIR}/cfg/main.json
            DESTINATION ${SAMPLE_DIST_ROOT}/${_subdir}/cfg
            COMPONENT CONFIGS)
    if(TARGET libusb::libusb)
        get_target_property(_LIBUSB_RUNTIME_DLL libusb::libusb IMPORTED_LOCATION_RELEASE)
        if(NOT _LIBUSB_RUNTIME_DLL OR _LIBUSB_RUNTIME_DLL MATCHES "NOTFOUND")
            get_target_property(_LIBUSB_RUNTIME_DLL libusb::libusb IMPORTED_LOCATION)
        endif()
        if(_LIBUSB_RUNTIME_DLL AND NOT _LIBUSB_RUNTIME_DLL MATCHES "NOTFOUND" AND EXISTS "${_LIBUSB_RUNTIME_DLL}")
            install(FILES "${_LIBUSB_RUNTIME_DLL}"
                    DESTINATION ${SAMPLE_DIST_ROOT}/${_subdir}
                    COMPONENT LIBRARIES)
        endif()
    endif()
endfunction()

sample_install_variant(local)
sample_install_variant(remote)
