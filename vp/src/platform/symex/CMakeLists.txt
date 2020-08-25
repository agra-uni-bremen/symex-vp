file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_executable(symex-vp
        symex_main.cpp
        ${HEADERS})

target_link_libraries(symex-vp clover rv32 platform-common gdb-mc
	${Boost_LIBRARIES} ${SystemC_LIBRARIES} pthread)

INSTALL(TARGETS symex-vp RUNTIME DESTINATION bin)
