/* Module, where the main work loop is defined and realized.
 * Previous and current frames are saved in 'frame0' and 'frame1',
 * that are of the opencv type of 'Mat' and the members of class
 * 'MainWork' (one can access them through the object 'mainwork').
 * To work with them one should insert the necessary processing code
 * into the function 'bool processing()' which is alse the member of
 * class 'MainWork'. The function is automatically called each loop of
 * timer. The timer interval is equal 10 ms if video stream is taken from
 * the camera or is a bit less or equal than '1/fps' (where 'fps' is
 * the current FPS rate of the input video stream) if video stream is
 * taken from the video file. */
#ifndef MAINWORK_H
#define MAINWORK_H
#include <QObject>
#include <QTimer>
#include <opencv2/core/core.hpp>
#include "modules/gim.h"
//#include "wmodules/odm.h"
//#include "wmodules/detectors.h"
//#include "wmodules/tracking.h"
#include "wmodules/wtracker.h"
//--------------------------------------
class MainWork : public QObject
{
    Q_OBJECT
public:
    MainWork();
    ~MainWork();
    void init();
    void un_init();
    void init_gm(std::string filepath, bool cammode=false);
    void init_gm(int camnum=0, bool cammode=true);
    void un_init_gm();
    void doWork();
    void stopWork();
    Gim   *gim;
    WTracker  wtracker;
    bool working;
    bool isontimer;
    bool updateFrame();
    void Filterapply(bool showtime=true);
    cv::Mat frame0, frame1;
private:
    QTimer timer;

    void showInputVS();
    void showOutputVS();
    bool processing();
    int  filterprocess(cv::Mat *frame);
    void wtrackalgprocess();

private slots:
    void ontimerfunction();
    void onMouseBBDblClick();
    void onMouseRClick(bool param);
};
//--------------------------------------
#endif // MAINWORK_H
