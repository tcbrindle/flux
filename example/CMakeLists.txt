
function(ADD_DOC_EXAMPLE NAME SOURCE)
    add_executable(${NAME} ${SOURCE})
    target_link_libraries(${NAME} flux)
    add_test(NAME ${NAME} COMMAND ${NAME})
endfunction()

add_doc_example(example-docs-adjacent docs/adjacent.cpp)
add_doc_example(example-docs-all docs/all.cpp)
add_doc_example(example-docs-any docs/any.cpp)
add_doc_example(example-docs-compare docs/compare.cpp)
add_doc_example(example-docs-contains docs/contains.cpp)
add_doc_example(example-docs-count docs/count.cpp)

