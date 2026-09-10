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
)