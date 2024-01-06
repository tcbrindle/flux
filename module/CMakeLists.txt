
add_library(flux-mod)
target_sources(flux-mod PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${PROJECT_SOURCE_DIR}/module
    FILES flux.cpp
)

target_link_libraries(flux-mod PUBLIC flux)
set_target_properties(flux-mod PROPERTIES CXX_EXTENSIONS Off)

# Squish MSVC warning when building the module, hopefully we're not actually doing anything wrong
if(MSVC)
    target_compile_options(flux-mod PRIVATE
        /wd5244 # '#include <flux.hpp>' in the purview of module 'flux' appears erroneous
    )
endif()