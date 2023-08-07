include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(TestExeName StrashApiClient_tests)

set(TEST_REALSTRASH OFF CACHE BOOL "compile the real strashbot.fr test set")
if (TEST_REALSTRASH)
    set(REALSTRASHTESTMACROSET "REALSTRASHTEST=1")
endif(TEST_REALSTRASH)

set(TEST_COMPOSE OFF CACHE BOOL "within the project's specific docker-compose environment")
if (TEST_COMPOSE)
    message("WUUUUUUUUUUUUUUUUUu ${TEST_COMPOSE}")
    set(COMPOSETESTMACROSET "TESTWITHCOMPOSE=1")
endif(TEST_COMPOSE)

enable_testing()
find_package(GTest REQUIRED)

set(TEST_SOURCES
    src/test_configReader.cpp
    src/test_simpleStrashBotRealGetRequest.cpp
    src/test_apiCompose.cpp
    src/test_query.cpp
    src/test_error.cpp
)
set(TEST_HEADERS
    includes/test_simpleStrashBotRealGetRequest.h
    includes/test_apiCompose.h
    includes/test_error.h
)

add_executable( ${TestExeName}
    ${TEST_SOURCES}
    ${TEST_HEADERS}
)

target_compile_definitions( ${TestExeName} PRIVATE
    ${REALSTRASHTESTMACROSET}
    ${COMPOSETESTMACROSET}
)

target_link_libraries( ${TestExeName}
    GTest::gtest_main
    ${PROJECT_NAME}
    nlohmann_json::nlohmann_json Boost::boost OpenSSL::SSL OpenSSL::Crypto
)
target_include_directories( ${TestExeName} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
    ${STRASHAPICLIENT_INCLUDES_DIR}
)
message("==> ${STRASHAPICLIENT_INCLUDES_DIR}")

file(GLOB TEST_JSON "${CMAKE_CURRENT_SOURCE_DIR}/data/*.test.json")
file(COPY ${TEST_JSON} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/data")

message( "wait ${CMAKE_CURRENT_SOURCE_DIR}/includes ; ${STRASHMONGO_INCLUDES_DIR} ?; ${CMAKE_CURRENT_BINARY_DIR} bin!" )

include(GoogleTest)
gtest_discover_tests(${TestExeName})

if (CMAKE_BUILD_TYPE MATCHES Debug)
    message("debug mode (${CMAKE_BUILD_TYPE}) => exe is '${TestExeName}'")
    set(TestExeCmd "echo \"Debug mode - run 'cd $( pwd ); gdb ./${TestExeName}' from another entry point to run a gdb debug prompt\"; read l;")
else ()
    message("release mode (${CMAKE_BUILD_TYPE}) => exe is '${TestExeName}'")
    set(TestExeCmd "./${TestExeName}")
endif (CMAKE_BUILD_TYPE MATCHES Debug)

if (TEST_COMPOSE)
    add_custom_target(run_tests
    # Set the command to run when the target is built
    COMMAND bash -c "${CMAKE_CURRENT_SOURCE_DIR}/misc/wait_lock.compose.sh ${CMAKE_BUILD_TYPE} ${TestExeCmd}"
    VERBATIM
    # Set the working directory for the command
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    # Set the description for the target
    COMMENT "Running tests… (with compose): > ${TestExeCmd}"
    )
else ()
    add_custom_target(run_tests
    # Set the command to run when the target is built
    COMMAND bash -c "${TestExeCmd}"
    VERBATIM
    # Set the working directory for the command
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    # Set the description for the target
    COMMENT "Running tests… > ${TestExeCmd}"
    )
endif (TEST_COMPOSE)
