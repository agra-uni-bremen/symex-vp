subdirs(common)
subdirs(rv32)
subdirs(rv64)

if(SYMEX_BACKEND)
	subdirs(rv32-symex)
endif()
