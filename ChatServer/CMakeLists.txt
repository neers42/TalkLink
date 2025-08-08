# ChatServerのソースコードを集める
file(GLOB_RECURSE CHATSERVER_SRC
    src/*.cpp
)

# ChatServer 実行ファイルを生成
add_executable(ChatServer ${CHATSERVER_SRC})

# PkgConfigを使用して依存ライブラリを探す(Linuxのみ)
# UUIDを使用するための設定
if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SECRET REQUIRED libsecret-1)
    pkg_check_modules(GLIB REQUIRED glib-2.0)
endif()

# インクルードディレクトリを設定(プラットフォームごとに切り替えを実施)
target_include_directories(ChatServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SECRET_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${jwt-cpp_SOURCE_DIR}/include
)


# マクロ定義
target_compile_definitions(ChatServer PRIVATE CHAT_SERVER)

# ChatServerにライブラリをリンク
target_link_libraries(ChatServer PRIVATE 
    CommonLib
    hiredis
    ${OPENSSL_LIBRARIES}
)

# Linux固有のリンク
if(UNIX AND NOT APPLE)
    target_link_libraries(ChatServer PRIVATE
        uuid
        ${SECRET_LIBRARIES}
        ${GLIB_LIBRARIES}
    )
endif()

# 実行ファイルの出力ディレクトリを指定
set_target_properties(ChatServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)


# conf/socket_setting.ini をビルドディレクトリの bin/ にコピー
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/conf/socket_setting.ini
    ${CMAKE_BINARY_DIR}/bin/socket_setting.ini
    COPYONLY
)

# conf/db_config.ini をビルドディレクトリの bin/ にコピー
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/conf/db_config.ini
    ${CMAKE_BINARY_DIR}/bin/db_config.ini
    COPYONLY
)

