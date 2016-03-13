#################################################
#
#  (C) 2016 Golubev Alexander
#  fatzer2 (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

# Force to use C compiler for asm_scale.S instead of ASM
# because contains mixed code - ASM with C definitions
set_source_files_properties( asm_scale.S PROPERTIES LANGUAGE C )

# TODO add assembler flags here (2016-01-14, Fat-Zer)

##### fastscale (static) ########################

tde_add_library( fastscale STATIC_PIC AUTOMOC
  SOURCES scale.cpp asm_scale.S
  LINK tdecore-shared
)
