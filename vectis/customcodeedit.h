#ifndef CUSTOMCODEEDIT_H
#define CUSTOMCODEEDIT_H

#include <QTextEdit>
#include <QTimer>

class CustomCodeEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit CustomCodeEdit(QWidget *parent = 0);

private:
    void scrollContentsBy ( int dx, int dy ) override;

private slots:

signals:

public slots:

};

#endif // CUSTOMCODEEDIT_H
