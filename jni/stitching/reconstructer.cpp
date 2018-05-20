#include "reconstructer.h"

cv::Vec3b getColor(const cv::Mat& im, const cv::Point2f& pt)
{
	return im.at<cv::Vec3b>(cv::Point(pt.x, pt.y));
}

void gradientDomainFusion(std::vector<cv::Point2f>& maskPoints, cv::Mat& m, cv::Mat& s, cv::Mat& t, cv::Mat& res)
{
	int imH = m.rows;
	int imW = m.cols;
	int var = maskPoints.size();// number of variables to be solved
	//std::cout<<"var: "<<var<<std::endl;
	cv::Mat im2var(imH, imW, CV_32FC1, Scalar(0)); //matrix that maps each pixel to a variable number
	int i = 0;
	for (int j = 0; j < var; ++j)
	{
	    im2var.at<float>(maskPoints[j].y, maskPoints[j].x) = i;
	    i++;
	}

	cv::Mat A(var, var, CV_32FC1, Scalar::all(0));
	cv::Mat b(var, 3, CV_32FC1, Scalar::all(0));

	int e = 0;
	int y, x, ind;
	unsigned char * p1;
	unsigned char * p2;
	unsigned char * p3;

	for (int j = 0; j < maskPoints.size(); ++j)
	{
	    y = maskPoints[j].y;
	    x = maskPoints[j].x;
	    //set up coefficients for A; 4(center)-1(left)-1(right)-1(up)-1(down)
	    ind = im2var.at<float>(y, x); 
	    A.at<float>(e, ind) = 4;

	    if (m.at<int>(y-1, x) == 1)
	    {
	        //if pixel is within the mask, take the gradient from the source image
	        
		    ind = im2var.at<float>(y-1, x); 
		    A.at<float>(e,ind) = -1;
		    p1 = s.ptr(y-1,x);
	        p2 = s.ptr(y,x);
		    b.at<float>(e,0) = b.at<float>(e,0) - ((float)p1[0]/255 - (float)p2[0]/255);
		    b.at<float>(e,1) = b.at<float>(e,1) - ((float)p1[1]/255 - (float)p2[1]/255);
		    b.at<float>(e,2) = b.at<float>(e,2) - ((float)p1[2]/255 - (float)p2[2]/255);	     
	          
	    }
	    else
	    {
	        //otherwise, directly take the pixel value from the target image	
	        p3 = t.ptr(y-1,x);  
		    b.at<float>(e,0) = b.at<float>(e,0) + (float)p3[0]/255;
		    b.at<float>(e,1) = b.at<float>(e,1) + (float)p3[1]/255;
		    b.at<float>(e,2) = b.at<float>(e,2) + (float)p3[2]/255;
		    
	    }
	    if (m.at<int>(y+1, x) == 1)
	    {
	    	
		    ind = im2var.at<float>(y+1, x); 
		    A.at<float>(e,ind) = -1;
		    p1 = s.ptr(y+1,x);
	        p2 = s.ptr(y,x);

		    b.at<float>(e,0) = b.at<float>(e,0) - ((float)p1[0]/255 - (float)p2[0]/255);
		    b.at<float>(e,1) = b.at<float>(e,1) - ((float)p1[1]/255 - (float)p2[1]/255);
		    b.at<float>(e,2) = b.at<float>(e,2) - ((float)p1[2]/255 - (float)p2[2]/255);
		    
	    }
	    else
	    {
	    	p3 = t.ptr(y+1,x);  
	    	b.at<float>(e,0) = b.at<float>(e,0) + (float)p3[0]/255;
		    b.at<float>(e,1) = b.at<float>(e,1) + (float)p3[1]/255;
		    b.at<float>(e,2) = b.at<float>(e,2) + (float)p3[2]/255;

	    }
	    if (m.at<int>(y, x-1) == 1)
	    {
	        
		    ind = im2var.at<float>(y, x-1);
		    A.at<float>(e, ind) = -1;
		    p1 = s.ptr(y,x-1);
	        p2 = s.ptr(y,x);

		    b.at<float>(e,0) = b.at<float>(e,0) - ((float)p1[0]/255 - (float)p2[0]/255);
		    b.at<float>(e,1) = b.at<float>(e,1) - ((float)p1[1]/255 - (float)p2[1]/255);
		    b.at<float>(e,2) = b.at<float>(e,2) - ((float)p1[2]/255 - (float)p2[2]/255);
		    
	    }
	    else
	    {
	    		p3 = t.ptr(y,x-1);
		        b.at<float>(e,0) = b.at<float>(e,0) + (float)p3[0]/255;
		        b.at<float>(e,1) = b.at<float>(e,1) + (float)p3[1]/255;
		        b.at<float>(e,2) = b.at<float>(e,2) + (float)p3[2]/255;
		    
	    }
	    if (m.at<int>(y, x+1) == 1)
	    {
	    	ind = im2var.at<float>(y,x+1); 
		    A.at<float>(e, ind) = -1;
		    p1 = s.ptr(y,x+1);
	        p2 = s.ptr(y,x);

		    b.at<float>(e,0) = b.at<float>(e,0) - ((float)p1[0]/255 - (float)p2[0]/255);
		    b.at<float>(e,1) = b.at<float>(e,1) - ((float)p1[1]/255 - (float)p2[1]/255);
		    b.at<float>(e,2) = b.at<float>(e,2) - ((float)p1[2]/255 - (float)p2[2]/255);
		    
	    }
	    else
	    {
	    	p3 = t.ptr(y,x+1);
		    b.at<float>(e,0) = b.at<float>(e,0) + (float)p3[0]/255;
		    b.at<float>(e,1) = b.at<float>(e,1) + (float)p3[1]/255;
		    b.at<float>(e,2) = b.at<float>(e,2) + (float)p3[2]/255;
	    }    
	    e++;
	}
	//std::cout<<"System created."<<std::endl;
	cv::Mat v;
	//solve v for each rgb channel
	cv::solve(A, b, v);
	//std::cout<<"System solved."<<std::endl;
	//cv::Mat v = A.inv() * b;

	e = 0;
	//copy the values over to the target image to the area to be blended
	for (int i = 0; i < var; ++i)
	{
	    y = maskPoints[i].y;
	    x = maskPoints[i].x;
	    
	    res.at<cv::Vec3b>(cv::Point(x, y)) = cv::Vec3b(v.at<float>(e, 0)*255, v.at<float>(e, 1)*255, v.at<float>(e, 2)*255);
	    e++;
	}

}

cv::Mat Reconstructer::reconstructWithRemoval(cv::Mat imTgt, cv::Mat imSrc, cv::Size sizeTgt, 
											  std::vector <DataForMinimizer>& datas, std::vector <DataForMinimizer>& datasSrc,
											  std::vector<cv::KeyPoint>& matched1, std::vector<cv::KeyPoint>& matched2,
											  bool& answer)
{
	INIT_TIMER
	answer = false;
	//find several homographies which map source image to target coordinates
	homoManager.findSeveralHomo(m_homoNum, matched1, matched2);

	//remove all wrong homos
	homoManager.removeWrongHomo();

	std::vector<cv::Mat> homoSet = homoManager.getHomoSet();
	if(homoSet.size()==0)
		return imTgt;

	//homos map src image to target coordinates,
	homoManager.setTransformedSrcImgs(imSrc, cv::Size(imTgt.cols + imSrc.cols, imTgt.rows));

	//get rects of objects on tranfrmed src image
	//std::vector<std::vector<cv::Vec4f> > srcRectPoints = homoManager.getSrcRects(datasSrc);

	std::vector<cv::Mat> transformedSrcImgs = homoManager.getTransformedSrcImgs();	
	cv::Vec3b newValue;	
	cv::Mat mask, im_s;
	mask = cv::Mat::zeros( cv::Size(imTgt.cols, imTgt.rows), CV_8UC1 );
	im_s = cv::Mat::zeros( cv::Size(imTgt.cols, imTgt.rows), CV_8UC3 );

	cv::Mat result = imTgt.clone();
	for(auto& data: datas)
	{
		if(!data.onBorder)
			continue;
        __android_log_print(ANDROID_LOG_DEBUG, "Stitching", "START minimizing with %ld homos", m_homoNum);
		START_TIMER
		if(homoSet.size()>=2)
		    minimizer.optimize(transformedSrcImgs, imTgt, data, homoSet);
        else {
            int width = data.rect.width;
            int height = data.rect.height;
            int num_pixels = width*height;
            data.mainHomo = 0;
            data.homoIdxs.resize(num_pixels);
            std::fill(data.homoIdxs.begin(), data.homoIdxs.end(), 0);
        }
		STOP_TIMER("optimizer")

		if(m_useGdf){
			//copy rect
			cv::Rect re = boundingRect(data.maskPoints);
			cv::Mat part = transformedSrcImgs[data.mainHomo](re);
			cv::Mat roi1(im_s, re);
			part.copyTo(roi1);
		}
        __android_log_print(ANDROID_LOG_DEBUG, "Stitching", "START reconstructing with %ld points", data.points.size());
		for(int i = 0; i < data.points.size(); ++i)
		{
			int homoInd = data.homoIdxs[i];
			cv::Vec3b newValue = getColor(transformedSrcImgs[homoInd], data.points[i]);
			if(m_useGdf)
				im_s.at<cv::Vec3b>(cv::Point(data.points[i].x, data.points[i].y)) = newValue;

			if(data.needTransforms[i])
			{
				//if copied pixel from the srcImage is a pixel of another person
                for(DataForMinimizer& dataSrc: datasSrc)
				{
                    if(  (dataSrc.rect.x < data.points[i].x && data.points[i].x < dataSrc.rect.x + dataSrc.rect.width)
                       &&(dataSrc.rect.y < data.points[i].y && data.points[i].y < dataSrc.rect.y + dataSrc.rect.height) )
					{
						//std::cout<<"Reconstruct failed"<<std::endl;
						return imTgt;
					}
				}
				if(m_useGdf)
					mask.at<uchar>(cv::Point(data.points[i].x, data.points[i].y)) = 255;
				result.at<cv::Vec3b>(cv::Point(data.points[i].x, data.points[i].y)) = newValue;
			}
		}
	}
	if(m_useGdf){
		//run gradient domain fusion
		//cv::imshow("before gradient domain fusion", result);
		//std::cout<<"Now run poison image edditing"<<std::endl;
		START_TIMER
		blend::seamlessBlend(im_s, imTgt, mask, result);
		STOP_TIMER("blending")
	}
	//cv::imshow("result", result);
	answer = true;
	return result;
}

cv::Mat Reconstructer::reconstructWithAdding(const cv::Mat& homo, const cv::Mat& invHomo, std::vector <DataForMinimizer>& datas, std::vector <DataForMinimizer>& datasTgt,
							  				 const cv::Mat& imageTgt, const cv::Mat& imageSrc, const cv::Mat& res, int borderX, bool& answer)
{
	INIT_TIMER
	answer = false;
	if(homo.empty())
		return imageTgt;
	cv::Mat result = imageTgt.clone();
    cv::Mat mask = Mat::zeros( cv::Size(imageTgt.cols, imageTgt.rows), CV_8UC1 );
	for(auto& data: datas)
	{
		if(!data.onBorder)
			continue;
		std::vector<cv::Point2f> pts;
		cv::Vec3b newValue;
        __android_log_print(ANDROID_LOG_DEBUG, "Stitching", "START reconstructing with %ld points", data.points.size());
        for(cv::Point2f& pt: data.points)
        {
            if(pt.x < 0 || pt.x >= imageTgt.cols || pt.y < 0 || pt.y >= imageTgt.rows)
                continue;
            for(DataForMinimizer& dataTgt: datasTgt)
            {
                //if pixel is a pixel of another person on tgtImage
                if(   (dataTgt.rect.x  < pt.x && pt.x < dataTgt.rect.x + dataTgt.rect.width)
                      &&(dataTgt.rect.y  < pt.y && pt.y < dataTgt.rect.y + dataTgt.rect.height))
                {
                    //std::cout<<"Reconstruct failed, another object on reconstructed pixels"<<std::endl;
                    return imageTgt;
                }
            }
            newValue = getColor(res, pt);
            result.at<cv::Vec3b>(cv::Point(pt.x, pt.y)) = newValue;
            if(m_useGdf && pt.x < borderX){
                mask.at<uchar>(cv::Point(pt.x, pt.y)) = 255;
            }
        }
	}
	START_TIMER
    if(m_useGdf){
        //cv::imshow("before gradient domain fusion", result);
        //std::cout<<"Now run poison image editing"<<std::endl;

        cv::Rect rect(0, 0, imageTgt.cols, imageTgt.rows);
        cv::Mat croppedRes(res, rect);

        blend::seamlessBlend(croppedRes, result, mask, result);
		STOP_TIMER("blending")
    }
	//cv::imshow("ress",ress);
	//cv::waitKey(0);
	answer = true;
	return result;
}