#ifndef WEBCAM_CAPTURE_H
#define WEBCAM_CAPTURE_H

#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <atomic>

namespace pavic {

class WebcamCapture {
public:
    WebcamCapture(int cameraId = 0);
    ~WebcamCapture();

    // Controle
    bool start();
    void stop();
    bool isRunning() const;

    // Captura
    cv::Mat getFrame();
    bool hasNewFrame() const;

    // Configurações
    bool setResolution(int width, int height);
    bool setFPS(int fps);
    cv::Size getResolution() const;
    int getFPS() const;

    // Informações
    bool isOpened() const;
    int getCameraId() const;

private:
    cv::VideoCapture capture;
    int cameraId;
    
    std::thread captureThread;
    std::mutex frameMutex;
    std::atomic<bool> running;
    std::atomic<bool> newFrame;
    
    cv::Mat currentFrame;
    
    void captureLoop();
};

} // namespace pavic

#endif // WEBCAM_CAPTURE_H
