#include <QGuiApplication>
#include <QQuickStyle>

#ifdef Q_OS_WIN
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#endif

#include "app/Application.h"

int main(int argc, char *argv[]) {
#ifdef Q_OS_WIN
  SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
#endif
  QQuickStyle::setStyle(QStringLiteral("Basic"));
  QGuiApplication app(argc, argv);
  Application dshApp;
  if (!dshApp.init()) {
    return 1;
  }
  dshApp.start();
  return app.exec();
}
