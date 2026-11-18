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
    ChatServer/Domain/ChatroomIDGeneratorTest.cpp
    ChatServer/Domain/MessageValidationTest.cpp
    ChatServer/Domain/UserAuthStatusTest.cpp
    ChatServer/Domain/ChatroomSequenceHandlerTest.cpp
    ChatServer/Domain/MessageIDGeneratorTest.cpp
    ChatServer/Domain/UserAggregateStoreTest.cpp
    ChatServer/Domain/UserEntityTest.cpp
    ChatServer/Domain/ControlAndSessionMessageValidationTest.cpp
    ChatServer/Domain/UnimplementedMessageStubTest.cpp
    ChatServer/Domain/ChatroomRepositoryHandlerTest.cpp
    ChatServer/Domain/SessionHandlerTest.cpp
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
    ENVIRONMENT "CHAT_SERVER_MACHINE_ID=1"
)

# AuthenticationServerのテストをビルド(auth_coreライブラリが存在する場合のみ)
if(TARGET auth_core)
    add_executable(
        auth_tests
        AuthenticationServer/SessionTokenHandlerTest.cpp
        AuthenticationServer/AuthMessageValidationTest.cpp
        AuthenticationServer/AuthenticationServiceTest.cpp
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

# ChatServer Application層のテストをビルド(applicationライブラリが存在する場合のみ)
if(TARGET application)
    add_executable(
        application_tests
        ChatServer/Application/SyncServiceTest.cpp
        ChatServer/Application/TaskManagementServiceTest.cpp
        ChatServer/Application/ChatroomManagementServiceTest.cpp
        ChatServer/Application/ChatServiceTest.cpp
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
        ENVIRONMENT "CHAT_SERVER_MACHINE_ID=1"
    )
endif()