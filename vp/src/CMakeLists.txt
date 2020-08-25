include_directories(.)

subdirs(core)
subdirs(platform)

if(SYMEX_BACKEND)
	find_package(ZLIB REQUIRED)
	find_package(STP  REQUIRED)
	find_package(LLVM REQUIRED)
	find_package(Z3   REQUIRED)
	find_package(KLEE REQUIRED)

	include_directories(
		${Z3_INCLUDE_DIRS}
		${STP_INCLUDE_DIRS}
		${LLVM_INCLUDE_DIRS}
		${KLEE_INCLUDE_DIRS})
	link_directories(
		${Z3_LIBRARY_DIR}
		${STP_LIBRARY_DIR}
		${KLEE_LIBRARY_DIR})

	subdirs(clover)
endif()
