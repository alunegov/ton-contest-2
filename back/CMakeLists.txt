cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(lottery-back LANGUAGES CXX)

find_package(unofficial-http-parser CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(restinio CONFIG REQUIRED)

add_subdirectory(../_ext/ton ton)

add_executable(${PROJECT_NAME}
    main.cpp
    SpecEntry.cpp
    SpecEntry.h
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE tonlib
    PRIVATE tdactor
    PRIVATE restinio::restinio
)
if(NOT MSVC)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE pthread
    )
endif()
