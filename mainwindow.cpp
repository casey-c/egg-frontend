#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "colorpalette.h"

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
