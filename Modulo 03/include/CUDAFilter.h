#ifndef CUDA_FILTER_H
#define CUDA_FILTER_H

#include <opencv2/opencv.hpp>

namespace pavic {
namespace cuda {

// Verificar se CUDA está disponível
bool isCUDAAvailable();
int getCUDADeviceCount();
std::string getCUDADeviceName(int deviceId = 0);

// Filtros CUDA (GPU)
cv::Mat grayscale(const cv::Mat& input);
cv::Mat blur(const cv::Mat& input, int kernelSize = 5);
cv::Mat gaussianBlur(const cv::Mat& input, int kernelSize = 5);
cv::Mat sobel(const cv::Mat& input);
cv::Mat canny(const cv::Mat& input, double threshold1 = 50, double threshold2 = 150);
cv::Mat sharpen(const cv::Mat& input);
cv::Mat emboss(const cv::Mat& input);
cv::Mat negative(const cv::Mat& input);
cv::Mat sepia(const cv::Mat& input);
cv::Mat threshold(const cv::Mat& input, int thresholdValue = 128);
cv::Mat median(const cv::Mat& input, int kernelSize = 5);
cv::Mat bilateral(const cv::Mat& input, int d = 9, double sigmaColor = 75, double sigmaSpace = 75);

} // namespace cuda
} // namespace pavic

#endif // CUDA_FILTER_H
