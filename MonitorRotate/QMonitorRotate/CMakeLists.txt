cmake_minimum_required(VERSION 2.8.8)

#cmake_minimum_required(VERSION 2.8)
if(CMAKE_BUILD_TYPE STREQUAL Release)
  SET(CMAKE_BUILD_TYPE Release)
  SET (PROJECT_NAME MonitorRotate)
  message("Release mode")
else()
  SET(CMAKE_BUILD_TYPE Debug)
  SET (PROJECT_NAME MonitorRotate)
  SET(CMAKE_CXX_FILES "-g -Wall")
  message("Debug mode")

endif()

#find QT libraries
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )
# We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
add_definitions(${Qt5Widgets_DEFINITIONS})

SET(SRC
    main.cpp
    mainwindow.cpp
    qextserialport/qextserialbase.cpp
    qextserialport/posix_qextserialport.cpp
    qextserialport/qextserialport.cpp
    serialdevice.cpp
  )

set(EXECUTABLE_OUTPUT_PATH bin)
set(LIBRARY_OUTPUT_PATH lib)

FIND_PROGRAM(QT_UIC_EXECUTABLE uic
  $ENV{QTDIR}/bin
)


#QT5_WRAP_UI(ui_mainwindow.h mainwindow.ui -o ui_mainwindow.h)

QT5_WRAP_UI(UIS_HDRS mainwindow.ui)

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS}
)

add_executable(QMonitorRotate ${SRC} ${UIS_HDRS})
#target_link_libraries(${PROJECT_NAME} )

qt5_use_modules(QMonitorRotate Core Gui Widgets)
