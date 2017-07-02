#include "gim.h"
#include "mainwindow.h"
#include <QApplication>
#include <QTime>
extern QApplication *gapp;
//------------------------------------------------------------
cv::Point user_point(-1,-1);
cv::Rect_ <int> user_bb(-1,-1,0,0);
cv::Point startPoint(-1,-1);
bool bb_is_set = false;
//------------------------------------------------------------
static void onMouseDblClkForBB(int event,int x, int y, int flags, void *param)
{
    Gim *pgim = (Gim*) param;
    if(event == CV_EVENT_LBUTTONDBLCLK) {
        pgim->ROI.x = x - pgim->ROI.width/2;
        pgim->ROI.y = y - pgim->ROI.height/2;
        pgim->roiIsSet = true;
        emit pgim->mouseBBDblClick();
        return;
    }
    if(event == CV_EVENT_RBUTTONDBLCLK) {
        pgim->roiIsSet = false;
        emit pgim->mouseRClick(true);
        return;
    }
    if(event == CV_EVENT_RBUTTONDOWN) {
        pgim->roiIsSet = false;
        emit pgim->mouseRClick(false);
        return;
    }
}
//------------------------------------------------------------
static void onMouseForPoint(int event,int x, int y, int /*flags*/, void */*param*/)
{
    if(event == CV_EVENT_LBUTTONDOWN) {
        user_point.x = x;
        user_point.y = y;
    }
}
//------------------------------------------------------------
static void onMouseForBB(int event,int x, int y, int flags, void */*param*/)
{
    if(event == CV_EVENT_LBUTTONDOWN) {
        startPoint.x = x;
        startPoint.y = y;
        bb_is_set = false;
        return;
    }
    if((flags == CV_EVENT_FLAG_LBUTTON) & (event == CV_EVENT_MOUSEMOVE)) {
        user_bb.width  = abs(x - startPoint.x);
        user_bb.height = abs(y - startPoint.y);
        if(x < startPoint.x) user_bb.x = x;
        else                 user_bb.x = startPoint.x;
        if(y < startPoint.y) user_bb.y = y;
        else                 user_bb.y = startPoint.y;
        return;
    }
    if(event == CV_EVENT_LBUTTONUP) {
        user_bb.width  = abs(x - startPoint.x);
        user_bb.height = abs(y - startPoint.y);
        if(x < startPoint.x) user_bb.x = x;
        else                 user_bb.x = startPoint.x;
        if(y < startPoint.y) user_bb.y = y;
        else                 user_bb.y = startPoint.y;
        bb_is_set = true;
        return;
    }
}
//------------------------------------------------------------
void onTrackBar1Change(int position, void* userdata)
{
    cv::VideoCapture * lcapture = (cv::VideoCapture *) userdata;
    lcapture->set(CV_CAP_PROP_POS_FRAMES, position);
}
//------------------------------------------------------------
Gim::Gim()
{
    gim_window_name = "Input video stream";
    gim_trackbar1_name = "FrP";
    capture = NULL;   // For right correct working un_init() function
    canceled = false;
    roiIsSet = false;
    //gmainwindow->ctextEdit1->append("GIM has been created.");
}
//------------------------------------------------------------
Gim::~Gim()
{
    un_init();
}
//------------------------------------------------------------
void Gim::init_cam(int camnum)
{   // capturing from camera mode
    un_init(); // for correct operation if already was inited
    cv::namedWindow(gim_window_name.c_str(), CV_WINDOW_NORMAL | CV_GUI_NORMAL); //CV_WINDOW_AUTOSIZE
    cv::moveWindow(gim_window_name.c_str(), 200, 200);
    capture = new cv::VideoCapture(camnum);

    if(!capture->isOpened())
      gmainwindow->ctextEdit1->append("Unable to initialize capture!");
    // this to be modified
    else capture->operator >>(image1);
}
//------------------------------------------------------------
void Gim::init_vid(std::string filePath)
{   // capturing from videofile mode
    un_init(); // for correct operation if already was inited
    cv::namedWindow(gim_window_name.c_str(), CV_WINDOW_NORMAL | CV_GUI_NORMAL);
    cv::moveWindow(gim_window_name.c_str(), 200, 200);
    capture = new cv::VideoCapture(filePath.c_str());

    if(!capture->isOpened())
      gmainwindow->ctextEdit1->append("Unable to initialize capture!");
    // this to be modified
    else capture->operator >>(image1);
    // adding trackbar
    track_max_value = capture->get(CV_CAP_PROP_FRAME_COUNT);
    //if (track_max_value == 0) track_max_value = 1000;
    cv::createTrackbar(gim_trackbar1_name,gim_window_name,
                       &track_position,track_max_value,onTrackBar1Change, capture);
}
//------------------------------------------------------------
bool Gim::get_point(cv::Point *point)
{
    if(capture == NULL) return false;
    canceled = false;
    user_point = *point; // -1;-1
    cv::setMouseCallback(gim_window_name.c_str(),onMouseForPoint,0);
    int framewidth  = capture->get(CV_CAP_PROP_FRAME_WIDTH);
    int frameheight = capture->get(CV_CAP_PROP_FRAME_HEIGHT);
    for(;;) {
        cv::waitKey(1); // for mouse events to be solved
        if(user_point != *point) {
            point->x = user_point.x;
            point->y = user_point.y;
            correct_P(point,framewidth,frameheight);
            break;
        }
        if(canceled) {
            image0 = image1.clone();
            showImage(image0);
            cv::setMouseCallback(gim_window_name.c_str(),NULL,NULL);
            return false;
        }
    }
    cv::setMouseCallback(gim_window_name.c_str(),NULL,NULL);
    cv::circle(image0,cv::Point(point->x,point->y),3,cvScalar(0,255,255,0),3);
    showImage(image0);
    return true;
}
//------------------------------------------------------------
bool Gim::get_BB(cv::Rect_<int> *bb)
{
    if(capture == NULL) return false;
    canceled = false;
    user_bb = *bb;
    cv::setMouseCallback(gim_window_name.c_str(),onMouseForBB,0);
    int framewidth  = capture->get(CV_CAP_PROP_FRAME_WIDTH);
    int frameheight = capture->get(CV_CAP_PROP_FRAME_HEIGHT);
    for(;;) {
        cv::Mat tempframe;
        tempframe = image0.clone();
        cv::waitKey(1); // for mouse events to be solved
        if(bb_is_set) {
            bb->x = user_bb.x;
            bb->y = user_bb.y;
            bb->width  = user_bb.width;
            bb->height = user_bb.height;
            correct_BB(bb,framewidth,frameheight);
            bb_is_set = false;
            break;
        }
        if(canceled) {
            image0 = image1.clone();
            showImage(image0);
            bb_is_set = false;
            cv::setMouseCallback(gim_window_name.c_str(),NULL,NULL);
            return false;
        }
        //image0 = image1.clone();
        cv::rectangle(tempframe,user_bb,cvScalar(0,255,255,0),2);
        showImage(tempframe);
    }
    //image0 = image1.clone();
    cv::setMouseCallback(gim_window_name.c_str(),onMouseDblClkForBB,this);
    cv::rectangle(image0,user_bb,cvScalar(0,255,255,0),2);
    showImage(image0);
    ROI = user_bb;
    roiIsSet = false;
    return true;
}
//------------------------------------------------------------
void Gim::correct_P(cv::Point *point, int width, int height)
{
    if(point->x < 0)       point->x = 0;
    if(point->x >= width)  point->x = width  - 1;
    if(point->y < 0)       point->y = 0;
    if(point->y >= height) point->y = height - 1;
}
//------------------------------------------------------------
void Gim::correct_BB(cv::Rect_<int> *bb, int width, int height)
{
    if(bb->x < 0) {
        bb->width += bb->x;
        bb->x = 0;
    }
    if(bb->y < 0) {
        bb->height += bb->y;
        bb->y = 0;
    }
    if(bb->x >= width)  bb->x = width  - 1;
    if(bb->y >= height) bb->y = height - 1;
    if(bb->x + bb->width  >= width)  bb->width  = width  - bb->x;
    if(bb->y + bb->height >= height) bb->height = height - bb->y;
}
//------------------------------------------------------------
void Gim::un_init()
{
    canceled = true; // to stop loops if initialized
    if(capture != NULL) {
       delete capture;
       capture = NULL; // this for 'if' to operate correctly
       cv::destroyWindow(gim_window_name.c_str());
    }
}
//------------------------------------------------------------
bool Gim::updateFrame()  // getNewFrame
{
    QTime timeobj;
    timeobj.start();
    image0 = image1.clone();
    if( !capture->read(image1) ) return false;
    //cv::waitKey(1);   // very necessary for image showing in window
    //gmainwindow->ctextEdit1->append(tr("<b>Updating time = %1").arg(timeobj.elapsed()));
    return true;
}
//---------------------------------------------------------------------------
void Gim::showImage(cv::Mat image)
{
    cv::imshow(gim_window_name.c_str(), image);
}
//---------------------------------------------------------------------------
