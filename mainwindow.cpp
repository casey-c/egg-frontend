#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "colorpalette.h"
#include "tutorialwindow.h"
#include "aboutwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    canvas = new Canvas(this);
    ui->workArea->addWidget(canvas);

    QActionGroup* group = new QActionGroup( this );
    ui->actionDark->setActionGroup(group);
    ui->actionLight->setActionGroup(group);

    connect(canvas, SIGNAL(toggleTheme()), this, SLOT(toggleTheme()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_actionNew_triggered()
{
    MainWindow* w2 = new MainWindow();
    w2->show();
}

void MainWindow::on_actionLight_triggered()
{
   ColorPalette::lightTheme();
   canvas->updateAll();
}

void MainWindow::on_actionDark_triggered()
{
   ColorPalette::darkTheme();
   canvas->updateAll();
}

// Lazy, not good practice, but good for presentation and debug so sorry
void MainWindow::toggleTheme() {
    if (ui->actionDark->isChecked())
        ui->actionLight->trigger();
    else
        ui->actionDark->trigger();
}

void MainWindow::on_actionTutorial_triggered()
{
    TutorialWindow* window = new TutorialWindow();
    window->show();
}

void MainWindow::on_actionAbout_triggered()
{
    AboutWindow* window = new AboutWindow();
    window->show();
}
