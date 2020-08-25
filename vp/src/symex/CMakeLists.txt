add_library(symex main.cpp)
set_property(TARGET symex PROPERTY CXX_STANDARD 17)
target_link_libraries(symex PUBLIC clover)

subdirs(clover)
