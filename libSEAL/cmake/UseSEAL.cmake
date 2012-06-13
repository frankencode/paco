FIND_PACKAGE(CURL REQUIRED)
LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH})
IF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
LINK_LIBRARIES(${SEAL_LIBRARY} ${CURL_LIBRARY} crypto ssl)
ELSE(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
LINK_LIBRARIES(${SEAL_LIBRARY} ${CURL_LIBRARY} gnutls gcrypt idn)
ENDIF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
INCLUDE_DIRECTORIES(${SEAL_INCLUDE_DIR})
