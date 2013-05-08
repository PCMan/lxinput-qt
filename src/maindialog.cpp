/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2013  <copyright holder> <email>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "maindialog.h"
#include <string.h>
#include <math.h>
#include <alloca.h>
#include <stdlib.h>

// FIXME: Qt5 does not have QX11Info
#if QT_VERSION >= 0x050000
#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatformwindow.h>
#else
#include <QX11Info>
#endif

#include <QSettings>
#include <QDir>
#include <QFile>
#include <QStringBuilder>

// FIXME: how to support XCB or Wayland?
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

using namespace Lxinput;

MainDialog::MainDialog():
  QDialog(),
  accel(20),
  oldAccel(20),
  threshold(10),
  oldThreshold(10),
  leftHanded(false),
  oldLeftHanded(false),
  delay(500),
  oldDelay(500),
  interval(30),
  oldInterval(30),
  beep(true),
  oldBeep(true) {

  ui.setupUi(this);

  /* read the config flie */
  loadSettings();

  /* init the UI */
  ui.mouseAccel->setValue(accel);
  ui.mouseThreshold->setValue(110 - threshold);
  ui.mouseLeftHanded->setChecked(leftHanded);

  ui.keyboardDelay->setValue(delay);
  ui.keyboardInterval->setValue(interval);
  ui.keyboardBeep->setChecked(beep);

  // set_range_stops(ui.mouseAccel, 10);
  connect(ui.mouseAccel, SIGNAL(valueChanged(int)), SLOT(onMouseAccelChanged(int)));
  // set_range_stops(ui.mouseThreshold, 10);
  connect(ui.mouseThreshold, SIGNAL(valueChanged(int)), SLOT(onMouseThresholdChanged(int)));
  connect(ui.mouseLeftHanded, SIGNAL(toggled(bool)), SLOT(onMouseLeftHandedToggled(bool)));

  // set_range_stops(ui.keyboardDelay, 10);
  connect(ui.keyboardDelay, SIGNAL(valueChanged(int)), SLOT(onKeyboardSliderChanged(int)));
  // set_range_stops(ui.keyboardInterval, 10);
  connect(ui.keyboardInterval, SIGNAL(valueChanged(int)), SLOT(onKeyboardSliderChanged(int)));
  connect(ui.keyboardBeep, SIGNAL(toggled(bool)), SLOT(onKeyboardBeepToggled(bool)));
}

MainDialog::~MainDialog() {

}

inline static Display* x11Display() {
  // Qt 5 does not support QX11Info.
#if QT_VERSION >= 0x050000
  // references: https://github.com/hawaii-desktop/libkdeqt5staging/blob/master/src/qt5only/qx11info.cpp
  QPlatformNativeInterface* native = qApp->platformNativeInterface();

  void* display = native->nativeResourceForScreen(QByteArray("display"), QGuiApplication::primaryScreen());
  return reinterpret_cast<Display*>(display);
#else
  return QX11Info::display();
#endif
}

void MainDialog::onMouseAccelChanged(int value) {
  QSlider* slider = static_cast<QSlider*>(sender());
  accel = value;
  XChangePointerControl(x11Display(), True, False,
                        accel, 10, 0);
}

void MainDialog::onMouseThresholdChanged(int value) {
  QSlider* slider = static_cast<QSlider*>(sender());
  /* threshold = 110 - sensitivity. The lower the threshold, the higher the sensitivity */
  threshold = 110 - value;
  XChangePointerControl(x11Display(), False, True,
                        0, 10, threshold);
}

void MainDialog::onKeyboardSliderChanged(int value) {
  QSlider* slider = static_cast<QSlider*>(sender());

  if(slider == ui.keyboardDelay)
    delay = value;
  else if(slider == ui.keyboardInterval)
    interval = value;

  /* apply keyboard values */
  XkbSetAutoRepeatRate(x11Display(), XkbUseCoreKbd, delay, interval);
}

/* This function is taken from Gnome's control-center 2.6.0.3 (gnome-settings-mouse.c) and was modified*/
#define DEFAULT_PTR_MAP_SIZE 128
void MainDialog::setLeftHandedMouse() {
  unsigned char* buttons;
  int n_buttons, i;
  int idx_1 = 0, idx_3 = 1;

  buttons = (unsigned char*)alloca(DEFAULT_PTR_MAP_SIZE);
  n_buttons = XGetPointerMapping(x11Display(), buttons, DEFAULT_PTR_MAP_SIZE);

  if(n_buttons > DEFAULT_PTR_MAP_SIZE) {
    buttons = (unsigned char*)alloca(n_buttons);
    n_buttons = XGetPointerMapping(x11Display(), buttons, n_buttons);
  }

  for(i = 0; i < n_buttons; i++) {
    if(buttons[i] == 1)
      idx_1 = i;
    else if(buttons[i] == ((n_buttons < 3) ? 2 : 3))
      idx_3 = i;
  }

  if((leftHanded && idx_1 < idx_3) ||
      (!leftHanded && idx_1 > idx_3)) {
    buttons[idx_1] = ((n_buttons < 3) ? 2 : 3);
    buttons[idx_3] = 1;
    XSetPointerMapping(x11Display(), buttons, n_buttons);
  }
}

void MainDialog::onMouseLeftHandedToggled(bool checked) {
  leftHanded = checked;
  setLeftHandedMouse();
}

void MainDialog::onKeyboardBeepToggled(bool checked) {
  XKeyboardControl values;
  beep = checked;
  values.bell_percent = beep ? -1 : 0;
  XChangeKeyboardControl(x11Display(), KBBellPercent, &values);
}

void MainDialog::loadSettings() {
  const char* session_name = getenv("DESKTOP_SESSION");
  /* load settings from current session config files */
  if(!session_name)
    session_name = "LXDE";
  QString relativePath = QString("/lxsession/") % session_name % "/desktop.conf";

  // FIXME: should we make the xdg base dir stuff a module?
  // add user config home dir and xdg config dirs to a list
  QStringList dirs;
  QString configHome = getenv("XDG_CONFIG_HOME");
  if(configHome.isEmpty())
    configHome = QDir::homePath() % "/.config";
  userConfigFile = configHome % relativePath;
  dirs.append(configHome);
  const char* config_dirs = getenv("XDG_CONFIG_DIRS");
  if(config_dirs) {
    dirs.append(QString(config_dirs).split(':', QString::SkipEmptyParts));
  }
  else {
    dirs.append("/etc/xdg");
  }

  QString configFileName;
  // locate config file
  Q_FOREACH(QString dir, dirs) {
    QString filename = dir % relativePath;
    if(QFile(filename).exists()) {
      configFileName = filename;
      break;
    }
  }

 if(configFileName.isEmpty())
   return;
 
  QSettings settings(configFileName, QSettings::IniFormat);
  settings.beginGroup("Mouse");
  oldAccel = accel = settings.value("AccFactor", 20).toInt();
  oldThreshold = threshold = settings.value("AccThreshold", 10).toInt();
  oldLeftHanded = leftHanded = settings.value("LeftHanded", false).toBool();
  settings.endGroup();

  settings.beginGroup("Keyboard");
  oldDelay = delay = settings.value("Delay", 500).toInt();
  oldInterval = interval = settings.value("Interval", 30).toInt();
  oldBeep = beep = settings.value("Beep", true).toBool();
  settings.endGroup();
}

void MainDialog::accept() {
  QSettings settings(userConfigFile, QSettings::IniFormat);
  settings.beginGroup("Mouse");
  settings.setValue("AccFactor", accel);
  settings.setValue("AccThreshold", threshold);
  settings.setValue("LeftHanded", leftHanded);
  settings.endGroup();

  settings.beginGroup("Keyboard");
  settings.setValue("Delay", delay);
  settings.setValue("Interval", interval);
  settings.setValue("Beep", beep);
  settings.endGroup();
  /* ask the settigns daemon to reload */
  /* FIXME: is this needed? */
  /* g_spawn_command_line_sync("lxde-settings-daemon reload", NULL, NULL, NULL, NULL); */
  QDialog::accept();
}

void MainDialog::reject() {
  /* restore to original settings */

  /* keyboard */
  delay = oldDelay;
  interval = oldInterval;
  beep = oldBeep;
  XkbSetAutoRepeatRate(x11Display(), XkbUseCoreKbd, delay, interval);
  /* FIXME: beep? */

  /* mouse */
  accel = oldAccel;
  threshold = oldThreshold;
  leftHanded = oldLeftHanded;
  XChangePointerControl(x11Display(), True, True,
                        accel, 10, threshold);
  setLeftHandedMouse();

  QDialog::reject();
}
