cmake_minimum_required(VERSION 3.20)
project(scoundrel)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
        src/scoundrel.cpp
        src/log.cpp
        src/application.cpp
        src/renderer.cpp
        src/camera.cpp
        src/texture.cpp
        src/mouse.cpp
        src/keyboard.cpp
        src/audio.cpp
        libraries/glad/src/glad.c
        src/log.h
        src/application.h
        src/renderer.h
        src/camera.h
        src/texture.h
        src/mouse.h
        src/keyboard.h
        src/audio.h
        src/random.h
        src/typedefs.h
)

file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS "shaders/*.glsl")
file(GLOB_RECURSE ASSET_FILES CONFIGURE_DEPENDS "assets/*")
set(PROJECT_FILES .gitignore .projectile build.bat build.sh README.md IDEAS.md CLAUDE.md LICENSE)

set_source_files_properties(
        ${SHADER_FILES} ${ASSET_FILES} ${PROJECT_FILES}
        PROPERTIES HEADER_FILE_ONLY TRUE
)

add_executable(scoundrel ${SOURCES} ${SHADER_FILES} ${ASSET_FILES} ${PROJECT_FILES})

target_include_directories(scoundrel PRIVATE
        libraries/SDL-release-3.4.8/include
        libraries/glad/include
        libraries/glm-1.0.1-light
        libraries/stb
)

target_link_directories(scoundrel PRIVATE
        libraries/SDL-release-3.4.8
)

target_link_libraries(scoundrel PRIVATE SDL3)

if(MSVC)
    target_compile_options(scoundrel PRIVATE /WX /Zi /Od)
    target_link_options(scoundrel PRIVATE /SUBSYSTEM:CONSOLE)
else()
    target_compile_options(scoundrel PRIVATE -Werror)
endif()

add_custom_command(TARGET scoundrel POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:scoundrel>/assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:scoundrel>/shaders
)