cmake_minimum_required(VERSION 3.15)

set(KOI_VERSION 1.02)
set(KOI_SOVERSION 1)

project(koi 
        VERSION ${KOI_VERSION}
        LANGUAGES C 
        DESCRIPTION "stb-like single-file, public domain (or MIT-licensed) image processing libraries for C/C++" 
        HOMEPAGE_URL https://github.com/Muppetsg2/koi
)

# Configuration Options
option(KOI_ENABLE_IMAGE "Enable koi_image.h" ON)
option(KOI_ENABLE_IMAGE_WRITE "Enable koi_image_write.h" ON)

set(LIBRARY_NAME ${PROJECT_NAME})

set(KOI_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/koi.c
)

if(KOI_ENABLE_IMAGE)
    list(APPEND KOI_SOURCES 
        ${CMAKE_CURRENT_SOURCE_DIR}/koi_image.h
    )
endif()

if(KOI_ENABLE_IMAGE_WRITE)
    list(APPEND KOI_SOURCES 
        ${CMAKE_CURRENT_SOURCE_DIR}/koi_image_write.h
    )
endif()

# Install destinations
include(GNUInstallDirs)

set(KOI_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}/cmake)
set(KOI_DOC_DIR ${CMAKE_INSTALL_DOCDIR})
set(KOI_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR})
set(KOI_LIBRARY_DIR ${CMAKE_INSTALL_LIBDIR})
set(KOI_PKGCONFIG_DIR ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
set(KOI_RUNTIME_DIR ${CMAKE_INSTALL_BINDIR})

add_library(${LIBRARY_NAME} ${KOI_SOURCES})
add_library(${PROJECT_NAME}::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

# Include headers
target_include_directories(${LIBRARY_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${KOI_INCLUDE_DIR}>
)

# Compile definitions
if(KOI_ENABLE_IMAGE)
    target_compile_definitions(${LIBRARY_NAME} PUBLIC KOI_ENABLE_IMAGE)
endif()

if(KOI_ENABLE_IMAGE_WRITE)
    target_compile_definitions(${LIBRARY_NAME} PUBLIC KOI_ENABLE_IMAGE_WRITE)
endif()

if(BUILD_SHARED_LIBS)
    set_target_properties(${LIBRARY_NAME} PROPERTIES
        SOVERSION ${KOI_SOVERSION}
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endif()

set_target_properties(${LIBRARY_NAME} PROPERTIES VERSION ${KOI_VERSION})

# Write CMake package config files
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${PROJECT_NAME}-config.cmake.in
    ${PROJECT_NAME}-config.cmake
    INSTALL_DESTINATION ${KOI_CMAKE_DIR}
    PATH_VARS KOI_INCLUDE_DIR KOI_LIBRARY_DIR
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
    ${PROJECT_NAME}-config-version.cmake
    VERSION ${KOI_VERSION}
    COMPATIBILITY SameMajorVersion
)

# pkg-config file
configure_file(${PROJECT_NAME}.pc.in ${LIBRARY_NAME}.pc @ONLY)

# Installation
install(TARGETS
    ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    DESTINATION ${KOI_LIBRARY_DIR}
    PUBLIC_HEADER DESTINATION ${KOI_INCLUDE_DIR}
    RUNTIME DESTINATION ${KOI_RUNTIME_DIR}
)

install(EXPORT
    ${PROJECT_NAME}-targets
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${KOI_CMAKE_DIR}
)

install(FILES
    koi_image.h
    koi_image_write.h
    DESTINATION ${KOI_INCLUDE_DIR}
)

install(FILES
    LICENSE
    DESTINATION ${KOI_DOC_DIR}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    DESTINATION ${KOI_CMAKE_DIR}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}.pc"
    DESTINATION ${KOI_PKGCONFIG_DIR}
)