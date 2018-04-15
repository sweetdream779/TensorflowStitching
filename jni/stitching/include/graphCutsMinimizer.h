#ifndef GRAPHCUTSMINIMIZER
#define GRAPHCUTSMINIMIZER

#include <homographyManager.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "GCoptimization.h"

class GraphCutsMinimizer
{
public:
    GraphCutsMinimizer(){}
    GraphCutsMinimizer (const GraphCutsMinimizer& m_) = default;

    void optimize(std::vector<cv::Mat>& imsSrc, cv::Mat& imSrc, DataForMinimizer& data, std::vector<cv::Mat>& homoSet);

protected:

};
#endif // GRAPHCUTSMINIMIZER