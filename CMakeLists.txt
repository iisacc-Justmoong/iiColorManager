cmake_minimum_required(VERSION 3.24)
project(iiColorManager VERSION 0.1.1 LANGUAGES CXX)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(GenerateExportHeader)
include(CTest)
option(BUILD_SHARED_LIBS "Build iiColorManager as a shared library" ON)
if(PROJECT_IS_TOP_LEVEL AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if(DEFINED ENV{HOME} AND NOT "$ENV{HOME}" STREQUAL "")
        set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local/SDK/iiColorManager" CACHE PATH "Install prefix" FORCE)
    elseif(DEFINED ENV{USERPROFILE} AND NOT "$ENV{USERPROFILE}" STREQUAL "")
        set(CMAKE_INSTALL_PREFIX "$ENV{USERPROFILE}/.local/SDK/iiColorManager" CACHE PATH "Install prefix" FORCE)
    else()
        message(FATAL_ERROR "Set CMAKE_INSTALL_PREFIX explicitly")
    endif()
endif()
add_library(iiColorManager src/iiColorManager/ColorManager.cpp)
add_library(iiColorManager::iiColorManager ALIAS iiColorManager)
target_compile_features(iiColorManager PUBLIC cxx_std_23)
set_target_properties(iiColorManager PROPERTIES CXX_EXTENSIONS OFF
    CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN YES
    VERSION ${PROJECT_VERSION} SOVERSION 0.1)
generate_export_header(iiColorManager BASE_NAME IICOLORMANAGER
    EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/include/iiColorManager/Export.h")
target_include_directories(iiColorManager PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(iiColorManager PUBLIC IICOLORMANAGER_STATIC_DEFINE)
endif()
if(MSVC)
    target_compile_options(iiColorManager PRIVATE /W4 /permissive-)
else()
    target_compile_options(iiColorManager PRIVATE -Wall -Wextra -Wpedantic)
endif()
set(_package_dir "${CMAKE_INSTALL_LIBDIR}/cmake/iiColorManager")
install(TARGETS iiColorManager EXPORT iiColorManagerTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES
    src/iiColorManager/ColorManager.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/iiColorManager)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/iiColorManager/Export.h"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/iiColorManager)
install(EXPORT iiColorManagerTargets NAMESPACE iiColorManager:: DESTINATION "${_package_dir}")
configure_package_config_file(cmake/iiColorManagerConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/iiColorManagerConfig.cmake" INSTALL_DESTINATION "${_package_dir}")
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/iiColorManagerConfigVersion.cmake"
    VERSION ${PROJECT_VERSION} COMPATIBILITY SameMinorVersion)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/iiColorManagerConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/iiColorManagerConfigVersion.cmake" DESTINATION "${_package_dir}")
install(FILES README.md DESTINATION ${CMAKE_INSTALL_DATADIR}/iiColorManager)
install(DIRECTORY docs/ DESTINATION ${CMAKE_INSTALL_DATADIR}/iiColorManager/docs)
if(BUILD_TESTING)
    add_executable(iiColorManagerTests tests/ColorManagerTest.cpp)
    target_link_libraries(iiColorManagerTests PRIVATE iiColorManager::iiColorManager)
    foreach(_case conversions lab state modes surfaces validation)
        add_test(NAME iiColorManager.${_case} COMMAND iiColorManagerTests ${_case})
    endforeach()
endif()
