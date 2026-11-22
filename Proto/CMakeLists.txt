#-----------------------------------------
# Proto CMakeLists.txt
# session.protoからC++のメッセージ/gRPCスタブを生成し、session_protoという
# 静的ライブラリとして公開する。AuthenticationServer(サーバー側実装)と
# ChatServer(クライアント側実装)の両方がこれをリンクする。
#-----------------------------------------

find_package(PkgConfig REQUIRED)
pkg_check_modules(GRPC REQUIRED IMPORTED_TARGET grpc++)
pkg_check_modules(PROTOBUF REQUIRED IMPORTED_TARGET protobuf)

find_program(PROTOC_EXECUTABLE protoc REQUIRED)
find_program(GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin REQUIRED)

set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/session.proto)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

set(PROTO_SRCS ${GENERATED_DIR}/session.pb.cc)
set(PROTO_HDRS ${GENERATED_DIR}/session.pb.h)
set(GRPC_SRCS ${GENERATED_DIR}/session.grpc.pb.cc)
set(GRPC_HDRS ${GENERATED_DIR}/session.grpc.pb.h)

add_custom_command(
    OUTPUT ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS}
    COMMAND ${PROTOC_EXECUTABLE}
    ARGS --cpp_out=${GENERATED_DIR}
         --grpc_out=${GENERATED_DIR}
         --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
         -I ${CMAKE_CURRENT_SOURCE_DIR}
         ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating C++ protobuf/gRPC stubs from session.proto"
    VERBATIM
)

add_library(session_proto STATIC ${PROTO_SRCS} ${GRPC_SRCS})

target_include_directories(session_proto PUBLIC ${GENERATED_DIR})

target_link_libraries(session_proto PUBLIC
    PkgConfig::GRPC
    PkgConfig::PROTOBUF
)
