add_library(symex symbolic_extension.cpp symbolic_memory.cpp)
target_link_libraries(symex PUBLIC clover)
target_include_directories(symex PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

subdirs(clover)
