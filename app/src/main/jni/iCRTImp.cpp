#include "iCRTImp.h"
#include <boost/filesystem.hpp>
#include "FileUtils.hpp"
#include "ARReferenceInternal.h"
#include <JniHelper.h>

namespace BFS = boost::filesystem;

using namespace cv;
using namespace std;

#define MAX_QUEUE_SIZE 128

#include <android/log.h>
#define LOG(fmt, ...) __android_log_print(ANDROID_LOG_INFO, "iCRTImp", fmt, ##__VA_ARGS__)

static void __detect_worker_thread()
{
	iCRTImp::getInstance().detectThread();
}

static void __main_thread()
{
	iCRTImp::getInstance().mainThread();
}

static onUpdateARRef __updateCallback;
static onDetectCompelete __compeleteCallback;

/*
void showImage(string windowName, Mat matShow) {

	namedWindow(windowName, 1);
	imshow(windowName, matShow);
	waitKey();

}
*/

iCRTImp::iCRTImp()
{

	featureDetector_ = new cv::ORB(2300, 1.2f, 8, 31, 0, 2, ORB::HARRIS_SCORE, 31);
	descriptorExtractor_ = new cv::FREAK(true, true, 22.0f, 4);
	descriptorMatcher_ = new cv::BFMatcher(NORM_HAMMING, false);
	frameQueue_.reserve(MAX_QUEUE_SIZE);
	shouldDetectThreadExit = false;

}

iCRTImp::~iCRTImp()
{

}

iCRTImp::iCRTImp(iCRTImp const&)
{

}

//Mat sceneGrayMat, refDes, sceneDes;
//Mat homographyMat;
//int refid = 0;

//map<int, ARReferenceInternal> mapRefInternal;

void tracking(){

}


bool iCRTImp::getHomography(const std::vector<cv::KeyPoint> source, const std::vector<cv::KeyPoint> result, const std::vector<cv::DMatch>& input, std::vector<cv::DMatch>& inliers, cv::Mat& homography)
{
	if (input.size() < 4)
		return false;

	const int pointsCount = input.size();
	const float reprojectionThreshold = 2;

	//Prepare src and dst points
	std::vector<cv::Point2f> srcPoints, dstPoints;
	for (int i = 0; i < pointsCount; i++)
	{
		srcPoints.push_back(source[input[i].trainIdx].pt);
		dstPoints.push_back(result[input[i].queryIdx].pt);
	}

	// Find homography using RANSAC algorithm
	std::vector<unsigned char> status;
	homography = cv::findHomography(srcPoints, dstPoints, CV_RANSAC, reprojectionThreshold, status);

	// Warp dstPoints to srcPoints domain using inverted homography transformation
	std::vector<cv::Point2f> srcReprojected;
	cv::perspectiveTransform(dstPoints, srcReprojected, homography.inv());

	// Pass only matches with low reprojection error (less than reprojectionThreshold value in pixels)
	inliers.clear();
	for (int i = 0; i < pointsCount; i++)
	{
		cv::Point2f actual = srcPoints[i];
		cv::Point2f expect = srcReprojected[i];
		cv::Point2f v = actual - expect;
		float distanceSquared = v.dot(v);

		if (/*status[i] && */distanceSquared <= reprojectionThreshold * reprojectionThreshold)
		{
			inliers.push_back(input[i]);
		}
	}

	// Test for bad case
	if (inliers.size() < 4)
		return false;

	// Now use only good points to find refined homography:
	std::vector<cv::Point2f> refinedSrc, refinedDst;
	for (int i = 0; i < inliers.size(); i++)
	{
		refinedSrc.push_back(source[inliers[i].trainIdx].pt);
		refinedDst.push_back(result[inliers[i].queryIdx].pt);
	}

	// Use least squares method to find precise homography
	cv::Mat homography2 = cv::findHomography(refinedSrc, refinedDst, 0, reprojectionThreshold);

	// Reproject again:
	cv::perspectiveTransform(dstPoints, srcReprojected, homography2.inv());
	inliers.clear();

	for (int i = 0; i < pointsCount; i++)
	{
		cv::Point2f actual = srcPoints[i];
		cv::Point2f expect = srcReprojected[i];
		cv::Point2f v = actual - expect;
		float distanceSquared = v.dot(v);

		if (distanceSquared <= reprojectionThreshold * reprojectionThreshold)
		{
			inliers.push_back(input[i]);

			tracking();
		}
	}

	homography = homography2;
	return inliers.size() >= 4;
}

void iCRTImp::detectScene() {
	std::cout << "iCRTImp : processSceneFrame" << endl;
	
	BitmapData bmp;
	
	while (frameQueue_.pop(bmp) == false) {
		if (shouldDetectThreadExit == true)
			return;
		else 
			continue;
	}

	//  int bmplength = bmp.width* bmp.height * 3;
	//  cout << "bmplength : " << bmplength << endl;

	//  Mat sceneMat = Mat(bmp.height, bmp.width, CV_8UC3);
	//Mat sceneMat = Mat(bmp.height, bmp.width, CV_8UC4, bmp.bits32);
	//Mat sceneGrayMat = Mat(bmp.height, bmp.width, CV_8UC1);

    Mat sceneMat = Mat(bmp.height + bmp.height/2,bmp.width,CV_8UC1, bmp.bits32);
    Mat sceneGrayMat = Mat(bmp.height, bmp.width, CV_8UC1);



	// For Display
	//sceneMat.data = bmp.bits32;

	//for (int i = 0; i < bmplength; i += 3){
		//sceneMat.data[i] = bmp.bits32[i+2];
		//sceneMat.data[i+1] = bmp.bits32[i+1];
		//sceneMat.data[i+2] = bmp.bits32[i];
	//}

    // convert YUVNV21 to RGBA
    cvtColor(sceneMat, sceneMat,   CV_YUV2RGB_NV21 );
	//cvtColor(sceneMat, sceneMat, CV_BGRA2RGB);
	cvtColor(sceneMat, sceneGrayMat, CV_RGB2GRAY);

	//flip(sceneGrayMat, sceneGrayMat, 0);  //  圖像旋轉

	//namedWindow("", 1);
	//imshow("", sceneGrayMat);
	//waitKey();

	// make Keypoint
	featureDetector_->detect(sceneGrayMat, sceneKeypoints_);
	std::cout << "iCRT: sceneKeypoints size " << sceneKeypoints_.size() << endl; // print sceneKeypoints length

	// make Des
	descriptorExtractor_->compute(sceneGrayMat, sceneKeypoints_, sceneDes_);
	cout << "iCRT: sceneDes size " << sceneDes_.size() << endl; // print sceneDes lenght
}

bool iCRTImp::matchByKnn(const Mat patternDesc, const Mat sceneDesc, vector<DMatch> &resultMatches) {

	//vector<DMatch> xKnnMatches;
	if (patternDesc.empty() || sceneDesc.empty())
		return false;

	const float minRatio = 0.73f;
	resultMatches.clear();
	//xKnnMatches.clear();

	vector<vector<DMatch>> knnMatches;
	//knnMatches.clear();

	descriptorMatcher_->knnMatch(sceneDesc, patternDesc, knnMatches, 2);

	cout << "KNN Match: " << knnMatches.size() << endl;
	for (size_t i = 0; i < knnMatches.size(); i++) {
		const cv::DMatch& bestMatch = knnMatches[i][0];

		const cv::DMatch& betterMatch = knnMatches[i][1];
		float distanceRatio = bestMatch.distance / betterMatch.distance;

		if (distanceRatio < minRatio) {
			resultMatches.push_back(bestMatch);
		}
	}

	if (resultMatches.size()> 1){

		std::cout << "KNN resultMatches: " << resultMatches.size() << endl;
		return true;
	}
	else {
		return false;
	}
}


void iCRTImp::updateARInternal(int mapIndex, bool isTracking, bool detectType, Point2D TL, Point2D TR, Point2D BR, Point2D BL)
{
	if (mapRefInternal_.find(mapIndex) == std::end(mapRefInternal_))
		return;

	// update vectorRefInternal
	mapRefInternal_.at(mapIndex).detectType = detectType;
	mapRefInternal_.at(mapIndex).isTracking = isTracking;

	// clear four corners
	mapRefInternal_.at(mapIndex).BL = BL;
	mapRefInternal_.at(mapIndex).BR = BR;
	mapRefInternal_.at(mapIndex).TL = TL;
	mapRefInternal_.at(mapIndex).TR = TR;

		//cout << "BL: " << pBL.x << "," << pBL.y << endl;
    	//cout << "BR: " << pBR.x << "," << pBR.y << endl;
    	//cout << "TL: " << pTL.x << "," << pTL.y << endl;
    	//cout << "TR: " << pTR.x << "," << pTR.y << endl;

//Draw(BL.x, BL.y, BR.x, BR.y, TL.x, TL.y , TR.x, TRy )


}

void iCRTImp::cleanARInternal(int mapIndex){

	if(mapRefInternal_.find(mapIndex) == std::end(mapRefInternal_))
		return;

	mapRefInternal_.at(mapIndex).detectType = false;
	iCRTImp::getInstance().mapRefInternal_.at(mapIndex).isTracking = false;

        //mapRefInternal_.at(mapIndex).BL.x = 0;
        //mapRefInternal_.at(mapIndex).BL.y = 0;

        //mapRefInternal_.at(mapIndex).BR.x = 0;
        //mapRefInternal_.at(mapIndex).BR.y = 0;

        //mapRefInternal_.at(mapIndex).TL.x = 0;
        //mapRefInternal_.at(mapIndex).TL.y = 0;

        //mapRefInternal_.at(mapIndex).TR.x = 0;
        //mapRefInternal_.at(mapIndex).TR.y = 0;

	    mapRefInternal_.at(mapIndex).BL = Point2D();
	    mapRefInternal_.at(mapIndex).BR = Point2D();
	    mapRefInternal_.at(mapIndex).TL = Point2D();
	    mapRefInternal_.at(mapIndex).TR = Point2D();
}

void iCRTImp::mainThread() {

	boost::thread detectLoop(__detect_worker_thread);
	detectLoop.join();

}


void iCRTImp::detectThread(){
	std::cout << "iCRTImp : detect : " << endl;

	vector<DMatch> matches, inLinesMatches;

	matches.clear();  
	inLinesMatches.clear();

	while (shouldDetectThreadExit == false) {

		detectScene();
		std::map<int, ARReferenceInternal>::iterator it;
		for (it = mapRefInternal_.begin(); it != mapRefInternal_.end(); it++) {

            // match error
			matchByKnn(it->second.Descriptors, sceneDes_, matches);

			std::cout << "iCRTImp : matches : "<< matches.size()  << endl;


			if (matches.size() < 4) {
				cout << "Warning: matches.size()<4" << endl;

				it->second.isTracking = false;
				it->second.detectType = false;

				//__updateCallback(it->first);
				continue;
			}

			if (getHomography(it->second.Features, sceneKeypoints_, matches, inLinesMatches, homographyMat_)) {
				std::cout << "key: " << it->first << " --homographyMat: " << homographyMat_.size() << endl; // print refDesc length  

				// get refWidth,refHeight
				// CvSize size = it->second.Resolution;
				int refWidth = it->second.Resolution.width;
				int refHeight = it->second.Resolution.height;

				//  get matchedID and scene_corners from  perspectiveTransform
				vector<Point2f> obj_corners(4);
				vector<Point2f> scene_corners(4);
				obj_corners[0] = cvPoint(0, 0);
				obj_corners[1] = cvPoint(refWidth, 0);
				obj_corners[2] = cvPoint(refWidth, refHeight);
				obj_corners[3] = cvPoint(0, refHeight);
				perspectiveTransform(obj_corners, scene_corners, homographyMat_);

				int matchedID = it->first;
				cout << "INDEX: " << it->first << endl;
				cout << "matchedID: " << matchedID << endl;
				cout << "scene_corners[0] x: " << scene_corners[0].x << " scene_corners[0] y: " << scene_corners[0].y << endl;
				cout << "scene_corners[1] x: " << scene_corners[1].x << " scene_corners[1] y: " << scene_corners[1].y << endl;
				cout << "scene_corners[2] x: " << scene_corners[2].x << " scene_corners[2] y: " << scene_corners[2].y << endl;
				cout << "scene_corners[3] x: " << scene_corners[3].x << " scene_corners[3] y: " << scene_corners[3].y << endl;

				Point2D pBL, pBR, pTL, pTR;
				pTL.create(scene_corners[0].x, scene_corners[0].y);
				pTR.create(scene_corners[1].x, scene_corners[1].y);
				pBR.create(scene_corners[2].x, scene_corners[2].y);
				pBL.create(scene_corners[3].x, scene_corners[3].y);

                    //	pTL.create(20, 20);
                    //	pTR.create(120, 20);
                    //	pBL.create(20, 130);
                    //	pBR.create(120, 130);
				
				// update ARRef data
				iCRTImp::getInstance().updateARInternal(it->first, true, true, pTL, pTR, pBR, pBL);

				cout << "BL: " << pBL.x << "," << pBL.y << endl;
				cout << "BR: " << pBR.x << "," << pBR.y << endl;
				cout << "TL: " << pTL.x << "," << pTL.y << endl;
				cout << "TR: " << pTR.x << "," << pTR.y << endl;

				// 回傳 座標
               // Draw(pBL.x, pBL.y, pBR.x , pBR.y, pTL.x , pTL.y , pTR.x , pTR.y);

				//updateCallback(it->first,true);
				__updateCallback(it->first);

			}
			else{ // Homography Error
				  // set detectType ,  isTracking , clear all points
				iCRTImp::getInstance().cleanARInternal(it->first);
				cout << "Warning: getHomography parameter patternKpt.size()==0 || sceneKpt.size()==0 || matches.size()<4" << endl;
			}
		}
		__compeleteCallback();
	}
	return;
}


void iCRTImp::init(onInitCompelete callback) {
	bool flagInitSuccess = false;
	flagInitSuccess = true;


	boost::thread mainThread(__main_thread);



	callback(flagInitSuccess);
	return;
}


void iCRTImp::addARRef(int refid, char* path, onAddARRefCompelete callback)
{
	ARReferenceInternal refInternal;
	std::cout << "iCRTImp : addARRef " << endl;

	bool flagAddARREF = false;

    string strGzFileName = string(path);

    std::cout << "strBinFileName :" << strGzFileName << endl;

    FileUtils::MatStatsExt matGrayInfo;
    FileUtils::loadStatsFromZippedYaml(strGzFileName, matGrayInfo);

	CvSize size = cvSize(matGrayInfo.cols, matGrayInfo.rows);
	string strKeyPointPath = matGrayInfo.keyFilename;
	string strDesPath = matGrayInfo.descFilename;

    // 沒有 資料
	std::cout << "iCRT: strDesPath : " << strDesPath << endl;
	std::cout << "iCRT: strKeyPointPath : " << strKeyPointPath << endl;

	FileUtils::loadKeypoints(strKeyPointPath, refKeypoints_);

	cv::Mat refDes;
     //use strDesPath to get Des
    FileUtils::loadDescriptorsFromBin(strDesPath, refDes);

	refInternal.refID = refid;
	refInternal.path = path;
	refInternal.Features = refKeypoints_;
	refInternal.Resolution = size;
	refInternal.Descriptors = refDes;

	std::cout << "iCRT: refId : " << refid << endl;
	std::cout << "iCRT: path : " << path << endl;

	refInternal.isTracking = false;
	refInternal.detectType = false;

	refInternal.score = 0;
	refInternal.TL.x = 0;
	refInternal.TL.y = 0;
	refInternal.TR.x = 0;
	refInternal.TR.y = 0;

	refInternal.BL.x = 0;
	refInternal.BL.y = 0;
	refInternal.BR.x = 0;
	refInternal.BR.y = 0;

	mapRefInternal_.insert(pair<int, ARReferenceInternal>(refInternal.refID, refInternal));

	std::cout << "iCRT: RefKeypointSize : " << refKeypoints_.size() << endl;
	std::cout << "iCRT: Descriptors : " << refDes.size() << endl;

	flagAddARREF = true;
	callback(flagAddARREF, refid);

	return;
}

//void iCRTImp::detect(BitmapData bmp, onUpdateARRef updateCallback, onDetectCompelete compeleteCallback)
//{
//	__updateCallback = updateCallback;
//	__compeleteCallback = compeleteCallback;

//	frameQueue_.push(bmp);
//	return;
//}

void iCRTImp::detect(unsigned int width,unsigned int height,unsigned int hasAlpha,unsigned int isPremultiplied,unsigned int lineStride32, unsigned char*  bits32  , onUpdateARRef updateCallback, onDetectCompelete compeleteCallback)
{
       BitmapData bmp;
       bmp.width = width;
       bmp.height = height;
       bmp.hasAlpha = hasAlpha;
       bmp.isPremultiplied = isPremultiplied;
       bmp.lineStride32 = lineStride32;
       bmp.bits32 = bits32;

          std::cout << "iCRT: width : " << width << endl;
          std::cout << "iCRT: height : " << height << endl;

	    __updateCallback = updateCallback;
	    __compeleteCallback = compeleteCallback;

	      frameQueue_.push(bmp);

	return;
}


void iCRTImp::dispose(){
	cout << "iCRT: dispose " << endl;
	shouldDetectThreadExit = true;

	if (!featureDetector_.empty()){
		featureDetector_.empty();
	}

	if (!descriptorExtractor_.empty()){
		descriptorExtractor_.empty();
	}

	if (!descriptorMatcher_.empty()){
		descriptorMatcher_.empty();
	}

	if (sceneKeypoints_.size() != 0){
		sceneKeypoints_.clear();
	}

	if (refKeypoints_.size() != 0){
		refKeypoints_.clear();
	}

	if (!mapRefInternal_.empty()){
		mapRefInternal_.empty();
	}
	return;
}


ARReference & iCRTImp::getARRef(int refId){
	return mapRefInternal_.at(refId);
}

std::vector<ARReference>&  iCRTImp::getARRefs(){

	std::map<int, ARReferenceInternal>::const_iterator it;
	vectorARRef_.clear();
	for (it = mapRefInternal_.begin(); it != mapRefInternal_.end(); ++it) {
		vectorARRef_.push_back(it->second);
		cout << "vectorARRef_ item id : " << vectorARRef_.back().refID << endl;
	}

	return iCRTImp::getInstance().vectorARRef_;
}