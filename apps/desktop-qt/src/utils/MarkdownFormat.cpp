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
  QString normalized = markdown;
  normalized.replace(QRegularExpression(QStringLiteral("\\n{3,}")), QStringLiteral("\n\n"));

  const QStringList lines = normalized.split(QLatin1Char('\n'));
  QString html = QStringLiteral("<div style=\"margin:0;padding:0;line-height:1.22;\">");
  html.reserve(normalized.size() + 128);
  bool inList = false;
  bool pendingParagraphGap = false;

  auto closeList = [&]() {
    if (inList) {
      html += QStringLiteral("</ul>");
      inList = false;
    }
  };

  auto flushParagraphGap = [&]() {
    if (pendingParagraphGap) {
      html += QStringLiteral("<div style=\"margin:0;padding:0;height:0.08em;\"></div>");
      pendingParagraphGap = false;
    }
  };

  for (const QString &rawLine : lines) {
    const QString line = rawLine.trimmed();
    if (line.startsWith(QLatin1String("- ")) || line.startsWith(QLatin1String("* "))) {
      flushParagraphGap();
      if (!inList) {
        html += QStringLiteral(
            "<ul style=\"margin:0.02em 0 0.04em 0;padding-left:1.0em;line-height:1.2;\">");
        inList = true;
      }
      html += QStringLiteral("<li style=\"margin:0 0 0.01em 0;padding:0;\">") + inlineFormat(line.mid(2).trimmed()) +
              QStringLiteral("</li>");
      continue;
    }

    closeList();
    if (line.isEmpty()) {
      pendingParagraphGap = true;
      continue;
    }
    flushParagraphGap();
    html += QStringLiteral("<p style=\"margin:0 0 0.02em 0;line-height:1.22;\">") + inlineFormat(line) +
            QStringLiteral("</p>");
  }
  closeList();
  flushParagraphGap();
  html += QStringLiteral("</div>");
  return html;
}

}  // namespace dsh::study
