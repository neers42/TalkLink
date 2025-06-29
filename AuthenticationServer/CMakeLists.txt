file(GLOB_RECURSE AUTHSERVER_SRC src/*.cpp)

add_executable(AuthenticationServer ${AUTHSERVER_SRC})

target_include_directories(AuthenticationServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/CommonLib/src
)

target_link_libraries(AuthenticationServer PRIVATE CommonLib)

set_target_properties(AuthenticationServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)