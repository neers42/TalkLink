#-----------------------------------------
# CommonLib CMakeLists.txt
#-----------------------------------------
option(WITH_MYSQL "Enable MySQL support" OFF)  # デフォルトOFFに
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force static build" FORCE)

# OpenSSLの取得(SSL/TLS通信のライブラリ)
find_package(OpenSSL REQUIRED MODULE)

#-----------------------------------------
# 共通コードの追加
#-----------------------------------------
file(GLOB_RECURSE COMMON_SRC src/*.cpp)
add_library(CommonLib STATIC ${COMMON_SRC})
target_compile_definitions(CommonLib PUBLIC NO_ULID_UINT128)

#-----------------------------------------
# includeディレクトリ設定
#-----------------------------------------
target_include_directories(CommonLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/include
    ${OPENSSL_INCLUDE_DIR}
)

#-----------------------------------------
# MySQL Connector/C++ ライブラリの指定
#-----------------------------------------
if(WITH_MYSQL)
    find_library(MYSQLCPPCONN_LIB
        NAMES mysqlcppconn
        PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/lib/mysql-connector-c++-9.1.0/linux/lib64
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
    jwt-cpp
  )
endif()

# WITH_MYSQL が ON の場合は MYSQL ライブラリを追加
if(WITH_MYSQL)
  list(APPEND PLATFORM_LIBS ${MYSQLCPPCONN_LIB})
endif()

#-----------------------------------------
# リンク設定
#-----------------------------------------
target_link_libraries(CommonLib PUBLIC ${PLATFORM_LIBS} )
target_link_libraries(CommonLib PUBLIC ${OPENSSL_LIBRARIES})