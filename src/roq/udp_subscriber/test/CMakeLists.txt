set(TARGET_NAME ${PROJECT_NAME}-test)

set(SOURCES buffer.cpp main.cpp)

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_NAME}-tools
          roq-server::roq-server
          roq-core::roq-core
          roq-io::roq-io
          roq-utils::roq-utils
          fmt::fmt
          ${PROJECT_NAME}-tools
          Catch2::Catch2
          ${RT_LIBRARIES})

if(ROQ_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
