file(COPY misc DESTINATION .)

include_directories(
    ${VGears_INCLUDE_DIRS}
    ${V-Gears_SOURCE_DIR}/include
    ${VGears_SOURCE_DIR}/SupportedGames/FinalFantasy7/include
)

set(LIBRARIES
    ${VGears_TEST_LIBRARIES}
    FinalFantasy7
	V-Gears
	v-gears-data
)

add_executable(TestFF7FLevelFileSerializer
    src/test_FF7FLevelFileSerializer
)
SET_PROPERTY(TARGET TestFF7FLevelFileSerializer PROPERTY FOLDER "tests")


target_link_libraries(TestFF7FLevelFileSerializer ${LIBRARIES})
#add_test(NAME TestFF7FLevelFileSerializer COMMAND TestFF7FLevelFileSerializer)


add_executable(TestFF7ModelListFileSerializer
    src/test_FF7ModelListFileSerializer.cpp
)
SET_PROPERTY(TARGET TestFF7ModelListFileSerializer PROPERTY FOLDER "tests")


target_link_libraries(TestFF7ModelListFileSerializer ${LIBRARIES})
add_test(NAME TestFF7ModelListFileSerializer COMMAND TestFF7ModelListFileSerializer)


add_executable(TestFF7WalkmeshFileSerializer
    src/test_FF7WalkmeshFileSerializer.cpp
)
SET_PROPERTY(TARGET TestFF7WalkmeshFileSerializer PROPERTY FOLDER "tests")

target_link_libraries(TestFF7WalkmeshFileSerializer ${LIBRARIES})
add_test(NAME TestFF7WalkmeshFileSerializer COMMAND TestFF7WalkmeshFileSerializer)
