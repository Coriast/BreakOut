# Configuração para funcionar com x64 (Caso use VS): Em Debug->Options->Cmake desabilitar o Use CMakePresets.
# assim ele gera o CMakeSettings e o VCPKG consegue configurar a build para TRIPLET x64 

# Utilizando stb_image.h baixado nos pacotes do vcpkg usando o cmake funciona basicamente
# da mesma forma que se tivesse em uma pasta do sistema, o vcpkg basicamente configura a 
# Stb_INCLUDE_DIR para o caminho do sbt_image.h

cmake_minimum_required (VERSION 3.8)
set (CMAKE_CXX_STANDARD 17)
project (BreakOut)

include_directories(include "${PROJECT_SOURCE_DIR}/include/irrKlang/include") 

file(GLOB SOURCES "src/*.cpp" "src/*.c" "include/*.hpp" "include/*.h" ) 


add_executable (BreakOut ${SOURCES})

find_package(glfw3 REQUIRED)

find_package(glm REQUIRED)

find_package(glad REQUIRED)

find_package(OpenGL REQUIRED)


target_link_libraries(BreakOut PRIVATE glfw glm::glm glad::glad "${PROJECT_SOURCE_DIR}/include/irrKlang/lib" ${OPENGL_LIBRARY})


# Desabilitar Windows Console
if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(BreakOut PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS")
endif()

