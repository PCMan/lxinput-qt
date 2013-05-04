project(lxinput-qt)

cmake_minimum_required(VERSION 2.8.6)

# Qt
find_package(Qt4 REQUIRED QtCore QtGui)

# glib using pkg-config
find_package(PkgConfig)
pkg_check_modules(GLIB REQUIRED
  x11
)

# set visibility to hidden to hide symbols, unlesss they're exporeted manually in the code
set(CMAKE_CXX_FLAGS "-DQT_NO_KEYWORDS -fno-exceptions")

include_directories(
  ${QT_INCLUDES}
  ${X11_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(CMAKE_AUTOMOC TRUE)

set(lxinput-qt_SRCS
  lxinput-qt.cpp
  maindialog.cpp
)

set(lxinput-qt_UIS
  lxinput.ui
)

qt4_wrap_ui(lxinput-qt_UI_H
  ${lxinput-qt_UIS}
)

add_executable(lxinput-qt
  ${lxinput-qt_SRCS}
  ${lxinput-qt_UI_H}
)

target_link_libraries(lxinput-qt
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${X11_LIBRARIES}
)

install(TARGETS lxinput-qt RUNTIME DESTINATION bin)
