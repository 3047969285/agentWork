#include <cstdlib>
#include <cstdio>

#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QQuickStyle>
#include <QString>
#include <QtGlobal>

#ifdef Q_OS_WIN
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#endif

#include "app/Application.h"

namespace {

FILE *g_logFile = nullptr;

QString logFilePath(char *argv0) {
#ifdef Q_OS_WIN
  wchar_t modulePath[MAX_PATH];
  if (GetModuleFileNameW(nullptr, modulePath, MAX_PATH) > 0) {
    return QFileInfo(QString::fromWCharArray(modulePath)).dir().filePath(QStringLiteral("dsh-desktop.log"));
  }
#endif
  return QFileInfo(QString::fromLocal8Bit(argv0)).dir().filePath(QStringLiteral("dsh-desktop.log"));
}

void dshQtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message) {
  Q_UNUSED(context);
  const QByteArray line = qFormatLogMessage(type, context, message).toUtf8();
  if (g_logFile != nullptr) {
    std::fwrite(line.constData(), 1, static_cast<size_t>(line.size()), g_logFile);
    std::fputc('\n', g_logFile);
    std::fflush(g_logFile);
  }
  if (type == QtFatalMsg) {
    if (g_logFile != nullptr) {
      std::fclose(g_logFile);
      g_logFile = nullptr;
    }
    std::abort();
  }
}

}  // namespace

int main(int argc, char *argv[]) {
  qSetMessagePattern(QStringLiteral("[%{time yyyy-MM-dd hh:mm:ss.zzz}] %{type} %{message}"));
  const QString logPath = logFilePath(argc > 0 ? argv[0] : nullptr);
#ifdef Q_OS_WIN
  g_logFile = _wfopen(reinterpret_cast<const wchar_t *>(logPath.utf16()), L"w");
#else
  g_logFile = std::fopen(logPath.toLocal8Bit().constData(), "w");
#endif
  qInstallMessageHandler(dshQtMessageHandler);
#ifdef Q_OS_WIN
  SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
#endif
  QQuickStyle::setStyle(QStringLiteral("Basic"));
  QGuiApplication app(argc, argv);
  app.setApplicationName(QStringLiteral("深卷"));
  app.setApplicationDisplayName(QStringLiteral("深卷"));
  app.setOrganizationName(QStringLiteral("深度求索"));
  qInfo().noquote() << QStringLiteral("log") << logPath;
  Application dshApp;
  if (!dshApp.init()) {
    qWarning() << QStringLiteral("界面未能加载");
    return 1;
  }
  dshApp.start();
  return app.exec();
}
