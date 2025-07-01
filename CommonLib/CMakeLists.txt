include(FetchContent)


#---wolfsslの定義-------
FetchContent_Declare(
    wolfssl
    GIT_REPOSITORY https://github.com/wolfSSL/wolfssl.git
    GIT_TAG v5.8.0-stable
    CMAKE_ARGS
        -DWOLFSSL_CRYPT_TESTS=OFF
        -DBUILD_SHARED_LIBS=OFF
        -DWOLFSSL_DEBUG=OFF
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        -DWOLFSSL_CERT_EXT=ON
        -DWOLFSSL_USER_SETTINGS=${CMAKE_CURRENT_SOURCE_DIR}/external_lib/include/wolfssl/user_settings.h
)

# ビルド用に展開して有効化
FetchContent_MakeAvailable(wolfssl)

#---hiredisの定義
FetchContent_Declare(
    hiredis
    GIT_REPOSITORY https://github.com/redis/hiredis.git
    GIT_TAG v1.2.0
)

# hiredisオプション
set(ENABLE_SSL OFF CACHE BOOL "" FORCE)  
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
# ビルド用に展開して有効化
FetchContent_MakeAvailable(hiredis)



# 4. 自作コードを追加
file(GLOB_RECURSE COMMON_SRC src/*.cpp)
add_library(CommonLib STATIC ${COMMON_SRC})

target_compile_definitions(CommonLib PUBLIC NO_ULID_UINT128)

# 5. インクルードディレクトリ
target_include_directories(CommonLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external_lib/include
)

find_library(MYSQLCPPCONN_LIB
    NAMES libmysqlcppconn
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/external_lib/lib/mysql-connector-c++-9.1.0/linux/lib64
)

if(WIN32)
  target_link_libraries(CommonLib PUBLIC
    advapi32 shell32 ole32 oleaut32 odbc32 odbccp32
    wsock32 iphlpapi rpcrt4 crypt32 dbghelp bcrypt psapi userenv ws2_32
    mysqlcppconn
  )
  else()
  target_link_libraries(CommonLib PUBLIC 
    ${MYSQLCPPCONN_LIB}
    wolfssl
    hiredis
)
endif()




