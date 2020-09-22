include_directories(.)

subdirs(core)
subdirs(platform)

if(SYMEX_BACKEND)
	subdirs(symex)
endif()
