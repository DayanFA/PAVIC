/**
 * PAVIC LAB 2025 - Native Processor Wrapper
 * Ponte entre código gerenciado (.NET) e código nativo (C++/CUDA)
 */

#pragma once

#include <opencv2/opencv.hpp>
#include "../include/ImageProcessor.h"
#include "../include/SequentialFilter.h"
#include "../include/ParallelFilter.h"
#include "../include/MultithreadFilter.h"
#include "../include/CUDAFilter.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Drawing::Imaging;
using namespace System::Runtime::InteropServices;

namespace PAVIC_LAB_2025 {

    // Classe wrapper para processamento nativo
    public ref class NativeProcessor
    {
    private:
        cv::VideoCapture* camera;
        pavic::ImageProcessor* processor;
        
    public:
        NativeProcessor()
        {
            camera = nullptr;
            processor = new pavic::ImageProcessor();
        }
        
        ~NativeProcessor()
        {
            this->!NativeProcessor();
        }
        
        !NativeProcessor()
        {
            if (camera != nullptr)
            {
                camera->release();
                delete camera;
                camera = nullptr;
            }
            if (processor != nullptr)
            {
                delete processor;
                processor = nullptr;
            }
        }
        
        // === CÂMERA ===
        
        bool StartCamera(int deviceId)
        {
            if (camera != nullptr)
            {
                camera->release();
                delete camera;
            }
            
            camera = new cv::VideoCapture(deviceId);
            
            if (!camera->isOpened())
            {
                delete camera;
                camera = nullptr;
                return false;
            }
            
            // Configurar resolução
            camera->set(cv::CAP_PROP_FRAME_WIDTH, 640);
            camera->set(cv::CAP_PROP_FRAME_HEIGHT, 480);
            
            return true;
        }
        
        void StopCamera()
        {
            if (camera != nullptr)
            {
                camera->release();
                delete camera;
                camera = nullptr;
            }
        }
        
        Bitmap^ CaptureFrame()
        {
            if (camera == nullptr || !camera->isOpened())
                return nullptr;
            
            cv::Mat frame;
            *camera >> frame;
            
            if (frame.empty())
                return nullptr;
            
            return MatToBitmap(frame);
        }
        
        // === PROCESSAMENTO ===
        
        Bitmap^ ApplyFilter(Bitmap^ input, int filterType, int processingMode, double% timeMs)
        {
            if (input == nullptr)
                return nullptr;
            
            // Converter Bitmap para cv::Mat
            cv::Mat inputMat = BitmapToMat(input);
            
            if (inputMat.empty())
                return nullptr;
            
            // Carregar imagem no processor
            processor->loadImage(inputMat);
            
            // Mapear tipos
            pavic::FilterType filter = static_cast<pavic::FilterType>(filterType);
            pavic::ProcessingType proc = static_cast<pavic::ProcessingType>(processingMode);
            
            // Aplicar filtro
            pavic::ProcessingResult result = processor->applyFilter(filter, proc);
            
            timeMs = result.executionTimeMs;
            
            if (!result.success || result.image.empty())
                return nullptr;
            
            // Converter resultado para Bitmap
            return MatToBitmap(result.image);
        }
        
    private:
        // Converter cv::Mat para System::Drawing::Bitmap
        Bitmap^ MatToBitmap(cv::Mat& mat)
        {
            if (mat.empty())
                return nullptr;
            
            cv::Mat rgbMat;
            
            // Converter para RGB se necessário
            if (mat.channels() == 1)
            {
                cv::cvtColor(mat, rgbMat, cv::COLOR_GRAY2BGR);
            }
            else if (mat.channels() == 3)
            {
                cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
            }
            else if (mat.channels() == 4)
            {
                cv::cvtColor(mat, rgbMat, cv::COLOR_BGRA2RGBA);
            }
            else
            {
                rgbMat = mat;
            }
            
            // Criar Bitmap
            int width = rgbMat.cols;
            int height = rgbMat.rows;
            int channels = rgbMat.channels();
            int stride = rgbMat.step;
            
            PixelFormat format;
            if (channels == 1)
                format = PixelFormat::Format8bppIndexed;
            else if (channels == 3)
                format = PixelFormat::Format24bppRgb;
            else
                format = PixelFormat::Format32bppArgb;
            
            Bitmap^ bmp = gcnew Bitmap(width, height, format);
            
            BitmapData^ bmpData = bmp->LockBits(
                System::Drawing::Rectangle(0, 0, width, height),
                ImageLockMode::WriteOnly,
                format);
            
            // Copiar dados
            unsigned char* src = rgbMat.data;
            unsigned char* dst = (unsigned char*)bmpData->Scan0.ToPointer();
            
            for (int y = 0; y < height; y++)
            {
                memcpy(dst + y * bmpData->Stride, src + y * stride, width * channels);
            }
            
            bmp->UnlockBits(bmpData);
            
            return bmp;
        }
        
        // Converter System::Drawing::Bitmap para cv::Mat
        cv::Mat BitmapToMat(Bitmap^ bmp)
        {
            if (bmp == nullptr)
                return cv::Mat();
            
            int width = bmp->Width;
            int height = bmp->Height;
            
            // Determinar formato
            PixelFormat format = bmp->PixelFormat;
            int channels = Bitmap::GetPixelFormatSize(format) / 8;
            
            if (channels < 3)
            {
                // Converter para 24bpp
                Bitmap^ converted = gcnew Bitmap(width, height, PixelFormat::Format24bppRgb);
                Graphics^ g = Graphics::FromImage(converted);
                g->DrawImage(bmp, 0, 0, width, height);
                delete g;
                bmp = converted;
                format = PixelFormat::Format24bppRgb;
                channels = 3;
            }
            
            BitmapData^ bmpData = bmp->LockBits(
                System::Drawing::Rectangle(0, 0, width, height),
                ImageLockMode::ReadOnly,
                format);
            
            int cvType = (channels == 4) ? CV_8UC4 : CV_8UC3;
            cv::Mat mat(height, width, cvType);
            
            unsigned char* src = (unsigned char*)bmpData->Scan0.ToPointer();
            unsigned char* dst = mat.data;
            
            for (int y = 0; y < height; y++)
            {
                memcpy(dst + y * mat.step, src + y * bmpData->Stride, width * channels);
            }
            
            bmp->UnlockBits(bmpData);
            
            // Converter de RGB para BGR (OpenCV usa BGR)
            cv::Mat bgrMat;
            if (channels == 4)
                cv::cvtColor(mat, bgrMat, cv::COLOR_RGBA2BGRA);
            else
                cv::cvtColor(mat, bgrMat, cv::COLOR_RGB2BGR);
            
            return bgrMat;
        }
    };
}
