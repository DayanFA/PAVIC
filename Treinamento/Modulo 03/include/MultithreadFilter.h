#ifndef MULTITHREAD_FILTER_H
#define MULTITHREAD_FILTER_H

#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>

namespace pavic {
namespace multithread {

// Filtros multithread usando std::thread
cv::Mat grayscale(const cv::Mat& input, int numThreads = 0);
cv::Mat blur(const cv::Mat& input, int kernelSize = 5, int numThreads = 0);
cv::Mat gaussianBlur(const cv::Mat& input, int kernelSize = 5, int numThreads = 0);
cv::Mat sobel(const cv::Mat& input, int numThreads = 0);
cv::Mat canny(const cv::Mat& input, double threshold1 = 50, double threshold2 = 150, int numThreads = 0);
cv::Mat sharpen(const cv::Mat& input, int numThreads = 0);
cv::Mat emboss(const cv::Mat& input, int numThreads = 0);
cv::Mat negative(const cv::Mat& input, int numThreads = 0);
cv::Mat sepia(const cv::Mat& input, int numThreads = 0);
cv::Mat threshold(const cv::Mat& input, int thresholdValue = 128, int numThreads = 0);
cv::Mat median(const cv::Mat& input, int kernelSize = 5, int numThreads = 0);
cv::Mat bilateral(const cv::Mat& input, int d = 9, double sigmaColor = 75, double sigmaSpace = 75, int numThreads = 0);

// Função para dividir trabalho entre threads
void processRegion(const cv::Mat& input, cv::Mat& output, int startRow, int endRow,
                   std::function<void(const cv::Mat&, cv::Mat&, int, int)> processFunc);

// Obter número de threads disponíveis
int getOptimalThreadCount();

} // namespace multithread
} // namespace pavic

#endif // MULTITHREAD_FILTER_H
