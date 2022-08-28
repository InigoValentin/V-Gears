add_definitions(
    -DBOOST_TEST_MODULE="standalone"
)

if(UNIX OR APPLE)
    add_definitions(
        -DBOOST_TEST_DYN_LINK
    )
endif()

set(V-Gears_SOURCE_DIR ${VGears_SOURCE_DIR}/V-Gears)

add_subdirectory(V-Gears)
add_subdirectory(FinalFantasy7)
