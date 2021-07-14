#

#

macro(add_aingle_executable name)
    set(source "${ARGV1}")
    if (NOT source)
        set(source "${name}.c")
    endif (NOT source)
    add_executable(${name} ${source})
    target_link_libraries(${name} aingle-static)
endmacro(add_aingle_executable)

macro(add_aingle_test name)
    add_aingle_executable(${name} ${ARGN})
    if (WIN32)
        set(exec_name ${CMAKE_CURRENT_BINARY_DIR}/Debug/${name}.exe)
    else (WIN32)
        set(exec_name ${CMAKE_CURRENT_BINARY_DIR}/${name})
    endif (WIN32)

    add_test(${name}
        ${CMAKE_COMMAND} -E chdir ${AIngleC_SOURCE_DIR}/tests
        ${exec_name}
    )
endmacro(add_aingle_test)

macro(add_aingle_test_checkmem name)
    add_aingle_test(${name} ${ARGN})
    if(UNIX)
        find_program(MEMORYCHECK_COMMAND valgrind )
        if(MEMORYCHECK_COMMAND)
            add_test(memcheck_${name}
                ${CMAKE_COMMAND} -E chdir ${AIngleC_SOURCE_DIR}/tests
                ${MEMORYCHECK_COMMAND}
                    --log-file=${CMAKE_CURRENT_BINARY_DIR}/memcheck_${name}.log
                    --leak-check=full
                    --show-reachable=yes
                    --error-exitcode=1
                    ${exec_name}
            )
        endif(MEMORYCHECK_COMMAND)
    endif (UNIX)
endmacro(add_aingle_test_checkmem)

add_aingle_executable(generate_interop_data)
add_aingle_executable(performance)
add_aingle_executable(test_interop_data)

add_aingle_test_checkmem(test_data_structures)
add_aingle_test_checkmem(test_aingle_schema)
add_aingle_test_checkmem(test_aingle_schema_names)
add_aingle_test_checkmem(test_aingle_values)
add_aingle_test_checkmem(test_aingle_766)
add_aingle_test_checkmem(test_aingle_968)
add_aingle_test_checkmem(test_aingle_984)
add_aingle_test_checkmem(test_aingle_1034)
add_aingle_test_checkmem(test_aingle_1084)
add_aingle_test_checkmem(test_aingle_1087)
add_aingle_test_checkmem(test_aingle_1165)
add_aingle_test_checkmem(test_aingle_1167)
add_aingle_test_checkmem(test_aingle_1237)
add_aingle_test_checkmem(test_aingle_1238)
add_aingle_test_checkmem(test_aingle_1279)
add_aingle_test_checkmem(test_aingle_1405)
add_aingle_test_checkmem(test_aingle_1572)
add_aingle_test(test_aingle_data) # Skip memory check for datum. Deprecated and has a lot of memory issues
add_aingle_test_checkmem(test_refcount)
add_aingle_test_checkmem(test_aingle_1379)
add_aingle_test_checkmem(test_aingle_1691)
add_aingle_test_checkmem(test_aingle_1906)
add_aingle_test_checkmem(test_aingle_1904)
