#ifndef GIM_H
#define GIM_H
#include <QObject>
#include <opencv2/highgui/highgui.hpp>
//-----------------------------------
class Gim :public QObject
{
    Q_OBJECT
public:
    Gim();
    ~Gim();
    void init_cam(int camnum);
    void init_vid(std::string filePath);
    int  track_position;
    int  track_max_value;
    bool get_point(cv::Point *point);
    bool get_BB(cv::Rect_ <int> *bb);
    void correct_P(cv::Point *point, int width, int height);
    void correct_BB(cv::Rect_<int> *bb, int width, int height);
    void un_init();
    bool updateFrame();  // get new frame and save previous
    void showImage(cv::Mat image);
    cv::Mat image0; // previous frame
    cv::Mat image1; // current  frame
    cv::VideoCapture *capture;
    bool canceled;
    cv::Rect_<int> ROI;
    bool roiIsSet;
signals:
    void mouseBBDblClick();
    void mouseRClick(bool);
private:
    std::string gim_window_name;
    std::string gim_trackbar1_name;
};
//------------------------------------
#endif // GIM_H
