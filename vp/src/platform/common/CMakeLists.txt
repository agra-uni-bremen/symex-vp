file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(platform-common
		fe310_plic.cpp
		abstract_uart.cpp
		slip.cpp
		uart.cpp
		options.cpp
        ${HEADERS})

target_include_directories(platform-common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(platform-common systemc)
