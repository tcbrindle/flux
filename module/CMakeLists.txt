
include(modules)

add_module_library(flux-mod flux.cpp)
target_link_libraries(flux-mod PRIVATE flux)

