cmake_minimum_required(VERSION 3.16.3)

project(imjack LANGUAGES C CXX)


set (CMAKE_C_FLAGS_RELEASE "-O3 -march=native -flto=auto -Wno-pragmas")
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})

set (CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -march=native -g -Wno-pragmas")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO})

#set (CMAKE_C_FLAGS_DEBUG "-O0 -ggdb3 -fsanitize=address -Wno-pragmas")
set (CMAKE_C_FLAGS_DEBUG "-O0 -ggdb3 -Wno-pragmas")
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})

set (CMAKE_VERBOSE_MAKEFILE TRUE)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
# https://github.com/JoeyDeVries/LearnOpenGL/tree/master/cmake/modules
find_package(GLFW3 REQUIRED)
set (OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)


# https://github.com/Pesc0/imgui-cmake
######### IMGUI #######################

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
add_library(IMGUI STATIC)

target_sources( IMGUI
                PRIVATE
                    ${IMGUI_DIR}/imgui_demo.cpp
                    ${IMGUI_DIR}/imgui_draw.cpp
                    ${IMGUI_DIR}/imgui_tables.cpp
                    ${IMGUI_DIR}/imgui_widgets.cpp
                    ${IMGUI_DIR}/imgui.cpp

                PRIVATE
                    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
                    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
#                    ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp
                )

target_include_directories( IMGUI
                            PUBLIC ${IMGUI_DIR}
                            PUBLIC ${IMGUI_DIR}/backends
                            )

#target_link_libraries(IMGUI PUBLIC ${CMAKE_DL_LIBS})
#######################################################

add_library(jacks OBJECT
    myimgui.cpp
    imgui_elements.cpp
    timer.cpp
    #testjack.cpp
    )

add_library(jackcpp STATIC
    jackcpp/src/jackaudioio.cpp
    jackcpp/src/jackblockingaudioio.cpp
    )
set_property(TARGET jackcpp PROPERTY CXX_STANDARD 11)


target_include_directories(jackcpp PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/jackcpp/include>
    $<INSTALL_INTERFACE:include/jackcpp>
    )
set_property(TARGET jackcpp PROPERTY CXX_STANDARD 11)



target_include_directories(jacks PRIVATE .)
target_include_directories(jacks PRIVATE jackcpp/include)

target_link_libraries(jacks IMGUI glfw GL jackcpp)

add_executable(imjack main.cpp)
#add_executable(imjack main.cpp $<TARGET_OBJECTS:jacks>)
target_link_libraries(imjack jacks jackcpp)
set_property(TARGET imjack PROPERTY CXX_STANDARD 11)
target_include_directories(imjack PRIVATE jackcpp/include)
target_link_libraries(imjack IMGUI ${GLFW3_LIBRARY} ${OPENGL_LIBRARIES} jack)
set_target_properties(imjack PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
