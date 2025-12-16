/**
 * PAVIC LAB 2025 - CUDAFilter (Implementação Real)
 * Filtros de imagem usando CUDA para processamento na GPU
 */

#include "CUDAFilter.h"
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <iostream>
#include <cmath>

namespace pavic {
namespace cuda {

// ============== CUDA Kernels ==============

__global__ void grayscaleKernel(const uchar3* input, unsigned char* output, int width, int height) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x < width && y < height) {
        int idx = y * width + x;
        uchar3 pixel = input[idx];
        output[idx] = static_cast<unsigned char>(0.299f * pixel.x + 0.587f * pixel.y + 0.114f * pixel.z);
    }
}

__global__ void negativeKernel(const uchar3* input, uchar3* output, int width, int height) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x < width && y < height) {
        int idx = y * width + x;
        uchar3 pixel = input[idx];
        output[idx] = make_uchar3(255 - pixel.x, 255 - pixel.y, 255 - pixel.z);
    }
}

__global__ void sepiaKernel(const uchar3* input, uchar3* output, int width, int height) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x < width && y < height) {
        int idx = y * width + x;
        uchar3 pixel = input[idx];
        
        float b = pixel.x, g = pixel.y, r = pixel.z;
        int newR = min(255, (int)(0.393f * r + 0.769f * g + 0.189f * b));
        int newG = min(255, (int)(0.349f * r + 0.686f * g + 0.168f * b));
        int newB = min(255, (int)(0.272f * r + 0.534f * g + 0.131f * b));
        
        output[idx] = make_uchar3(newB, newG, newR);
    }
}

__global__ void thresholdKernel(const uchar3* input, unsigned char* output, int width, int height, int thresh) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x < width && y < height) {
        int idx = y * width + x;
        uchar3 pixel = input[idx];
        unsigned char gray = static_cast<unsigned char>(0.299f * pixel.x + 0.587f * pixel.y + 0.114f * pixel.z);
        output[idx] = (gray > thresh) ? 255 : 0;
    }
}

__global__ void blurKernel(const uchar3* input, uchar3* output, int width, int height, int kernelSize) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x < width && y < height) {
        int half = kernelSize / 2;
        float3 sum = make_float3(0.0f, 0.0f, 0.0f);
        int count = 0;
        
        for (int ky = -half; ky <= half; ++ky) {
            for (int kx = -half; kx <= half; ++kx) {
                int nx = x + kx;
                int ny = y + ky;
                if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                    uchar3 pixel = input[ny * width + nx];
                    sum.x += pixel.x;
                    sum.y += pixel.y;
                    sum.z += pixel.z;
                    count++;
                }
            }
        }
        
        output[y * width + x] = make_uchar3(
            static_cast<unsigned char>(sum.x / count),
            static_cast<unsigned char>(sum.y / count),
            static_cast<unsigned char>(sum.z / count)
        );
    }
}

// ============== Helper Functions ==============

static bool cudaAvailable = false;
static bool cudaChecked = false;

bool isCUDAAvailable() {
    if (!cudaChecked) {
        int deviceCount = 0;
        cudaError_t err = cudaGetDeviceCount(&deviceCount);
        cudaAvailable = (err == cudaSuccess && deviceCount > 0);
        cudaChecked = true;
    }
    return cudaAvailable;
}

int getCUDADeviceCount() {
    int count = 0;
    cudaGetDeviceCount(&count);
    return count;
}

std::string getCUDADeviceName(int deviceId) {
    cudaDeviceProp prop;
    if (cudaGetDeviceProperties(&prop, deviceId) == cudaSuccess) {
        return std::string(prop.name);
    }
    return "Unknown";
}

// ============== Filter Implementations ==============

cv::Mat grayscale(const cv::Mat& input) {
    if (!isCUDAAvailable() || input.empty()) {
        return cv::Mat();
    }
    
    int width = input.cols;
    int height = input.rows;
    
    cv::Mat inputBGR;
    if (input.channels() == 1) {
        return input.clone();
    }
    inputBGR = input;
    
    // Allocate device memory
    uchar3* d_input;
    unsigned char* d_output;
    size_t inputSize = width * height * sizeof(uchar3);
    size_t outputSize = width * height * sizeof(unsigned char);
    
    cudaMalloc(&d_input, inputSize);
    cudaMalloc(&d_output, outputSize);
    
    // Copy input to device
    cudaMemcpy(d_input, inputBGR.data, inputSize, cudaMemcpyHostToDevice);
    
    // Launch kernel
    dim3 block(16, 16);
    dim3 grid((width + block.x - 1) / block.x, (height + block.y - 1) / block.y);
    grayscaleKernel<<<grid, block>>>(d_input, d_output, width, height);
    
    // Copy result back
    cv::Mat output(height, width, CV_8UC1);
    cudaMemcpy(output.data, d_output, outputSize, cudaMemcpyDeviceToHost);
    
    // Free device memory
    cudaFree(d_input);
    cudaFree(d_output);
    
    return output;
}

cv::Mat negative(const cv::Mat& input) {
    if (!isCUDAAvailable() || input.empty()) {
        return cv::Mat();
    }
    
    cv::Mat inputBGR;
    if (input.channels() == 1) {
        cv::cvtColor(input, inputBGR, cv::COLOR_GRAY2BGR);
    } else {
        inputBGR = input;
    }
    
    int width = inputBGR.cols;
    int height = inputBGR.rows;
    
    uchar3* d_input;
    uchar3* d_output;
    size_t size = width * height * sizeof(uchar3);
    
    cudaMalloc(&d_input, size);
    cudaMalloc(&d_output, size);
    cudaMemcpy(d_input, inputBGR.data, size, cudaMemcpyHostToDevice);
    
    dim3 block(16, 16);
    dim3 grid((width + block.x - 1) / block.x, (height + block.y - 1) / block.y);
    negativeKernel<<<grid, block>>>(d_input, d_output, width, height);
    
    cv::Mat output(height, width, CV_8UC3);
    cudaMemcpy(output.data, d_output, size, cudaMemcpyDeviceToHost);
    
    cudaFree(d_input);
    cudaFree(d_output);
    
    return output;
}

cv::Mat sepia(const cv::Mat& input) {
    if (!isCUDAAvailable() || input.empty()) {
        return cv::Mat();
    }
    
    cv::Mat inputBGR;
    if (input.channels() == 1) {
        cv::cvtColor(input, inputBGR, cv::COLOR_GRAY2BGR);
    } else {
        inputBGR = input;
    }
    
    int width = inputBGR.cols;
    int height = inputBGR.rows;
    
    uchar3* d_input;
    uchar3* d_output;
    size_t size = width * height * sizeof(uchar3);
    
    cudaMalloc(&d_input, size);
    cudaMalloc(&d_output, size);
    cudaMemcpy(d_input, inputBGR.data, size, cudaMemcpyHostToDevice);
    
    dim3 block(16, 16);
    dim3 grid((width + block.x - 1) / block.x, (height + block.y - 1) / block.y);
    sepiaKernel<<<grid, block>>>(d_input, d_output, width, height);
    
    cv::Mat output(height, width, CV_8UC3);
    cudaMemcpy(output.data, d_output, size, cudaMemcpyDeviceToHost);
    
    cudaFree(d_input);
    cudaFree(d_output);
    
    return output;
}

cv::Mat threshold(const cv::Mat& input, int thresholdValue) {
    if (!isCUDAAvailable() || input.empty()) {
        return cv::Mat();
    }
    
    cv::Mat inputBGR;
    if (input.channels() == 1) {
        cv::cvtColor(input, inputBGR, cv::COLOR_GRAY2BGR);
    } else {
        inputBGR = input;
    }
    
    int width = inputBGR.cols;
    int height = inputBGR.rows;
    
    uchar3* d_input;
    unsigned char* d_output;
    
    cudaMalloc(&d_input, width * height * sizeof(uchar3));
    cudaMalloc(&d_output, width * height * sizeof(unsigned char));
    cudaMemcpy(d_input, inputBGR.data, width * height * sizeof(uchar3), cudaMemcpyHostToDevice);
    
    dim3 block(16, 16);
    dim3 grid((width + block.x - 1) / block.x, (height + block.y - 1) / block.y);
    thresholdKernel<<<grid, block>>>(d_input, d_output, width, height, thresholdValue);
    
    cv::Mat output(height, width, CV_8UC1);
    cudaMemcpy(output.data, d_output, width * height, cudaMemcpyDeviceToHost);
    
    cudaFree(d_input);
    cudaFree(d_output);
    
    return output;
}

cv::Mat blur(const cv::Mat& input, int kernelSize) {
    if (!isCUDAAvailable() || input.empty()) {
        return cv::Mat();
    }
    
    cv::Mat inputBGR;
    if (input.channels() == 1) {
        cv::cvtColor(input, inputBGR, cv::COLOR_GRAY2BGR);
    } else {
        inputBGR = input;
    }
    
    int width = inputBGR.cols;
    int height = inputBGR.rows;
    
    uchar3* d_input;
    uchar3* d_output;
    size_t size = width * height * sizeof(uchar3);
    
    cudaMalloc(&d_input, size);
    cudaMalloc(&d_output, size);
    cudaMemcpy(d_input, inputBGR.data, size, cudaMemcpyHostToDevice);
    
    dim3 block(16, 16);
    dim3 grid((width + block.x - 1) / block.x, (height + block.y - 1) / block.y);
    blurKernel<<<grid, block>>>(d_input, d_output, width, height, kernelSize);
    
    cv::Mat output(height, width, CV_8UC3);
    cudaMemcpy(output.data, d_output, size, cudaMemcpyDeviceToHost);
    
    cudaFree(d_input);
    cudaFree(d_output);
    
    return output;
}

cv::Mat gaussianBlur(const cv::Mat& input, int kernelSize) {
    // Para Gaussian, usamos blur simples por enquanto (pode ser melhorado com kernel gaussiano)
    return blur(input, kernelSize);
}

cv::Mat sobel(const cv::Mat& input) {
    // Sobel é complexo em CUDA, usar OpenCV com GPU como fallback
    if (!isCUDAAvailable() || input.empty()) return cv::Mat();
    
    cv::Mat gray;
    if (input.channels() == 3) {
        gray = grayscale(input);
    } else {
        gray = input;
    }
    
    cv::Mat gradX, gradY, output;
    cv::Sobel(gray, gradX, CV_16S, 1, 0, 3);
    cv::Sobel(gray, gradY, CV_16S, 0, 1, 3);
    cv::convertScaleAbs(gradX, gradX);
    cv::convertScaleAbs(gradY, gradY);
    cv::addWeighted(gradX, 0.5, gradY, 0.5, 0, output);
    
    return output;
}

cv::Mat canny(const cv::Mat& input, double threshold1, double threshold2) {
    if (!isCUDAAvailable() || input.empty()) return cv::Mat();
    
    cv::Mat gray;
    if (input.channels() == 3) {
        gray = grayscale(input);
    } else {
        gray = input;
    }
    
    cv::Mat output;
    cv::Canny(gray, output, threshold1, threshold2);
    return output;
}

cv::Mat sharpen(const cv::Mat& input) {
    if (!isCUDAAvailable() || input.empty()) return cv::Mat();
    
    cv::Mat blurred = blur(input, 5);
    cv::Mat output;
    cv::addWeighted(input, 1.5, blurred, -0.5, 0, output);
    return output;
}

cv::Mat emboss(const cv::Mat& input) {
    if (!isCUDAAvailable() || input.empty()) return cv::Mat();
    
    cv::Mat gray;
    if (input.channels() == 3) {
        gray = grayscale(input);
    } else {
        gray = input;
    }
    
    cv::Mat kernel = (cv::Mat_<float>(3,3) << -2, -1, 0, -1, 1, 1, 0, 1, 2);
    cv::Mat output;
    cv::filter2D(gray, output, -1, kernel);
    return output;
}

cv::Mat median(const cv::Mat& input, int kernelSize) {
    if (!isCUDAAvailable() || input.empty()) return cv::Mat();
    
    cv::Mat output;
    cv::medianBlur(input, output, kernelSize);
    return output;
}

cv::Mat bilateral(const cv::Mat& input, int d, double sigmaColor, double sigmaSpace) {
    if (!isCUDAAvailable() || input.empty()) return cv::Mat();
    
    cv::Mat output;
    cv::bilateralFilter(input, output, d, sigmaColor, sigmaSpace);
    return output;
}

} // namespace cuda
} // namespace pavic
