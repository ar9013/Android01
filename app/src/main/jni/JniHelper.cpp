/****************************************************************************
   Copyright (c) 2010 cocos2d-x.org

   http://www.cocos2d-x.org

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
****************************************************************************/
#include "JniHelper.h"
#include <android/log.h>
#include <string.h>


#define  LOG_TAG    "JniHelper"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)



#define JAVAVM    JniHelper::getJavaVM()

using namespace std;

//static pthread_key_t g_key;


extern "C"
{
	jint JNI_OnLoad(JavaVM *vm, void *reserved)
	{
	   JniHelper::setJavaVM(vm);
		return JNI_VERSION_1_6;
	}

//////////////////////////////////////////////////////////////////////////
// java vm helper function
//////////////////////////////////////////////////////////////////////////

    static bool getEnv_(JNIEnv **env) {
        bool bRet = false;

        do {
            if (JAVAVM->GetEnv((void **)env, JNI_VERSION_1_6) != JNI_OK) {
                LOGD("Failed to get the environment using GetEnv()");
                break;
            }

            if (JAVAVM->AttachCurrentThread(env, 0) < 0) {
                LOGD("Failed to get the environment using AttachCurrentThread()");
                break;
            }
            bRet = true;
        }
        while (0);

        return bRet;
    }

    static jclass getClassID_(const char *className, JNIEnv *env) {
        JNIEnv *pEnv = env;
        jclass ret = 0;

        do {
            if (!pEnv) {
                if (!getEnv_(&pEnv)) {
                    break;
                }
            }

            ret = pEnv->FindClass(className);
            if (!ret) {
                LOGD("Failed to find class of %s", className);
                break;
            }
        }
        while (0);

        return ret;
    }

    static bool getStaticMethodInfo_(JniMethodInfo &methodinfo, const char *className, const char *methodName, const char *paramCode) {
        jmethodID methodID = 0;
        JNIEnv *pEnv = 0;
        bool bRet = false;

        do {
             if (!getEnv_(&pEnv)) {
                 break;
             }


            jclass classID = getClassID_(className, pEnv);

            methodID = pEnv->GetStaticMethodID(classID, methodName, paramCode);
            if (!methodID) {
                LOGD("Failed to find static method id of %s", methodName);
                break;
            }

            methodinfo.classID = classID;
            methodinfo.env = pEnv;
            methodinfo.methodID = methodID;

            bRet = true;
        }
        while (0);

        return bRet;
    }

    static bool getMethodInfo_(JniMethodInfo &methodinfo, const char *className, const char *methodName, const char *paramCode) {
        jmethodID methodID = 0;
        JNIEnv *pEnv = 0;
        bool bRet = false;

        do {
            if (!getEnv_(&pEnv)) {
                break;
            }

            jclass classID = getClassID_(className, pEnv);

            methodID = pEnv->GetMethodID(classID, methodName, paramCode);
            if (!methodID) {
                LOGD("Failed to find method id of %s", methodName);
                break;
            }

            methodinfo.classID = classID;
            methodinfo.env = pEnv;
            methodinfo.methodID = methodID;

            bRet = true;
        }
        while (0);

        return bRet;
    }

    static string jstring2string_(jstring jstr) {
        if (jstr == NULL) {
            return "";
        }

        JNIEnv *env = 0;

        if (!getEnv_(&env)) {
            return 0;
        }

        const char *chars = env->GetStringUTFChars(jstr, NULL);
        string ret(chars);
        env->ReleaseStringUTFChars(jstr, chars);

        return ret;
    }

    void _detachCurrentThread(void *a) {
        JniHelper::getJavaVM()->DetachCurrentThread();
    }
}


JavaVM *JniHelper::_psJavaVM = 0;


JavaVM *JniHelper::getJavaVM() {
//	pthread_t thisthread = pthread_self();
//	LOGD("JniHelper::getJavaVM(), pthread_self() = %ld", thisthread);
	return _psJavaVM;
}

void JniHelper::setJavaVM(JavaVM *javaVM) {
//	pthread_t thisthread = pthread_self();
//	LOGD("JniHelper::setJavaVM(%p), pthread_self() = %ld", javaVM, thisthread);
	_psJavaVM = javaVM;

//	pthread_key_create(&g_key, _detachCurrentThread);
}

jclass JniHelper::getClassID(const char *className, JNIEnv *env) {
	return getClassID_(className, env);
}

bool JniHelper::getStaticMethodInfo(JniMethodInfo &methodinfo, const char *className, const char *methodName, const char *paramCode) {
	return getStaticMethodInfo_(methodinfo, className, methodName, paramCode);
}

bool JniHelper::getMethodInfo(JniMethodInfo &methodinfo, const char *className, const char *methodName, const char *paramCode) {
	return getMethodInfo_(methodinfo, className, methodName, paramCode);
}

string JniHelper::jstring2string(jstring str) {
	return jstring2string_(str);
}

JNIEnv *JniHelper::getEnv() {
	JNIEnv *env;
	getEnv_(&env);

	return env;
//	JNIEnv *_env = (JNIEnv *)pthread_getspecific(g_key);
//	if (_env == nullptr)
//		_env = JniHelper::cacheEnv(_psJavaVM);
//	return _env;
}

//
//JNIEnv *JniHelper::cacheEnv(JavaVM *jvm) {
//	JNIEnv *_env = nullptr;
//	// get jni environment
//	jint ret = jvm->GetEnv((void **)&_env, JNI_VERSION_1_4);
//
//	switch (ret) {
//		case JNI_OK:
//			// Success!
//			pthread_setspecific(g_key, _env);
//			return _env;
//
//		case JNI_EDETACHED:
//			// Thread not attached
//			if (jvm->AttachCurrentThread(&_env, nullptr) < 0) {
//				LOGE("Failed to get the environment using AttachCurrentThread()");
//
//				return nullptr;
//			}
//			else {
//				// Success : Attached and obtained JNIEnv!
//				pthread_setspecific(g_key, _env);
//				return _env;
//			}
//
//		case JNI_EVERSION:
//			// Cannot recover from this error
//			LOGE("JNI interface version 1.4 not supported");
//
//		default:
//			LOGE("Failed to get the environment using GetEnv()");
//			return nullptr;
//	}
//}
