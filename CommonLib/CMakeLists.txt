include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force static build" FORCE)

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
        -DENABLE_OPENSSL_EXTRA=ON
        -DENABLE_X509=ON
        -DENABLE_CERTGEN=ON
        -DENABLE_CERTREQ=ON
        -DWOLFSSL_OPENSSL_ALL=ON         # OpenSSL互換APIを追加
        -DWOLFSSL_CERT_EXT=ON            # 証明書拡張を有効化
        -DHAVE_X509=ON                   # X.509サポートを有効化
        -DKEEP_PEER_CERT=ON              # 相手の証明書を保持（X509の処理に必要）
)

FetchContent_MakeAvailable(wolfssl)

# wolfsslのターゲットからインクルードディレクトリを取得（失敗する可能性がある）
get_target_property(WOLFSSL_INCLUDE_DIR wolfssl INTERFACE_INCLUDE_DIRECTORIES)

message(STATUS "wolfssl source dir = ${wolfssl_SOURCE_DIR}")
message(STATUS "wolfssl include dir (from target) = ${WOLFSSL_INCLUDE_DIR}")

# 失敗した場合は手動でインクルードパスを設定
if(WOLFSSL_INCLUDE_DIR STREQUAL "" OR NOT WOLFSSL_INCLUDE_DIR)
    set(WOLFSSL_INCLUDE_DIR
        ${wolfssl_SOURCE_DIR}
        ${wolfssl_SOURCE_DIR}/wolfssl
        ${wolfssl_SOURCE_DIR}/wolfcrypt
    )
    message(STATUS "wolfssl include dir manually set to: ${WOLFSSL_INCLUDE_DIR}")
endif()


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

# 4. 共通コードを追加
file(GLOB_RECURSE COMMON_SRC src/*.cpp)
add_library(CommonLib STATIC ${COMMON_SRC})

target_compile_definitions(CommonLib PUBLIC NO_ULID_UINT128)

# 5. インクルードディレクトリ設定
target_include_directories(CommonLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external_lib/include/wolfssl      # 独自のヘッダー群
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external_lib/include
    ${wolfssl_SOURCE_DIR}                     # wolfssl のトップ
    ${wolfssl_SOURCE_DIR}/wolfssl             # 独自のヘッダーフォルダ
    ${wolfssl_SOURCE_DIR}/wolfcrypt           # 独自のヘッダーフォルダ
)

find_library(MYSQLCPPCONN_LIB
    NAMES mysqlcppconn
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