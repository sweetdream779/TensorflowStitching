#include "homographyManager.h"

std::vector<cv::Point2f> Points(std::vector<cv::KeyPoint> matched){
    std::vector<cv::Point2f> res;
    for(int i=0;i<matched.size();i++){
        res.push_back(Point2f(matched[i].pt.x, matched[i].pt.y));
    } 
    return res;
}

cv::Mat HomographyManager::findOneHomo(std::vector<KeyPoint>& inliers1, std::vector<KeyPoint>& inliers2, 
									   std::vector<KeyPoint>& outliers1, std::vector<KeyPoint>& outliers2)
{
    //TODO speed up by replace KeyPoint => cv::Point2f 
    int matchesi = (int)m_matched1.size();
    //std::cout<<std::endl<<"matchesi: "<<matchesi<<std::endl;
    Mat inlier_mask, homography, inlier_mask2;
    if(m_matched1.size() >= 4) {
        homography = findHomography(Points(m_matched1), Points(m_matched2),
                                    RANSAC, ransac_thresh, inlier_mask);
        m_H_inv  = findHomography(Points(m_matched2), Points(m_matched1),
                                    RANSAC, ransac_thresh, inlier_mask2);
    }
    if(m_matched1.size() < 4 || homography.empty()) {
        return homography;
    }

    std::vector<DMatch> inlier_matches, outlier_matches;
    int j = 0;
    int new_i;
    for(unsigned i = 0; i < m_matched1.size(); i++) {
        //only process the match if the keypoint was successfully matched
        if(inlier_mask.at<uchar>(i)) {
            new_i = static_cast<int>(inliers1.size());
            inliers1.push_back(m_matched1[i]);
            inliers2.push_back(m_matched2[i]);
            inlier_matches.push_back(DMatch(new_i, new_i, 0));
        }
        else
        {
           new_i = static_cast<int>(outliers1.size());
           outliers1.push_back(m_matched1[i]);
           outliers2.push_back(m_matched2[i]);
           outlier_matches.push_back(DMatch(new_i, new_i, 0));
        }
    }
    int inliers = (int)inliers1.size();
    //std::cout<<"Inliers: "<<inliers1.size()<<std::endl;
    //std::cout<<"Outliers: "<<outliers1.size()<<std::endl;
    double ratio = inliers * 1.0 / matchesi;

    setMatchedPoints(outliers1, outliers2);

    return homography;
}


cv::Mat HomographyManager::findMainHomo(int k)
{
    //several homos, choose one by minimum differensec in color on border
}

void HomographyManager::findSeveralHomo(const int num, std::vector<cv::KeyPoint> matched1, std::vector<cv::KeyPoint> matched2)
{
    setMatchedPoints(matched1, matched2);
    for(int i = 0; i < num; ++i)
    {
        std::vector<KeyPoint> inliers1, inliers2;
        std::vector<KeyPoint> outliers1, outliers2;
        cv::Mat homo = findOneHomo(inliers1, inliers2, outliers1, outliers2);
        if(homo.empty())
            continue;
        m_homographySet.push_back(homo);
        m_invHomographySet.push_back(m_H_inv);
    }
}

std::vector<cv::Point2f> HomographyManager::transformPoints(std::vector<cv::Point2f>& pts_in, cv::Mat& homo)
{
    std::vector<cv::Point2f> pts_out(pts_in.size());
    cv::perspectiveTransform(pts_in, pts_out, homo);
    return pts_out;
}

//check if a homography not saves orientation
void HomographyManager::removeWrongHomo()
{
    //std::cout<<"Num of homography before wrongs deleting: "<<m_homographySet.size()<<std::endl;
    
    int i = 0;
    while(i < m_homographySet.size())
    {
        std::vector<cv::Point2f> pts_src(2);
        pts_src[0] = cv::Point2f(0,100);
        pts_src[1] = cv::Point2f(100,200);
        std::vector<cv::Point2f> pts_target = transformPoints(pts_src, m_homographySet[i]);
        if(pts_target[0].x > pts_target[1].x || pts_target[0].y > pts_target[1].y)
        {
            m_homographySet.erase(m_homographySet.begin() + i);
            m_invHomographySet.erase(m_invHomographySet.begin() + i);            
        }
        else
            ++i;
    }

    //std::cout<<"Num of homography after wrongs deleting: "<<m_homographySet.size()<<std::endl;

}

void HomographyManager::setTransformedSrcImgs(const cv::Mat imSrc, cv::Size size)
{
    int num = m_homographySet.size();
    m_transformedSrcImgs.resize(num);
    for(int i = 0; i < num; ++i)
    {
        cv::warpPerspective(imSrc, m_transformedSrcImgs[i], m_homographySet[i], size);
        //cv::imshow("src", m_transformedSrcImgs[i]);
        //cv::waitKey(0);
    }
}

void HomographyManager::setTransformedTgtImgs(const cv::Mat& imTgt, cv::Size size)
{   
    int num = m_homographySet.size();
    m_transformedTgtImgs.resize(num);
    for(int i = 0; i < num; ++i)
    {
        cv::warpPerspective(imTgt, m_transformedTgtImgs[i], m_homographySet[i], size);
        //cv::imshow("tg", m_transformedTgtImgs[i]);
        //cv::waitKey(0);
    }
}

std::vector<std::vector<cv::Vec4f> > HomographyManager::getSrcRects(std::vector<DataForMinimizer> datasSrc)
{
    std::vector<std::vector<cv::Vec4f> > out(datasSrc.size());

    std::vector<cv::Point2f> pts_in(4);
    std::vector<cv::Point2f> pts_out(4);
    for(int i = 0; i < datasSrc.size(); ++i)
    {
        out[i].resize(m_homographySet.size());
        pts_in[0] = cv::Point2f(datasSrc[i].rect.x, datasSrc[i].rect.y);//topLeft
        pts_in[1] = cv::Point2f(datasSrc[i].rect.x + datasSrc[i].rect.width, datasSrc[i].rect.y); //topRight
        pts_in[2] = cv::Point2f(datasSrc[i].rect.x + datasSrc[i].rect.width, datasSrc[i].rect.y + datasSrc[i].rect.height);//bottomRight
        pts_in[3] = cv::Point2f(datasSrc[i].rect.x, datasSrc[i].rect.y + datasSrc[i].rect.height);//bottomLeft
        for(int h = 0; h < m_homographySet.size(); ++h)
        {        
            cv::perspectiveTransform(pts_in, pts_out, m_homographySet[h]);
            cv::Vec4f v(pts_out[0].x, pts_out[1].x, pts_out[3].x, pts_out[2].x);
            out[i][h] = v;
            //std::cout<<"Transformed rect: "<<(float)v[0]<<" "<<(float)v[1]<<" "<<(float)v[2]<<" "<<(float)v[3]<<std::endl;
            //topLeft.x     = pts_out[0].x < pts_out[3].x ? pts_out[0].x : pts_out[3].x;
            //bottomRight.x = pts_out[1].x > pts_out[2].x ? pts_out[1].x : pts_out[2].x;
            //bottomRight.y = pts_out[2].y > pts_out[3].y ? pts_out[2].y : pts_out[3].y;
            //topLeft.y     = pts_out[0].y < pts_out[1].y ? pts_out[0].y : pts_out[1].y;
        }
    }
    return out;
}
