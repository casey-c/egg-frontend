#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "canvas.h"
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionExit_triggered();
    void on_actionNew_triggered();
    void on_actionLight_triggered();
    void on_actionDark_triggered();
    void toggleTheme();

    void on_actionTutorial_triggered();

    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;
    Canvas* canvas;
};

#endif // MAINWINDOW_H
