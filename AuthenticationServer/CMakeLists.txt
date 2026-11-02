#-----------------------------------------
# AuthenticationServer CMakeLists.txt
#-----------------------------------------

# AuthenticationServerのソースコードを集める(main.cppはライブラリ化しない)
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
        AUTHENTICATION_SERVER
        TALKLINK_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(auth_core PUBLIC
    CommonLib
)

# AuthenticationServer 実行ファイルを生成(main.cppのみ)
add_executable(AuthenticationServer src/main.cpp)

target_link_libraries(AuthenticationServer PRIVATE
    auth_core
)

set_target_properties(AuthenticationServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# config/auth-server/socket_setting.conf をビルドディレクトリの bin/ にコピー
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../config/auth-server/socket_setting.conf
    ${CMAKE_BINARY_DIR}/bin/socket_setting.conf
    COPYONLY
)

# config/auth-server/auth.conf をビルドディレクトリの bin/ にコピー
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../config/auth-server/auth.conf
    ${CMAKE_BINARY_DIR}/bin/auth.conf
    COPYONLY
)