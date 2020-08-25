file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_executable(symex-vp
        symex_main.cpp
        ${HEADERS})

target_link_libraries(symex-vp rv32-symex platform-common gdb-mc
	${Boost_LIBRARIES} ${SystemC_LIBRARIES} pthread)
set_property(TARGET symex-vp PROPERTY CXX_STANDARD 17)

INSTALL(TARGETS symex-vp RUNTIME DESTINATION bin)
