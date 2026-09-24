vcpkg_cmake_configure(
        SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}"
        OPTIONS
        -DMETAH_BUILD_TESTS=OFF
        -DMETAH_BUILD_EXAMPLES=OFF
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(PACKAGE_NAME metah CONFIG_PATH share/metah)