set(KLEE_SOURCE_DIR $ENV{HOME}/klee_src)
set(KLEE_BUILD_DIR  $ENV{HOME}/klee_build)

set(KLEE_LIBRARY_DIR  ${KLEE_BUILD_DIR}/lib)
set(KLEE_INCLUDE_DIRS ${KLEE_SOURCE_DIR}/include ${KLEE_BUILD_DIR}/include)

set(KLEE_LIBS
	libkleaverSolver.a
	libkleaverExpr.a
	libkleeBasic.a
	libkleeCore.a
	libkleeModule.a
	libkleeSupport.a)
