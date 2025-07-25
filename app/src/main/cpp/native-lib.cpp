#include <jni.h>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <android/bitmap.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <vector>
#include <cmath>
#include <limits>
#include <sstream>
#include <algorithm>
#include <android/log.h>

// 🔹 Configuración
constexpr double MAX_DISTANCE_THRESHOLD_HU = 2.0;
constexpr double MAX_DISTANCE_THRESHOLD_SS = 10.0;
constexpr int SIGNATURE_LENGTH = 128;
#define LOG_TAG "ShapeClassifier"

// 🔹 Esqueletización
cv::Mat skeletonize(const cv::Mat& img_bin) {
    CV_Assert(img_bin.type() == CV_8UC1);
    cv::Mat skel(img_bin.size(), CV_8UC1, cv::Scalar(0));
    cv::Mat temp, eroded;
    cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, {3, 3});
    cv::Mat img = img_bin.clone();

    while (true) {
        cv::erode(img, eroded, element);
        cv::dilate(eroded, temp, element);
        cv::subtract(img, temp, temp);
        cv::bitwise_or(skel, temp, skel);
        eroded.copyTo(img);
        if (cv::countNonZero(img) == 0)
            break;
    }
    return skel;
}

// 🔹 Shape Signature computation
std::vector<double> computeShapeSignature(const std::vector<cv::Point>& contour, int numSamples = SIGNATURE_LENGTH) {
    cv::Moments mu = cv::moments(contour);
    double cx = mu.m10 / (mu.m00 + 1e-5);
    double cy = mu.m01 / (mu.m00 + 1e-5);
    cv::Point2f centroid(cx, cy);

    std::vector<double> distances;
    for (const auto& pt : contour) {
        double d = cv::norm(cv::Point2f(pt.x, pt.y) - centroid);
        distances.push_back(d);
    }

    std::vector<double> signature(numSamples, 0.0);
    int len = distances.size();
    for (int i = 0; i < numSamples; ++i) {
        double pos = (static_cast<double>(i) / numSamples) * len;
        int idx = static_cast<int>(pos) % len;
        signature[i] = distances[idx];
    }

    double maxVal = *std::max_element(signature.begin(), signature.end());
    if (maxVal > 0) {
        for (auto& v : signature) v /= maxVal;
    }
    return signature;
}

// 🔹 Estructura de referencia
struct Reference {
    std::string clase;
    std::vector<double> hu;
    std::vector<double> signature;
};

AAssetManager* g_assetManager = nullptr;
std::vector<Reference> referenciasHu;
std::vector<Reference> referenciasSS;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_app_MainActivity_initAssetManager(JNIEnv *env, jobject thiz, jobject assetManager) {
    g_assetManager = AAssetManager_fromJava(env, assetManager);
}

void cargarReferenciasHu() {
    if (!g_assetManager) return;

    AAsset* asset = AAssetManager_open(g_assetManager, "hu_moments_reference.csv", AASSET_MODE_BUFFER);
    if (!asset) return;

    size_t size = AAsset_getLength(asset);
    std::string buffer(size, 0);
    AAsset_read(asset, &buffer[0], size);
    AAsset_close(asset);

    referenciasHu.clear();

    std::istringstream ss(buffer);
    std::string line;
    std::getline(ss, line);  // Skip header

    while (std::getline(ss, line)) {
        std::istringstream linestream(line);
        std::string token;
        Reference ref;

        std::getline(linestream, token, ',');
        ref.clase = token;
        for (int i = 0; i < 7; ++i) {
            std::getline(linestream, token, ',');
            ref.hu.push_back(std::stod(token));
        }
        referenciasHu.push_back(ref);
    }
}

void cargarReferenciasShapeSignature() {
    if (!g_assetManager) return;

    AAsset* asset = AAssetManager_open(g_assetManager, "shape_signature_reference.csv", AASSET_MODE_BUFFER);
    if (!asset) return;

    size_t size = AAsset_getLength(asset);
    std::string buffer(size, 0);
    AAsset_read(asset, &buffer[0], size);
    AAsset_close(asset);

    referenciasSS.clear();

    std::istringstream ss(buffer);
    std::string line;
    std::getline(ss, line);  // Skip header

    while (std::getline(ss, line)) {
        std::istringstream linestream(line);
        std::string token;
        Reference ref;

        std::getline(linestream, token, ',');
        ref.clase = token;
        for (int i = 0; i < SIGNATURE_LENGTH; ++i) {
            std::getline(linestream, token, ',');
            ref.signature.push_back(std::stod(token));
        }
        referenciasSS.push_back(ref);
    }
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_app_MainActivity_classifyShape(JNIEnv *env, jobject thiz, jobject bitmap, jint descriptor) {
    AndroidBitmapInfo info;
    void* pixels;

    if (AndroidBitmap_getInfo(env, bitmap, &info) < 0) {
        return env->NewStringUTF("Error al obtener info del Bitmap.");
    }

    if (AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0) {
        return env->NewStringUTF("Error al acceder a los píxeles.");
    }

    cv::Mat matBitmap(info.height, info.width, CV_8UC4, pixels);
    cv::Mat gray, blurred, inverted, binary;

    cv::cvtColor(matBitmap, gray, cv::COLOR_RGBA2GRAY);
    cv::GaussianBlur(gray, blurred, {3, 3}, 0);
    cv::bitwise_not(blurred, inverted);
    cv::threshold(inverted, binary, 50, 255, cv::THRESH_BINARY);

    AndroidBitmap_unlockPixels(env, bitmap);

    std::string msg;

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    if (contours.empty()) {
        return env->NewStringUTF("No figura detectada");
    }

    auto largestContour = std::max_element(contours.begin(), contours.end(),
                                           [](const auto& c1, const auto& c2) {
                                               return cv::contourArea(c1) < cv::contourArea(c2);
                                           });

    if (descriptor == 0) {
        if (referenciasHu.empty()) {
            cargarReferenciasHu();
        }

        cv::Rect bbox = cv::boundingRect(*largestContour);
        cv::Mat roi = binary(bbox);
        cv::Mat norm_roi;
        cv::resize(roi, norm_roi, {300, 300});
        cv::Mat skel = skeletonize(norm_roi);

        cv::Moments m = cv::moments(skel, true);
        double hu[7];
        cv::HuMoments(m, hu);
        for (int i = 0; i < 7; ++i) {
            hu[i] = std::abs(-1 * copysign(1.0, hu[i]) * log10(std::abs(hu[i]) + 1e-30));
        }

        double min_dist = std::numeric_limits<double>::max();
        std::string best_class = "Desconocido";

        for (const auto& ref : referenciasHu) {
            double dist = 0.0;
            for (int i = 0; i < 7; ++i) {
                dist += std::pow(hu[i] - ref.hu[i], 2);
            }
            dist = std::sqrt(dist);
            if (dist < min_dist) {
                min_dist = dist;
                best_class = ref.clase;
            }
        }

        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Distancia mínima HU: %.3f Clase: %s", min_dist, best_class.c_str());

        if (min_dist > MAX_DISTANCE_THRESHOLD_HU) {
            msg = "Figura no reconocida (HU)";
        } else {
            msg = "Clasificado (HU) como: " + best_class;
        }

    } else if (descriptor == 1) {
        if (referenciasSS.empty()) {
            cargarReferenciasShapeSignature();
        }

        std::vector<double> sigInput = computeShapeSignature(*largestContour);

        double min_dist = std::numeric_limits<double>::max();
        std::string best_class = "Desconocido";

        for (const auto& ref : referenciasSS) {
            if (ref.signature.size() != SIGNATURE_LENGTH) continue;
            double dist = 0.0;
            for (int i = 0; i < SIGNATURE_LENGTH; ++i) {
                dist += std::pow(sigInput[i] - ref.signature[i], 2);
            }
            dist = std::sqrt(dist);
            if (dist < min_dist) {
                min_dist = dist;
                best_class = ref.clase;
            }
        }

        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Distancia mínima SS: %.3f Clase: %s", min_dist, best_class.c_str());

        if (min_dist > MAX_DISTANCE_THRESHOLD_SS) {
            msg = "Figura no reconocida (SS)";
        } else {
            msg = "Clasificado (SS) como: " + best_class;
        }

    } else {
        msg = "Descriptor no válido.";
    }

    return env->NewStringUTF(msg.c_str());
}
