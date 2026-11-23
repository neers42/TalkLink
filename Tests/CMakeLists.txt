include(FetchContent)

#GoogleTestの取得
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)

# GoogleTestのインストールを無効化
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

# GoogleTestのビルドを有効化
FetchContent_MakeAvailable(googletest)

# Domainのテストをビルド
# テストしたいソースファイルを指定
add_executable(
    domain_tests
    ChatService/Domain/ChatroomIDGeneratorTest.cpp
    ChatService/Domain/MessageValidationTest.cpp
    ChatService/Domain/UserAuthStatusTest.cpp
    ChatService/Domain/ChatroomSequenceHandlerTest.cpp
    ChatService/Domain/MessageIDGeneratorTest.cpp
    ChatService/Domain/UserAggregateStoreTest.cpp
    ChatService/Domain/UserEntityTest.cpp
    ChatService/Domain/ControlAndSessionMessageValidationTest.cpp
    ChatService/Domain/UnimplementedMessageStubTest.cpp
    ChatService/Domain/ChatroomRepositoryHandlerTest.cpp
    ChatService/Domain/SessionHandlerTest.cpp
    ChatService/Domain/GrpcSessionRepositoryTest.cpp
)

# Domainのテストに必要なインクルードディレクトリを設定
target_link_libraries(
    domain_tests
    PRIVATE
    domain
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(
    domain_tests
    PROPERTIES
    LABELS "domain"
    ENVIRONMENT "CHAT_SERVICE_MACHINE_ID=1"
)

# AuthenticationServiceのテストをビルド(auth_coreライブラリが存在する場合のみ)
if(TARGET auth_core)
    add_executable(
        auth_tests
        AuthenticationService/SessionTokenHandlerTest.cpp
        AuthenticationService/AuthMessageValidationTest.cpp
        AuthenticationService/AuthenticationUseCaseTest.cpp
    )

    target_link_libraries(
        auth_tests
        PRIVATE
        auth_core
        GTest::gtest
        GTest::gtest_main
    )

    gtest_discover_tests(
        auth_tests
        PROPERTIES
        LABELS "auth"
    )
endif()

# ChatService Application層のテストをビルド(applicationライブラリが存在する場合のみ)
if(TARGET application)
    add_executable(
        application_tests
        ChatService/Application/SyncUseCaseTest.cpp
        ChatService/Application/TaskSchedulerTest.cpp
        ChatService/Application/ChatroomUseCaseTest.cpp
        ChatService/Application/ChatUseCaseTest.cpp
    )

    target_link_libraries(
        application_tests
        PRIVATE
        application
        GTest::gtest
        GTest::gtest_main
    )

    gtest_discover_tests(
        application_tests
        PROPERTIES
        LABELS "application"
        ENVIRONMENT "CHAT_SERVICE_MACHINE_ID=1"
    )
endif()