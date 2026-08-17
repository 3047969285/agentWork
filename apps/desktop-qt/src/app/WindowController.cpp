#include "app/WindowController.h"
#include "hooks/ConnectionHook.h"

#include <QQmlContext>
#include <QDebug>

WindowController::WindowController(QObject *parent)
    : QObject(parent), m_engine(new QQmlApplicationEngine(this)) {
  connect(m_engine, &QQmlApplicationEngine::objectCreationFailed, this,
          [](const QUrl &url) { qWarning() << "Failed to create QML object:" << url; });
}

WindowController::~WindowController() = default;

bool WindowController::init(ConnectionHook *connectionHook) {
  if (connectionHook != nullptr) {
    m_engine->rootContext()->setContextProperty(QStringLiteral("connection"), connectionHook);
  }
  m_engine->load(QUrl(QStringLiteral("qrc:/dsh/src/qml/Main.qml")));
  return !m_engine->rootObjects().isEmpty();
}

QQmlApplicationEngine *WindowController::engine() const {
  return m_engine;
}
