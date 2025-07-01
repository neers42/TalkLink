# ChatServer のソースコードを集める
file(GLOB_RECURSE CHATSERVER_SRC
    src/*.cpp
)

# ChatServer 実行ファイルを定義
add_executable(ChatServer ${CHATSERVER_SRC})

# インクルードディレクトリ（ChatServer固有のものを指定）
if(WIN32)
    target_include_directories(ChatServer PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/Authentication/firebase_windows
    )
    else()
    target_include_directories(ChatServer PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/Authentication/firebase_linux
    )
endif()

target_link_directories(ChatServer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)
find_library(FIREBASE_AUTH_LIB
    NAMES libfirebase_auth
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib/firebase/linux/x86_64/cxx11
)

find_library(FIREBASE_APP_LIB
    NAMES libfirebase_app
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib/firebase/linux/x86_64/cxx11
)
# CommonLib をリンク（CommonLib 側で外部ライブラリも設定済み）
target_link_libraries(ChatServer PRIVATE 
    CommonLib
    ${FIREBASE_AUTH_LIB}
    ${FIREBASE_APP_LIB}

)

# 出力先ディレクトリの設定（任意）
set_target_properties(ChatServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

