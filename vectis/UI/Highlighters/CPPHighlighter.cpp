#include <UI/Highlighters/CPPHighlighter.h>

CPPHighlighter::CPPHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    normalText.setForeground(Qt::white);

    keywordFormat.setForeground(QColor(102, 217, 239)); // Light blue
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\balignas\\b" << "\\balignof\\b" << "\\basm\\b"
                    << "\\bauto\\b" << "\\bbitand\\b" << "\\bbitor\\b" << "\\bbool\\b" << "\\bchar\\b" << "\\bchar16_t\\b" << "\\bchar32_t\\b"
                    << "\\bclass\\b" << "\\bcompl\\b" << "\\bconcept\\b" << "\\bdecltype\\b" << "\\bdefault\\b" << "\\bdelete\\b"
                    << "\\bdouble\\b" << "\\bdynamic_cast\\b" << "\\benum\\b"
                    << "\\bexplicit\\b" << "\\bexport\\b" << "\\bextern\\b" << "\\bfalse\\b" << "\\bfloat\\b"
                    << "\\bfriend\\b" << "\\bgoto\\b" << "\\binline\\b" << "\\bint\\b"
                    << "\\blong\\b" << "\\bmutable\\b" << "\\bnamespace\\b" << "\\bnew\\b" << "\\bnoexcept\\b"
                    << "\\bnullptr\\b" << "\\boperator\\b"
                    << "\\bregister\\b" << "\\breinterpret_cast\\b"
                    << "\\brequires\\b" << "\\breturn\\b" << "\\bshort\\b" << "\\bsigned\\b" << "\\bsizeof\\b"
                    << "\\bstatic\\b" << "\\bstatic_assert\\b" << "\\bstatic_cast\\b" << "\\bstruct\\b"
                    << "\\btemplate\\b" << "\\bthis\\b" << "\\bthread_local\\b" << "\\bthrow\\b" << "\\btrue\\b"
                    << "\\btypedef\\b" << "\\btypeid\\b" << "\\btypename\\b" << "\\bunion\\b" << "\\bunsigned\\b" << "\\busing\\b"
                    << "\\bvirtual\\b" << "\\bvoid\\b" << "\\bvolatile\\b" << "\\bwchar_t\\b" << "\\bnullptr\\b";
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    statementKeywordFormat.setForeground(QColor(249, 38, 114)); // Pink-ish
    QStringList statementKeywordPatterns;
    statementKeywordPatterns << "\\bif\\b" << "\\bfor\\b" << "\\bswitch\\b" << "\\bdo\\b" << "\\bwhile\\b" << "\\belse\\b"
                             << "\\band\\b" << "\\band_eq\\b" << "\\bxor\\b" << "\\bxor_eq\\b" << "\\bor\\b" << "\\bor_eq\\b"
                             << "\\bnot\\b" << "\\bnot_eq\\b" << "\\bbreak\\b" << "\\bcase\\b" << "\\bcatch\\b" << "\\btry\\b"
                             << "\\bconst\\b" << "\\bconstexpr\\b" << "\\bconst_cast\\b" << "\\bcontinue\\b"
                             << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b" << "#include\\b";
    foreach (const QString &pattern, statementKeywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = statementKeywordFormat;
        highlightingRules.append(rule);
    }

    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(QColor(166, 226, 46)); // Green-ish
    rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(QColor(117, 113, 94)); // Gray-ish
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(QColor(166, 226, 46)); // Green-ish

    quotationFormat.setForeground(QColor(230, 219, 88)); // Yellow-ish
    rule.pattern = QRegExp("\".*\""); // ".."
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    functionFormat.setFontItalic(true);
    functionFormat.setForeground(QColor(166, 226, 46)); // Light blue
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");
}

void CPPHighlighter::highlightBlock(const QString &text)
{
    setFormat(0, text.length(), normalText);

    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }

    // Special handling for #include <...> (the <..> part has to be distinguished from templates)
    QRegExp includeAngleBrackets("#include\\s*(<.*>)");
    int index = includeAngleBrackets.indexIn(text);
    while (index >= 0 && includeAngleBrackets.captureCount() > 0) {
       int length = includeAngleBrackets.matchedLength();
       setFormat(index + includeAngleBrackets.pos(1), includeAngleBrackets.cap(1).length(), quotationFormat);
       index = includeAngleBrackets.indexIn(text, index + length);
    }


    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text);

    while (startIndex >= 0) {

        int endIndex = commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }
}
