cmake_minimum_required(VERSION 2.8.8)

#project(MonitorRotate)

# Find includes in corresponding build directories
#set(CMAKE_INCLUDE_CURRENT_DIR ON)

#set(CMAKE_AUTOMOC ON)

#find_package(Qt5Core)
#find_package(Qt5Gui)

#INCLUDE(${QT_USE_FILE})
#ADD_DEFINITIONS(${QT_DEFINITIONS})

#add_subdirectory (qextserialport)
#include_directories( ${MonitorRotate_SOURCE_DIR}/qextserialport )

#add_executable(QMonitorRotate main.cpp mainwindow.cpp serialdevice.cpp qextserialport/qextserialbase.cpp qextserialport/posix_qextserialport.cpp qextserialport/qextserialport.cpp ${UIS_HDRS})

#qt5_use_modules(QMonitorRotate Core Widgets)
#QT5_WRAP_UI(UIS_HDRS mainwindow.ui)
#qt5_wrap_ui(ui_mainwindow.h mainwindow.ui)


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

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

FIND_PROGRAM(QT_UIC_EXECUTABLE uic
  $ENV{QTDIR}/bin
  )

#IF (QT_UIC_EXECUTABLE)
  # SET ( QT_WRAP_UI "YES")
  #message("WOW")
#ENDIF (QT_UIC_EXECUTABLE)

#QT5_WRAP_UI(ui_mainwindow.h mainwindow.ui -o ui_mainwindow.h)

QT5_WRAP_UI(UIS_HDRS mainwindow.ui)

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS}
)

# set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${perception_kit_VERSION} )
add_executable(QMonitorRotate ${SRC} ${UIS_HDRS})
#target_link_libraries(${PROJECT_NAME} )

qt5_use_modules(QMonitorRotate Core Gui Widgets)
