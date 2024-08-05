cmake_minimum_required(VERSION 3.10)

option(ADD_BASIC "Creates example basic" OFF)

if(ADD_BASIC)
    add_subdirectory(basic_scan)
    add_subdirectory(extended_scan)
endif()
