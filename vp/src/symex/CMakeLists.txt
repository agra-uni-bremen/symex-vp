add_library(symex main.cpp)
target_link_libraries(symex PUBLIC clover)

subdirs(clover)
