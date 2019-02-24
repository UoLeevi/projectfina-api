cmake_minimum_required(VERSION 3.12)

find_package(PostgreSQL CONFIG REQUIRED)

if(NOT TARGET uo::uo_conf)
    find_package(uo_conf CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_err)
    find_package(uo_err CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_http)
    find_package(uo_http CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_jwt)
    find_package(uo_jwt CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_base64)
    find_package(uo_base64 CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_prog)
    find_package(uo_prog CONFIG REQUIRED)
endif()

add_executable(projectfina-api
    projectfina-api.c)

target_link_libraries(projectfina-api
    PRIVATE
        uo::uo_conf
        uo::uo_err
        uo::uo_http
        uo::uo_jwt
        uo::uo_base64
        uo::uo_prog
        PostgreSQL)

target_compile_features(projectfina-api
    PUBLIC
        c_std_11)

install(DIRECTORY conf/
    DESTINATION bin)

install(TARGETS projectfina-api
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)
