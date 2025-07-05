# ChatServer �̃\�[�X�R�[�h���W�߂�
file(GLOB_RECURSE CHATSERVER_SRC
    src/*.cpp
)

# ChatServer ���s�t�@�C�����`
add_executable(ChatServer ${CHATSERVER_SRC})

# PkgConfig �ˑ������o����iLinux�̂݁j
if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SECRET REQUIRED libsecret-1)
    pkg_check_modules(GLIB REQUIRED glib-2.0)
endif()

# �C���N���[�h�f�B���N�g���ݒ�i�v���b�g�t�H�[�����Ƃɐ؂�ւ��j
if(WIN32)
    target_include_directories(ChatServer PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/Authentication/firebase_windows
    )
else()
    target_include_directories(ChatServer PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/Authentication/firebase_linux
        ${CMAKE_BINARY_DIR}/_deps/wolfssl-src
        ${CMAKE_BINARY_DIR}/_deps/wolfssl-build
        ${SECRET_INCLUDE_DIRS}
        ${GLIB_INCLUDE_DIRS}
    )
endif()

# Firebase���C�u�����̃p�X�� find_library �Ō���
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

# ChatServer �Ƀ��C�u�����������N
target_link_libraries(ChatServer PRIVATE 
    CommonLib
    wolfssl     # �� �����I�� target �����w��
    hiredis
    ${FIREBASE_AUTH_LIB}
    ${FIREBASE_APP_LIB}
)

# Linux ��p�����N
if(UNIX AND NOT APPLE)
    target_link_libraries(ChatServer PRIVATE
        uuid
        ${SECRET_LIBRARIES}
        ${GLIB_LIBRARIES}
    )
endif()

# �o�͐�f�B���N�g���̐ݒ�
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

