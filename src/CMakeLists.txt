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

add_definitions(
-DPACKAGE_DATA_DIR="${CMAKE_INSTALL_PREFIX}/share/lxinput-qt"
)

target_link_libraries(lxinput-qt
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${X11_LIBRARIES}
)

install(TARGETS lxinput-qt RUNTIME DESTINATION bin)

# add translation for lxinput-qt
# See http://www.cmake.org/Wiki/CMake:How_To_Build_Qt4_Software
file(GLOB TS_FILES translations/*.ts)
qt4_create_translation(QM_FILES
  ${lxinput-qt_SRCS}
  ${lxinput-qt_UI_H}
  ${TS_FILES})
qt4_add_translation(QM_FILES ${TS_FILES})
add_custom_target (lxinput-qt_translations DEPENDS ${QM_FILES})
install(FILES ${QM_FILES} DESTINATION share/lxinput-qt/translations)

# prevent the generated files from being deleted during make clean
set_directory_properties(PROPERTIES
  CLEAN_NO_CUSTOM true
)
