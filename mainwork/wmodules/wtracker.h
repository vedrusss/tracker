#ifndef WTRACKER_H
#define WTRACKER_H

#define MAX_CN_XY 6
#define TM_THRSH  0.96
#define TM_ENCOEF 3.0
#include <opencv/cv.h>
#include <QThread>
using namespace cv;
//--------------------------------------------------------------------------------------
struct WFrameInfo
{
    Mat                 frame;
    Mat                 fr_template;
    Rect_  <int>        roi;
    vector <Point2f>    corners;
    vector <float>      error;
    vector <uchar>      status;
    vector <float>      ncc;
    float               tmNcc;
    float               scChX, scChY;
    float               iscChX, iscChY;
    int                 rw, rh;
    float               shX, shY;
    WFrameInfo() {
        iscChX = 1.0;
        iscChY = 1.0;
    }
    ~WFrameInfo() {
        resetAll();
    }
    void    resetVectors(){
        corners.clear();
        error.clear();
        status.clear();
        ncc.clear();
    }
    void    resetAll(){
        frame.release();
        fr_template.release();
        iscChX = 1.0;
        iscChY = 1.0;
        resetVectors();
    }
};
//-----------------------------------------------------------------------------------------
struct WShifts
{
    float           shiftCXtoW, shiftCYtoH;
    float           shiftXtoW, shiftYtoH;
    float           shiftX, shiftY;
    float           shiftCX, shiftCY;
};
//-----------------------------------------------------------------------------------------
class WTracker : public QThread
{
    Q_OBJECT
private:
    WFrameInfo      *prevFrameInfo;
    //WFrameInfo      *currFrameInfo;
    int         track(WFrameInfo *pf0, WFrameInfo *pf1, Rect_<int> *block=NULL);
    int         findCorners(WFrameInfo *pf);
    int         cleanOFresults(WFrameInfo *pf0, WFrameInfo *pf1);
    void        calcNewROILocation(WFrameInfo *pf0, WFrameInfo *pf1);
    double      getMedianValue(vector < float > arr);
    float       precision(float a, float prec);
    bool        detectWithTM(WFrameInfo *pf0, WFrameInfo *pf1);
    void        calcSaveAllShifts(WFrameInfo *pf1, WShifts *shs);
    void        changeTemplate(WFrameInfo *pf0, WFrameInfo *pf1, bool anyway=false);
    void        changeScaleROI(Rect_<int> *roi, float dw, float dh);


public:
    WFrameInfo      *currFrameInfo;
    WTracker(QObject *parent=0) : QThread(parent) {
      prevFrameInfo = new WFrameInfo;
      currFrameInfo  = new WFrameInfo;
      trackingEnabled = false;
      resIsValid      = false;
    }
    ~WTracker() {
      prevFrameInfo->resetAll();
      currFrameInfo->resetAll();
      delete prevFrameInfo;
      delete currFrameInfo;
    }
    void run();
    Rect_<int>  ROI;
    WShifts     shifts;
    bool        trackingEnabled;
    bool        resIsValid;
    int     init_bb_tracking(Mat currFrame, Rect_<int> roi);
    int     init_fr_tracking(Mat currFrame);
    int     track_bb(Mat newFrame);
    int     track_fr(Mat newFrame);
    void    resetTracking();
};
//-----------------------------------------------------------------------------------------
#endif // WTRACKER_H
