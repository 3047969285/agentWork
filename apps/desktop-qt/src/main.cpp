#include <QGuiApplication>
#include <QQuickStyle>

#include "app/Application.h"

int main(int argc, char *argv[]) {
  QQuickStyle::setStyle(QStringLiteral("Basic"));
  QGuiApplication app(argc, argv);
  Application dshApp;
  if (!dshApp.init()) {
    return 1;
  }
  dshApp.start();
  return app.exec();
}
