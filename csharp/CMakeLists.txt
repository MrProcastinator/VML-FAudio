# Enable asm for AOT library support
enable_language(ASM)

project(FAudio-csharp)

set(LIBRARY_NAME FAudio)
compile_mono_single_assembly_aot(
  CODE_FILE ${LIBRARY_NAME}
  ASSEMBLY ${LIBRARY_NAME}-CS
  FLAGS unsafe
)
add_library(VML${LIBRARY_NAME} STATIC
  ${LIBRARY_NAME}.c
  ${LIBRARY_NAME}-CS.dll.s
)

target_include_directories(VML${LIBRARY_NAME} PUBLIC
	$ENV{VITASDK}/arm-vita-eabi/include/SDL2
)

install(
  TARGETS VML${LIBRARY_NAME}
  DESTINATION lib
)

install(
  FILES 
    ${CMAKE_BINARY_DIR}/${LIBRARY_NAME}-CS.dll 
  DESTINATION lib/mono
)

install(
  FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/${LIBRARY_NAME}.h
  DESTINATION include/VML
)
