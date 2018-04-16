#include "myStitcher.h"

const double akaze_thresh = 0.01; // 3e-4 AKAZE detection threshold set to locate about 1000 keypoints
const double nn_match_ratio = 0.8f; // Nearest-neighbour matching ratio
const int bb_min_inliers = 100; // Minimal number of inliers to draw bounding box


void getDifferences(const cv::Mat& im1, const cv::Mat& im2){
    cv::Mat backgroundImage = im1.clone();
    cv::Mat currentImage = im2.clone();
    cv::Mat diffImage;
    cv::absdiff(backgroundImage, currentImage, diffImage);

    cv::Mat foregroundMask = cv::Mat::zeros(diffImage.rows, diffImage.cols, CV_8UC1);

    float threshold = 30.0f;
    float dist;

    for(int j=0; j<diffImage.rows; ++j)
        for(int i=0; i<diffImage.cols; ++i)
        {
            cv::Vec3b pix = diffImage.at<cv::Vec3b>(j,i);

            dist = (pix[0]*pix[0] + pix[1]*pix[1] + pix[2]*pix[2]);
            dist = sqrt(dist);

            if(dist>threshold)
            {
                foregroundMask.at<unsigned char>(j,i) = 255;
            }
        }
    //cv::imshow("diff", foregroundMask);
}


void MyStitcher::detect_and_compute(const cv::Mat& frame, std::vector<cv::KeyPoint> *kp, cv::Mat* desc){
    detector->detect(frame,*kp);

    if(!kp->empty())
        extractor->compute(frame, *kp, *desc);
    //std::cout<<"kp: "<<kp->size()<<std::endl;
}

cv::Mat resizeImg(const cv::Mat& im, int newWidth){
        cv::Mat  img = im.clone();
        //int newWidth = 600;
        double scale = (double)newWidth/(double)im.cols;
        int newHeight = (double)im.rows * (double)scale;
        //std::cout<<scale<<std::endl;
        cv::resize(img, img, cv::Size(), scale, scale);
        return img;
    }

cv::Mat resizeMask(const cv::Mat& im, cv::Size size)
{
    cv::Mat  img = im.clone();
    cv::resize(img, img, size);
    return img;
}

void visualizePoints(cv::Mat& canvas, const std::vector<cv::KeyPoint> points, const cv::Scalar color)
{
    for(auto point: points)
        cv::circle(canvas,Point2f(point.pt.x, point.pt.y), 2, color);
}

void MyStitcher::visualize(cv::Mat& frame1, const std::vector<cv::KeyPoint> points1, 
                           cv::Mat& frame2, const std::vector<cv::KeyPoint> points2,
                           const cv::Scalar color)
{
    
    //drawKeypoints(image2, kp2, image2);
    //drawKeypoints(image1, kp1, image1);

    /*
    drawMatches(image2, inliers2, image1, inliers1,
                inlier_matches, *vis,
                Scalar(255, 0, 0), Scalar(255, 0, 0));
    drawMatches(image2, outliers2, image1, outliers1,
                outlier_matches, *vis,
                Scalar(0, 0, 255), Scalar(0, 0, 255));
    */ 
    visualizePoints(frame1, points1, color);
    visualizePoints(frame2, points2, color);
}

cv::Mat MyStitcher::stitch(const cv::Mat& img2, const cv::Mat& img1,
                           std::vector<cv::KeyPoint>& matched1, std::vector<cv::KeyPoint>& matched2, int& borderX)
{
    cv::Mat image1 = img1.clone();
    cv::Mat image2 = img2.clone();

    vector<KeyPoint> kp1, kp2;
    Mat desc1, desc2;
    int inliers,matchesi;
    double ratio;
    Rect2d rect_for_searching;

    detect_and_compute(image1,&kp1,&desc1);
    detect_and_compute(image2,&kp2,&desc2);

    std::vector< vector<cv::DMatch> > matches;
    matcher->knnMatch(desc1, desc2, matches, 2); //дольше всех, другие матчинги????
    //std::cout<<"matches: "<<matches.size()<<std::endl;
    for(unsigned i = 0; i < matches.size(); i++) {
        //0.8: David Lowe’s ratio test for false-positive match pruning
        if(matches[i][0].distance < nn_match_ratio * matches[i][1].distance) {
            matched1.push_back(kp1[matches[i][0].queryIdx]);
            matched2.push_back(kp2[matches[i][0].trainIdx]);
        }
    }
    //save visualzation of matches
    std::vector<KeyPoint> inliers1, inliers2;
    std::vector<KeyPoint> outliers1, outliers2;
    homoManager.setMatchedPoints(matched1, matched2);
    cv::Mat homography = homoManager.findOneHomo(inliers1, inliers2, outliers1, outliers2);
    m_mainHomo = homography;

    cv::Mat vis;
    cv::Mat imclone1 = image1.clone();
    cv::Mat imclone2 = image2.clone();
    if (homography.empty())
    {
        Mat res;
        hconcat(image2, image1, res);
        //visualize(imclone1, matched1, imclone2, matched2, cv::Scalar(255,255,0));
        //hconcat(imclone2, imclone1, vis);
        //std::cout<<"Stitching failed!"<<std::endl;
        //imshow( "Vis", vis );
        return res;
    }

    //visualize(imclone1, inliers1, imclone2, inliers2, cv::Scalar(255,0,0));
    //visualize(imclone1, outliers1, imclone2, outliers2, cv::Scalar(0,0,255));
    //hconcat(imclone2, imclone1, vis);
    //imshow( "Vis", vis );

    //stitch
    cv::Mat res;
    cv::warpPerspective(image1, res, homography, cv::Size(image2.cols + image1.cols, image2.rows));
    
    //find y coordinate of the joint for image1
    std::vector<cv::Point2f> pts_in;
    pts_in.resize(1);
    std::vector<cv::Point2f> pts_out;
    pts_in[0] = cv::Point2f(image2.cols, image2.rows/2);
    cv::perspectiveTransform(pts_in, pts_out, homoManager.getInvertedH());
    borderX = pts_out[0].x;
    //std::cout<<pts_out[0].x<<std::endl;

    return res;
}


cv::Mat MyStitcher::fill_and_crop(cv::Mat& res, const cv::Mat& image2, const cv::Mat& image1)
{
    if(m_mainHomo.empty())
        return res;

    std::vector<cv::Point2f> pts_in;
    pts_in.resize(2);
    std::vector<cv::Point2f> pts_out;
    //fill left part of panorama
    cv::Mat roi1(res, Rect(0, 0,  image2.cols, image2.rows));
    image2.copyTo(roi1);

    //crop
    pts_in[0] = cv::Point2f(image1.cols,0);
    pts_in[1] = cv::Point2f(image1.cols, image1.rows);
    cv::perspectiveTransform(pts_in, pts_out, m_mainHomo);

    float newWidth  = pts_out[0].x < pts_out[1].x ? pts_out[0].x : pts_out[1].x;
    float newHeight = image2.rows;

    cv::Rect croppedroi(0, 0, newWidth, newHeight);
    res = res(croppedroi);
    
    return res;

}