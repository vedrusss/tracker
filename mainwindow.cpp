#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDateTime>
#include <QApplication>
extern QApplication *gapp;
MainWindow *gmainwindow;
//-------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ctextEdit1 = ui->textEdit1;
    ctextEdit1->setFont(QFont("Arial",8,0,false));
    clineEditCamNum = ui->lineEditCamNum;
    crbBGRF     = ui->rbBGRF;
    crbHSVF     = ui->rbHSVF;
    crbGrayF    = ui->rbGrayF;
    ccbWTrackROI    = ui->cbTrackBB;
    ccbWTrackFrames = ui->cbTrackFrame;
    connect(ui->pushButtonCam,SIGNAL(clicked(bool)),this,SLOT(initcambtnclicked(bool)));
    connect(ui->pushButtonFile,SIGNAL(clicked(bool)),this,SLOT(initvidbtnclicked(bool)));
    connect(ui->pushButtonStart,SIGNAL(clicked()),this,SLOT(startbtnclicked()));
    connect(ui->pushButtonStop,SIGNAL(clicked()),this,SLOT(stopbtnclicked()));
    connect(ui->pushButtonQuit,SIGNAL(clicked()),gapp,SLOT(quit()));// for correct deleting
    connect(gapp,SIGNAL(destroyed()),this,SLOT(quitbtnclicked())); //     on winclose
    //ui->textEdit1->append("MainWindow has been created.");
}
//-------------------------------------------------------
MainWindow::~MainWindow()
{
    mainwork->~MainWork();
    delete ui;
}
//--------------------------------------------------------
void MainWindow::init()
{
    mainwork = new MainWork;
    mainwork->init();
}
//--------------------------------------------------------
void MainWindow::un_init()
{
    mainwork->un_init();
    delete mainwork;
}
//--------------------------------------------------------
void MainWindow::initcambtnclicked(bool checked)
{
    if(!checked & mainwork->working) { // first button becomes checked then clicked
        emit ui->pushButtonStop->clicked(); // cam is already running
        ui->pushButtonCam->setChecked(true);
        return;
    }
    if(checked) {  // first button becomes checked then clicked
        ui->textEdit1->append("Initializing camera.");
        int camnum = gmainwindow->clineEditCamNum->text().toInt();
        mainwork->init_gm(camnum);
        ui->pushButtonFile->setEnabled(false);        
    }
    else {
        mainwork->un_init_gm();
        ui->pushButtonFile->setEnabled(true);
        ui->textEdit1->append("Un_initializing camera.");
    }
}
//--------------------------------------------------------
void MainWindow::initvidbtnclicked(bool checked)
{
    if(!checked & mainwork->working) { //video file is already running
        emit ui->pushButtonStop->clicked();
        ui->pushButtonFile->setChecked(true);
        return;
    }
    if(checked) {  // switch on video
        QString filename = QFileDialog::getOpenFileName(this,tr("Choose video file..."),
                                                        "",tr("Video files (*.avi *.mp4 *.wmv *.mpg)"));
        if(filename != "") {
            ui->textEdit1->append("Initializing video.");
            ui->textEdit1->append("File <b>" + filename + "</b> has been opened");
            mainwork->init_gm(filename.toStdString());
            ui->pushButtonCam->setEnabled(false);
        }
        else { // no file was choosen
            ui->pushButtonFile->setChecked(false);
            return;
        }
    }
    else { // switch off video
        mainwork->un_init_gm();
        ui->pushButtonCam->setEnabled(true);
        ui->textEdit1->append("Un_initializing video.");
    }
}
//--------------------------------------------------------
void MainWindow::startbtnclicked()
{
    mainwork->doWork();
}
//--------------------------------------------------------
void MainWindow::stopbtnclicked()
{
    mainwork->stopWork();
}
//-------------------------------------------------------
void MainWindow::quitbtnclicked()
{
    mainwork->~MainWork();
    gapp->quit();
}
//--------------------------------------------------------
