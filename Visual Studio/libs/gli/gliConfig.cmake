set(GLI_VERSION "0.8.3")
set(GLI_INCLUDE_DIRS "D:/source/Nyxis-Engine/libs/gli")

if (NOT CMAKE_VERSION VERSION_LESS "3.0")
	include("${CMAKE_CURRENT_LIST_DIR}/gliTargets.cmake")
endif()
