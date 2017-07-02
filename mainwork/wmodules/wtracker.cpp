#include "wtracker.h"
#include <qmath.h>
#include <QtAlgorithms>
#include "../../mainwindow.h"
//---------------------------------------------------------------
double WTracker::getMedianValue(cv::vector < float > arr)
{
    if(arr.size() < 1) return 0;
    if(arr.size() < 3) return arr[0];
    qSort(arr.begin(),arr.end());
    int mid = floor(arr.size()/2 +0.5);
    return arr[mid];
}
//---------------------------------------------------------------
float WTracker::precision(float a, float prec)
{
    return floor(prec * a) / prec;
}
//----------------------------------------------------------------------
int WTracker::findCorners(WFrameInfo *pf)
{
    // to prevent data access overflow
    pf->roi &= Rect_<int>(0,0,pf->frame.cols,pf->frame.rows);
    if(pf->roi.width  < MAX_CN_XY || pf->roi.height < MAX_CN_XY) return 0; // roi too little
    // find corners
    int stepX = floor(pf->roi.width  / MAX_CN_XY);
    int stepY = floor(pf->roi.height / MAX_CN_XY);
    int xtl   = pf->roi.x;
    int ytl   = pf->roi.y;
    int xbr = pf->roi.br().x;
    int ybr = pf->roi.br().y;
    pf->corners.clear();
    for(int y=ytl; y<=ybr; y+=stepY) {
        for(int x=xtl; x<=xbr; x+=stepX)
            pf->corners.push_back(Point2f(x,y));
    }
    return pf->corners.size();
}
//----------------------------------------------------------------------
int WTracker::cleanOFresults(WFrameInfo *pf0, WFrameInfo *pf1)
{
    if(pf1->corners.size() != pf0->corners.size()) return -1;
    vector <Point2f>::iterator it_con0      = pf0->corners.begin();
    vector <Point2f>::iterator it_con1      = pf1->corners.begin();
    vector <uchar>::iterator   it_status1   = pf1->status.begin();
    for(unsigned int i=0;i<pf1->corners.size();i++) {
        if(pf1->status[i] != 1) {
           pf0->corners.erase(it_con0    + i);
           pf1->corners.erase(it_con1    + i);
           pf1->status.erase( it_status1 + i);
        }
    }
    return pf1->corners.size();
}
//----------------------------------------------------------------------
void WTracker::changeScaleROI(Rect_<int> *roi, float dw, float dh)
{
    float sx, sy;
    sx = 0.5 * dw;
    sy = 0.5 * dh;
    int x1, y1, x2, y2;
    x1 = floor(roi->tl().x - sx +0.5);
    y1 = floor(roi->tl().y - sy +0.5);
    x2 = floor(roi->br().x + sx +0.5);
    y2 = floor(roi->br().y + sy +0.5);
    *roi  = Rect_<int>(Point(x1,y1),Point(x2,y2));
}
//----------------------------------------------------------------------
void WTracker::calcNewROILocation(WFrameInfo *pf0, WFrameInfo *pf1)
{
    // calc average scale changing and average ROI shift
    vector <float> vdx, vdy;
    float dx0, dx1, dy0, dy1;
    float scmid = 0.0;
    int   k = 0;
    for(unsigned int i=0;i<pf1->corners.size();i++) {
        for(unsigned int j=i+1;j<pf1->corners.size();j++) {
            dx0 = pf0->corners[i].x - pf0->corners[j].x;
            dx1 = pf1->corners[i].x - pf1->corners[j].x;
            dy0 = pf0->corners[i].y - pf0->corners[j].y;
            dy1 = pf1->corners[i].y - pf1->corners[j].y;
            if(dx0 != 0.0 && dy0 != 0.0) {
                scmid += sqrt( (dx1*dx1 + dy1*dy1)/(dx0*dx0 + dy0*dy0) );
                k++;
            }
        }
        vdx.push_back(pf1->corners[i].x - pf0->corners[i].x);
        vdy.push_back(pf1->corners[i].y - pf0->corners[i].y);
    }
    pf1->scChX = scmid / k;
    pf1->scChY = pf1->scChX;
    pf1->shX = precision(getMedianValue(vdx), 100.0);
    pf1->shY = precision(getMedianValue(vdy), 100.0);
    // calc new ROI using old and found shifts and scale
    pf1->roi = pf0->roi;
    pf1->roi.x = ceil(pf0->roi.x + pf1->shX);
    pf1->roi.y = ceil(pf0->roi.y + pf1->shY);
    float dw = (pf1->scChX - 1) * pf0->roi.width; //pf0->rw * pf0->iscChX - pf1->roi.width;//
    float dh = (pf1->scChY - 1) * pf0->roi.height; //pf0->rh * pf0->iscChY - pf1->roi.height;//
    changeScaleROI(&pf1->roi,dw, dh);

    //rectangle(gmainwindow->mainwork->frame1,pf1->roi,Scalar(0,255,255),2);
    pf1->roi &= Rect_<int>(0,0,pf1->frame.cols, pf1->frame.rows);
}
//----------------------------------------------------------------------
void WTracker::calcSaveAllShifts(WFrameInfo *pf1, WShifts *shs)
{
    shs->shiftX = pf1->shX;
    shs->shiftY = pf1->shY;
    shs->shiftXtoW = precision(pf1->shX / (pf1->frame.cols - 1.0), 10000.0);
    shs->shiftYtoH = precision(pf1->shY / (pf1->frame.rows - 1.0), 10000.0);

    shs->shiftCX = precision(pf1->roi.x +
                  (pf1->roi.width  - pf1->frame.cols)/2.0, 100.0);
    shs->shiftCY = precision(pf1->roi.y +
                  (pf1->roi.height - pf1->frame.rows)/2.0, 100.0);
    shs->shiftCXtoW = precision(2.0 * shs->shiftCX / (pf1->frame.cols-1.0), 10000.0);
    shs->shiftCYtoH = precision(2.0 * shs->shiftCY / (pf1->frame.rows-1.0), 10000.0);
}
//----------------------------------------------------------------------
void WTracker::changeTemplate(WFrameInfo *pf0, WFrameInfo *pf1, bool anyway)
{
    if((pf1->tmNcc > (TM_THRSH-0.05) && pf1->tmNcc < TM_THRSH) || anyway) {
       pf0->fr_template.release();
       pf0->fr_template = pf1->frame.operator ()(pf1->roi);

       pf0->rw = pf1->roi.width;
       pf0->rh = pf1->roi.height;
       pf0->iscChX = 1.0;
       pf0->iscChY = 1.0;
    } // else keep previous
}
//----------------------------------------------------------------------
bool WTracker::detectWithTM(WFrameInfo *pf0, WFrameInfo *pf1)
{
    Rect_<int> local_roi;
    local_roi.x = pf1->roi.x - pf1->roi.width  * (TM_ENCOEF - 1.0)/2.0;
    local_roi.y = pf1->roi.y - pf1->roi.height * (TM_ENCOEF - 1.0)/2.0;
    local_roi.width  = pf0->fr_template.cols * TM_ENCOEF;
    local_roi.height = pf0->fr_template.rows * TM_ENCOEF;
    //rectangle(gmainwindow->mainwork->frame1,local_roi,Scalar(0,0,255),2);
    // check roi correct
    local_roi &= Rect_<int>(0,0,pf1->frame.cols, pf1->frame.rows);
    // check for capability to detect template
    if(local_roi.width  < pf0->fr_template.cols ||
       local_roi.height < pf0->fr_template.rows) {
        gmainwindow->ctextEdit1->append("local_roi to low");
        return false;
    }
    // template matcher
    Mat src = pf1->frame.operator ()(local_roi);
    Mat res(local_roi.width  - pf0->fr_template.cols + 1,
            local_roi.height - pf0->fr_template.rows + 1, CV_32FC1);
    matchTemplate(src,pf0->fr_template,res,CV_TM_CCOEFF_NORMED);
    double minVal, maxVal;
    Point minLoc, maxLoc;
    minMaxLoc(res,&minVal,&maxVal,&minLoc,&maxLoc,Mat());
    pf1->tmNcc = maxVal;
    if(maxVal < (TM_THRSH - 0.4)) {
        gmainwindow->ctextEdit1->append("roi was not detected");
        return false;
    }
    // re-calc new roi location
    pf1->roi.x = maxLoc.x + local_roi.x;
    pf1->roi.y = maxLoc.y + local_roi.y;
    // change scale back if very good matching
    if(pf1->tmNcc >= TM_THRSH) {
        int dw = pf0->fr_template.cols - pf1->roi.width;
        int dh = pf0->fr_template.rows - pf1->roi.height;
        pf1->roi.width  = pf0->fr_template.cols;
        pf1->roi.height = pf0->fr_template.rows;        
        changeScaleROI(&pf1->roi,dw, dh);
    }
    //else { // keep as was calculated
    pf1->roi &= Rect_<int>(0,0,pf1->frame.cols, pf1->frame.rows);
    return true;
}
//----------------------------------------------------------------------
int WTracker::init_bb_tracking(Mat currFrame, Rect_<int> roi)
{
    roi &= Rect_<int>(0,0,currFrame.cols,currFrame.rows);
    if(roi.width <= MAX_CN_XY || roi.height <= MAX_CN_XY) return 0;
    // save prevFrame
    prevFrameInfo->resetAll();
    prevFrameInfo->frame        = currFrame.clone();
    prevFrameInfo->fr_template  = currFrame.operator ()(roi);
    prevFrameInfo->roi          = roi;
    prevFrameInfo->rw           = roi.width;
    prevFrameInfo->rh           = roi.height;
    ROI                         = roi;
    // calculate corners in ROI
    int cn = findCorners(prevFrameInfo);
    trackingEnabled = true;
    return cn;
}
//----------------------------------------------------------------------
int WTracker::init_fr_tracking(Mat currFrame)
{
    Rect_<int> block;
    block.x = 3*currFrame.cols / 8;
    block.y = 3*currFrame.rows / 8;
    block.width = currFrame.cols / 4;
    block.height= currFrame.rows / 4;
    int cn = init_bb_tracking(currFrame, block);
    trackingEnabled = true;
    return cn;
}
//----------------------------------------------------------------------
int WTracker::track(WFrameInfo *pf0, WFrameInfo *pf1, Rect_<int> *block)
{
    calcOpticalFlowPyrLK(pf0->frame,pf1->frame,
                         pf0->corners,pf1->corners,
                         pf1->status,pf1->error,Size(21,21),3,
                         TermCriteria(TermCriteria::COUNT+TermCriteria::EPS,50,0.001));
//pf1->corners.clear();
//pf1->status.clear();
//for(unsigned int i=0;i<pf0->corners.size();i++) {
//    pf1->corners.push_back(pf0->corners[i]);
//    pf1->status.push_back(1);
//}
    int cn = cleanOFresults(pf0, pf1);
    if(cn < (MAX_CN_XY * MAX_CN_XY * 0.2)) return 1;   // incomplete result, keep previous information for next loop
    // calculate shifts and scale change, locate new ROI
//pf1->roi = pf0->roi;
    calcNewROILocation(pf0,pf1);
    if(block == NULL) {
        // detect ROI by template match
        if(!detectWithTM(pf0, pf1)) return 0;
        if(pf1->roi.width  <= MAX_CN_XY || pf1->roi.height <= MAX_CN_XY) {
            gmainwindow->ctextEdit1->append("roi is too little");
            return 0;
        }
    }
    // save info of currFrame to prevFrame for next loop
 //   if(pf1->shX > 4 || pf1->shY > 4) {
        pf0->resetVectors();
        pf0->frame.release();
        pf0->frame = pf1->frame.clone();

    if(block == NULL) {
            pf0->roi = pf1->roi;
            changeTemplate(pf0,pf1);
    }
    else    {
        pf0->roi = *block;
        //changeTemplate(pf0,pf1);
    }
    //}

    // calculate corners in new ROI for next loop
    cn = findCorners(pf0);
    return cn;
}
//----------------------------------------------------------------------
int WTracker::track_bb(Mat newFrame)
{
    resIsValid = false;
    currFrameInfo->resetAll();
    currFrameInfo->frame = newFrame.clone();
    // find optical flow (same corners but in new frame)
    resIsValid = false;
    this->start();
    //int res = track(prevFrameInfo,currFrameInfo);
    //if(!res) {
    //   resetTracking();
    //   return 0;
    // }
    //if(res == 1) return 1;
    // calculate and save all shifts and ROI
    //calcSaveAllShifts(currFrameInfo, &shifts);
    //ROI = currFrameInfo->roi;
    //resIsValid = true;
    return 2; //res;
}
//----------------------------------------------------------------------
int WTracker::track_fr(Mat newFrame)
{
    resIsValid = false;
    currFrameInfo->resetAll();
    currFrameInfo->frame = newFrame.clone();
    // find optical flow (same corners but in new frame)
    Rect_<int> block;
    block.x = 3*newFrame.cols / 8;
    block.y = 3*newFrame.rows / 8;
    block.width = newFrame.cols / 4;
    block.height= newFrame.rows / 4;
    int res = track(prevFrameInfo,currFrameInfo,&block);
    if(!res) {
       resetTracking();
       return 0;
    }
    if(res == 1) return 1;
    // calculate and save all shifts and ROI
    calcSaveAllShifts(currFrameInfo, &shifts);
    ROI = currFrameInfo->roi;
    resIsValid = true;
    return res;
}
//----------------------------------------------------------------------
void WTracker::run()
{
    int res = track(prevFrameInfo,currFrameInfo);
    if(!res)  resetTracking();
    if(res == 1) return;
    calcSaveAllShifts(currFrameInfo, &shifts);
    ROI = currFrameInfo->roi;
    resIsValid = true;
}
//----------------------------------------------------------------------
void WTracker::resetTracking()
{
    trackingEnabled = false;
    resIsValid      = false;
    prevFrameInfo->resetAll();
    currFrameInfo->resetAll();
}
//---------------------------------------------------------------
