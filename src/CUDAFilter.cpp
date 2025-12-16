/**
 * PAVIC LAB 2025 - CUDAFilter (Stub Implementation)
 * 
 * Esta é uma implementação placeholder que usa CPU quando CUDA não está disponível.
 * Quando o CUDA Toolkit estiver instalado, substitua este arquivo por CUDAFilter.cu
 * com implementações reais usando CUDA kernels.
 */

#include "CUDAFilter.h"
#include "SequentialFilter.h"
#include <iostream>

namespace pavic {
namespace cuda {

// Flag para mostrar aviso apenas uma vez
static bool warningShown = false;

static void showWarning() {
    if (!warningShown) {
        std::cerr << "[AVISO] CUDA nao disponivel. Usando implementacao CPU como fallback.\n";
        warningShown = true;
    }
}

bool isCUDAAvailable() {
    return false; // Stub: CUDA não compilado
}

int getCUDADeviceCount() {
    return 0;
}

std::string getCUDADeviceName(int deviceId) {
    (void)deviceId;
    return "CUDA não disponível (stub)";
}

// Todos os filtros usam a implementação sequencial como fallback
cv::Mat grayscale(const cv::Mat& input) {
    showWarning();
    return sequential::grayscale(input);
}

cv::Mat blur(const cv::Mat& input, int kernelSize) {
    showWarning();
    return sequential::blur(input, kernelSize);
}

cv::Mat gaussianBlur(const cv::Mat& input, int kernelSize) {
    showWarning();
    return sequential::gaussianBlur(input, kernelSize);
}

cv::Mat sobel(const cv::Mat& input) {
    showWarning();
    return sequential::sobel(input);
}

cv::Mat canny(const cv::Mat& input, double threshold1, double threshold2) {
    showWarning();
    return sequential::canny(input, threshold1, threshold2);
}

cv::Mat sharpen(const cv::Mat& input) {
    showWarning();
    return sequential::sharpen(input);
}

cv::Mat emboss(const cv::Mat& input) {
    showWarning();
    return sequential::emboss(input);
}

cv::Mat negative(const cv::Mat& input) {
    showWarning();
    return sequential::negative(input);
}

cv::Mat sepia(const cv::Mat& input) {
    showWarning();
    return sequential::sepia(input);
}

cv::Mat threshold(const cv::Mat& input, int thresholdValue) {
    showWarning();
    return sequential::threshold(input, thresholdValue);
}

cv::Mat median(const cv::Mat& input, int kernelSize) {
    showWarning();
    return sequential::median(input, kernelSize);
}

cv::Mat bilateral(const cv::Mat& input, int d, double sigmaColor, double sigmaSpace) {
    showWarning();
    return sequential::bilateral(input, d, sigmaColor, sigmaSpace);
}

} // namespace cuda
} // namespace pavic
