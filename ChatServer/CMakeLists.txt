# ChatServer のソースコードを集める
file(GLOB_RECURSE CHATSERVER_SRC
    src/*.cpp
)

# ChatServer 実行ファイルを定義
add_executable(ChatServer ${CHATSERVER_SRC})

# PkgConfig 依存を検出する（Linuxのみ）
if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SECRET REQUIRED libsecret-1)
    pkg_check_modules(GLIB REQUIRED glib-2.0)
endif()

# インクルードディレクトリ設定（プラットフォームごとに切り替え）
if(WIN32)
    target_include_directories(ChatServer PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/Authentication/firebase_windows
    )
else()
    target_include_directories(ChatServer PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/Authentication/firebase_linux
        ${SECRET_INCLUDE_DIRS}
        ${GLIB_INCLUDE_DIRS}
    )
endif()

# Firebaseライブラリのパスを find_library で検索
if(WIN32)
    find_library(FIREBASE_AUTH_LIB
        NAMES firebase_auth
        PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib/firebase/windows/MD/x64/Release
    )
    find_library(FIREBASE_APP_LIB
        NAMES firebase_app
        PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib/firebase/windows/MD/x64/Release
    )
else()
    find_library(FIREBASE_AUTH_LIB
        NAMES firebase_auth
        PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib/firebase/linux/x86_64/cxx11
    )
    find_library(FIREBASE_APP_LIB
        NAMES firebase_app
        PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib/firebase/linux/x86_64/cxx11
    )

    message(STATUS "FIREBASE_AUTH_LIB = ${FIREBASE_AUTH_LIB}")
    message(STATUS "FIREBASE_APP_LIB = ${FIREBASE_APP_LIB}")
endif()

# ChatServer にライブラリをリンク
target_link_libraries(ChatServer PRIVATE 
    CommonLib
    wolfssl::wolfssl     # ← 明示的に target 名を指定
    hiredis
    ${FIREBASE_AUTH_LIB}
    ${FIREBASE_APP_LIB}
)

# Linux 専用リンク
if(UNIX AND NOT APPLE)
    target_link_libraries(ChatServer PRIVATE
        uuid
        ${SECRET_LIBRARIES}
        ${GLIB_LIBRARIES}
    )
endif()

# 出力先ディレクトリの設定
set_target_properties(ChatServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
