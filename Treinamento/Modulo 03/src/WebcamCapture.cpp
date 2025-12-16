/**
 * PAVIC LAB 2025 - WebcamCapture
 */

#include "WebcamCapture.h"

namespace pavic {

WebcamCapture::WebcamCapture(int cameraId) : cameraId(cameraId), running(false), newFrame(false) {}
WebcamCapture::~WebcamCapture() { stop(); }

bool WebcamCapture::start() {
    if (running.load()) return true;
    if (!capture.open(cameraId)) return false;
    running.store(true);
    newFrame.store(false);
    captureThread = std::thread(&WebcamCapture::captureLoop, this);
    return true;
}

void WebcamCapture::stop() {
    if (!running.load()) return;
    running.store(false);
    if (captureThread.joinable()) captureThread.join();
    if (capture.isOpened()) capture.release();
}

bool WebcamCapture::isRunning() const { return running.load(); }

cv::Mat WebcamCapture::getFrame() {
    std::lock_guard<std::mutex> lock(frameMutex);
    newFrame.store(false);
    return currentFrame.clone();
}

bool WebcamCapture::hasNewFrame() const { return newFrame.load(); }

bool WebcamCapture::setResolution(int width, int height) {
    return capture.set(cv::CAP_PROP_FRAME_WIDTH, width) && capture.set(cv::CAP_PROP_FRAME_HEIGHT, height);
}

bool WebcamCapture::setFPS(int fps) { return capture.set(cv::CAP_PROP_FPS, fps); }

cv::Size WebcamCapture::getResolution() const {
    return cv::Size((int)capture.get(cv::CAP_PROP_FRAME_WIDTH), (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT));
}

int WebcamCapture::getFPS() const { return (int)capture.get(cv::CAP_PROP_FPS); }

bool WebcamCapture::isOpened() const { return capture.isOpened(); }

int WebcamCapture::getCameraId() const { return cameraId; }

void WebcamCapture::captureLoop() {
    while (running.load()) {
        cv::Mat frame;
        if (!capture.read(frame)) {
            // pequena pausa para evitar busy-loop em erro
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        {
            std::lock_guard<std::mutex> lock(frameMutex);
            currentFrame = frame;
            newFrame.store(true);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

} // namespace pavic
