cmake_minimum_required(VERSION 3.25)
project(metah VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(METAH_BUILD_TESTS "Build tests for metah" ${PROJECT_IS_TOP_LEVEL})
option(METAH_BUILD_EXAMPLES "Build examples for metah" ${PROJECT_IS_TOP_LEVEL})
message(STATUS "metah: Building tests    -> ${METAH_BUILD_TESTS}")
message(STATUS "metah: Building examples -> ${METAH_BUILD_EXAMPLES}")

add_subdirectory(src)

if(METAH_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
    add_subdirectory(bench)
endif()

if(METAH_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

install(EXPORT metah_Targets
        FILE metahTargets.cmake
        NAMESPACE metah::
        DESTINATION ${CMAKE_INSTALL_DATADIR}/metah
)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/metahConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/metahConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/metah
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/metahConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/metahConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/metah
)