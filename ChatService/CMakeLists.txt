#-----------------------------------------
# ChatService CMakeLists.txt
#-----------------------------------------

# Application層のソースコードを集める
file(GLOB_RECURSE APPLICATION_SRC
    src/Application/*.cpp
)

# Domain層のソースコードを集める
file(GLOB_RECURSE DOMAIN_SRC
    src/Domain/*.cpp
)

# Infrastructure層のソースコードを集める
file(GLOB_RECURSE INFRASTRUCTURE_SRC
    src/Infrastructure/*.cpp
)

# ChatServiceのソースコードを集める
file(GLOB_RECURSE CHATSERVICE_ENTRY_SRC
    src/ChatServiceFacade.cpp
    src/main.cpp
)

# Application層の静的ライブラリを作成
add_library(application STATIC ${APPLICATION_SRC})

# Domain層の静的ライブラリを作成
add_library(domain STATIC ${DOMAIN_SRC})

# Infrastructure層の静的ライブラリを作成
add_library(infrastructure STATIC ${INFRASTRUCTURE_SRC})

# ChatService 実行ファイルを生成
add_executable(ChatService ${CHATSERVICE_ENTRY_SRC})

# PkgConfigを使用して依存ライブラリを探す(Linuxのみ)
# UUIDを使用するための設定
if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SECRET REQUIRED libsecret-1)
    pkg_check_modules(GLIB REQUIRED glib-2.0)
endif()

# インクルードディレクトリを設定(プラットフォームごとに切り替えを実施)
# ChatServiceのインクルードディレクトリを設定
target_include_directories(
    ChatService PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SECRET_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${jwt-cpp_SOURCE_DIR}/include
)

# Applicationのインクルードディレクトリを設定
target_include_directories(
    application
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Domainのインクルードディレクトリを設定
target_include_directories(
    domain
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Infrastructureのインクルードディレクトリを設定
target_include_directories(
    infrastructure
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# マクロ定義
target_compile_definitions(
    ChatService 
    PRIVATE 
    CHAT_SERVICE
    TALKLINK_VERSION="${PROJECT_VERSION}" )

# ChatServiceにライブラリをリンク
target_link_libraries(
    ChatService 
    PRIVATE
    application
    CommonLib
)

# Appication, Domain, Infrastructureのライブラリにリンク
target_link_libraries(
    application 
    PUBLIC
    domain
    CommonLib
)

target_link_libraries(
    domain 
    PUBLIC
    infrastructure
    CommonLib
)

target_link_libraries(
    infrastructure
    PUBLIC
    CommonLib
    hiredis
    ${OPENSSL_LIBRARIES}
    session_proto
)

# Linux固有のリンク
if(UNIX AND NOT APPLE)
    target_link_libraries(ChatService PRIVATE
        uuid
        ${SECRET_LIBRARIES}
        ${GLIB_LIBRARIES}
    )
endif()

# 実行ファイルの出力ディレクトリを指定
set_target_properties(ChatService PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)


# 設定ファイルをビルドディレクトリの bin/ にコピー
set(CONFIG_FILES
    socket_setting.conf
    chat.conf
)

foreach(CONFIG_FILE ${CONFIG_FILES})
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/../config/chat-service/${CONFIG_FILE}
        ${CMAKE_BINARY_DIR}/bin/${CONFIG_FILE}
        COPYONLY
    )
endforeach()

