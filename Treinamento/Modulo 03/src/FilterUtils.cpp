/**
 * PAVIC LAB 2025 - Filter Utilities Implementation
 * Funções utilitárias para processamento de imagens
 */

#include "FilterUtils.h"
#include <cmath>

namespace pavic {
namespace utils {

cv::Mat getGaussianKernel(int size, double sigma) {
    if (sigma <= 0) {
        sigma = 0.3 * ((size - 1) * 0.5 - 1) + 0.8;
    }
    
    cv::Mat kernel(size, size, CV_64F);
    int half = size / 2;
    double sum = 0.0;
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int x = i - half;
            int y = j - half;
            double value = exp(-(x*x + y*y) / (2 * sigma * sigma));
            kernel.at<double>(i, j) = value;
            sum += value;
        }
    }
    
    // Normalizar
    kernel /= sum;
    return kernel;
}

cv::Mat getSharpenKernel() {
    cv::Mat kernel = (cv::Mat_<double>(3, 3) <<
        0, -1, 0,
        -1, 5, -1,
        0, -1, 0);
    return kernel;
}

cv::Mat getEmbossKernel() {
    cv::Mat kernel = (cv::Mat_<double>(3, 3) <<
        -2, -1, 0,
        -1, 1, 1,
        0, 1, 2);
    return kernel;
}

cv::Mat getSobelKernelX() {
    cv::Mat kernel = (cv::Mat_<double>(3, 3) <<
        -1, 0, 1,
        -2, 0, 2,
        -1, 0, 1);
    return kernel;
}

cv::Mat getSobelKernelY() {
    cv::Mat kernel = (cv::Mat_<double>(3, 3) <<
        -1, -2, -1,
        0, 0, 0,
        1, 2, 1);
    return kernel;
}

cv::Mat getBoxBlurKernel(int size) {
    cv::Mat kernel = cv::Mat::ones(size, size, CV_64F);
    kernel /= (size * size);
    return kernel;
}

cv::Mat padImage(const cv::Mat& input, int padding, int borderType) {
    cv::Mat padded;
    cv::copyMakeBorder(input, padded, padding, padding, padding, padding, borderType);
    return padded;
}

void clampValues(cv::Mat& image) {
    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            if (image.channels() == 1) {
                image.at<uchar>(i, j) = std::max(0, std::min(255, (int)image.at<uchar>(i, j)));
            } else if (image.channels() == 3) {
                cv::Vec3b& pixel = image.at<cv::Vec3b>(i, j);
                pixel[0] = std::max(0, std::min(255, (int)pixel[0]));
                pixel[1] = std::max(0, std::min(255, (int)pixel[1]));
                pixel[2] = std::max(0, std::min(255, (int)pixel[2]));
            }
        }
    }
}

bool isValidImage(const cv::Mat& image) {
    return !image.empty() && image.data != nullptr;
}

bool isGrayscale(const cv::Mat& image) {
    return image.channels() == 1;
}

cv::Mat toGrayscale(const cv::Mat& input) {
    if (input.channels() == 1) {
        return input.clone();
    }
    cv::Mat gray;
    cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    return gray;
}

cv::Mat toColor(const cv::Mat& input) {
    if (input.channels() == 3) {
        return input.clone();
    }
    cv::Mat color;
    cv::cvtColor(input, color, cv::COLOR_GRAY2BGR);
    return color;
}

} // namespace utils
} // namespace pavic
