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
    advapi32.lib
    shell32.lib
    ole32.lib
    oleaut32.lib
    uuid.lib
    odbc32.lib
    odbccp32.lib
    wsock32.lib
    iphlpapi.lib
    Rpcrt4.lib
    firebase_auth.lib
    firebase_app.lib
    firebase_app_check.lib
    firebase_firestore.lib
    firebase_functions.lib
    firebase_database.lib
    firebase_remote_config.lib
    firebase_storage.lib
    firebase_analytics.lib
    firebase_dynamic_links.lib
    firebase_gma.lib
    firebase_installations.lib
    firebase_messaging.lib
    crypt32.lib
    rpcrt4.lib
    dbghelp.lib
    bcrypt.lib
    psapi.lib
    userenv.lib
    icu.lib
    ws2_32.lib
    Ws2_32.lib
    mysqlcppconn.lib
    mysqlcppconn-static.lib
    wolfssl.lib
    hiredis.lib
)
