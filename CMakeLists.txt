cmake_minimum_required (VERSION 2.6)

project (loglang)

add_definitions(--std=c++11 -O2)

add_subdirectory(src)
add_subdirectory(examples)
