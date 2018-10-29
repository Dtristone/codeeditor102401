#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkMagenta);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    QStringList functionPatterns;
    keywordPatterns << "\\bifeq\\b" << "\\bifdef\\b" << "\\belse\\b" << "\\bendif\\b";

    functionPatterns << "\\bsubst\\b" << "\\bpatsubst\\b" << "\\bstrip\\b"
                     << "\\bfindstring\\b" << "\\bfilter\\b" << "\\bfilter-out\\b"
                     << "\\bsort\\b" << "\\bword\\b" << "\\bwordlist\\b"
                     << "\\bwords\\b" << "\\bfirstword\\b" << "\\bdir\\b"
                     << "\\bnotdir\\b" << "\\bsuffix\\b" << "\\bbasename\\b"
                     << "\\baddsuffix\\b" << "\\baddprefix\\b" << "\\bjoin\\b"
                     << "\\bforeach\\b" << "\\bif\\b" << "\\bcall\\b"
                     << "\\borigin\\b" << "\\bshell\\b" << "\\berror\\b"
                     << "\\bwarning\\b";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
//! [0] //! [1]
    }
//! [1]

//! [2]
    signalFormat.setFontWeight(QFont::Bold);
    signalFormat.setForeground(Qt::red);
    rule.pattern = QRegularExpression("\\b[a-zA-Z_]+-*\\.*\\w*\\:(?=([a-zA-Z].*|\\s[a-zA-Z].*|$))");
    rule.format = signalFormat;
    highlightingRules.append(rule);
//! [2]

//! [3]
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    //rule.pattern = QRegularExpression("(?<=\\()[a-zA-Z]+(?=.*\\))");
    rule.pattern = QRegularExpression("(?<=\\()[a-zA-Z]+\\s+.*(?=.*\\))");
    rule.format = functionFormat;
    highlightingRules.append(rule);
//! [3]

//! [4]
    variableFormat.setForeground(Qt::darkYellow);
    rule.pattern = QRegularExpression("\\$\\([a-zA-Z\u4E00-\u9FA5]+\\)|[_A-Za-z]+-*\\w*\\s*=|[_A-Za-z]+-*\\w*\\s*\\+=");
    rule.format = variableFormat;
    highlightingRules.append(rule);
//! [4]

//! [5]
    pathFormat.setForeground(Qt::darkGray);
    rule.pattern = QRegularExpression("(\\.\\.|[a-zA-Z]\\:)\\\\.+");
    rule.format = pathFormat;
    highlightingRules.append(rule);
//! [5]

//! [6]
    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("#[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);
//! [6]

//! [7]
    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);
//! [7]
}

void Highlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
//! [7] //! [8]
    setCurrentBlockState(0);
//! [8]
}
