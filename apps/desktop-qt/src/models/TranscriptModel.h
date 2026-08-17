#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QVector>

/** Ink-scroll transcript. Inserts and in-place role updates; never rebuilds the list per token. */
class TranscriptModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

 public:
  enum Role {
    KindRole = Qt::UserRole + 1,
    RoleNameRole,
    TextRole,
    SeqRole,
    CallIdRole,
    ToolNameRole,
    CardRole,
    TitleRole,
    BodyRole,
    StatusRole,
    StreamingRole
  };

  explicit TranscriptModel(QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  void resetFromHistory(const QJsonArray &events);
  void applySessionEvent(const QJsonObject &event, const QJsonValue &view);
  void appendUser(const QString &text);
  void appendStreamDelta(const QString &delta);
  void finishStreaming();
  void clear();
  int lastSeq() const;
  bool hasStreaming() const;

 signals:
  void countChanged();

 private:
  struct Row {
    QString kind;
    QString role;
    QString text;
    int seq = 0;
    QString callId;
    QString toolName;
    QString card;
    QString title;
    QString body;
    QString status;
    bool streaming = false;
  };

  void appendRow(const Row &row);
  void replaceRow(int index, const Row &row, const QVector<int> &roles);
  void updateText(int index, const QString &text, int seq, bool streaming);
  int findToolRow(const QString &callId) const;
  int findStreamingRow() const;
  static Row fromHistoryEvent(const QJsonValue &entry);
  static Row fromSessionEvent(const QJsonObject &event, const QJsonValue &view);

  QVector<Row> m_rows;
  QHash<QString, int> m_toolIndex;
};
