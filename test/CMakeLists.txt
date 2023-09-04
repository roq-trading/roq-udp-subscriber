set(TARGET_NAME ${PROJECT_NAME}-test)

set(SOURCES buffer.cpp main.cpp)

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(
  ${TARGET_NAME}
  PUBLIC roq-server::roq-server roq-core::roq-core fmt::fmt
  PRIVATE ${PROJECT_NAME}-tools Catch2::Catch2 ${RT_LIBRARIES})

if(ROQ_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
