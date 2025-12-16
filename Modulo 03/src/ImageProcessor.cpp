/**
 * PAVIC LAB 2025 - ImageProcessor
 * Orquestra aplicação de filtros e mede tempos por abordagem
 */

#include "ImageProcessor.h"
#include "SequentialFilter.h"
#include "ParallelFilter.h"
#include "MultithreadFilter.h"
#include "CUDAFilter.h"
#include "FilterUtils.h"

#include <opencv2/opencv.hpp>
#include <stdexcept>

namespace pavic {

ImageProcessor::ImageProcessor() {}
ImageProcessor::~ImageProcessor() {}

bool ImageProcessor::loadImage(const std::string& filepath) {
    originalImage = cv::imread(filepath, cv::IMREAD_COLOR);
    processedImage.release();
    return !originalImage.empty();
}

bool ImageProcessor::loadImage(const cv::Mat& image) {
    if (image.empty()) return false;
    originalImage = image.clone();
    processedImage.release();
    return true;
}

bool ImageProcessor::saveImage(const std::string& filepath, const cv::Mat& image) {
    if (image.empty()) return false;
    return cv::imwrite(filepath, image);
}

cv::Mat ImageProcessor::getOriginalImage() const { return originalImage; }
cv::Mat ImageProcessor::getProcessedImage() const { return processedImage; }

static cv::Mat applyFilterImpl(const cv::Mat& input, FilterType filter, ProcessingType processing) {
    using namespace pavic;
    switch (processing) {
        case ProcessingType::SEQUENTIAL: {
            using namespace sequential;
            switch (filter) {
                case FilterType::GRAYSCALE: return grayscale(input);
                case FilterType::BLUR: return blur(input, 5);
                case FilterType::GAUSSIAN_BLUR: return gaussianBlur(input, 5);
                case FilterType::SOBEL: return sobel(input);
                case FilterType::CANNY: return canny(input, 50, 150);
                case FilterType::SHARPEN: return sharpen(input);
                case FilterType::EMBOSS: return emboss(input);
                case FilterType::NEGATIVE: return negative(input);
                case FilterType::SEPIA: return sepia(input);
                case FilterType::THRESHOLD: return threshold(input, 128);
                case FilterType::MEDIAN: return median(input, 5);
                case FilterType::BILATERAL: return bilateral(input, 9, 75, 75);
            }
            break;
        }
        case ProcessingType::PARALLEL: {
            using namespace parallel;
            switch (filter) {
                case FilterType::GRAYSCALE: return grayscale(input);
                case FilterType::BLUR: return blur(input, 5);
                case FilterType::GAUSSIAN_BLUR: return gaussianBlur(input, 5);
                case FilterType::SOBEL: return sobel(input);
                case FilterType::CANNY: return canny(input, 50, 150);
                case FilterType::SHARPEN: return sharpen(input);
                case FilterType::EMBOSS: return emboss(input);
                case FilterType::NEGATIVE: return negative(input);
                case FilterType::SEPIA: return sepia(input);
                case FilterType::THRESHOLD: return threshold(input, 128);
                case FilterType::MEDIAN: return median(input, 5);
                case FilterType::BILATERAL: return bilateral(input, 9, 75, 75);
            }
            break;
        }
        case ProcessingType::MULTITHREAD: {
            using namespace multithread;
            switch (filter) {
                case FilterType::GRAYSCALE: return grayscale(input);
                case FilterType::BLUR: return blur(input, 5);
                case FilterType::GAUSSIAN_BLUR: return gaussianBlur(input, 5);
                case FilterType::SOBEL: return sobel(input);
                case FilterType::CANNY: return canny(input, 50, 150);
                case FilterType::SHARPEN: return sharpen(input);
                case FilterType::EMBOSS: return emboss(input);
                case FilterType::NEGATIVE: return negative(input);
                case FilterType::SEPIA: return sepia(input);
                case FilterType::THRESHOLD: return threshold(input, 128);
                case FilterType::MEDIAN: return median(input, 5);
                case FilterType::BILATERAL: return bilateral(input, 9, 75, 75);
            }
            break;
        }
        case ProcessingType::CUDA: {
            // Usa stubs que fazem fallback para CPU quando CUDA não está disponível
            using namespace cuda;
            switch (filter) {
                case FilterType::GRAYSCALE: return grayscale(input);
                case FilterType::BLUR: return blur(input, 5);
                case FilterType::GAUSSIAN_BLUR: return gaussianBlur(input, 5);
                case FilterType::SOBEL: return sobel(input);
                case FilterType::CANNY: return canny(input, 50, 150);
                case FilterType::SHARPEN: return sharpen(input);
                case FilterType::EMBOSS: return emboss(input);
                case FilterType::NEGATIVE: return negative(input);
                case FilterType::SEPIA: return sepia(input);
                case FilterType::THRESHOLD: return threshold(input, 128);
                case FilterType::MEDIAN: return median(input, 5);
                case FilterType::BILATERAL: return bilateral(input, 9, 75, 75);
            }
            break;
        }
    }
    throw std::runtime_error("Filtro/Processamento inválido");
}

ProcessingResult ImageProcessor::applyFilter(FilterType filter, ProcessingType processing) {
    ProcessingResult result{};
    result.filterType = filter;
    result.processingType = processing;
    result.success = false;

    if (originalImage.empty()) {
        result.errorMessage = "Nenhuma imagem carregada";
        return result;
    }

    auto start = std::chrono::high_resolution_clock::now();
    try {
        processedImage = applyFilterImpl(originalImage, filter, processing);
        auto end = std::chrono::high_resolution_clock::now();
        result.executionTimeMs = std::chrono::duration<double, std::milli>(end - start).count();
        result.image = processedImage;
        result.success = !processedImage.empty();
    } catch (const std::exception& ex) {
        result.errorMessage = ex.what();
    }
    return result;
}

ProcessingResult ImageProcessor::processFrame(const cv::Mat& frame, FilterType filter, ProcessingType processing) {
    ProcessingResult result{};
    result.filterType = filter;
    result.processingType = processing;
    result.success = false;

    if (frame.empty()) {
        result.errorMessage = "Frame vazio";
        return result;
    }

    auto start = std::chrono::high_resolution_clock::now();
    try {
        cv::Mat out = applyFilterImpl(frame, filter, processing);
        auto end = std::chrono::high_resolution_clock::now();
        result.executionTimeMs = std::chrono::duration<double, std::milli>(end - start).count();
        result.image = out;
        result.success = !out.empty();
    } catch (const std::exception& ex) {
        result.errorMessage = ex.what();
    }
    return result;
}

std::string ImageProcessor::getFilterName(FilterType filter) {
    switch (filter) {
        case FilterType::GRAYSCALE: return "Grayscale";
        case FilterType::BLUR: return "Blur";
        case FilterType::GAUSSIAN_BLUR: return "GaussianBlur";
        case FilterType::SOBEL: return "Sobel";
        case FilterType::CANNY: return "Canny";
        case FilterType::SHARPEN: return "Sharpen";
        case FilterType::EMBOSS: return "Emboss";
        case FilterType::NEGATIVE: return "Negative";
        case FilterType::SEPIA: return "Sepia";
        case FilterType::THRESHOLD: return "Threshold";
        case FilterType::MEDIAN: return "Median";
        case FilterType::BILATERAL: return "Bilateral";
    }
    return "Unknown";
}

std::string ImageProcessor::getProcessingName(ProcessingType p) {
    switch (p) {
        case ProcessingType::SEQUENTIAL: return "Sequential";
        case ProcessingType::PARALLEL: return "Parallel(OpenMP)";
        case ProcessingType::MULTITHREAD: return "Multithread";
        case ProcessingType::CUDA: return "CUDA";
    }
    return "Unknown";
}

} // namespace pavic
