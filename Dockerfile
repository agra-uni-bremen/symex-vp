FROM klee/klee

RUN sudo apt-get update && sudo apt-get -y install gdb valgrind libboost-all-dev wget
