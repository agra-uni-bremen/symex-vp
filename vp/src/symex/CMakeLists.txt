add_library(symex
	symbolic_extension.cpp
	symbolic_memory.cpp
	symbolic_context.cpp
	symbolic_explore.cpp
	symbolic_ctrl.cpp)

# Older C++ compiler may still require linking with -lstdc++fs to
# support std::filesystem as used in symbolic_explore.cpp.
target_link_libraries(symex PUBLIC "${SystemC_LIBRARIES}" clover
	core-common stdc++fs)
target_include_directories(symex PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

subdirs(clover)
