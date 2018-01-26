#pragma once
#ifndef ICRT_IMP_H
#define ICRT_IMP_H

#include "iCRT.h"
#include <vector>
#include <iostream>
#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <boost/lockfree/queue.hpp>
#include <boost/thread/thread.hpp>

#include "ARReferenceInternal.h"

class iCRTImp
{

public:
	static iCRTImp& getInstance() {
		static iCRTImp _instance;
		return _instance;
	}
	void init(onInitCompelete callback);
	void addARRef(int refid, char* path, onAddARRefCompelete callback);
	
	//void detect(BitmapData bmp, onUpdateARRef updateCallback, onDetectCompelete compeleteCallback);
	void detect(unsigned int width,unsigned int height,unsigned int hasAlpha,unsigned int isPremultiplied,unsigned int lineStride32, unsigned char*  bits32 , onUpdateARRef updateCallback, onDetectCompelete compeleteCallback);
	void dispose();

	ARReference& getARRef(int refid);
	std::vector<ARReference>& getARRefs();

	void mainThread();
	void detectThread();

private:
	iCRTImp();
	iCRTImp(iCRTImp const&);
	iCRTImp& operator = (iCRTImp const&);
	~iCRTImp();

	void detectScene();
	void updateARInternal( int mapIndex, bool isTracking, bool detectType, Point2D TL, Point2D TR, Point2D BR, Point2D BL);
	void cleanARInternal(int mapIndex);
	

	bool matchByKnn(const cv::Mat patternDesc, const cv::Mat sceneDesc, std::vector<cv::DMatch> &resultMatches);
	bool getHomography(const std::vector<cv::KeyPoint> source, const std::vector<cv::KeyPoint> result, const std::vector<cv::DMatch>& input, std::vector<cv::DMatch>& inliers, cv::Mat& homography);

	std::map<int , ARReferenceInternal> mapRefInternal_;
	cv::Ptr<cv::FeatureDetector> featureDetector_;
	cv::Ptr<cv::DescriptorExtractor> descriptorExtractor_;
	cv::Ptr<cv::DescriptorMatcher> descriptorMatcher_;
	std::vector<cv::KeyPoint> sceneKeypoints_, refKeypoints_;	
	std::vector<ARReference> vectorARRef_;
	cv::Mat sceneDes_;
	cv::Mat homographyMat_;

	boost::lockfree::queue<BitmapData> frameQueue_{128};
	bool shouldDetectThreadExit;
};

#endif



