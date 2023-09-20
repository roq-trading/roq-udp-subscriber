if(BUILD_TESTING)
  add_subdirectory(test)
endif()

if(BUILD_BENCHMARK)
  add_subdirectory(benchmark)
endif()

set(TARGET_NAME ${PROJECT_NAME})

add_subdirectory(flags)
add_subdirectory(tools)

set(SOURCES
    application.cpp
    config.cpp
    fbs_parser.cpp
    gateway.cpp
    incremental.cpp
    parser.cpp
    reader.cpp
    settings.cpp
    shared.cpp
    snapshot.cpp
    main.cpp)

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_NAME}-flags
          ${PROJECT_NAME}-tools
          roq-server::roq-server
          roq-server::roq-server-flags
          roq-io::roq-io
          roq-core::roq-core
          roq-utils::roq-utils
          roq-logging::roq-logging
          roq-logging::roq-logging-flags
          roq-flags::roq-flags
          roq-api::roq-api
          fmt::fmt
          absl::flat_hash_map
          absl::flat_hash_set
          aklomp::base64
          ${UUID_LIBRARIES}
          ${RT_LIBRARIES})

if(ROQ_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

target_compile_definitions(
  ${TARGET_NAME}
  PRIVATE ROQ_PACKAGE_NAME="${TARGET_NAME}" ROQ_HOST="${ROQ_HOST}"
          ROQ_BUILD_VERSION="${GIT_REPO_VERSION}" ROQ_GIT_DESCRIBE_HASH="${GIT_DESCRIBE_HASH}"
          ROQ_BUILD_NUMBER="${ROQ_BUILD_NUMBER}" ROQ_BUILD_TYPE="${ROQ_BUILD_TYPE}")

install(TARGETS ${TARGET_NAME})
