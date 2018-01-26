
#include <iostream>
#include <vector>
#include <stdio.h>
#include "BasicStructs.h"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/opencv.hpp"

#ifndef _ARREFERENCEINTERNAL_
#define _ARREFERENCEINTERNAL_
typedef struct _ARReferenceInternal : ARReference
{
	std::vector<cv::KeyPoint> Features;
	cv::Mat Descriptors;
	CvSize Resolution;

} ARReferenceInternal;
#endif _ARREFERENCEINTERNAL_


