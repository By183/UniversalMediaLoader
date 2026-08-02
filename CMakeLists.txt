cmake_minimum_required(VERSION 3.16)
project(UniversalMediaLoader VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_PREFIX_PATH AND EXISTS "C:/Qt/6.7.0/msvc2019_64")
    list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.7.0/msvc2019_64")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets Network REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/SidebarWidget.cpp
    src/ToggleSwitch.cpp
    src/MainPanel.cpp
    src/CommandBuilder.cpp
    src/ProgressParser.cpp
    src/DownloadProgressAggregator.cpp
    src/ProcessRunner.cpp
    src/EngineUpdater.cpp
    src/EmbeddedToolsManager.cpp
)

set(HEADERS
    include/MainWindow.h
    include/SidebarWidget.h
    include/ToggleSwitch.h
    include/MainPanel.h
    include/CommandBuilder.h
    include/ProgressParser.h
    include/DownloadProgressAggregator.h
    include/ProcessRunner.h
    include/EngineUpdater.h
    include/EmbeddedToolsManager.h
)

set(RESOURCES
    resources/resources.qrc
    resources/app.rc
)

add_executable(UniversalMediaLoader WIN32
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)
target_link_libraries(UniversalMediaLoader PRIVATE Qt6::Widgets Qt6::Network)

option(BUILD_TESTS "Build unit/integration tests" OFF)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "C:/Qt/6.7.0/msvc2019_64/bin")
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET UniversalMediaLoader POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE}
                --dir $<TARGET_FILE_DIR:UniversalMediaLoader>
                $<TARGET_FILE:UniversalMediaLoader>
            COMMENT "Running windeployqt for UniversalMediaLoader..."
        )
    endif()
endif()
