#-----------------------------------------
# ThirdParty CMakeLists.txt
#-----------------------------------------
include(FetchContent)
cmake_minimum_required(VERSION 3.16)
project(ThirdParty LANGUAGES CXX)

#-----------------------------------------
# hiredis と jwt-cpp の取得
#-----------------------------------------
FetchContent_Declare(
    hiredis
    GIT_REPOSITORY https://github.com/redis/hiredis.git
    GIT_TAG v1.2.0
)
set(ENABLE_SSL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(hiredis)

# jwt-cpp
FetchContent_Declare(
  jwt-cpp
  GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
  GIT_TAG 08bcf77a687fb06e34138e9e9fa12a4ecbe12332
)
set(JWT_BUILD_EXAMPLES OFF CACHE BOOL "Disable examples" FORCE)
FetchContent_MakeAvailable(jwt-cpp)