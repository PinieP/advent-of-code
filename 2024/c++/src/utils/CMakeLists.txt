add_library(utils)
target_sources(utils
    PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    utils.cpp
    core.cpp
    pretty.cpp
    assert.cpp
    strings.cpp
)

