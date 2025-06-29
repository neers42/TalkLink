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
#    ${CMAKE_CURRENT_SOURCE_DIR}/external_lib/mysql-connector-c++-9.1.0/lib64/vs14
    ${CMAKE_CURRENT_SOURCE_DIR}/external_lib/boost/x64/lib
#    ${CMAKE_CURRENT_SOURCE_DIR}/external_lib/lib/wolfssl/x64
#    ${CMAKE_CURRENT_SOURCE_DIR}/external_lib/lib/hiredis/x64
)
find_library(MYSQLCPPCONN_LIB mysqlcppconn PATHS ${CMAKE_CURRENT_SOURCE_DIR}/external_lib/lib/wolfssl/x64)
find_library(WOLFSSL_LIB wolfssl PATHS ${CMAKE_CURRENT_SOURCE_DIR}/external_lib/lib/wolfssl/x64)
find_library(HIREDIS_LIB hiredis PATHS ${CMAKE_CURRENT_SOURCE_DIR}/external_lib/lib/hiredis/x64)

if(WIN32)
  target_link_libraries(CommonLib PUBLIC
    advapi32 shell32 ole32 oleaut32 uuid odbc32 odbccp32
    wsock32 iphlpapi Rpcrt4 crypt32 dbghelp bcrypt psapi userenv ws2_32
  )
  else()
  target_link_libraries(CommonLib PUBLIC 
    mysqlcppconn
    uuid
    firebase_auth
    firebase_app
    rpcrt4
    icu
    mysqlcppconn
    mysqlcppconn-static
    ${WOLFSSL_LIB}
    ${HIREDIS_LIB}
)
endif()



