#include "mainwork.h"
#include "mainwindow.h" // for back pointer
#include <QtCore/qmath.h>
#include <QTime>
//------------------------------------------------------------------------
MainWork::MainWork()
{
    connect(&timer,SIGNAL(timeout()),this,SLOT(ontimerfunction()));
    working = false;
    //gmainwindow->ctextEdit1->append("MainWork module has been created.");
}
//------------------------------------------------------------------------
MainWork::~MainWork()
{
    if(working) stopWork();
    un_init();
}
//------------------------------------------------------------------------
void MainWork::init()
{
    gim   = new Gim;
    connect(gim,SIGNAL(mouseBBDblClick()),this,SLOT(onMouseBBDblClick()));
    connect(gim,SIGNAL(mouseRClick(bool)),this,SLOT(onMouseRClick(bool)));
}
//------------------------------------------------------------------------
void MainWork::un_init()
{
    un_init_gm();
    delete gim;
}
//------------------------------------------------------------------------
void MainWork::init_gm(int camnum, bool cammode)
{
    if(cammode) gim->init_cam(camnum);
    if(gim->updateFrame()) {
        gim->showImage(gim->image1);
    }
}
//------------------------------------------------------------------------
void MainWork::init_gm(std::string filepath, bool cammode)
{
    if(!cammode) gim->init_vid(filepath); // for input stream
    if(gim->updateFrame()) {
        gim->showImage(gim->image1);
        double fps = gim->capture->get(CV_CAP_PROP_FPS);
        gmainwindow->ctextEdit1->append(tr("Current FPS is %1").arg(fps));
    }
}
//------------------------------------------------------------------------
void MainWork::un_init_gm()
{
    gim->un_init();
}
//------------------------------------------------------------------------
void MainWork::doWork()
{
    if(gim->capture != NULL) {
        timer.setInterval(33.0); // first default value. During first loop it'll be changed
        working = true;
        gmainwindow->ctextEdit1->append("Working has been <b>started</b>");
        isontimer = false;
        timer.start();
    }    
}
//------------------------------------------------------------------------
void MainWork::stopWork()
{
    if(gim->capture != NULL) {
        timer.stop();
        working = false;
        gmainwindow->ctextEdit1->append("Working has been <b>stopped</b>");
    }
}
//------------------------------------------------------------------------
bool MainWork::updateFrame()
{
    if( !gim->updateFrame() ) return false;  // no more frames
    // else
    if(gmainwindow->crbHSVF->isChecked())
        cv::cvtColor(gim->image1,gim->image1,CV_BGR2HSV);
    else if(gmainwindow->crbGrayF->isChecked())
        cv::cvtColor(gim->image1,gim->image1,CV_BGR2GRAY);
    frame0 = gim->image0;//.clone();
    frame1 = gim->image1;//.clone();
    return true;
}
//------------------------------------------------------------------------
void MainWork::showInputVS()
{
    gim->showImage(gim->image1);
}
//------------------------------------------------------------------------
void MainWork::showOutputVS()
{
    gim->showImage(frame1);
}
//------------------------------------------------------------------------
void MainWork::wtrackalgprocess()
{
    cv::Mat tframe;
    tframe = frame1.clone();

    if(gmainwindow->ccbWTrackROI->isChecked()) {
        if(tframe.channels() > 1) cv::cvtColor(tframe,tframe,CV_BGR2GRAY);
        if(!wtracker.trackingEnabled) {
            cv::Rect_<int> roi;
            if(gim->roiIsSet) {
                roi = gim->ROI & cv::Rect_<int>(0,0,frame1.cols,frame1.rows);
                gim->roiIsSet = false;
            }
            else {
                gmainwindow->ctextEdit1->append("<b>Choose ROI on input image</b>");
                gim->get_BB(&roi);
            }
            if(!wtracker.init_bb_tracking(tframe,roi)) {
                gmainwindow->ctextEdit1->append("<b>Tracking has not been initialized</b>");
                gmainwindow->ccbWTrackROI->setChecked(false);
            }
        }
        else {
            int res = wtracker.track_bb(tframe);
            if(!res) {
                wtracker.resetTracking();
                gmainwindow->ccbWTrackROI->setChecked(false);
                gmainwindow->ctextEdit1->append("<b>Tracking object has been lost</b>");
                return;
            }
            //if(wtracker.resIsValid) {
            if(wtracker.trackingEnabled) {
                gmainwindow->ctextEdit1->append("----------------------------------");
                gmainwindow->ctextEdit1->append("<b>Shifts from center</b>");
                gmainwindow->ctextEdit1->append(tr("dX = <b>%1</b>").arg(wtracker.shifts.shiftCX));
                gmainwindow->ctextEdit1->append(tr("dY = <b>%1</b>").arg(wtracker.shifts.shiftCY));
                gmainwindow->ctextEdit1->append("<b>Relative shifts from center</b>");
                gmainwindow->ctextEdit1->append(tr("dX/w = <b>%1</b>").arg(wtracker.shifts.shiftCXtoW));
                gmainwindow->ctextEdit1->append(tr("dY/h = <b>%1</b>").arg(wtracker.shifts.shiftCYtoH));
                cv::rectangle(frame1,wtracker.ROI,cv::Scalar(0,255,0),2);
            }
        }
        return;
    }
    else if(gmainwindow->ccbWTrackFrames->isChecked()) {
        if(tframe.channels() > 1) cv::cvtColor(tframe,tframe,CV_BGR2GRAY);
        if(!wtracker.trackingEnabled) {
            wtracker.init_fr_tracking(tframe);
        }
        else {
            int res = wtracker.track_fr(tframe);
            if(!res) {
                wtracker.resetTracking();
                gmainwindow->ccbWTrackFrames->setChecked(false);
                gmainwindow->ctextEdit1->append("<b>Tracking has been lost</b>");
                return;
            }
            if(wtracker.resIsValid) {
                gmainwindow->ctextEdit1->append("----------------------------------");
                gmainwindow->ctextEdit1->append("<b>Absolute shifts");
                gmainwindow->ctextEdit1->append(tr("dX = <b>%1</b>").arg(wtracker.shifts.shiftX));
                gmainwindow->ctextEdit1->append(tr("dY = <b>%1</b>").arg(wtracker.shifts.shiftY));
                gmainwindow->ctextEdit1->append("<b>Relative shifts</b>");
                gmainwindow->ctextEdit1->append(tr("dX/w = <b>%1</b>").arg(wtracker.shifts.shiftXtoW));
                gmainwindow->ctextEdit1->append(tr("dY/h = <b>%1</b>").arg(wtracker.shifts.shiftYtoH));
            }            
        }
        return;
    }
    else {
        wtracker.resetTracking();
    }
}
//------------------------------------------------------------------------
void MainWork::ontimerfunction()
{
    if(isontimer) return; // to defend from self outrun
    QTime timeobj1, timeobj2;
    timeobj1.start();
    timeobj2.start();

    isontimer = true;
    //
    if(!updateFrame()) { // write new gim->image1 to frame1, image0 to frame0
        gmainwindow->ctextEdit1->append("<b>No more frames</b>. Working has been <b>stopped</b>");
        stopWork();
        return;
    }
    gmainwindow->ctextEdit1->append(tr("Frame updated, took time = <b>%1</b>").
                                    arg(timeobj1.restart()));
    //showInputVS();             // shows image1 (input)
    //
    bool res = processing();  // procceses frame1
    gmainwindow->ctextEdit1->append(tr("Frame was proccessed, took time = <b>%1</b>").
                                    arg(timeobj1.restart()));
    if(res) showOutputVS();   // show frame1 after it has been proccessed
    else showInputVS();
    //gmainwindow->ctextEdit1->append(tr("Proccessed frame was shown, took time = <b>%1</b>").
    //                                arg(timeobj1.restart()));
    double fps = gim->capture->get(CV_CAP_PROP_FPS);
    //gmainwindow->ctextEdit1->append(tr("Current FPS is %1").arg(fps));
    if(fps != 0) timer.setInterval(qCeil(1000.0 / fps));
    //
    if(wtracker.trackingEnabled)
        gmainwindow->ctextEdit1->append(tr("Proccessing took <b>%1 ms</b>").
                                                    arg(timeobj2.elapsed()));
    isontimer = false;
}
//------------------------------------------------------------------------
void MainWork::onMouseBBDblClick()
{
    if(!wtracker.trackingEnabled) return;
    if(gmainwindow->ccbWTrackROI->isChecked()) {
        wtracker.resetTracking();
    }
}
//------------------------------------------------------------------------
void MainWork::onMouseRClick(bool param)
{
    if(!param) wtracker.resetTracking();
    gmainwindow->ccbWTrackROI->setChecked(param);
}
//------------------------------------------------------------------------
bool MainWork::processing()
{
    /* Here one can call his own functions that will work with frame0, frame1.
     * The necessary functions you can define globally or by creating and including
     * your own class */
    // 1st process
    // working process
    wtrackalgprocess();
    // 2nd process
    //cv::addText(frame1,"Processed image",cv::Point(15,45),
    //            cv::fontQt("Arial",24,cvScalar(255,255,0,0),CV_FONT_BOLD,CV_STYLE_NORMAL));
    return true; // return TRUE if success
}
//------------------------------------------------------------------------

