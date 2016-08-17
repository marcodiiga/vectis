#ifndef WHITETEXTHIGHLIGHTER_H
#define WHITETEXTHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

class WhiteTextHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    WhiteTextHighlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text) Q_DECL_OVERRIDE;

private:
    QTextCharFormat normalText;
};

#endif // WHITETEXTHIGHLIGHTER_H
