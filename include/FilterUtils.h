#ifndef FILTER_UTILS_H
#define FILTER_UTILS_H

#include <opencv2/opencv.hpp>

namespace pavic {
namespace utils {

// Kernels de convolução pré-definidos
cv::Mat getGaussianKernel(int size, double sigma = 0);
cv::Mat getSharpenKernel();
cv::Mat getEmbossKernel();
cv::Mat getSobelKernelX();
cv::Mat getSobelKernelY();
cv::Mat getBoxBlurKernel(int size);

// Funções utilitárias
cv::Mat padImage(const cv::Mat& input, int padding, int borderType = cv::BORDER_REPLICATE);
void clampValues(cv::Mat& image);

// Validação
bool isValidImage(const cv::Mat& image);
bool isGrayscale(const cv::Mat& image);

// Conversões
cv::Mat toGrayscale(const cv::Mat& input);
cv::Mat toColor(const cv::Mat& input);

} // namespace utils
} // namespace pavic

#endif // FILTER_UTILS_H
