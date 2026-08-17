#include <QtTest>

#include "models/TranscriptModel.h"
#include "utils/StudyJson.h"

class TstStudyJson : public QObject {
  Q_OBJECT

 private slots:
  void displayTitle_prefersProjectionThenBlankThenCwd() {
    QJsonObject titled;
    titled.insert(QStringLiteral("sessionId"), QStringLiteral("abc123456"));
    titled.insert(QStringLiteral("blank"), false);
    QJsonObject values;
    values.insert(QStringLiteral("title"), QStringLiteral("冬夜抄经"));
    QJsonObject projections;
    projections.insert(QStringLiteral("values"), values);
    titled.insert(QStringLiteral("projections"), projections);
    QCOMPARE(dsh::study::displayTitle(titled), QStringLiteral("冬夜抄经"));

    QJsonObject blank;
    blank.insert(QStringLiteral("sessionId"), QStringLiteral("blank-id"));
    blank.insert(QStringLiteral("blank"), true);
    QCOMPARE(dsh::study::displayTitle(blank), QStringLiteral("新会话"));

    QJsonObject cwd;
    cwd.insert(QStringLiteral("sessionId"), QStringLiteral("cwd-session"));
    cwd.insert(QStringLiteral("blank"), false);
    cwd.insert(QStringLiteral("cwd"), QStringLiteral("D:/work/deepseek-harness/"));
    QCOMPARE(dsh::study::displayTitle(cwd), QStringLiteral("deepseek-harness"));
  }

  void sessionRows_skipsSubagentAndArchived() {
    QJsonArray items;
    items.append(QJsonObject{{QStringLiteral("sessionId"), QStringLiteral("keep")},
                             {QStringLiteral("blank"), false},
                             {QStringLiteral("running"), true},
                             {QStringLiteral("cwd"), QStringLiteral("/tmp/keep")}});
    items.append(QJsonObject{{QStringLiteral("sessionId"), QStringLiteral("child")},
                             {QStringLiteral("parentSessionId"), QStringLiteral("keep")},
                             {QStringLiteral("blank"), false}});
    items.append(QJsonObject{{QStringLiteral("sessionId"), QStringLiteral("gone")},
                             {QStringLiteral("blank"), false}});
    const QSet<QString> archived{QStringLiteral("gone")};
    const QVariantList rows = dsh::study::sessionRows(items, nullptr, archived);
    QCOMPARE(rows.size(), 1);
    QCOMPARE(rows.at(0).toMap().value(QStringLiteral("sessionId")).toString(), QStringLiteral("keep"));
  }

  void transcript_keepsUserAssistantAndSkipsPlugin() {
    QJsonArray events;
    events.append(QJsonObject{
        {QStringLiteral("event"),
         QJsonObject{{QStringLiteral("type"), QStringLiteral("user/message")},
                     {QStringLiteral("seq"), 1},
                     {QStringLiteral("data"),
                      QJsonObject{{QStringLiteral("source"), QJsonObject{{QStringLiteral("kind"), QStringLiteral("user")}}},
                                  {QStringLiteral("content"),
                                   QJsonArray{QJsonObject{{QStringLiteral("type"), QStringLiteral("text")},
                                                          {QStringLiteral("text"), QStringLiteral("今日抄哪一页？")}}}}}}}}});
    events.append(QJsonObject{
        {QStringLiteral("event"),
         QJsonObject{{QStringLiteral("type"), QStringLiteral("user/message")},
                     {QStringLiteral("seq"), 2},
                     {QStringLiteral("data"),
                      QJsonObject{{QStringLiteral("source"),
                                   QJsonObject{{QStringLiteral("kind"), QStringLiteral("plugin")},
                                               {QStringLiteral("plugin"), QStringLiteral("agent-instructions")}}},
                                  {QStringLiteral("content"),
                                   QJsonArray{QJsonObject{{QStringLiteral("type"), QStringLiteral("text")},
                                                          {QStringLiteral("text"), QStringLiteral("hidden")}}}}}}}}});
    events.append(QJsonObject{
        {QStringLiteral("event"),
         QJsonObject{{QStringLiteral("type"), QStringLiteral("assistant/message")},
                     {QStringLiteral("seq"), 3},
                     {QStringLiteral("data"),
                      QJsonObject{{QStringLiteral("message"),
                                   QJsonObject{{QStringLiteral("content"),
                                                QJsonArray{QJsonObject{{QStringLiteral("type"), QStringLiteral("text")},
                                                                       {QStringLiteral("text"), QStringLiteral("先从目录起笔。")}}}}}}}}}}});
    TranscriptModel model;
    model.resetFromHistory(events);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.data(model.index(0), TranscriptModel::RoleNameRole).toString(), QStringLiteral("user"));
    QCOMPARE(model.data(model.index(1), TranscriptModel::RoleNameRole).toString(), QStringLiteral("assistant"));
    QCOMPARE(model.data(model.index(1), TranscriptModel::TextRole).toString(), QStringLiteral("先从目录起笔。"));
  }

  void promptPayload_matchesSessionPromptWire() {
    const QJsonObject payload = dsh::study::promptPayload(QStringLiteral("sid-1"), QStringLiteral("落墨"));
    QCOMPARE(payload.value(QStringLiteral("sessionId")).toString(), QStringLiteral("sid-1"));
    QCOMPARE(payload.value(QStringLiteral("mode")).toString(), QStringLiteral("queue"));
    const QJsonArray content = payload.value(QStringLiteral("content")).toArray();
    QCOMPARE(content.size(), 1);
    QCOMPARE(content.at(0).toObject().value(QStringLiteral("type")).toString(), QStringLiteral("text"));
    QCOMPARE(content.at(0).toObject().value(QStringLiteral("text")).toString(), QStringLiteral("落墨"));
  }

  void eventText_readsNestedAssistantMessage() {
    QJsonObject data;
    QJsonObject message;
    message.insert(QStringLiteral("content"),
                   QJsonArray{QJsonObject{{QStringLiteral("type"), QStringLiteral("text")},
                                          {QStringLiteral("text"), QStringLiteral("先从目录起笔。")}}});
    data.insert(QStringLiteral("message"), message);
    QCOMPARE(dsh::study::eventText(data), QStringLiteral("先从目录起笔。"));
  }

  void toolRow_usesHostViewTitle() {
    QJsonObject event;
    event.insert(QStringLiteral("type"), QStringLiteral("tool/call"));
    event.insert(QStringLiteral("seq"), 4);
    event.insert(QStringLiteral("data"),
                 QJsonObject{{QStringLiteral("callId"), QStringLiteral("c1")},
                             {QStringLiteral("name"), QStringLiteral("bash")},
                             {QStringLiteral("arguments"), QStringLiteral("{\"cmd\":\"ls\"}")}});
    QJsonObject view;
    view.insert(QStringLiteral("for"), QStringLiteral("call"));
    view.insert(QStringLiteral("view"),
                QJsonObject{{QStringLiteral("card"), QStringLiteral("terminal")},
                            {QStringLiteral("title"), QStringLiteral("ls")}});
    const QVariantMap row = dsh::study::toolRow(event, view);
    QCOMPARE(row.value(QStringLiteral("kind")).toString(), QStringLiteral("tool"));
    QCOMPARE(row.value(QStringLiteral("title")).toString(), QStringLiteral("ls"));
    QCOMPARE(row.value(QStringLiteral("card")).toString(), QStringLiteral("terminal"));
    QCOMPARE(row.value(QStringLiteral("status")).toString(), QStringLiteral("pending"));
  }

  void workspaceRows_keepsIdAndTitle() {
    QJsonObject list;
    list.insert(QStringLiteral("items"),
                QJsonArray{QJsonObject{{QStringLiteral("workspaceId"), QStringLiteral("w1")},
                                       {QStringLiteral("title"), QStringLiteral("东斋")},
                                       {QStringLiteral("path"), QStringLiteral("D:/work/east")},
                                       {QStringLiteral("sessionIds"), QJsonArray{}},
                                       {QStringLiteral("createdAt"), QStringLiteral("t")},
                                       {QStringLiteral("updatedAt"), QStringLiteral("t")}}});
    const QVariantList rows = dsh::study::workspaceRows(list);
    QCOMPARE(rows.size(), 1);
    QCOMPARE(rows.at(0).toMap().value(QStringLiteral("title")).toString(), QStringLiteral("东斋"));
  }

  void blankSessionId_returnsFirstBlank() {
    QVariantList rows;
    rows.append(QVariantMap{{QStringLiteral("sessionId"), QStringLiteral("a")}, {QStringLiteral("blank"), false}});
    rows.append(QVariantMap{{QStringLiteral("sessionId"), QStringLiteral("b")}, {QStringLiteral("blank"), true}});
    QCOMPARE(dsh::study::blankSessionId(rows), QStringLiteral("b"));
  }
};

QTEST_MAIN(TstStudyJson)
#include "tst_study_json.moc"
