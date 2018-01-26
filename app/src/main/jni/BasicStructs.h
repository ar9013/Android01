#pragma once
/**
Theme: FlashAR
Compiler: Microsoft Visual Studio 2012 Express
Date: 2017/02/17
Author: LeesonHsu
Email:leesonhsu@arplanet.com.tw
Blog: www.arplanet.com.tw
*/

#ifndef BasicStructs_h
#define BasicStructs_h
#include <stdio.h>
#include <vector>

//
// This structure describes a Reference Object
//
//
// This structure describes a point in 2D space
//

#ifndef _Point2D_
#define _Point2D_
typedef struct _Point2D
{
	double x;
	double y;
	void create(double x, double y)
	{
		this->x = x;
		this->y = y;
	};
} Point2D;
#endif _Point2D_


#ifndef _ARReference_
#define _ARReference_
typedef struct _ARReference
{
	int refID;/// reference ID 
	int score;// reference score
	char* path; /// reference path
	bool isTracking; ///reference isTracking
	bool detectType; ///reference detectType

	Point2D BL; /// reference Bottom Left
	Point2D BR; /// reference Bottom Right
	Point2D TL; /// reference Top Left
	Point2D TR; /// reference Top Right

} ARReference;
#endif _ARReference_

//
// This structure describes a sourceImage
//

#ifndef _BitmapData_
#define _BitmapData_
typedef struct _BitmapData
{
	unsigned int     width;
	unsigned int     height;
	unsigned int     hasAlpha;
	unsigned int     isPremultiplied;
	unsigned int     lineStride32;
	unsigned char*   bits32;

} BitmapData;
#endif _BitmapData_

//
// This structure describes a point in 3D space
//
#ifndef _Point3D_
#define _Point3D_
typedef struct _Point3D :Point2D
{
	double z;

	void create(double x, double y, double z)
	{
		Point2D::create(x, y);
		this->z = z;
	}
} Point3D;
#endif _Point3D_

#endif 
