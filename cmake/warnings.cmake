# warnings.cmake

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(WARNING_FLAGS "-Wall -Wextra -Wno-unused-variable -Wno-unused-but-set-variable")
	message(STATUS "Using GNU compiler warnings: ${WARNING_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")

	if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_GREATER_EQUAL 10.2)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")
		message(STATUS "Adding GNU coroutine support: -fcoroutines")
	endif()

elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	if(DEFINED CMAKE_CXX_SIMULATE_ID AND "${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
		# Clang in MSVC mode
		set(WARNING_FLAGS "/W4 /wd4101 /wd4189")
		message(STATUS "Using Clang in MSVC mode warnings: ${WARNING_FLAGS}")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")

		if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_GREATER_EQUAL 10.0)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /await")
			message(STATUS "Adding Clang in MSVC mode coroutine support: /await")
		endif()
	else()
		# Clang in non-MSVC mode
		set(WARNING_FLAGS "-Wall -Wextra -Wno-unused-variable -Wno-unused-but-set-variable")
		message(STATUS "Using Clang warnings: ${WARNING_FLAGS}")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")
	endif()

elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# MSVC equivalent flags
	set(WARNING_FLAGS "/W4 /wd4101 /wd4189")
	message(STATUS "Using MSVC compiler warnings: ${WARNING_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")

	if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_GREATER_EQUAL 19.20)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /await")
		message(STATUS "Adding MSVC coroutine support: /await")
	endif()
endif()
