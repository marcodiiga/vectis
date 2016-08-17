#include <UI/Highlighters/WhiteTextHighlighter.h>

WhiteTextHighlighter::WhiteTextHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    normalText.setForeground(Qt::white);
}

void WhiteTextHighlighter::highlightBlock(const QString &text)
{
    setFormat(0, text.length(), normalText);
}
