
add_library(flux.module)
add_library(flux::module ALIAS flux.module)

target_sources(flux.module PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${PROJECT_SOURCE_DIR}/module
    FILES flux.cpp
)

target_sources(flux.module PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include
    FILES ${PROJECT_SOURCE_DIR}/include/flux/macros.hpp
)

target_link_libraries(flux.module PRIVATE flux::flux)
target_compile_features(flux.module PUBLIC $<IF:$<CXX_COMPILER_ID:MSVC>,cxx_std_23,cxx_std_20>)
set_target_properties(flux.module PROPERTIES CXX_EXTENSIONS Off)
