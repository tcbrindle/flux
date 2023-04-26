
function(ADD_EXAMPLE NAME SOURCE)
    add_executable(${NAME} ${SOURCE})
    target_link_libraries(${NAME} flux)
    add_test(NAME ${NAME} COMMAND ${NAME})
endfunction()

add_example(example-config-parser config_parser.cpp)
add_example(example-calendar calendar.cpp)

add_example(example-docs-adjacent docs/adjacent.cpp)
add_example(example-docs-all docs/all.cpp)
add_example(example-docs-any docs/any.cpp)
add_example(example-docs-compare docs/compare.cpp)
add_example(example-docs-contains docs/contains.cpp)
add_example(example-docs-count docs/count.cpp)

