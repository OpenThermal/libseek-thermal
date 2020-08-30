include_directories (
    ${libseek-thermal_SOURCE_DIR}/src
    ${OpenCV_INCLUDE_DIRS}
    ${LIBUSB_INCLUDE_DIRS}
)

link_libraries (
    seek_static
    ${OpenCV_LIBRARIES}
    ${LIBUSB_LIBRARIES}
)

add_executable (seek_test seek_test.cpp)
add_executable (seek_test_pro seek_test_pro.cpp)
add_executable (seek_viewer seek_viewer.cpp args.h)
add_executable (seek_create_flat_field seek_create_flat_field.cpp)

install (TARGETS
    seek_test
    seek_test_pro
    seek_viewer
    seek_create_flat_field
    DESTINATION "bin"
)
