#-----------------------------------------
# AuthenticationService CMakeLists.txt
#-----------------------------------------

# AuthenticationServiceのソースコードを集める(main.cppはライブラリ化しない)
file(GLOB_RECURSE AUTH_CORE_SRC
    src/*.cpp
)
list(REMOVE_ITEM AUTH_CORE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

# main.cpp以外を静的ライブラリ化する(Testsからリンクできるようにするため)
add_library(auth_core STATIC ${AUTH_CORE_SRC})

# インクルードディレクトリを設定(プラットフォームごとに切り替えを実施)
target_include_directories(auth_core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${jwt-cpp_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/CommonLib/src
)

# マクロ定義
target_compile_definitions(
    auth_core
    PUBLIC
        AUTHENTICATION_SERVICE
        TALKLINK_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(auth_core PUBLIC
    CommonLib
    session_proto
)

# AuthenticationService 実行ファイルを生成(main.cppのみ)
add_executable(AuthenticationService src/main.cpp)

target_link_libraries(AuthenticationService PRIVATE
    auth_core
)

set_target_properties(AuthenticationService PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# config/auth-service/socket_setting.conf をビルドディレクトリの bin/ にコピー
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../config/auth-service/socket_setting.conf
    ${CMAKE_BINARY_DIR}/bin/socket_setting.conf
    COPYONLY
)

# config/auth-service/auth.conf をビルドディレクトリの bin/ にコピー
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../config/auth-service/auth.conf
    ${CMAKE_BINARY_DIR}/bin/auth.conf
    COPYONLY
)