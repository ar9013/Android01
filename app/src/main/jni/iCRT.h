#pragma once
#ifndef ICRT_H
#define ICRT_H

#include "BasicStructs.h";
#include <vector>
#include <jni.h>


extern "C"
{
	typedef void(*onInitCompelete)(bool);
	typedef void(*onAddARRefCompelete)(bool, int);
	typedef void(*onUpdateARRef)(int);
	typedef void(*onDetectCompelete)();
}


class iCRT
{
private:
	iCRT();
	iCRT(iCRT const&);
	iCRT& operator = (iCRT const&);
	~iCRT();

public:
	static iCRT& getInstance() {
		static iCRT _instance;
		return _instance;
	}

	void init(onInitCompelete callback);
	void addARRef(int refid, char* path, onAddARRefCompelete callback);
	//void detect(BitmapData bmp, onUpdateARRef updateCallback, onDetectCompelete compeleteCallback);
	void detect(unsigned int width,unsigned int height,unsigned int hasAlpha,unsigned int isPremultiplied,unsigned int lineStride32, unsigned char*  bits32  , onUpdateARRef updateCallback, onDetectCompelete compeleteCallback);


	void dispose();

	ARReference& getARRef(int refid);
	std::vector<ARReference>& getARRefs();
};

#endif
