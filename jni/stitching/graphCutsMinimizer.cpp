#include "graphCutsMinimizer.h"

struct ForSmoothFn{
	DataForMinimizer *data;
};

float euclidNorm(cv::Vec3b& color1, cv::Vec3b& color2)
{
	float dr = 1.0*color1[0] - 1.0*color2[0];
	float dg = 1.0*color1[1] - 1.0*color2[1];
	float db = 1.0*color1[2] - 1.0*color2[2];

	return std::sqrt(dr*dr + dg*dg + db*db);
}

void getVtVs(const cv::Point2f& pt, const cv::Mat& homo, const cv::Mat& imTgt, const cv::Mat& imScr, cv::Vec3b& Vt, cv::Vec3b& Vs)
{
	/*std::vector<cv::Point2f> pts_in(1);
    pts_in[0] = pt;
    std::vector<cv::Point2f> pts_out(1);
    cv::perspectiveTransform(pts_in, pts_out, homo);*/

    Vt = imTgt.at<cv::Vec3b>(cv::Point(pt.x, pt.y));
    Vs = imScr.at<cv::Vec3b>(cv::Point(pt.x, pt.y));
}

void getVs(const cv::Point2f& pt, const cv::Mat& homo, const cv::Mat& imSrc, cv::Vec3b& Vs)
{
	/*std::vector<cv::Point2f> pts_in(1);
    pts_in[0] = pt;
    std::vector<cv::Point2f> pts_out(1);
    cv::perspectiveTransform(pts_in, pts_out, homo);*/

    Vs = imSrc.at<cv::Vec3b>(cv::Point(pt.x, pt.y));
}

int smoothFn(int p, int q, int l1, int l2, void *exData)
{
	ForSmoothFn *myData = (ForSmoothFn *) exData;
	DataForMinimizer* d = myData->data;

	cv::Vec3b Vs_p1 = d->colorsSrc[l1][p];
	cv::Vec3b Vs_p2 = d->colorsSrc[l2][p];

	cv::Vec3b Vs_q1 = d->colorsSrc[l1][q];
	cv::Vec3b Vs_q2 = d->colorsSrc[l2][q];


	return 10*(euclidNorm(Vs_p1, Vs_p2) + euclidNorm(Vs_q1, Vs_q2));
}


void GridGraph_DArraySArray(DataForMinimizer& d, int width, int height, 
							int num_pixels, int num_labels, std::vector<int>& result)
{

	result.resize(num_pixels);   // stores result of optimization

	// first set up the array for data costs
	int *data = new int[num_pixels*num_labels];
	std::vector<int> counts(num_labels,0);
	for ( int i = 0; i < num_pixels; i++ )
		for (int l = 0; l < num_labels; l++ )
		{
			cv::Vec3b Vt = d.colorsTgt[i];
			cv::Vec3b Vs = d.colorsSrc[l][i];

			data[i*num_labels+l] = euclidNorm(Vt, Vs);
		}
	
	try{
		GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(width, height, num_labels);
		gc->setDataCost(data);
		//gc->setSmoothCost(smooth);
		ForSmoothFn toFn;
		toFn.data = &d;

		gc->setSmoothCost(&smoothFn,&toFn);
		//printf("\nBefore optimization energy is %lld",gc->compute_energy());
		gc->swap(2);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		//printf("\nAfter optimization energy is %lld \n",gc->compute_energy());

		for ( int  i = 0; i < num_pixels; i++ )
		{
			int label = gc->whatLabel(i);
			result[i] = label;
			counts[label]++;
		}

		int maxVal = 0;
		int maxLabel = 0;
		for(int i = 0; i < num_labels; ++i)
		{
			if(counts[i] > maxVal)
			{
				maxVal = counts[i];
				maxLabel = i;
			}
		}
		d.mainHomo = maxLabel;

		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	delete [] data;

}

void GraphCutsMinimizer::optimize(std::vector<cv::Mat>& imsSrc, cv::Mat& imTgt, DataForMinimizer& data, std::vector<cv::Mat>& homoSet)
{
	int width = data.rect.width;
	int height = data.rect.height;
	int num_pixels = width*height;

	int num_labels = homoSet.size();

	data.colorsTgt.resize(num_pixels);
	data.colorsSrc.resize(num_labels);
	for(int l = 0; l< num_labels; ++l)
		data.colorsSrc[l].resize(num_pixels);

	for(int i = 0; i < num_pixels; ++i){
		for(int l = 0; l < num_labels; ++l)
		{
			/*if(!data.needTransforms[i])
			{
				data.colorsSrc[l][i] = Vec3b(0,0,0);
				if(l == 0)
					data.colorsTgt[i] = Vec3b(0,0,0);
			}*/
			if(l == 0)
				getVtVs(data.points[i], homoSet[l], imTgt, imsSrc[l],  data.colorsTgt[i], data.colorsSrc[l][i]);
			else
				getVs(data.points[i], homoSet[l], imsSrc[l], data.colorsSrc[l][i]);
		}
	}

	// smoothness and data costs are set up using arrays
	GridGraph_DArraySArray(data, width, height, num_pixels, num_labels, data.homoIdxs);

	/*std::cout<<"\nResult indexes: ";
	for(auto id: data.homoIdxs)
		std::cout<<id<<" ";
	std::cout<<"\n";*/
}
