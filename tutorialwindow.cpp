#include "tutorialwindow.h"
#include "ui_tutorialwindow.h"

TutorialWindow::TutorialWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TutorialWindow)
{
    ui->setupUi(this);
}

TutorialWindow::~TutorialWindow()
{
    delete ui;
}
