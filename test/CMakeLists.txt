set(TARGET_NAME "${PROJECT_NAME}-test")

set(SOURCES dummy.cpp main.cpp)

add_executable("${TARGET_NAME}" ${SOURCES})

target_link_libraries("${TARGET_NAME}" roq-server::roq-server roq-core::roq-core fmt::fmt
                      Catch2::Catch2 ${RT_LIBRARIES})

target_compile_features("${TARGET_NAME}" PUBLIC cxx_std_20)

set_target_properties("${TARGET_NAME}" PROPERTIES LINK_FLAGS_RELEASE -s)

# run

add_test(NAME "${TARGET_NAME}" COMMAND "${TARGET_NAME}")
