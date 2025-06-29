cmake_minimum_required(VERSION 3.20)
project(MyChat VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#サブディレクトリの追加
add_subdirectory(CommonLib)
add_subdirectory(ChatServer)
add_subdirectory(AuthenticationServer)