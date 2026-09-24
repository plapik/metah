add_library(metah INTERFACE)
add_library(metah::metah ALIAS metah)

target_compile_features(metah INTERFACE cxx_std_20)

find_package(OpenMP REQUIRED COMPONENTS CXX)
find_package(Boost CONFIG REQUIRED COMPONENTS random)
find_package(fmt CONFIG REQUIRED)
find_package(absl CONFIG REQUIRED)

target_link_libraries(metah INTERFACE
        OpenMP::OpenMP_CXX
        Boost::random
        fmt::fmt
        absl::linked_hash_set
)

target_include_directories(metah INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

install(TARGETS metah
        EXPORT metah_Targets
        INCLUDES DESTINATION include
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION include/metah
        FILES_MATCHING PATTERN "*.hpp"
)