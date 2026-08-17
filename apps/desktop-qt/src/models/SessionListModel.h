#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVariantList>
#include <QVector>

/** Sidebar session rows. Whole-list replace on study reload; in-place title/running updates from host frames. */
class SessionListModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

 public:
  enum Role {
    SessionIdRole = Qt::UserRole + 1,
    TitleRole,
    BlankRole,
    RunningRole
  };

  explicit SessionListModel(QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  void replaceAll(const QVariantList &rows);
  void setTitle(const QString &sessionId, const QString &title);
  void setRunning(const QString &sessionId, bool running);
  QString titleFor(const QString &sessionId) const;
  QString blankSessionId() const;
  QString firstSessionId() const;
  bool contains(const QString &sessionId) const;

 signals:
  void countChanged();

 private:
  struct Row {
    QString sessionId;
    QString title;
    bool blank = false;
    bool running = false;
  };

  int indexOf(const QString &sessionId) const;

  QVector<Row> m_rows;
};
