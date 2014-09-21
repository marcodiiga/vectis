#ifndef CUSTOMCODEEDIT_H
#define CUSTOMCODEEDIT_H

#include <QTextEdit>
#include <QTimer>
#include <memory>

class CustomCodeEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit CustomCodeEdit(QWidget *parent = 0);

private:
    void scrollContentsBy ( int dx, int dy ) override;
    std::unique_ptr<QTimer> m_scrollTimer;

private slots:
    void timerScroll();

signals:

public slots:

};

#endif // CUSTOMCODEEDIT_H
