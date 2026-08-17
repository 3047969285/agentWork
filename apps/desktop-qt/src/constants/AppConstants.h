#pragma once

#include <QString>
#include <QtGlobal>

namespace AppConstants {

inline constexpr int kHostReadyTimeoutMs = 60000;
inline constexpr int kHostStopTimeoutMs = 5000;
inline constexpr int kConnectRetryCount = 5;
inline constexpr int kConnectRetryMs = 1000;

inline const QString kDefaultHost = QStringLiteral("127.0.0.1");
inline const QString kProfileWeb = QStringLiteral("web");
inline constexpr quint16 kDefaultPort = 0;

} // namespace AppConstants
