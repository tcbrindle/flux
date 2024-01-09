
add_library(flux-mod)
add_library(flux::module ALIAS flux-mod)

target_sources(flux-mod PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${PROJECT_SOURCE_DIR}/module
    FILES flux.cpp
)

target_sources(flux-mod PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include
    FILES ${PROJECT_SOURCE_DIR}/include/flux/macros.hpp
)

target_link_libraries(flux-mod PRIVATE flux)
target_compile_features(flux-mod PUBLIC $<IF:$<CXX_COMPILER_ID:MSVC>,cxx_std_23,cxx_std_20>)
set_target_properties(flux-mod PROPERTIES CXX_EXTENSIONS Off)

# Squish MSVC warning when building the module, hopefully we're not actually doing anything wrong
if(MSVC)
    target_compile_options(flux-mod PRIVATE
        /wd5244 # '#include <flux.hpp>' in the purview of module 'flux' appears erroneous
    )
endif()