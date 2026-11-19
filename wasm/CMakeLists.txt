find_package(unofficial-noise CONFIG REQUIRED)
find_package(unofficial-noiseutils CONFIG REQUIRED)
find_package(minizip CONFIG REQUIRED)

set(noise "${PROJECT_SOURCE_DIR}/backend/projects/NoiseGenerator/noise")
add_executable(tools tools.cpp "${noise}/NoiseMaker.cpp" "${noise}/NoiseRender.cpp")
target_compile_features(tools PRIVATE cxx_std_20)
target_include_directories(tools PRIVATE "${PROJECT_SOURCE_DIR}/backend" "${noise}")
target_link_libraries(tools PRIVATE unofficial::noise::noise-static unofficial::noiseutils::noiseutils-static MINIZIP::minizipstatic)
target_compile_options(tools PRIVATE -fexceptions)
target_link_options(tools PRIVATE --no-entry -fexceptions
    -sMODULARIZE=1 -sEXPORT_ES6=1 -sALLOW_MEMORY_GROWTH=1
    -sEXPORTED_RUNTIME_METHODS=FS,cwrap,HEAPU8
    -sEXPORTED_FUNCTIONS=_malloc,_free,_zipWriteInFileInZip,_zipCloseFileInZip,_zipClose)
set(wasm_assets "${PROJECT_SOURCE_DIR}/static/wasm")
set_target_properties(tools PROPERTIES SUFFIX ".js" RUNTIME_OUTPUT_DIRECTORY "${wasm_assets}")
add_custom_command(TARGET tools POST_BUILD
    BYPRODUCTS "${wasm_assets}/tools.wasm"
    COMMAND "${CMAKE_COMMAND}" -E true)
foreach(library libnoise minizip zlib)
    configure_file("${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/${library}/copyright"
                   "${wasm_assets}/${library}-license.txt" COPYONLY)
endforeach()
