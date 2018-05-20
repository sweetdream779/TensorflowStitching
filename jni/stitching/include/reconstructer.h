#ifndef RECONSTRUCTER
#define RECONSTRUCTER

#include "homographyManager.h"
#include "graphCutsMinimizer.h"
#include <blend/blend.h>
#include "timer.h"

class Reconstructer
{
public:
    Reconstructer(){ 
    	homoManager = HomographyManager();
    	minimizer 	= GraphCutsMinimizer();
    }
    Reconstructer(bool use_gdf, int homoNum): m_useGdf(use_gdf), m_homoNum(homoNum){ 
    	homoManager = HomographyManager();
    	minimizer 	= GraphCutsMinimizer();
    }

    cv::Mat reconstructWithRemoval(cv::Mat imTgt, cv::Mat imSrc, cv::Size size, std::vector <DataForMinimizer>& datas, 
    		std::vector <DataForMinimizer>& datasSrc, std::vector<cv::KeyPoint>& matched1, std::vector<cv::KeyPoint>& matched2, bool& answer);
	cv::Mat reconstructWithAdding(const cv::Mat& homo, const cv::Mat& invHomo, std::vector <DataForMinimizer>& datas,
	    		std::vector <DataForMinimizer>& datasTgt, const cv::Mat& image1, const cv::Mat& image2, const cv::Mat& res, int borderX, bool& answer);
protected:
    HomographyManager homoManager;
    GraphCutsMinimizer minimizer;

    int m_homoNum;
    bool m_useGdf;

};
#endif // RECONSTRUCTER