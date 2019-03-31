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
if(NOT TARGET uo::uo_prog)
    find_package(uo_prog CONFIG REQUIRED)
endif()

add_executable(projectfina-api
    projectfina-api.c
    uo_http_util.c
    uo_pg.c
    v01/groups.c
    v01/instruments.c
    v01/markets.c
    v01/notes.c
    v01/watchlists.c)

target_include_directories(projectfina-api
    PRIVATE
        include)

target_link_libraries(projectfina-api
    PRIVATE
        uo::uo_conf
        uo::uo_err
        uo::uo_http
        uo::uo_jwt
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
