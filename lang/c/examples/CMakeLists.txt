#

#

add_executable(quickstop quickstop.c)
target_link_libraries(quickstop aingle-static)

if (WIN32)
    set(exec_name ${CMAKE_CURRENT_BINARY_DIR}/Debug/quickstop.exe)
else (WIN32)
    set(exec_name ${CMAKE_CURRENT_BINARY_DIR}/quickstop)
endif (WIN32)

add_test(quickstop
    ${CMAKE_COMMAND} -E chdir ${AIngleC_SOURCE_DIR}/examples
    ${exec_name}
)
