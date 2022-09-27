file(COPY misc DESTINATION .)

include_directories(
    ${VGears_INCLUDE_DIRS}
    ${V-Gears_SOURCE_DIR}/include
)

set(LIBRARIES
    ${VGears_TEST_LIBRARIES}
    V-Gears
    v-gears-data
)

add_executable(TestVGearsAFileSerializer src/test_VGearsAFileSerializer.cpp)
SET_PROPERTY(TARGET TestVGearsAFileSerializer PROPERTY FOLDER "tests")

target_link_libraries(TestVGearsAFileSerializer ${LIBRARIES})
add_test(NAME TestVGearsAFileSerializer COMMAND TestVGearsAFileSerializer)

add_executable(TestVGearsMapFileXMLSerializer src/test_VGearsMapFileXMLSerializer.cpp)
SET_PROPERTY(TARGET TestVGearsMapFileXMLSerializer PROPERTY FOLDER "tests")

target_link_libraries(TestVGearsMapFileXMLSerializer ${LIBRARIES})
add_test(NAME TestVGearsMapFileXMLSerializer COMMAND TestVGearsMapFileXMLSerializer)

# Field walk mesh.
add_executable(TestVGearsWalkmeshFileXMLSerializer src/test_VGearsWalkmeshFileXMLSerializer.cpp)
SET_PROPERTY(TARGET TestVGearsWalkmeshFileXMLSerializer PROPERTY FOLDER "tests")

target_link_libraries(TestVGearsWalkmeshFileXMLSerializer ${LIBRARIES})
add_test(NAME TestVGearsWalkmeshFileXMLSerializer COMMAND TestVGearsWalkmeshFileXMLSerializer)

# LGP archive.
add_executable(TestVGearsLGPArchive src/test_VGearsLGPArchive.cpp)
SET_PROPERTY(TARGET TestVGearsLGPArchive PROPERTY FOLDER "tests")

target_link_libraries(TestVGearsLGPArchive ${LIBRARIES})
add_test(NAME TestVGearsLGPArchive COMMAND TestVGearsLGPArchive)

# HRC archive.
add_executable(TestVGearsHRCSerializer src/test_VGearsHRCSerializer.cpp)
SET_PROPERTY(TARGET TestVGearsHRCSerializer PROPERTY FOLDER "tests")

target_link_libraries(TestVGearsHRCSerializer ${LIBRARIES})
add_test(NAME TestVGearsHRCSerializer COMMAND TestVGearsHRCSerializer)

# World Map
add_executable(TestMapSerializer src/test_MapSerializer.cpp)
SET_PROPERTY(TARGET TestMapSerializer PROPERTY FOLDER "tests")

target_link_libraries(TestMapSerializer ${LIBRARIES})
# TODO FIX ME - test not currently working.
#add_test(TestMapSerializer TestMapSerializer)

add_executable(TestTxzSerializer src/test_TxzSerializer.cpp)
SET_PROPERTY(TARGET TestTxzSerializer PROPERTY FOLDER "tests")

target_link_libraries(TestTxzSerializer ${LIBRARIES})
# TODO FIX ME - test not currently working
#add_test(TestTxzSerializer TestTxzSerializer)