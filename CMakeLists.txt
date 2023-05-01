# Configuração para funcionar com x64 (Caso use VS): Em Debug->Options->Cmake desabilitar o Use CMakePresets.
# assim ele gera o CMakeSettings e o VCPKG consegue configurar a build para TRIPLET x64 

# Utilizando stb_image.h baixado nos pacotes do vcpkg usando o cmake funciona basicamente
# da mesma forma que se tivesse em uma pasta do sistema, o vcpkg basicamente configura a 
# Stb_INCLUDE_DIR para o caminho do sbt_image.h

cmake_minimum_required (VERSION 3.13.4)
set (CMAKE_CXX_STANDARD 17)
project (BreakOut)

file(GLOB SOURCES "src/*.cpp" "src/*.c" "include/*.hpp" "include/*.h") 

add_executable (BreakOut ${SOURCES})


include_directories(BreakOut include "${PROJECT_SOURCE_DIR}/include/irrKlang/include")

find_library(IRRKLANG_LIB irrKlang NAMES irrKlang.lib PATHS  "${PROJECT_SOURCE_DIR}/include/irrKlang"
    PATH_SUFFIXES "lib/Winx64-visualStudio" "bin/winx64-visualStudio" 
)

if(${IRRKLANG_LIB} STREQUAL IRRKLANG_LIB-NOTFOUND)
    message(FATAL_ERROR "no irrKlang")
else()
    message("irrKlang found as ${IRRKLANG_LIB}")
endif()

add_custom_command(TARGET BreakOut POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${PROJECT_SOURCE_DIR}/include/irrKlang/bin/winx64-visualStudio"
    $<TARGET_FILE_DIR:BreakOut>
)

find_package(glfw3 REQUIRED)

find_package(glm REQUIRED)

find_package(glad REQUIRED)

find_package(OpenGL REQUIRED)


target_link_libraries(BreakOut PRIVATE glfw glm::glm glad::glad ${IRRKLANG_LIB} ${OPENGL_LIBRARY})



# Desabilitar Windows Console
if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(BreakOut PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS")
endif()

