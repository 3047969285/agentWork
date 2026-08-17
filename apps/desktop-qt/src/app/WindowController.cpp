#include "app/WindowController.h"
#include "hooks/ConnectionHook.h"
#include "hooks/StudyHook.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QDebug>

WindowController::WindowController(QObject *parent)
    : QObject(parent), m_engine(new QQmlApplicationEngine(this)) {
  connect(m_engine, &QQmlApplicationEngine::objectCreationFailed, this,
          [](const QUrl &url) { qWarning() << "Failed to create QML object:" << url; });
}

WindowController::~WindowController() = default;

bool WindowController::init(ConnectionHook *connectionHook, StudyHook *studyHook) {
  if (connectionHook != nullptr) {
    m_engine->rootContext()->setContextProperty(QStringLiteral("connection"), connectionHook);
  }
  if (studyHook != nullptr) {
    m_engine->rootContext()->setContextProperty(QStringLiteral("study"), studyHook);
  }
  m_engine->setOutputWarningsToStandardError(false);
  m_engine->loadFromModule(QLatin1String("dsh"), QLatin1String("Main"));
  return !m_engine->rootObjects().isEmpty();
}

QQmlApplicationEngine *WindowController::engine() const {
  return m_engine;
}
