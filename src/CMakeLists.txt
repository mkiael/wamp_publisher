add_executable(atb_publisher publisher.cpp)

target_link_libraries(atb_publisher ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})
