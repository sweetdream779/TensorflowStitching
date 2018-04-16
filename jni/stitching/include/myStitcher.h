#ifndef MYSTITCHER
#define MYSTITCHER

#include "homographyManager.h"

cv::Mat resizeImg(const cv::Mat& im, int newWidth = 600);
cv::Mat resizeMask(const cv::Mat& im, cv::Size size);

class MyStitcher
{
public:
    MyStitcher(Ptr<Feature2D> _detector, Ptr<DescriptorMatcher> _matcher, Ptr<DescriptorExtractor> _extractor) :
        detector(_detector),
        matcher(_matcher),
        extractor(_extractor)
    {
        homoManager = HomographyManager();
    }
    cv::Mat stitch(const cv::Mat& image1, const cv::Mat& image2, std::vector<cv::KeyPoint>& matched1, 
                                                                 std::vector<cv::KeyPoint>& matched2, int& borderX);
    cv::Mat fill_and_crop(cv::Mat& res, const cv::Mat& image2, const cv::Mat& image1);
    cv::Mat getMainHomo() {return m_mainHomo;}
    Ptr<Feature2D> getDetector() { return detector;}
protected:
    Ptr<Feature2D> detector;
    Ptr<DescriptorExtractor> extractor;
    Ptr<DescriptorMatcher> matcher;
    HomographyManager homoManager;
    cv::Mat m_mainHomo;

    void detect_and_compute(const cv::Mat& frame, std::vector<cv::KeyPoint> *kps, cv::Mat* descs);

    void visualize(cv::Mat& frame1, const std::vector<cv::KeyPoint> points1,
                   cv::Mat& frame2, const std::vector<cv::KeyPoint> points2, const cv::Scalar color);
};

#endif // MYSTITCHER