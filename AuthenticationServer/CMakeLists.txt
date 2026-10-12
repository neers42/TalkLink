#-----------------------------------------
# AuthenticationServer CMakeLists.txt
#-----------------------------------------

# AuthenticationServerのソースコードを集める
file(GLOB_RECURSE AUTHENTICATIONSERVER_SRC 
    src/*.cpp
)


# AuthenticationServer 実行ファイルを生成
add_executable(AuthenticationServer ${AUTHENTICATIONSERVER_SRC})

# インクルードディレクトリを設定(プラットフォームごとに切り替えを実施)
target_include_directories(AuthenticationServer PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${jwt-cpp_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/CommonLib/src
)

# マクロ定義
target_compile_definitions(
    AuthenticationServer
    PRIVATE
        AUTHENTICATION_SERVER
        TALKLINK_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(AuthenticationServer PRIVATE 
    CommonLib
)

set_target_properties(AuthenticationServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# config/auth-server/socket_setting.ini をビルドディレクトリの bin/ にコピー
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../config/auth-server/socket_setting.ini
    ${CMAKE_BINARY_DIR}/bin/socket_setting.ini
    COPYONLY
)