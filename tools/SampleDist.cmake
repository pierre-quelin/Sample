# dist/<target>/ — same layout as Foundation/Sample convention
if(DEFINED ENV{BUILD_TARGET} AND NOT "$ENV{BUILD_TARGET}" STREQUAL "")
    set(SAMPLE_DIST_TARGET "$ENV{BUILD_TARGET}")
elseif(MSVC)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SAMPLE_DIST_TARGET "msvc${MSVC_VERSION}-x86_64")
    else()
        set(SAMPLE_DIST_TARGET "msvc${MSVC_VERSION}-x86")
    endif()
elseif(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(SAMPLE_DIST_TARGET "mingw64")
else()
    set(SAMPLE_DIST_TARGET "unknown")
endif()

set(SAMPLE_DIST_ROOT "${CMAKE_SOURCE_DIR}/dist/${SAMPLE_DIST_TARGET}")
# Nested Foundation may install relative to FOUNDATION_SRC_ROOT; app install uses SAMPLE_DIST_ROOT.
set(FOUNDATION_DIST_ROOT "${SAMPLE_DIST_ROOT}")
message(STATUS "SAMPLE_DIST_ROOT: ${SAMPLE_DIST_ROOT}")
