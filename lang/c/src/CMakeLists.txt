#

#

set(AINGLE_SRC
    allocation.c
    array.c
    aingle.h
    aingle/allocation.h
    aingle/basics.h
    aingle/consumer.h
    aingle/data.h
    aingle/errors.h
    aingle/generic.h
    aingle/io.h
    aingle/legacy.h
    aingle/refcount.h
    aingle/resolver.h
    aingle/schema.h
    aingle/value.h
    aingle_generic_internal.h
    aingle_private.h
    codec.c
    codec.h
    consumer.c
    consume-binary.c
    datafile.c
    datum.c
    datum.h
    datum_equal.c
    datum_read.c
    datum_size.c
    datum_skip.c
    datum_validate.c
    datum_value.c
    datum_write.c
    dump.c
    dump.h
    encoding.h
    encoding_binary.c
    errors.c
    generic.c
    io.c
    map.c
    memoize.c
    resolved-reader.c
    resolved-writer.c
    resolver.c
    schema.c
    schema.h
    schema_equal.c
    st.c
    st.h
    string.c
    value.c
    value-hash.c
    value-json.c
    value-read.c
    value-sizeof.c
    value-write.c
    wrapped-buffer.c
)


source_group(AIngle FILES ${AINGLE_SRC})

# The version.sh script gives us a VERSION that uses colon as a
# separator; we need periods.

string(REPLACE ":" "." LIBAINGLE_DOT_VERSION ${LIBAINGLE_VERSION})

add_library(aingle-static STATIC ${AINGLE_SRC})
target_link_libraries(aingle-static ${JANSSON_LIBRARIES} ${CODEC_LIBRARIES} ${THREADS_LIBRARIES})
set_target_properties(aingle-static PROPERTIES OUTPUT_NAME aingle)

if (NOT WIN32)
# TODO: Create Windows DLLs. See https://www.cmake.org/Wiki/BuildingWinDLL
add_library(aingle-shared SHARED ${AINGLE_SRC})
target_link_libraries(aingle-shared ${JANSSON_LIBRARIES} ${CODEC_LIBRARIES} ${THREADS_LIBRARIES})
set_target_properties(aingle-shared PROPERTIES
        OUTPUT_NAME aingle
        VERSION ${LIBAINGLE_DOT_VERSION}
        SOVERSION ${LIBAINGLE_SOVERSION})
endif(NOT WIN32)

install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/aingle.h
        DESTINATION include)
install(DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/aingle
        DESTINATION include
        FILES_MATCHING PATTERN "*.h")

include(GNUInstallDirs)

if (WIN32)
install(TARGETS aingle-static
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
       )
else(WIN32)
install(TARGETS aingle-static aingle-shared
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
       )
endif(WIN32)

# Install pkg-config file

set(prefix ${CMAKE_INSTALL_PREFIX})
set(VERSION ${AINGLE_VERSION})
configure_file(aingle-c.pc.in aingle-c.pc)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/aingle-c.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

add_executable(ainglecat ainglecat.c)
target_link_libraries(ainglecat aingle-static)
install(TARGETS ainglecat RUNTIME DESTINATION bin)

add_executable(aingleappend aingleappend.c)
target_link_libraries(aingleappend aingle-static)
install(TARGETS aingleappend RUNTIME DESTINATION bin)

if (NOT WIN32)
#TODO: Port getopt() to Windows to compile ainglepipe.c and ainglemod.c
add_executable(ainglepipe ainglepipe.c)
target_link_libraries(ainglepipe aingle-static)
install(TARGETS ainglepipe RUNTIME DESTINATION bin)

add_executable(ainglemod ainglemod.c)
target_link_libraries(ainglemod aingle-static)
install(TARGETS ainglemod RUNTIME DESTINATION bin)
endif(NOT WIN32)
