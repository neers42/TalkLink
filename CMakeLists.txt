cmake_minimum_required(VERSION 3.10)
project(TalkLink VERSION 0.1.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GoogleTest の有効化
enable_testing()

# オプションで AuthenticationServer のビルドを切り替え
option(BUILD_AUTH_SERVER "Build the authentication server" OFF)
option(BUILD_CHAT_SERVER "Build the chat server" OFF)



#サブディレクトリを追加
# 共通ライブラリは常にビルド
add_subdirectory(CommonLib)
add_subdirectory(ThirdParty)

# ChatServer をビルドする場合
if (BUILD_CHAT_SERVER)
    add_subdirectory(ChatServer)
endif()

# AuthenticationServer をビルドする場合
if (BUILD_AUTH_SERVER)
    add_subdirectory(AuthenticationServer)
endif()

# Test
if(BUILD_TESTS)
    add_subdirectory(Tests)
endif()