//
// Created by irina on 20.02.18.
//
#include <string.h>
#include <jni.h>
#include <android/log.h>
#include <string>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <advanceStitcher.h>


using std::string;
using std::vector;

#ifdef __cplusplus
extern "C" {
#endif
using stitching::AdvanceStitcher;
using stitching::Status;

string jstring2string(JNIEnv *env, jstring jstr) {
    const char *cstr = env->GetStringUTFChars(jstr, 0);
    string str(cstr);
    env->ReleaseStringUTFChars(jstr, cstr);
    return str;
}

string bytes2string(JNIEnv *env, jbyteArray buf) {
    jbyte *ptr = env->GetByteArrayElements(buf, 0);
    string s((char *)ptr, env->GetArrayLength(buf));
    env->ReleaseByteArrayElements(buf, ptr, 0);
    return s;
}

cv::Mat imgbuf2mat(JNIEnv *env, jbyteArray buf, int width, int height) {
    jbyte *ptr = env->GetByteArrayElements(buf, 0);
    cv::Mat img(height + height / 2, width, CV_8UC1, (unsigned char *)ptr);
    cv::cvtColor(img, img, CV_YUV2RGBA_NV21);
    env->ReleaseByteArrayElements(buf, ptr, 0);
    return img;
}

cv::Mat getImage(JNIEnv *env, jbyteArray buf, int width, int height, bool isGRAY) {
    return (isGRAY) ? cv::imread(bytes2string(env, buf), 0): cv::imread(bytes2string(env, buf), -1);
}


JNIEXPORT void JNICALL
Java_com_irina_tensorflowexample_Stitching_stitchImages(
        JNIEnv *env, jobject thiz, jbyteArray buf, jbyteArray buf2,
        jbyteArray buf3,  jbyteArray buf4,
        jint num_homo, jboolean useGdf, jlong addrResult)
{
    AdvanceStitcher *stitcher = AdvanceStitcher::Get(useGdf, num_homo);
    cv::Mat* pMat=(cv::Mat*)addrResult;
    cv::Mat image1 = getImage(env, buf, 0, 0, false);
    cv::Mat image2 = getImage(env, buf2, 0, 0, false);
    cv::Mat seg1 = getImage(env, buf3, 0, 0, true);
    cv::Mat seg2 = getImage(env, buf4, 0, 0, true);
    cv::Mat result = stitcher->process(image1, image2, seg1, seg2);
    result.copyTo(*pMat);
    __android_log_print(ANDROID_LOG_VERBOSE, "Stitching", "DONE");
    Status status = stitcher->getStatus();
    switch(status){
            case Status::SUCCESS_WITHOUT_RECONSTRCT:
                __android_log_print(ANDROID_LOG_VERBOSE, "STITCHING STATUS ", "Stitching is successful; reconstruction was not required");
                break;
            case Status::SUCCESS_WITH_FIRST_RECONSTRUCT:
                __android_log_print(ANDROID_LOG_VERBOSE, "STITCHING STATUS ", "Stitching is successful with reconstruction with removal");
                break;
            case Status::SUCCESS_WITH_SECOND_RECONSTRUCT:
                __android_log_print(ANDROID_LOG_VERBOSE, "STITCHING STATUS ", "Stitching is successful with reconstruction with adding");
                break;
            case Status::SUCCESS_FIRST_RECONSTRUCT_FAILED:
                __android_log_print(ANDROID_LOG_VERBOSE, "STITCHING STATUS ", "Stitching is successful, but reconstruction with removal failed");
                break;
            case Status::SUCCESS_SECOND_RECONSTRUCT_FAILED:
                __android_log_print(ANDROID_LOG_VERBOSE, "STITCHING STATUS ", "Stitching is successful, but reconstruction with adding failed");
                break;
            case Status::STITCHING_FAILED:
                __android_log_print(ANDROID_LOG_VERBOSE, "STITCHING STATUS ", "Stitching failed");
                break;
            default:
                __android_log_print(ANDROID_LOG_VERBOSE, "STITCHING STATUS ", "Recieved udefined status");
                break;
    }

}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    // Get jclass with env->FindClass.
    // Register methods with env->RegisterNatives.

    return JNI_VERSION_1_6;
}

#ifdef __cplusplus
}
#endif