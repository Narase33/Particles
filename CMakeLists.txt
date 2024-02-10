# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

set (CMAKE_GENERATOR "Ninja")
message("generator is set to ${CMAKE_GENERATOR}")

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

project ("Particle")

set(MSVC_GLOBAL_FLAGS
	/std:c++latest
	/permissive- # standards conformance mode for MSVC compiler.
	/MP
	/WX

	/wd4244
	/wd4242 # 'identifier': conversion from 'type1' to 'type1', possible loss of data
	/wd4996

	/W4 # Baseline reasonable warnings
	/w14254 # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
	/w14263 # 'function': member function does not override any base class virtual member function
	/w14265 # 'classname': class has virtual functions, but destructor is not virtual instances of this class may not be destructed correctly
	/w14287 # 'operator': unsigned/negative constant mismatch
	/we4289 # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside the for-loop scope
	/w14296 # 'operator': expression is always 'boolean_value'
	/w14311 # 'variable': pointer truncation from 'type1' to 'type2'
	/w14545 # expression before comma evaluates to a function which is missing an argument list
	/w14546 # function call before comma missing argument list
	/w14547 # 'operator': operator before comma has no effect; expected operator with side-effect
	/w14549 # 'operator': operator before comma has no effect; did you intend 'operator'?
	/w14555 # expression has no effect; expected expression with side- effect
	/w14619 # pragma warning: there is no warning number 'number'
	/w14640 # Enable warning on thread un-safe static member initialization
	/w14826 # Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
	/w14905 # wide string literal cast to 'LPSTR'
	/w14906 # string literal cast to 'LPWSTR'
	/w14928 # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
)

set(MSVC_DEBUG_FLAGS
	${MSVC_GLOBAL_FLAGS}
	#/fsanitize=address
)

set(MSVC_RELEASE_FLAGS
	${MSVC_GLOBAL_FLAGS}
	/O2
	/GL
	/fp:fast
	/arch:AVX
)

set(MSVC_RELDEBUG_FLAGS
	${MSVC_RELEASE_FLAGS}
	/MDd
	/Zi
	/Ob0
	/Od
	/RTC1
)

add_subdirectory("libs/SFML-2.5.1")
add_executable (Particle "Particle/main.cpp" "Particle/BarnesHut.cpp")

target_link_libraries(Particle
	PRIVATE
		sfml-graphics sfml-window sfml-system
)

target_include_directories(Particle PRIVATE
	SYSTEM "libs/SFML-2.5.1/include"
)


if(CMAKE_BUILD_TYPE  STREQUAL "Debug")
	target_compile_options(Particle PUBLIC ${MSVC_DEBUG_FLAGS})
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	target_compile_options(Particle PUBLIC ${MSVC_RELDEBUG_FLAGS})
else()
	target_compile_options(Particle PUBLIC ${MSVC_RELEASE_FLAGS})
endif()