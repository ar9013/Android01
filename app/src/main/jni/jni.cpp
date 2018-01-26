#include <string.h>
#include <stdio.h>
#include <jni.h>

#include <iCRT.H>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <thread>

#include <JniHelper.h>
#include <iostream>
#include <fstream>
#include <boost/archive/text_oarchive.hpp>

#include <android/log.h>


// Android log function wrappers
static const char* kTAG = "hello-jniCallback";
#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))
#define LOGD(...) \
  ((void)__android_log_print(ANDROID_LOG_DEBUG, kTAG, __VA_ARGS__))

#include "gps.hpp"

static JniMethodInfo methodInfo;

char* jstringToChar(JNIEnv* env, jstring jstr) {
    char* rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("GB2312");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (alen > 0) {
        rtn = (char*) malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    LOGD("%c", rtn);

    return rtn;
}

unsigned char* as_unsigned_char_array(JNIEnv* env,jbyteArray array) {
    int len = env->GetArrayLength (array);
    unsigned char* buf = new unsigned char[len];
    env->GetByteArrayRegion (array, 0, len, reinterpret_cast<jbyte*>(buf));
    return buf;
}

void addARRefCompelete(bool isRefAdd, int addRefId) {

	if(isRefAdd){
		LOGD("iCRT: addARRefCompelete : true (success)");
	}else{
		LOGD("iCRT: addARRefCompelete : false (failed)");
	}
	std::cout << "iCRT: addARRefCompelete : " << isRefAdd << " , " << addRefId<< std::endl;

	return;
}

void detectCompelete() {
    LOGD("iCRT: addARRefCompelete : true (success)");
	bool detectEnd = true;
	return;
}


void updateCallback(int refid) {
    LOGD("updateCallback");

	std::cout << "iCRT: updateCallback " << std::endl;
	std::cout << "iCRT: refid: " << refid << std::endl;

 // JNIEnv* env =  methodInfo.env;
 // jclass      classID =   methodInfo.classID;
 // jmethodID   methodID = methodInfo.methodID;
 // jint id = (jint) refid;

  // methodInfo.env->CallStaticVoidMethod(classID,methodID,id);

   jint id = (jint) refid;
   methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID, id);

    // jthrowable exception = methodInfo.env->ExceptionOccurred();
    //  if (exception) {
   //       LOGI("Unable to call UnitySendMessage");
     //     methodInfo.env->ExceptionDescribe();
       //   methodInfo.env->DeleteLocalRef(exception);
         // methodInfo.env->ExceptionClear();
     // }
     // else {
      //    LOGI("Called UnitySendMessage for");
     // }

      //methodInfo.env->DeleteLocalRef(refid);
    //  methodInfo.env->DeleteLocalRef(methodInfo.classID);

	return;
}

std::string gps(std::string const &root)
{
    const gps_position g(35, 59, 24.567f);
    save(root, g);

    gps_position newg;
        load(root, newg);

    std::ostringstream ostr;

    if (g != newg)
        return std::string();

    ostr << "GPS coordinates: " << newg;
    return ostr.str();
}

void OnInitCompelete(bool is_Success) {
	if(is_Success){

		LOGD("Test: onInitCompelete : true (success)");

	}else{

		LOGD("Test: onInitCompelete : false (failed)");
	}
	return;
}

extern "C" jstring
Java_ar_yue_examples_myapplication_CameraActivity_getGPSCoordinates( JNIEnv* env, jobject thiz, jstring rootPath )
{

    const char *s = env->GetStringUTFChars(rootPath, 0);
    std::string root(s);
    env->ReleaseStringUTFChars(rootPath, s);
    LOGD("root: %s", root.c_str());

    try {
        std::string ret = gps(root);

        return env->NewStringUTF(ret.c_str());
    }

    catch (std::exception &e) {
        LOGD("ERROR: %s", e.what());
        abort();
    }

    catch (...) {
        LOGD("Unknown error");
        abort();
    }
}

extern "C" void
Java_ar_yue_examples_myapplication_CameraActivity_initAR( JNIEnv* env, jobject jobj){

bool isMethodInfo = JniHelper::getStaticMethodInfo(methodInfo, "ar/yue/examples/myapplication/CameraActivity", "UpdateARID", "(I)V");


if(isMethodInfo){
     LOGD("Get MethodInfo ");

    iCRT::getInstance().init(OnInitCompelete);
}else{
     LOGD("Error Get MethodInfo ");

}

}

extern "C" void
Java_ar_yue_examples_myapplication_CameraActivity_addARRef( JNIEnv* env, jobject thiz, int refId, jstring markerPath ){

   char* charMarkerPath = jstringToChar(env,markerPath);

   LOGD("stats yaml Path : %s",charMarkerPath);

   iCRT::getInstance().addARRef(refId,charMarkerPath,addARRefCompelete);

}



// Pass data to BitmapData  construct BitmapData
extern "C" void
Java_ar_yue_examples_myapplication_CameraActivity_detect( JNIEnv* env, jobject thiz, int width, int height, int hasAlpha , int isPremultiplied , int lineStride32 ,jbyteArray frameData ){

//unsigned char* data = as_unsigned_char_array(env,frameData);


iCRT::getInstance().detect(width,height,hasAlpha,isPremultiplied,lineStride32,as_unsigned_char_array(env,frameData),updateCallback,detectCompelete);

}
