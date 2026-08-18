#include "utils/MarkdownFormat.h"

#include <QRegularExpression>

namespace dsh::study {

namespace {

QString escapeHtml(const QString &text) {
  QString out;
  out.reserve(text.size() + 16);
  for (const QChar ch : text) {
    if (ch == QLatin1Char('&')) {
      out += QStringLiteral("&amp;");
    } else if (ch == QLatin1Char('<')) {
      out += QStringLiteral("&lt;");
    } else if (ch == QLatin1Char('>')) {
      out += QStringLiteral("&gt;");
    } else if (ch == QLatin1Char('"')) {
      out += QStringLiteral("&quot;");
    } else {
      out += ch;
    }
  }
  return out;
}

QString inlineFormat(const QString &line) {
  QString html = escapeHtml(line);
  static const QRegularExpression codePattern(QStringLiteral("`([^`]+)`"));
  html.replace(codePattern, QStringLiteral("<code>\\1</code>"));
  static const QRegularExpression boldPattern(QStringLiteral("\\*\\*([^*]+)\\*\\*"));
  html.replace(boldPattern, QStringLiteral("<b>\\1</b>"));
  static const QRegularExpression italicPattern(QStringLiteral("\\*([^*]+)\\*"));
  html.replace(italicPattern, QStringLiteral("<i>\\1</i>"));
  return html;
}

}  // namespace

QString markdownToHtml(const QString &markdown) {
  const QStringList lines = markdown.split(QLatin1Char('\n'));
  QString html;
  html.reserve(markdown.size() + 64);
  bool inList = false;

  auto closeList = [&]() {
    if (inList) {
      html += QStringLiteral("</ul>");
      inList = false;
    }
  };

  for (const QString &rawLine : lines) {
    const QString line = rawLine.trimmed();
    if (line.startsWith(QLatin1String("- ")) || line.startsWith(QLatin1String("* "))) {
      if (!inList) {
        html += QStringLiteral("<ul style=\"margin:0;padding-left:1.2em;\">");
        inList = true;
      }
      html += QStringLiteral("<li>") + inlineFormat(line.mid(2).trimmed()) + QStringLiteral("</li>");
      continue;
    }

    closeList();
    if (line.isEmpty()) {
      html += QStringLiteral("<br/>");
      continue;
    }
    html += QStringLiteral("<p style=\"margin:0 0 0.35em 0;\">") + inlineFormat(line) + QStringLiteral("</p>");
  }
  closeList();
  return html;
}

}  // namespace dsh::study
