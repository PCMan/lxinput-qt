#include "lxinput-qt.h"
#include <QApplication>
#include "maindialog.h"

using namespace Lxinput;

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  MainDialog dlg;
  dlg.exec();
  return 0;
}
