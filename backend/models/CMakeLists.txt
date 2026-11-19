

set(MODEL_SRC_DIR   ${CMAKE_CURRENT_SOURCE_DIR})
set(MODEL_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(MODEL_TEMPLATE  ${MODEL_SRC_DIR}/model.json.in)
set(MODEL_BLD_JSON  ${MODEL_BUILD_DIR}/model.json)

set(DB_FILE ${CMAKE_CURRENT_BINARY_DIR}/mydb.db)

file(MAKE_DIRECTORY ${MODEL_BUILD_DIR})

configure_file(${MODEL_TEMPLATE} ${MODEL_BLD_JSON} @ONLY)

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${MODEL_TEMPLATE})

set(GENERATED_MODELS
    ${MODEL_BUILD_DIR}/Users.h
    ${MODEL_BUILD_DIR}/Users.cc
)

if(WIN32)
    add_custom_command(
        OUTPUT ${GENERATED_MODELS}
        COMMAND powershell -NoProfile -Command
                "'y' | & '${DROGON_CTL_EXECUTABLE}' create model '${MODEL_BUILD_DIR}'"
        WORKING_DIRECTORY ${MODEL_BUILD_DIR}
        DEPENDS ${MODEL_BLD_JSON} ${MODEL_TEMPLATE}
        COMMENT "Generate ORM model from ${MODEL_BUILD_DIR}"
        VERBATIM
    )
else()
    add_custom_command(
        OUTPUT ${GENERATED_MODELS}
        COMMAND ${CMAKE_COMMAND} -E rm -f ${GENERATED_MODELS}
        COMMAND ${DROGON_CTL_EXECUTABLE} create model ${MODEL_BUILD_DIR}
        WORKING_DIRECTORY ${MODEL_BUILD_DIR}
        DEPENDS ${MODEL_BLD_JSON} ${MODEL_TEMPLATE}
        COMMENT "Generate ORM model from ${MODEL_BUILD_DIR}"
    )
endif()

add_custom_target(generate_models DEPENDS ${GENERATED_MODELS})

add_dependencies(${PROJECT_NAME} generate_models)

target_sources(${PROJECT_NAME} PRIVATE ${MODEL_BUILD_DIR}/Users.cc)

target_include_directories(${PROJECT_NAME} PRIVATE ${MODEL_BUILD_DIR})