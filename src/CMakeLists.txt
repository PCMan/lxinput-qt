# set visibility to hidden to hide symbols, unlesss they're exporeted manually in the code
set(CMAKE_CXX_FLAGS "-DQT_NO_KEYWORDS -fno-exceptions")

include_directories(
  ${QT_INCLUDES}
  ${X11_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(CMAKE_AUTOMOC TRUE)

set(lxqt-config-input_SRCS
  lxqt-config-input.cpp
  maindialog.cpp
)

set(lxqt-config-input_UIS
  lxinput.ui
)

qt4_wrap_ui(lxqt-config-input_UI_H
  ${lxqt-config-input_UIS}
)

# add translation for lxqt-config-input
# See http://www.cmake.org/Wiki/CMake:How_To_Build_Qt4_Software
file(GLOB TS_FILES translations/*.ts)
option (UPDATE_TRANSLATIONS "Update source translation translations/*.ts files")
if (UPDATE_TRANSLATIONS)
  qt4_create_translation(QM_FILES
    ${lxqt-config-input_SRCS}
    ${lxqt-config-input_UI_H}
    ${TS_FILES})
else (UPDATE_TRANSLATIONS)
  qt4_add_translation(QM_FILES ${TS_FILES})
endif (UPDATE_TRANSLATIONS)
add_custom_target (lxqt-config-input_translations DEPENDS ${QM_FILES})
install(FILES ${QM_FILES} DESTINATION share/lxqt-config-input/translations)
# prevent the generated files from being deleted during make clean
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM true)

add_executable(lxqt-config-input
  ${lxqt-config-input_SRCS}
  ${lxqt-config-input_UI_H}
  ${QM_FILES}
)

add_definitions(
-DPACKAGE_DATA_DIR="${CMAKE_INSTALL_PREFIX}/share/lxqt-config-input"
)

target_link_libraries(lxqt-config-input
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${X11_LIBRARIES}
)

install(TARGETS lxqt-config-input RUNTIME DESTINATION bin)
