#pragma once

#include <QString>

namespace dsh::study {

/** Lightweight Markdown → HTML for assistant transcript bubbles (bold, code, lists). */
QString markdownToHtml(const QString &markdown);

}  // namespace dsh::study
