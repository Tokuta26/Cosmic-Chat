cmake_minimum_required(VERSION 3.16)
project(Chat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/sources/*.cpp")
file(GLOB_RECURSE HEADERS_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/sources/*.h")
file(GLOB_RECURSE UI_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/sources/*.ui")
file(GLOB_RECURSE RC_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/sources/*.qrc")

add_executable(Chat ${SRC_FILES} ${HEADERS_FILES} ${UI_FILES} ${RC_FILES})
target_link_libraries(Chat PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql)

if(MSVC)
  target_compile_options(Chat PRIVATE /W4)
else()
  target_compile_options(Chat PRIVATE -Wall -Wextra)
endif()

# Copy the existing precompiled Russian translation next to the executable.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/translations/my_ru.qm")
  add_custom_command(TARGET Chat POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Chat>/translations"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_SOURCE_DIR}/translations/my_ru.qm"
      "$<TARGET_FILE_DIR:Chat>/translations/my_ru.qm")
endif()

target_include_directories(Chat PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/sources
)
set_property(SOURCE sources/serverpanel.cpp
    PROPERTY AUTOUIC_SEARCH_PATHS
    "${CMAKE_CURRENT_SOURCE_DIR}/sources"
)
if(WIN32)
    target_link_libraries(Chat PRIVATE crypt32)
endif()
