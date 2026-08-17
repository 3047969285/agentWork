#include <cstdlib>
#include <cstdio>

#include <QGuiApplication>
#include <QQuickStyle>
#include <QtGlobal>

#ifdef Q_OS_WIN
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#endif

#include "app/Application.h"

namespace {

void dshQtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message) {
#ifdef QT_NO_DEBUG
  Q_UNUSED(context);
  Q_UNUSED(message);
  if (type == QtFatalMsg) {
    std::abort();
  }
#else
  const QByteArray line = qFormatLogMessage(type, context, message).toUtf8();
  std::fputs(line.constData(), stderr);
  std::fputc('\n', stderr);
  std::fflush(stderr);
  if (type == QtFatalMsg) {
    std::abort();
  }
#endif
}

}  // namespace

int main(int argc, char *argv[]) {
  qInstallMessageHandler(dshQtMessageHandler);
#ifdef Q_OS_WIN
  SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
#endif
  QQuickStyle::setStyle(QStringLiteral("Basic"));
  QGuiApplication app(argc, argv);
  app.setApplicationName(QStringLiteral("深卷"));
  app.setApplicationDisplayName(QStringLiteral("深卷"));
  app.setOrganizationName(QStringLiteral("深度求索"));
  Application dshApp;
  if (!dshApp.init()) {
    return 1;
  }
  dshApp.start();
  return app.exec();
}
