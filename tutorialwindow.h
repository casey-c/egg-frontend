#ifndef TUTORIALWINDOW_H
#define TUTORIALWINDOW_H

#include <QWidget>

namespace Ui {
class TutorialWindow;
}

class TutorialWindow : public QWidget
{
    Q_OBJECT

public:
    explicit TutorialWindow(QWidget *parent = 0);
    ~TutorialWindow();

private:
    Ui::TutorialWindow *ui;
};

#endif // TUTORIALWINDOW_H
