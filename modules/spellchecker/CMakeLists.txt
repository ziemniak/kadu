set (SOURCES
	spellchecker.cpp
	highlighter.cpp
)

set (MOC_SOURCES
	spellchecker.h
)

set (TRANSLATION_SOURCES
	translations/spellchecker_pl.ts
)

set (CONFIGURATION_FILES
	configuration/spellchecker.ui)

include (FindASPELL)

if (NOT ASPELL_FOUND)
	message (FATAL_ERROR "libaspell not found!!")
endif (NOT ASPELL_FOUND)

kadu_module (spellchecker
	MODULE_SOURCES ${SOURCES}
	MODULE_MOC_SOURCES ${MOC_SOURCES}
	MODULE_TRANSLATION_SOURCES ${TRANSLATION_SOURCES}
	MODULE_CONFIGURATION_FILES ${CONFIGURATION_FILES}
)

target_link_libraries (spellchecker ${ASPELL_LIBRARIES})
include_directories (${ASPELL_INCLUDE_DIRS})
add_definitions (${ASPELL_DEFINITIONS})
