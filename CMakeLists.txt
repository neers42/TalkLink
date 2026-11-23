cmake_minimum_required(VERSION 3.10)
project(TalkLink VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GoogleTest の有効化
enable_testing()

# オプションで AuthenticationService のビルドを切り替え
option(BUILD_AUTH_SERVICE "Build the authentication service" OFF)
option(BUILD_CHAT_SERVICE "Build the chat service" OFF)



#サブディレクトリを追加
# 共通ライブラリは常にビルド
add_subdirectory(CommonLib)
add_subdirectory(ThirdParty)

# AuthenticationService<->ChatService間のgRPC API用スタブ。
# 両サーバーが参照する可能性があるため常にビルドする。
add_subdirectory(Proto)

# ChatService をビルドする場合
if (BUILD_CHAT_SERVICE)
    add_subdirectory(ChatService)
endif()

# AuthenticationService をビルドする場合
if (BUILD_AUTH_SERVICE)
    add_subdirectory(AuthenticationService)
endif()

# Test
if(BUILD_TESTS)
    add_subdirectory(Tests)
endif()