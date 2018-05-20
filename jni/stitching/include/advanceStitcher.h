//
// Created by irina on 15.04.18.
//

#ifndef TENSORFLOWSTITCHING_ADVANCESTITCHER_H
#define TENSORFLOWSTITCHING_ADVANCESTITCHER_H
#include "homographyManager.h"
#include "myStitcher.h"
#include "reconstructer.h"

namespace stitching {
    enum Status {   SUCCESS_WITHOUT_RECONSTRCT,
                    SUCCESS_WITH_FIRST_RECONSTRUCT,
                    SUCCESS_WITH_SECOND_RECONSTRUCT,
                    SUCCESS_FIRST_RECONSTRUCT_FAILED,
                    SUCCESS_SECOND_RECONSTRUCT_FAILED,
                    STITCHING_FAILED
    };

    class AdvanceStitcher {
    public:
        static AdvanceStitcher *Get(bool use_gdf, int homoNum);
        cv::Mat process(cv::Mat image1, cv::Mat image2, cv::Mat seg1, cv::Mat seg2);
        Status getStatus(){return m_status;}
        //changeParameters

    private:
        static AdvanceStitcher * advance_stitcher;
        MyStitcher* m_myStitcher;
        bool m_useGdf;
        int m_homoNum;
        Reconstructer* m_reconstructer;
        Status m_status;
        AdvanceStitcher(bool use_gdf, int homoNum): m_useGdf(use_gdf), m_homoNum(homoNum) {
            //Ptr<KAZE> akaze = KAZE::create();
            //akaze->setThreshold(akaze_thresh);

            //Ptr<FastFeatureDetector> detector=FastFeatureDetector::create();
            //detector->setThreshold(100);

            //Ptr<GFTTDetector> detector=GFTTDetector::create();

            Ptr<SURF> detector = SURF::create(400);
            Ptr<SurfDescriptorExtractor> extractor = SurfDescriptorExtractor::create();
            Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("FlannBased");

            m_myStitcher = new MyStitcher(detector, matcher, extractor);
            m_reconstructer = new Reconstructer(m_useGdf, m_homoNum);
        }
        void setStatus(Status s){
            m_status = s;
        }

    };
}

#endif //TENSORFLOWSTITCHING_ADVANCESTITCHER_H
