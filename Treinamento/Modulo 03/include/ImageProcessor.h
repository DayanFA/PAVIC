#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <opencv2/opencv.hpp>
#include <string>
#include <functional>
#include <chrono>

namespace pavic {

// Enum para tipos de processamento
enum class ProcessingType {
    SEQUENTIAL,
    PARALLEL,
    MULTITHREAD,
    CUDA
};

// Enum para tipos de filtro
enum class FilterType {
    GRAYSCALE,
    BLUR,
    GAUSSIAN_BLUR,
    SOBEL,
    CANNY,
    SHARPEN,
    EMBOSS,
    NEGATIVE,
    SEPIA,
    THRESHOLD,
    MEDIAN,
    BILATERAL
};

// Estrutura para resultado do processamento
struct ProcessingResult {
    cv::Mat image;
    double executionTimeMs;
    ProcessingType processingType;
    FilterType filterType;
    bool success;
    std::string errorMessage;
};

// Classe principal de processamento de imagens
class ImageProcessor {
public:
    ImageProcessor();
    ~ImageProcessor();

    // Carregar e salvar imagens
    bool loadImage(const std::string& filepath);
    bool loadImage(const cv::Mat& image);  // Carregar de Mat (para câmera)
    bool saveImage(const std::string& filepath, const cv::Mat& image);
    
    // Obter imagem atual
    cv::Mat getOriginalImage() const;
    cv::Mat getProcessedImage() const;

    // Aplicar filtro
    ProcessingResult applyFilter(FilterType filter, ProcessingType processing);
    
    // Processar imagem de webcam
    ProcessingResult processFrame(const cv::Mat& frame, FilterType filter, ProcessingType processing);

    // Obter nome do filtro/processamento
    static std::string getFilterName(FilterType filter);
    static std::string getProcessingName(ProcessingType processing);

private:
    cv::Mat originalImage;
    cv::Mat processedImage;
};

} // namespace pavic

#endif // IMAGE_PROCESSOR_H
