/**
 * PAVIC LAB 2025 - Sequential Filter Implementation
 * Implementação de filtros usando processamento sequencial (CPU single-thread)
 */

#include "SequentialFilter.h"
#include "FilterUtils.h"
#include <cmath>
#include <algorithm>

namespace pavic {
namespace sequential {

void applyConvolution(const cv::Mat& input, cv::Mat& output, const cv::Mat& kernel) {
    int kRows = kernel.rows;
    int kCols = kernel.cols;
    int kCenterX = kCols / 2;
    int kCenterY = kRows / 2;
    
    // Criar imagem com padding
    cv::Mat padded;
    cv::copyMakeBorder(input, padded, kCenterY, kCenterY, kCenterX, kCenterX, cv::BORDER_REPLICATE);
    
    output = cv::Mat::zeros(input.size(), input.type());
    
    for (int i = 0; i < input.rows; i++) {
        for (int j = 0; j < input.cols; j++) {
            if (input.channels() == 1) {
                double sum = 0.0;
                for (int ki = 0; ki < kRows; ki++) {
                    for (int kj = 0; kj < kCols; kj++) {
                        int pi = i + ki;
                        int pj = j + kj;
                        sum += padded.at<uchar>(pi, pj) * kernel.at<double>(ki, kj);
                    }
                }
                output.at<uchar>(i, j) = cv::saturate_cast<uchar>(sum);
            } else if (input.channels() == 3) {
                double sumB = 0.0, sumG = 0.0, sumR = 0.0;
                for (int ki = 0; ki < kRows; ki++) {
                    for (int kj = 0; kj < kCols; kj++) {
                        int pi = i + ki;
                        int pj = j + kj;
                        cv::Vec3b pixel = padded.at<cv::Vec3b>(pi, pj);
                        double kVal = kernel.at<double>(ki, kj);
                        sumB += pixel[0] * kVal;
                        sumG += pixel[1] * kVal;
                        sumR += pixel[2] * kVal;
                    }
                }
                output.at<cv::Vec3b>(i, j) = cv::Vec3b(
                    cv::saturate_cast<uchar>(sumB),
                    cv::saturate_cast<uchar>(sumG),
                    cv::saturate_cast<uchar>(sumR)
                );
            }
        }
    }
}

cv::Mat grayscale(const cv::Mat& input) {
    if (input.empty()) return cv::Mat();
    
    if (input.channels() == 1) {
        return input.clone();
    }
    
    cv::Mat output(input.rows, input.cols, CV_8UC1);
    
    for (int i = 0; i < input.rows; i++) {
        for (int j = 0; j < input.cols; j++) {
            cv::Vec3b pixel = input.at<cv::Vec3b>(i, j);
            // Fórmula padrão: Y = 0.299*R + 0.587*G + 0.114*B
            uchar gray = static_cast<uchar>(0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0]);
            output.at<uchar>(i, j) = gray;
        }
    }
    
    return output;
}

cv::Mat blur(const cv::Mat& input, int kernelSize) {
    if (input.empty()) return cv::Mat();
    
    cv::Mat kernel = utils::getBoxBlurKernel(kernelSize);
    cv::Mat output;
    applyConvolution(input, output, kernel);
    return output;
}

cv::Mat gaussianBlur(const cv::Mat& input, int kernelSize) {
    if (input.empty()) return cv::Mat();
    
    cv::Mat kernel = utils::getGaussianKernel(kernelSize);
    cv::Mat output;
    applyConvolution(input, output, kernel);
    return output;
}

cv::Mat sobel(const cv::Mat& input) {
    if (input.empty()) return cv::Mat();
    
    cv::Mat gray = input.channels() == 3 ? grayscale(input) : input.clone();
    
    cv::Mat kernelX = utils::getSobelKernelX();
    cv::Mat kernelY = utils::getSobelKernelY();
    
    cv::Mat gradX, gradY;
    applyConvolution(gray, gradX, kernelX);
    applyConvolution(gray, gradY, kernelY);
    
    cv::Mat output(gray.size(), CV_8UC1);
    
    for (int i = 0; i < gray.rows; i++) {
        for (int j = 0; j < gray.cols; j++) {
            int gx = gradX.at<uchar>(i, j);
            int gy = gradY.at<uchar>(i, j);
            int magnitude = static_cast<int>(std::sqrt(gx * gx + gy * gy));
            output.at<uchar>(i, j) = cv::saturate_cast<uchar>(magnitude);
        }
    }
    
    return output;
}

cv::Mat canny(const cv::Mat& input, double threshold1, double threshold2) {
    if (input.empty()) return cv::Mat();
    
    // Para Canny, usamos a implementação do OpenCV por complexidade
    cv::Mat gray = input.channels() == 3 ? grayscale(input) : input.clone();
    cv::Mat blurred = gaussianBlur(gray, 5);
    
    // Aplicar Sobel
    cv::Mat gradX, gradY;
    cv::Mat kernelX = utils::getSobelKernelX();
    cv::Mat kernelY = utils::getSobelKernelY();
    
    applyConvolution(blurred, gradX, kernelX);
    applyConvolution(blurred, gradY, kernelY);
    
    cv::Mat magnitude(gray.size(), CV_8UC1);
    cv::Mat direction(gray.size(), CV_64F);
    
    for (int i = 0; i < gray.rows; i++) {
        for (int j = 0; j < gray.cols; j++) {
            double gx = gradX.at<uchar>(i, j) - 128;
            double gy = gradY.at<uchar>(i, j) - 128;
            magnitude.at<uchar>(i, j) = cv::saturate_cast<uchar>(std::sqrt(gx * gx + gy * gy));
            direction.at<double>(i, j) = std::atan2(gy, gx);
        }
    }
    
    // Non-maximum suppression simplificado
    cv::Mat output(gray.size(), CV_8UC1, cv::Scalar(0));
    
    for (int i = 1; i < gray.rows - 1; i++) {
        for (int j = 1; j < gray.cols - 1; j++) {
            double angle = direction.at<double>(i, j) * 180.0 / CV_PI;
            if (angle < 0) angle += 180;
            
            uchar mag = magnitude.at<uchar>(i, j);
            uchar q = 255, r = 255;
            
            if ((angle >= 0 && angle < 22.5) || (angle >= 157.5 && angle <= 180)) {
                q = magnitude.at<uchar>(i, j + 1);
                r = magnitude.at<uchar>(i, j - 1);
            } else if (angle >= 22.5 && angle < 67.5) {
                q = magnitude.at<uchar>(i + 1, j - 1);
                r = magnitude.at<uchar>(i - 1, j + 1);
            } else if (angle >= 67.5 && angle < 112.5) {
                q = magnitude.at<uchar>(i + 1, j);
                r = magnitude.at<uchar>(i - 1, j);
            } else if (angle >= 112.5 && angle < 157.5) {
                q = magnitude.at<uchar>(i - 1, j - 1);
                r = magnitude.at<uchar>(i + 1, j + 1);
            }
            
            if (mag >= q && mag >= r) {
                if (mag >= threshold2) {
                    output.at<uchar>(i, j) = 255;
                } else if (mag >= threshold1) {
                    output.at<uchar>(i, j) = 128;
                }
            }
        }
    }
    
    // Hysteresis simples
    for (int i = 1; i < gray.rows - 1; i++) {
        for (int j = 1; j < gray.cols - 1; j++) {
            if (output.at<uchar>(i, j) == 128) {
                bool hasStrongNeighbor = false;
                for (int di = -1; di <= 1; di++) {
                    for (int dj = -1; dj <= 1; dj++) {
                        if (output.at<uchar>(i + di, j + dj) == 255) {
                            hasStrongNeighbor = true;
                            break;
                        }
                    }
                    if (hasStrongNeighbor) break;
                }
                output.at<uchar>(i, j) = hasStrongNeighbor ? 255 : 0;
            }
        }
    }
    
    return output;
}

cv::Mat sharpen(const cv::Mat& input) {
    if (input.empty()) return cv::Mat();
    
    cv::Mat kernel = utils::getSharpenKernel();
    cv::Mat output;
    applyConvolution(input, output, kernel);
    return output;
}

cv::Mat emboss(const cv::Mat& input) {
    if (input.empty()) return cv::Mat();
    
    cv::Mat kernel = utils::getEmbossKernel();
    cv::Mat output;
    applyConvolution(input, output, kernel);
    
    // Adicionar 128 para centralizar os valores
    for (int i = 0; i < output.rows; i++) {
        for (int j = 0; j < output.cols; j++) {
            if (output.channels() == 1) {
                output.at<uchar>(i, j) = cv::saturate_cast<uchar>(output.at<uchar>(i, j) + 128);
            } else {
                cv::Vec3b& pixel = output.at<cv::Vec3b>(i, j);
                pixel[0] = cv::saturate_cast<uchar>(pixel[0] + 128);
                pixel[1] = cv::saturate_cast<uchar>(pixel[1] + 128);
                pixel[2] = cv::saturate_cast<uchar>(pixel[2] + 128);
            }
        }
    }
    
    return output;
}

cv::Mat negative(const cv::Mat& input) {
    if (input.empty()) return cv::Mat();
    
    cv::Mat output = input.clone();
    
    for (int i = 0; i < input.rows; i++) {
        for (int j = 0; j < input.cols; j++) {
            if (input.channels() == 1) {
                output.at<uchar>(i, j) = 255 - input.at<uchar>(i, j);
            } else {
                cv::Vec3b pixel = input.at<cv::Vec3b>(i, j);
                output.at<cv::Vec3b>(i, j) = cv::Vec3b(255 - pixel[0], 255 - pixel[1], 255 - pixel[2]);
            }
        }
    }
    
    return output;
}

cv::Mat sepia(const cv::Mat& input) {
    if (input.empty()) return cv::Mat();
    
    cv::Mat colorInput = input.channels() == 1 ? utils::toColor(input) : input.clone();
    cv::Mat output = colorInput.clone();
    
    for (int i = 0; i < colorInput.rows; i++) {
        for (int j = 0; j < colorInput.cols; j++) {
            cv::Vec3b pixel = colorInput.at<cv::Vec3b>(i, j);
            int b = pixel[0], g = pixel[1], r = pixel[2];
            
            int newR = static_cast<int>(0.393 * r + 0.769 * g + 0.189 * b);
            int newG = static_cast<int>(0.349 * r + 0.686 * g + 0.168 * b);
            int newB = static_cast<int>(0.272 * r + 0.534 * g + 0.131 * b);
            
            output.at<cv::Vec3b>(i, j) = cv::Vec3b(
                cv::saturate_cast<uchar>(newB),
                cv::saturate_cast<uchar>(newG),
                cv::saturate_cast<uchar>(newR)
            );
        }
    }
    
    return output;
}

cv::Mat threshold(const cv::Mat& input, int thresholdValue) {
    if (input.empty()) return cv::Mat();
    
    cv::Mat gray = input.channels() == 3 ? grayscale(input) : input.clone();
    cv::Mat output(gray.size(), CV_8UC1);
    
    for (int i = 0; i < gray.rows; i++) {
        for (int j = 0; j < gray.cols; j++) {
            output.at<uchar>(i, j) = gray.at<uchar>(i, j) > thresholdValue ? 255 : 0;
        }
    }
    
    return output;
}

cv::Mat median(const cv::Mat& input, int kernelSize) {
    if (input.empty()) return cv::Mat();
    
    int k = kernelSize / 2;
    cv::Mat padded;
    cv::copyMakeBorder(input, padded, k, k, k, k, cv::BORDER_REPLICATE);
    
    cv::Mat output = cv::Mat::zeros(input.size(), input.type());
    std::vector<uchar> values(kernelSize * kernelSize);
    
    for (int i = 0; i < input.rows; i++) {
        for (int j = 0; j < input.cols; j++) {
            if (input.channels() == 1) {
                int idx = 0;
                for (int ki = 0; ki < kernelSize; ki++) {
                    for (int kj = 0; kj < kernelSize; kj++) {
                        values[idx++] = padded.at<uchar>(i + ki, j + kj);
                    }
                }
                std::sort(values.begin(), values.begin() + idx);
                output.at<uchar>(i, j) = values[idx / 2];
            } else {
                std::vector<uchar> bValues(kernelSize * kernelSize);
                std::vector<uchar> gValues(kernelSize * kernelSize);
                std::vector<uchar> rValues(kernelSize * kernelSize);
                int idx = 0;
                for (int ki = 0; ki < kernelSize; ki++) {
                    for (int kj = 0; kj < kernelSize; kj++) {
                        cv::Vec3b pixel = padded.at<cv::Vec3b>(i + ki, j + kj);
                        bValues[idx] = pixel[0];
                        gValues[idx] = pixel[1];
                        rValues[idx] = pixel[2];
                        idx++;
                    }
                }
                std::sort(bValues.begin(), bValues.begin() + idx);
                std::sort(gValues.begin(), gValues.begin() + idx);
                std::sort(rValues.begin(), rValues.begin() + idx);
                output.at<cv::Vec3b>(i, j) = cv::Vec3b(bValues[idx / 2], gValues[idx / 2], rValues[idx / 2]);
            }
        }
    }
    
    return output;
}

cv::Mat bilateral(const cv::Mat& input, int d, double sigmaColor, double sigmaSpace) {
    if (input.empty()) return cv::Mat();
    
    int radius = d / 2;
    cv::Mat padded;
    cv::copyMakeBorder(input, padded, radius, radius, radius, radius, cv::BORDER_REPLICATE);
    
    cv::Mat output = cv::Mat::zeros(input.size(), input.type());
    
    // Pré-calcular pesos espaciais
    std::vector<std::vector<double>> spatialWeights(d, std::vector<double>(d));
    for (int i = 0; i < d; i++) {
        for (int j = 0; j < d; j++) {
            int dx = i - radius;
            int dy = j - radius;
            spatialWeights[i][j] = std::exp(-(dx * dx + dy * dy) / (2 * sigmaSpace * sigmaSpace));
        }
    }
    
    for (int i = 0; i < input.rows; i++) {
        for (int j = 0; j < input.cols; j++) {
            if (input.channels() == 1) {
                double sumWeight = 0.0;
                double sumValue = 0.0;
                uchar centerVal = padded.at<uchar>(i + radius, j + radius);
                
                for (int ki = 0; ki < d; ki++) {
                    for (int kj = 0; kj < d; kj++) {
                        uchar neighVal = padded.at<uchar>(i + ki, j + kj);
                        double colorDiff = centerVal - neighVal;
                        double colorWeight = std::exp(-(colorDiff * colorDiff) / (2 * sigmaColor * sigmaColor));
                        double weight = spatialWeights[ki][kj] * colorWeight;
                        
                        sumWeight += weight;
                        sumValue += weight * neighVal;
                    }
                }
                
                output.at<uchar>(i, j) = cv::saturate_cast<uchar>(sumValue / sumWeight);
            } else {
                double sumWeightB = 0.0, sumWeightG = 0.0, sumWeightR = 0.0;
                double sumB = 0.0, sumG = 0.0, sumR = 0.0;
                cv::Vec3b centerPixel = padded.at<cv::Vec3b>(i + radius, j + radius);
                
                for (int ki = 0; ki < d; ki++) {
                    for (int kj = 0; kj < d; kj++) {
                        cv::Vec3b neighPixel = padded.at<cv::Vec3b>(i + ki, j + kj);
                        
                        for (int c = 0; c < 3; c++) {
                            double colorDiff = centerPixel[c] - neighPixel[c];
                            double colorWeight = std::exp(-(colorDiff * colorDiff) / (2 * sigmaColor * sigmaColor));
                            double weight = spatialWeights[ki][kj] * colorWeight;
                            
                            if (c == 0) { sumWeightB += weight; sumB += weight * neighPixel[0]; }
                            else if (c == 1) { sumWeightG += weight; sumG += weight * neighPixel[1]; }
                            else { sumWeightR += weight; sumR += weight * neighPixel[2]; }
                        }
                    }
                }
                
                output.at<cv::Vec3b>(i, j) = cv::Vec3b(
                    cv::saturate_cast<uchar>(sumB / sumWeightB),
                    cv::saturate_cast<uchar>(sumG / sumWeightG),
                    cv::saturate_cast<uchar>(sumR / sumWeightR)
                );
            }
        }
    }
    
    return output;
}

} // namespace sequential
} // namespace pavic
