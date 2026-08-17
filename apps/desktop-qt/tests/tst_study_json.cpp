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

  void settingsFields_readsPrimitiveAndSecret() {
    QJsonObject describe;
    describe.insert(QStringLiteral("writable"), true);
    QJsonObject schema;
    schema.insert(QStringLiteral("type"), QStringLiteral("object"));
    QJsonObject dict;
    dict.insert(QStringLiteral("preference"),
                QJsonObject{{QStringLiteral("type"), QStringLiteral("union")},
                            {QStringLiteral("list"),
                             QJsonArray{QJsonObject{{QStringLiteral("type"), QStringLiteral("const")},
                                                    {QStringLiteral("value"), QStringLiteral("light")}},
                                        QJsonObject{{QStringLiteral("type"), QStringLiteral("const")},
                                                    {QStringLiteral("value"), QStringLiteral("dark")}}}}});
    dict.insert(QStringLiteral("timeoutMs"), QJsonObject{{QStringLiteral("type"), QStringLiteral("number")}});
    schema.insert(QStringLiteral("dict"), dict);
    QJsonObject ns;
    ns.insert(QStringLiteral("ns"), QStringLiteral("ui-theme"));
    ns.insert(QStringLiteral("schema"), schema);
    ns.insert(QStringLiteral("value"), QJsonObject{{QStringLiteral("preference"), QStringLiteral("light")},
                                                   {QStringLiteral("timeoutMs"), 10}});
    ns.insert(QStringLiteral("applies"), QStringLiteral("live"));
    ns.insert(QStringLiteral("revision"), 3);
    ns.insert(QStringLiteral("secrets"),
              QJsonArray{QJsonObject{{QStringLiteral("path"), QJsonArray{QStringLiteral("apiKey")}},
                                     {QStringLiteral("set"), true}}});
    describe.insert(QStringLiteral("namespaces"), QJsonArray{ns});
    const QVariantList fields = dsh::study::settingsFields(describe);
    QVERIFY(fields.size() >= 3);
    bool sawEnum = false;
    bool sawSecret = false;
    for (const QVariant &fieldValue : fields) {
      const QVariantMap field = fieldValue.toMap();
      if (field.value(QStringLiteral("key")).toString() == QLatin1String("preference")) {
        QCOMPARE(field.value(QStringLiteral("kind")).toString(), QStringLiteral("enum"));
        QCOMPARE(field.value(QStringLiteral("section")).toString(), QStringLiteral("general"));
        sawEnum = true;
      }
      if (field.value(QStringLiteral("kind")).toString() == QLatin1String("secret")) {
        QVERIFY(field.value(QStringLiteral("secretSet")).toBool());
        sawSecret = true;
      }
    }
    QVERIFY(sawEnum);
    QVERIFY(sawSecret);
  }

  void promptPayload_includesImages() {
    QVariantList images;
    images.append(QVariantMap{{QStringLiteral("mediaType"), QStringLiteral("image/png")},
                              {QStringLiteral("data"), QStringLiteral("YWJj")},
                              {QStringLiteral("name"), QStringLiteral("a.png")}});
    const QJsonObject payload = dsh::study::promptPayload(QStringLiteral("sid"), QStringLiteral("看图"), images);
    const QJsonArray content = payload.value(QStringLiteral("content")).toArray();
    QCOMPARE(content.size(), 2);
    QCOMPARE(content.at(1).toObject().value(QStringLiteral("type")).toString(), QStringLiteral("image"));
  }

  void permissionAndSlashCatalog() {
    QCOMPARE(dsh::study::permissionLabel(QStringLiteral("danger-full-access")), QStringLiteral("完全访问"));
    QVariantList skills;
    skills.append(QVariantMap{{QStringLiteral("name"), QStringLiteral("review")},
                              {QStringLiteral("description"), QStringLiteral("审稿")}});
    QVariantList permissions;
    permissions.append(QVariantMap{{QStringLiteral("id"), QStringLiteral("read-only")},
                                   {QStringLiteral("label"), QStringLiteral("只读")}});
    const QVariantList items = dsh::study::slashItems(skills, permissions, true);
    QCOMPARE(items.at(0).toMap().value(QStringLiteral("line")).toString(), QStringLiteral("/plan off"));
    QVERIFY(items.size() >= 3);
  }

  void apiKeyAndOnboarding() {
    QVERIFY(dsh::study::apiKeyFailure(QString()).isEmpty());
    QVERIFY(!dsh::study::apiKeyFailure(QStringLiteral("DEEPSEEK_API_KEY=sk")).isEmpty());
    QVERIFY(dsh::study::apiKeyFailure(QStringLiteral("sk-live")).isEmpty());
    QVariantList providers;
    providers.append(QVariantMap{{QStringLiteral("official"), true},
                                 {QStringLiteral("active"), true},
                                 {QStringLiteral("configured"), false},
                                 {QStringLiteral("credentialWritable"), true},
                                 {QStringLiteral("usable"), false}});
    QVERIFY(dsh::study::onboardingNeeded(providers));
    providers[0] = QVariantMap{{QStringLiteral("official"), true},
                               {QStringLiteral("active"), true},
                               {QStringLiteral("configured"), true},
                               {QStringLiteral("credentialWritable"), true},
                               {QStringLiteral("usable"), true}};
    QVERIFY(!dsh::study::onboardingNeeded(providers));
  }

  void jobRowsAndImageType() {
    QJsonArray jobs;
    jobs.append(QJsonObject{{QStringLiteral("id"), QStringLiteral("bash-1")},
                            {QStringLiteral("kind"), QStringLiteral("bash")},
                            {QStringLiteral("label"), QStringLiteral("ls")},
                            {QStringLiteral("status"), QStringLiteral("running")}});
    const QVariantList rows = dsh::study::jobRows(jobs);
    QCOMPARE(rows.size(), 1);
    QCOMPARE(rows.at(0).toMap().value(QStringLiteral("statusLabel")).toString(), QStringLiteral("进行中"));
    QCOMPARE(dsh::study::imageMediaType(QStringLiteral("C:/a.PNG")), QStringLiteral("image/png"));
    QVERIFY(dsh::study::imageMediaType(QStringLiteral("C:/a.txt")).isEmpty());
  }
};

QTEST_MAIN(TstStudyJson)
#include "tst_study_json.moc"
