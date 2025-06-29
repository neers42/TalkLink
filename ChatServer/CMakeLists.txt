# ChatServer のソースコードを集める
file(GLOB_RECURSE CHATSERVER_SRC
    src/*.cpp
)

# ChatServer 実行ファイルを定義
add_executable(ChatServer ${CHATSERVER_SRC})

# インクルードディレクトリ（ChatServer固有のものを指定）
target_include_directories(ChatServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Authentication
)

target_link_directories(ChatServer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)
# CommonLib をリンク（CommonLib 側で外部ライブラリも設定済み）
target_link_libraries(ChatServer PRIVATE 
    CommonLib
    firebase/MD/x64/Release

)

# 出力先ディレクトリの設定（任意）
set_target_properties(ChatServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)


# conf/socket_setting.ini を実行ファイルの出力先にコピー（任意）
#configure_file(
#    ${CMAKE_CURRENT_SOURCE_DIR}/conf/socket_setting.ini
#    ${CMAKE_BINARY_DIR}/bin/socket_setting.ini
#    COPYONLY
#)