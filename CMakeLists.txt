cmake_minimum_required(VERSION 3.20)
project(MyChat VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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