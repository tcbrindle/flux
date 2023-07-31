
include(modules)

add_module_library(flux-mod flux.cpp)
target_link_libraries(flux-mod PUBLIC flux)

# Squish MSVC warning when building the module, hopefully we're not actually doing anything wrong
if(MSVC)
    target_compile_options(flux-mod PRIVATE
        /wd5244 # '#include <flux.hpp>' in the purview of module 'flux' appears erroneous
    )
endif()