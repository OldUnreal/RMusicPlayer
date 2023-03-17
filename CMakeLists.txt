project(RMusicPlayer)

cmake_minimum_required(VERSION 3.13) # we need add_link_options
if(APPLE)
  set(CMAKE_CXX_STANDARD 17) # required by metal
else()
  set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PROJECT_VERSION_MAJOR "")
set(CMAKE_PROJECT_VERSION_MINOR "")
set(CMAKE_PROJECT_VERSION_PATCH "")
# uncomment this to see compiler/linker commands
# set(CMAKE_VERBOSE_MAKEFILE ON)

include(CheckCCompilerFlag)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(SelectLibraryConfigurations)

################################################################################
# Figure out build type and target platform
################################################################################

# Sanity checks
if(WIN32 AND NOT WINDOWS)
  set(WINDOWS TRUE)
  set(PLATFORM_PREFIX "Win")
endif()
if(APPLE AND NOT MACOSX)
  set(MACOSX TRUE)
  set(PLATFORM_PREFIX "Mac")
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(LINUX TRUE)
  set(PLATFORM_PREFIX "Linux")
endif()

# See what CPU we appear to be targeting... 
macro(check_cpu _CDEFS _NAME _VALUE)
  check_c_source_compiles(
    "
        #if ( ${_CDEFS} )
        int main(int argc, char **argv) { int yup = 1; return 0; }
        #else
        #error Not targeting this CPU architecture.
        #endif
        "
    ${_VALUE}
  )

  if(${_VALUE})
    if(OLDUNREAL_CHOSE_CPU)
      message(STATUS "We appear to see two different CPU architectures!")
      message(STATUS "We saw '${OLDUNREAL_CPU}' and '${_NAME}'.")
      message(FATAL_ERROR "Please fix this before continuing.")
    endif()
    set(OLDUNREAL_CHOSE_CPU TRUE)
    set(OLDUNREAL_CPU ${_NAME})
  endif()
endmacro(check_cpu)

check_cpu(
  "defined(__i386__) || defined(__i686__) || defined(_M_IX86) || defined(i386)"
  "x86" OLDUNREAL_X86
)

check_cpu("defined(__x86_64__) || defined(_M_X64)" "amd64" OLDUNREAL_AMD64)

check_cpu("defined(__EMSCRIPTEN__)" "emscripten" OLDUNREAL_EMSCRIPTEN)

check_cpu("defined(__arm__)" "arm" OLDUNREAL_ARM)

check_cpu("defined(__arm64__)" "arm64" OLDUNREAL_ARM64)

if (NOT OLDUNREAL_CHOSE_CPU)
  message(FATAL_ERROR "We don't support this architecture yet")
endif()

################################################################################
# Figure out prefix/module path
################################################################################

if(NOT CMAKE_PREFIX_PATH)
  if(OLDUNREAL_DEPENDENCIES_PATH)
    set(CMAKE_PREFIX_PATH "${OLDUNREAL_DEPENDENCIES_PATH}")
  else()
    set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/build/${PLATFORM_PREFIX}/${OLDUNREAL_CPU}/${CMAKE_BUILD_TYPE}")
  endif()
endif()

message(STATUS "Using dependencies path: ${CMAKE_PREFIX_PATH}")

################################################################################
# Locate all 3rd party headers and libs
################################################################################

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindFMOD.cmake)

################################################################################
# Set compiler and linker flags
################################################################################

add_compile_definitions("$<$<CONFIG:Debug>:_DEBUG>")
add_compile_definitions("$<$<CONFIG:Release>:_RELEASE>")
add_compile_definitions("$<$<CONFIG:Release>:NDEBUG>")
add_compile_definitions("$<$<CONFIG:Release>:RELEASE>")

add_compile_definitions(UTGLR_UT_BUILD)
add_compile_definitions(MOD_BUILD)

if(APPLE AND OLDUNREAL_MAKE_BUNDLE)
  set(OLDUNREAL_STATIC_LINK TRUE)
endif()

if(LINUX)
  # Don't use -rpath (at least, not the silly one CMake forces).
  set(CMAKE_SKIP_RPATH
    ON
      CACHE BOOL "Skip RPATH" FORCE
  )
  add_compile_definitions(UNICODE=1)
  add_compile_definitions(_UNICODE=1)
  add_compile_definitions(_REENTRANT=1)
  add_compile_definitions(HAVE_PRAGMA_PACK=1)
  add_definitions(-fPIC)

  # disable A3D in Cluster
  add_compile_definitions(USE_A3D2=0)

  if(OLDUNREAL_ARM)
    add_compile_definitions(__LINUX_ARM__=1)
    add_compile_definitions(SDL_DISABLE_MM3DNOW_H=1)
    add_compile_definitions(SDL_DISABLE_IMMINTRIN_H=1)
    add_compile_definitions(SDL_DISABLE_MMINTRIN_H=1)
    add_compile_definitions(SDL_DISABLE_XMMINTRIN_H=1)
    add_compile_definitions(SDL_DISABLE_EMMINTRIN_H=1)
    add_compile_definitions(SDL_DISABLE_PMMINTRIN_H=1)
    add_definitions(-mfpu=neon-vfpv4)
    add_definitions(-mfloat-abi=hard)
  else()
    add_compile_definitions(__LINUX_X86__=1)
	
	# stijn: statically link libstdc++ and libgcc as permitted by the GPL
	# runtime exception

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse -msse2")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -msse2")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}  -static-libgcc -static-libstdc++ -fuse-ld=gold -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/Version.script")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}  -static-libgcc -static-libstdc++ -fuse-ld=gold")

	# strip symbols in release builds
	add_link_options("$<$<CONFIG:Release>:-Wl,--strip-all>")
  endif()
  
elseif(APPLE)
  add_compile_definitions(MACOSX=1)
  add_compile_definitions(UNICODE=1)
  add_compile_definitions(_UNICODE=1)
  add_compile_definitions(_REENTRANT=1)
  add_compile_definitions(HAVE_PRAGMA_PACK=1)
  # disable A3D in Cluster
  add_compile_definitions(USE_A3D2=0)

  if (OLDUNREAL_ARM64)
    add_compile_definitions(__APPLE_ARM64__=1)
  else()
    add_definitions(-mmacosx-version-min=10.9)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -mmacosx-version-min=10.9")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mmacosx-version-min=10.9")
  endif()

  set(CMAKE_INSTALL_RPATH "@executable_path")
  
else()
  # use multi-threaded version of C runtime lib and link it statically
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS=1)
  add_compile_definitions(_CRT_NO_VA_START_VALIDATION)
  add_compile_definitions(UNICODE=1)
  add_compile_definitions(_UNICODE=1)
  add_compile_definitions(WIN32=1)
  if(OLDUNREAL_AMD64)
    add_compile_definitions(WIN64=1)
  else()
	#
	# /Zc:wchar_t-
    # do not treat wchar_t as a built-in type. Doing so would break binary
    # compat with almost all native mods for earlier UT versions
	#
	# /arch:SSE2
    # assume we have at least a Pentium 4
	#
	# /LARGEADDRESSAWARE
    # we can handle more than 2Gb of RAM even in 32bit builds
	#
	# /Zp4
	# use 4-byte alignment for structs. This is necessary for binary compatibility
	#
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t- /arch:SSE2 /Zp4 -DWINDOWS_IGNORE_PACKING_MISMATCH")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Zc:wchar_t- /arch:SSE2 /Zp4 -DWINDOWS_IGNORE_PACKING_MISMATCH")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LARGEADDRESSAWARE")
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /LARGEADDRESSAWARE")
	set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /LARGEADDRESSAWARE")	
  endif()
  #
  # /MP
  # enable parallel builds
  #
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

  # the recursive garbage collector needs a lot of stack space
  set(CMAKE_EXE_LINKER_FLAGS
      "${CMAKE_EXE_LINKER_FLAGS} /STACK:16000000,1000000 /LTCG"
  )	
  set(LIBRARY_SUFFIX ".dll")

endif()

if(CMAKE_COMPILER_IS_GNUCC OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -fsigned-char -fno-strict-aliasing -Wno-inline-new-delete")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pipe -fsigned-char -fno-strict-aliasing")
endif()

if(OLDUNREAL_STATIC_LINK)
  add_compile_definitions(__STATIC_LINK=1)
  set(OLDUNREAL_PACKAGE_LINKAGE STATIC)
else()
  set(OLDUNREAL_PACKAGE_LINKAGE SHARED)
endif()

if(OLDUNREAL_USE_ASAN)
  add_compile_options("-fsanitize=address")
  link_libraries("-fsanitize=address")
endif()

################################################################################
# Linker macros
################################################################################

macro(oldunreal_set_package_properties _PACKAGE)
  string(TOUPPER "${_PACKAGE}" _UPACKAGE)
    if (WINDOWS)    
	# stijn: the GPackage symbol has local (intra-module) visibility on Windows
	# platforms so there's no need to give it a unique name here
    set_target_properties(${_PACKAGE} PROPERTIES COMPILE_DEFINITIONS "${_UPACKAGE}_API=__declspec(dllexport)" PREFIX "")
  else()
	# stijn: for WIN32 static and for NIX, we need to make sure that the various
	# GPackage symbols don't clash so we rename them using a preprocessor
	# definition
    set_target_properties(${_PACKAGE} PROPERTIES COMPILE_DEFINITIONS "GPackage=LocalPackageName${_PACKAGE}" PREFIX "" LINK_FLAGS "-Wl,-rpath,$ORIGIN")
  endif()

  if(NOT ${_PACKAGE} STREQUAL "Core")
    target_link_libraries(${_PACKAGE} Core)
	if(NOT ${_PACKAGE} STREQUAL "Engine")
	  target_link_libraries(${_PACKAGE} Engine)
	endif()
  endif()
endmacro()

macro(oldunreal_link_core_library _LIBRARY)
	add_library(${_LIBRARY} SHARED IMPORTED)
	set_target_properties(${_LIBRARY} PROPERTIES
		IMPORTED_LOCATION_DEBUG          ${CMAKE_CURRENT_SOURCE_DIR}/../System/${_LIBRARY}${LIBRARY_SUFFIX}
		IMPORTED_LOCATION_RELEASE        ${CMAKE_CURRENT_SOURCE_DIR}/../System/${_LIBRARY}${LIBRARY_SUFFIX}
		IMPORTED_LOCATION_MINSIZEREL     ${CMAKE_CURRENT_SOURCE_DIR}/../System/${_LIBRARY}${LIBRARY_SUFFIX}
		IMPORTED_LOCATION_RELWITHDEBINFO ${CMAKE_CURRENT_SOURCE_DIR}/../System/${_LIBRARY}${LIBRARY_SUFFIX}		
		IMPORTED_IMPLIB_DEBUG            ${CMAKE_CURRENT_SOURCE_DIR}/../${_LIBRARY}/Lib/${_LIBRARY}.lib
		IMPORTED_IMPLIB_RELEASE          ${CMAKE_CURRENT_SOURCE_DIR}/../${_LIBRARY}/Lib/${_LIBRARY}.lib
		IMPORTED_IMPLIB_MINSIZEREL       ${CMAKE_CURRENT_SOURCE_DIR}/../${_LIBRARY}/Lib/${_LIBRARY}.lib
		IMPORTED_IMPLIB_RELWITHDEBINFO   ${CMAKE_CURRENT_SOURCE_DIR}/../${_LIBRARY}/Lib/${_LIBRARY}.lib		
	)
endmacro()

################################################################################
# Include Directories
################################################################################

include_directories(../Core/Inc)
include_directories(../Engine/Inc)
include_directories(./inc)

################################################################################
# Dynamically link with core libs
################################################################################

oldunreal_link_core_library(Core)
oldunreal_link_core_library(Engine)

################################################################################
# Target definitions
################################################################################

file(GLOB RMUSICPLAYER_HEADERS Inc/*.h)
file(GLOB RMUSICPLAYER_SOURCES Src/*.cpp)
add_library(RMusicPlayer
  SHARED
  ${RMUSICPLAYER_HEADERS}
  ${RMUSICPLAYER_SOURCES}
)
oldunreal_set_package_properties(RMusicPlayer)

################################################################################
# Linking Rules
################################################################################

target_link_libraries(RMusicPlayer
  ${FMOD_LINK_LIBRARY}
)

################################################################################
# Installation Rules
################################################################################

install(
  TARGETS RMusicPlayer
  DESTINATION ./
)

install(
  FILES ${FMOD_INSTALL_LIBRARY}
  DESTINATION ./
)
