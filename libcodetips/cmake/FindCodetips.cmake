FIND_PATH(CODETIPS_INCLUDE_DIR codetips/generics.hpp)
FIND_LIBRARY(CODETIPS_LIBRARY NAMES codetips)
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CODETIPS DEFAULT_MSG CODETIPS_LIBRARY CODETIPS_INCLUDE_DIR)