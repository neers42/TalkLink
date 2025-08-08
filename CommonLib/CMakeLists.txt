include(FetchContent)
option(WITH_MYSQL "Enable MySQL support" OFF)  # デフォルトOFFに
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force static build" FORCE)

find_package(OpenSSL REQUIRED MODULE)


#---hiredisの取得
FetchContent_Declare(
    hiredis
    GIT_REPOSITORY https://github.com/redis/hiredis.git
    GIT_TAG v1.2.0
)

# hiredisオプション設定
set(ENABLE_SSL OFF CACHE BOOL "" FORCE)  
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
# ビルド時に自動で取得・ビルド
FetchContent_MakeAvailable(hiredis)

#jwt-cppの取得(jwtトークンのライブラリ)
FetchContent_Declare(
  jwt-cpp
  GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
  GIT_TAG 08bcf77a687fb06e34138e9e9fa12a4ecbe12332 # v0.7.0など安定版タグ指定
)
set(JWT_BUILD_EXAMPLES OFF CACHE BOOL "Disable examples" FORCE)
FetchContent_MakeAvailable(jwt-cpp)

# 4. 共通コードを追加
file(GLOB_RECURSE COMMON_SRC src/*.cpp)
add_library(CommonLib STATIC ${COMMON_SRC})

target_compile_definitions(CommonLib PUBLIC NO_ULID_UINT128)

# 5. インクルードディレクトリ設定
target_include_directories(CommonLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external_lib/include
     ${OPENSSL_INCLUDE_DIR}
)

#MYSQL Connector/C++のライブラリを指定
if(WITH_MYSQL)
    find_library(MYSQLCPPCONN_LIB
        NAMES mysqlcppconn
        PATHS ${CMAKE_CURRENT_SOURCE_DIR}/external_lib/lib/mysql-connector-c++-9.1.0/linux/lib64
    )
    if(NOT MYSQLCPPCONN_LIB)
        message(FATAL_ERROR "MySQL Connector/C++ library not found!")
    endif()
endif()


if(WIN32)
  set(PLATFORM_LIBS 
    advapi32 shell32 ole32 oleaut32 odbc32 odbccp32
    wsock32 iphlpapi rpcrt4 crypt32 dbghelp bcrypt psapi userenv ws2_32
  )
else()
  set(PLATFORM_LIBS 
    hiredis
  )
endif()

# WITH_MYSQL が ON の場合は MYSQL ライブラリを追加
if(WITH_MYSQL)
  list(APPEND PLATFORM_LIBS ${MYSQLCPPCONN_LIB})
endif()

# CommonLib にまとめてリンク
target_link_libraries(CommonLib PUBLIC ${PLATFORM_LIBS} )
target_link_libraries(CommonLib PUBLIC ${OPENSSL_LIBRARIES})