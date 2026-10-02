# The stage data: tools/gen_stage.py lays out the stage (a stage2d spec and the entity layout),
# LibSaturn's installed stage2d tool compiles the spec into C arrays. Everything lands in the
# build tree; nothing generated is checked in.
set(HSP_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(HSP_GENERATED_SOURCES "${HSP_GENERATED_DIR}/stage.c" "${HSP_GENERATED_DIR}/layout.c")
set(HSP_GENERATED_HEADERS "${HSP_GENERATED_DIR}/stage.h" "${HSP_GENERATED_DIR}/layout.h")

file(GLOB _hsp_stage2d_package CONFIGURE_DEPENDS "${LIBSATURN_DISC_TOOLS_DIR}/stage2d/*.py")

# Every output is listed in one rule, so the two commands run once however many of them a
# target needs.
add_custom_command(
    OUTPUT ${HSP_GENERATED_SOURCES} ${HSP_GENERATED_HEADERS}
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${HSP_GENERATED_DIR}"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/gen_stage.py"
            --tools-dir "${LIBSATURN_DISC_TOOLS_DIR}" --out-dir "${HSP_GENERATED_DIR}"
    COMMAND "${Python3_EXECUTABLE}" "${LIBSATURN_STAGE2D_TOOL}" build
            "${HSP_GENERATED_DIR}/stage_spec.json" --out-dir "${HSP_GENERATED_DIR}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tools/gen_stage.py" "${LIBSATURN_STAGE2D_TOOL}" ${_hsp_stage2d_package}
    COMMENT "Generating the stage data"
    VERBATIM)
