project (kadu)

# configuration

option (ENABLE_AUTODOWNLOAD "Use automatic download of modules/icons/emoticons" ON)
option (SIG_HANDLING_ENABLED "Define if you want system signals handling" ON)

if (NOT BUILD_DESCRIPTION)
	set (BUILD_DESCRIPTION "Compilied sources" CACHE STRING "Build description" FORCE)
endif (NOT BUILD_DESCRIPTION)

if (NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo." FORCE)
endif (NOT CMAKE_BUILD_TYPE)

cmake_minimum_required (VERSION 2.6.0 FATAL_ERROR)

include (FindPkgConfig)
include (CheckIncludeFiles)
include (CheckFunctionExists)

find_package (Subversion)
if (Subversion_FOUND)
	execute_process (COMMAND svnversion -n ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE DETAILED_VERSION)
endif (Subversion_FOUND)

file (READ ${CMAKE_CURRENT_SOURCE_DIR}/VERSION KADU_VERSION)
string (REPLACE "\n" "" KADU_VERSION ${KADU_VERSION})

if (WIN32)
	# Add more version info to files
	string(REGEX REPLACE "(.+)\\.(.+)\\.(.+)[.-].*" "\\1, \\2, \\3" NUMERIC_VERSION ${KADU_VERSION})
	set(PACKAGE_VERSION ${KADU_VERSION})

	if(DETAILED_VERSION)
		string(REGEX REPLACE "([0-9]+)[^0-9].*" "\\1" SVN_REVISION ${DETAILED_VERSION})
		set(NUMERIC_VERSION "${NUMERIC_VERSION}, ${SVN_REVISION}")
		set(PACKAGE_VERSION "${PACKAGE_VERSION}-r${SVN_REVISION}")
	endif(DETAILED_VERSION)

	# package
	set (CPACK_GENERATOR "ZIP")
	set (CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})	
	include (CPack)
endif (WIN32)

if (NOT WIN32)
	# TODO: Add libexecinfo support
	check_include_files ("execinfo.h" HAVE_EXECINFO)
	if (HAVE_EXECINFO)
		check_function_exists ("backtrace" HAVE_EXECINFO)
	endif (HAVE_EXECINFO)

	if (SIG_HANDLING_ENABLED)
		set (SIG_HANDLING_ENABLED 1)
	endif (SIG_HANDLING_ENABLED)
endif (NOT WIN32)

if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR WIN32)
	add_definitions(-DDEBUG_ENABLED)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug" OR WIN32)

if (MSVC)
	add_definitions (/D_CRT_SECURE_NO_WARNINGS=1)
endif (MSVC)

if(MINGW)
	# override cmake bug/feature?
	set (CMAKE_SHARED_LIBRARY_PREFIX "")
endif(MINGW)

if (NOT DATADIR)
	set (DATADIR ${CMAKE_INSTALL_PREFIX}/share)
endif (NOT DATADIR)
if (NOT LIBDIR)
	set (LIBDIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
endif (NOT LIBDIR)
if (NOT BINDIR)
	set (BINDIR ${CMAKE_INSTALL_PREFIX}/bin)
endif (NOT BINDIR)

# macros

if(WIN32)
	set(KADU_DATA_DIR ".")	
	set(KADU_MODULES_LIBDIR "modules")	
	set(KADU_BIN_DIR ".")
else(WIN32)
	set(KADU_DATA_DIR "share/kadu")
	set(KADU_MODULES_LIBDIR lib${LIB_SUFFIX}/kadu/modules)
	set(KADU_BIN_DIR "bin")
endif(WIN32)

set(KADU_MODULES_DIR ${KADU_DATA_DIR}/modules)

macro (kadu_configuration)
	install (FILES ${ARGN}
		DESTINATION ${KADU_DATA_DIR}/configuration)
endmacro (kadu_configuration)

macro (kadu_module_configuration)
	install (FILES ${ARGN}
		DESTINATION ${KADU_MODULES_DIR}/configuration)
endmacro (kadu_module_configuration)

macro (kadu_module_desc MODULE_DESC)
	install (FILES ${ARGN}
		DESTINATION ${KADU_MODULES_DIR})
endmacro (kadu_module_desc)

macro (kadu_module)

	set (VARIABLE_NAME "MODULE_NAME")

	set (MODULE_NAME "")
	set (MODULE_SOURCES "")
	set (MODULE_MOC_SOURCES "")
	set (MODULE_TRANSLATION_SOURCES "")
	set (MODULE_CONFIGURATION_FILES "")
	set (MODULE_BUILDDEF "")
	set (MODULE_DEPENDENCIES "")
	set (MODULE_LIBRARIES "")

	include_directories (".")

	foreach (ARG ${ARGV})

		if ("${VARIABLE_NAME}" STREQUAL "MODULE_NAME")
			set (MODULE_NAME ${ARG})
			set (VARIABLE_NAME "")
		elseif (ARG MATCHES "(MODULE_SOURCES|MODULE_MOC_SOURCES|MODULE_TRANSLATION_SOURCES|MODULE_CONFIGURATION_FILES|MODULE_BUILDDEF|MODULE_DEPENDENCIES|MODULE_LIBRARIES)")
			set (VARIABLE_NAME ${ARG})
		elseif (VARIABLE_NAME STREQUAL "")
			message (FATAL_ERROR "Invalid invocation of kadu_module macro")
		else ("${VARIABLE_NAME}" STREQUAL "MODULE_NAME")
			set (${VARIABLE_NAME} "${${VARIABLE_NAME}};${ARG}")
		endif ("${VARIABLE_NAME}" STREQUAL "MODULE_NAME")

	endforeach (ARG ${ARGV})

	if (WIN32)
		include_directories (${CMAKE_SOURCE_DIR}/modules)
		# wygeneruj plik z wersja modulu
		if (${${MODULE_NAME}} STREQUAL SHARED)
			set (MODULE_SOURCES ${MODULE_SOURCES} ${MODULE_NAME}.rc)		
			add_custom_command(OUTPUT ${MODULE_NAME}.rc
				COMMAND ${CMAKE_SOURCE_DIR}/modules/modver.bat ARGS ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_NAME}.desc ${MODULE_NAME}.rc
				WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
				DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_NAME}.desc
				COMMENT "Building RC source ${MODULE_NAME}.rc")
		endif(${${MODULE_NAME}} STREQUAL SHARED)
		# naglowki do SDK
		install (DIRECTORY "." DESTINATION ${KADU_DATA_DIR}/kadu-sdk/include/kadu-modules/${MODULE_NAME} FILES_MATCHING PATTERN "*.h" PATTERN ".svn" EXCLUDE)
	endif (WIN32)

	install (FILES ${MODULE_NAME}.desc
		DESTINATION ${KADU_MODULES_DIR})

	if (MODULE_MOC_SOURCES)
		qt_wrap_cpp (${MODULE_NAME} MODULE_MOC_FILES ${MODULE_MOC_SOURCES})
	endif (MODULE_MOC_SOURCES)

	if (MODULE_TRANSLATION_SOURCES)
		qt4_add_translation (MODULE_TRANSLATION_FILES ${MODULE_TRANSLATION_SOURCES})
	endif (MODULE_TRANSLATION_SOURCES)

	kadu_module_configuration(${MODULE_CONFIGURATION_FILES})

	install (FILES ${MODULE_TRANSLATION_FILES}
		DESTINATION ${KADU_MODULES_DIR}/translations)

	if (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	endif (COMMAND cmake_policy)

	add_library (${MODULE_NAME} ${${MODULE_NAME}} ${MODULE_SOURCES} ${MODULE_MOC_FILES})
	add_custom_target (${MODULE_NAME}-translations DEPENDS ${MODULE_TRANSLATION_FILES})

	add_dependencies (${MODULE_NAME} ${MODULE_NAME}-translations)

	if (NOT ${MODULE_BUILDDEF} STREQUAL "")
		set_target_properties(${MODULE_NAME} PROPERTIES COMPILE_DEFINITIONS ${MODULE_BUILDDEF} )
	endif (NOT ${MODULE_BUILDDEF} STREQUAL "")

	if (NOT "${MODULE_LIBRARIES}" STREQUAL "")
		target_link_libraries (${MODULE_NAME} ${MODULE_LIBRARIES})
	endif (NOT "${MODULE_LIBRARIES}" STREQUAL "")

	if (${${MODULE_NAME}} STREQUAL STATIC)
		set_target_properties(${MODULE_NAME} PROPERTIES COMPILE_FLAGS "-DKADULIB" )
	else (${${MODULE_NAME}} STREQUAL STATIC)
		if (WIN32)
			target_link_libraries (${MODULE_NAME} kadu_core ${MODULE_DEPENDENCIES} ${QT_LIBRARIES})
			install (TARGETS ${MODULE_NAME} ARCHIVE DESTINATION ${KADU_DATA_DIR}/kadu-sdk/lib)
		endif (WIN32)
		if (APPLE)
			set_target_properties (${MODULE_NAME} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
		endif (APPLE)
		install (TARGETS ${MODULE_NAME} RUNTIME DESTINATION ${KADU_MODULES_LIBDIR} LIBRARY DESTINATION ${KADU_MODULES_LIBDIR})
	endif (${${MODULE_NAME}} STREQUAL STATIC)

endmacro (kadu_module)

# libraries

set (QT_MIN_VERSION "4.5.0")
find_package (Qt4 REQUIRED)
set(QT_USE_QTXML 1)
set(QT_USE_QTNETWORK 1)
set(QT_USE_QTWEBKIT 1)
include(${QT_USE_FILE})
include_directories (${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/kadu-core ${QT_INCLUDES})

pkg_search_module (LIBGADU REQUIRED libgadu>=1.8.0)
include_directories (${LIBGADU_INCLUDE_DIRS})
link_directories (${LIBGADU_LIBRARY_DIRS})

# build

configure_file (${CMAKE_SOURCE_DIR}/kadu-config.h.cmake ${CMAKE_BINARY_DIR}/kadu-config.h)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
  IMMEDIATE @ONLY)

add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

add_subdirectory (modules)
add_subdirectory (kadu-core)
add_subdirectory (varia)
add_subdirectory (translations)
add_subdirectory (kthxbye)

message (STATUS "Kadu (version: ${KADU_VERSION}) will be build:")
message (STATUS " * libgadu (version ${LIBGADU_VERSION}) in ${LIBGADU_LIBRARY_DIRS}")
message (STATUS " * install prefix: ${CMAKE_INSTALL_PREFIX}")
message (STATUS " * build type: ${CMAKE_BUILD_TYPE}")

# install data files
install (FILES AUTHORS HISTORY README THANKS ChangeLog COPYING DESTINATION ${KADU_DATA_DIR})
