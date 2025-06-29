# 自作コード
file(GLOB_RECURSE COMMON_SRC src/*.cpp)

add_library(CommonLib STATIC ${COMMON_SRC})

# インクルードディレクトリ（自作 + サードパーティ）
target_include_directories(CommonLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external_lib/include
)

# サードパーティライブラリをリンク（必要に応じて）
# - DLLではなく.lib/.a を使う前提です（動的なら .dll の配置も別途必要）
target_link_directories(CommonLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external_lib/lib
)

# 例: MySQL Connector/C++ のライブラリ名（変更可）
target_link_libraries(CommonLib PUBLIC 
    mysqlcppconn
    advapi32
    shell32
    ole32
    oleaut32
    uuid
    odbc32
    odbccp32
    wsock32
    iphlpapi
    Rpcrt4
    firebase_auth
    firebase_app
    crypt32
    rpcrt4
    dbghelp
    bcrypt
    psapi
    userenv
    icu
    ws2_32
    mysqlcppconn
    mysqlcppconn-static
    wolfssl
    hiredis
)
