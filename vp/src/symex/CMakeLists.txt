add_library(symex
	symbolic_extension.cpp
	symbolic_memory.cpp
	symbolic_context.cpp)

target_link_libraries(symex PUBLIC clover core-common)
target_include_directories(symex PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

subdirs(clover)
