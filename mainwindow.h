#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QLineEdit>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QToolButton>
#include <QMenu>
#include "mainwork/mainwork.h"
//------------------------------------------------
namespace Ui {
class MainWindow;
}
//------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void init();
    void un_init();
    Ui::MainWindow *ui;
    QTextEdit *ctextEdit1;
    QLineEdit *clineEditCamNum;
    QLineEdit *ceditFSize;
    QRadioButton *crbBGRF, *crbHSVF, *crbGrayF;
    QCheckBox *ccbWTrackROI, *ccbWTrackFrames;

    MainWork *mainwork;
private:

private slots:
    void initcambtnclicked(bool checked);
    void initvidbtnclicked(bool checked);
    void startbtnclicked();
    void stopbtnclicked();
    void quitbtnclicked();
};
extern MainWindow *gmainwindow;
//------------------------------------------------
#endif // MAINWINDOW_H
