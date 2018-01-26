/*
* iCRT.cpp
*
*  Created on: Mar 14, 2017
*      Author: ar9013
*/
#include "iCRTImp.h"
#include "iCRT.h"
#include <iostream>
#include <stdio.h>
#include <vector>

#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <JniHelper.h>

using namespace std;


iCRT::iCRT()
{

}

iCRT::~iCRT()
{

}

iCRT::iCRT(iCRT const&)
{

}


void iCRT::init(onInitCompelete callback ) {




	iCRTImp::getInstance().init(callback );

	return;
}


void iCRT::addARRef(int refId, char* path, onAddARRefCompelete callback)
{
	std::cout << "iCRT: addARRef " << endl;
    iCRTImp::getInstance().addARRef(refId, path, callback);
}

//void iCRT::detect(BitmapData bmp, onUpdateARRef updateCallback,
//	onDetectCompelete compeleteCallback) {

	//cout << "iCRT: detect " << endl;
	//iCRTImp::getInstance().detect(bmp, updateCallback, compeleteCallback);
//}

void iCRT::detect(unsigned int width,unsigned int height , unsigned int hasAlpha, unsigned int isPremultiplied , unsigned int lineStride32 , unsigned char*  bits32 , onUpdateARRef updateCallback ,
	onDetectCompelete compeleteCallback) {

       std::cout << "iCRT: detect " << endl;
       iCRTImp::getInstance().detect(width, height, hasAlpha, isPremultiplied, lineStride32,  bits32 ,  updateCallback, compeleteCallback);
}

void iCRT::dispose() {

	iCRTImp::getInstance().dispose();
}


ARReference& iCRT::getARRef(int refId){

	return iCRTImp::getInstance().getARRef(refId);
}


std::vector<ARReference>& iCRT::getARRefs(){

	return	iCRTImp::getInstance().getARRefs();
}
