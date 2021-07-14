#

#

set (AINGLE_DOC_SRC
    index.txt
)

# TODO(dln): Use FindAsciidoc script instead.
message(STATUS "Searching for asciidoc...")
find_program(ASCIIDOC_EXECUTABLE asciidoc)
find_program(SOURCE_HIGHLIGHT_EXECUTABLE source-highlight)

if (ASCIIDOC_EXECUTABLE AND SOURCE_HIGHLIGHT_EXECUTABLE)
    foreach(_file ${AINGLE_DOC_SRC})
        get_filename_component(_file_we ${_file} NAME_WE)
        set(_file_path "${CMAKE_CURRENT_SOURCE_DIR}/${_file}")
        set(_html_out "${CMAKE_CURRENT_BINARY_DIR}/${_file_we}.html")
        add_custom_command(
            OUTPUT "${_html_out}"
            COMMAND ${ASCIIDOC_EXECUTABLE}
                -a aingle_version=${AINGLE_VERSION}
                -a libaingle_version=${LIBAINGLE_VERSION}
                -a toc
                --unsafe -n -o "${_html_out}" "${_file_path}"
            DEPENDS "${_file_path}"
            COMMENT "asciidoc ${_file}"
        )
        install(FILES "${_html_out}" DESTINATION share/doc/aingle-c)
        add_custom_target("${_file_we}_html" ALL echo -n
            DEPENDS "${_file}" "${_html_out}"
        )
        add_custom_target(docs DEPENDS "${_html_out}")
    endforeach(_file)
else(ASCIIDOC_EXECUTABLE AND SOURCE_HIGHLIGHT_EXECUTABLE)
    message(WARNING "asciidoc and/or source-highlight not found. HTML documentation will *NOT* be built.")
endif(ASCIIDOC_EXECUTABLE AND SOURCE_HIGHLIGHT_EXECUTABLE)

