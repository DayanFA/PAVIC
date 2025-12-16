/**
 * PAVIC LAB 2025 - Multithread Filter Implementation
 * Implementação usando std::thread dividindo a imagem por faixas de linhas.
 */

#include "MultithreadFilter.h"
#include "FilterUtils.h"
#include <thread>
#include <vector>
#include <functional>
#include <algorithm>
#include <cmath>

namespace pavic {
namespace multithread {

static void runInThreads(int rows, int numThreads, const std::function<void(int,int)>& worker) {
    if (numThreads <= 0) numThreads = getOptimalThreadCount();
    numThreads = std::max(1, std::min(numThreads, rows));
    int chunk = rows / numThreads;
    int remainder = rows % numThreads;
    std::vector<std::thread> ts;
    int start = 0;
    for (int t = 0; t < numThreads; ++t) {
        int extra = (t < remainder) ? 1 : 0;
        int end = start + chunk + extra;
        ts.emplace_back([=]() { worker(start, end); });
        start = end;
    }
    for (auto& th : ts) th.join();
}

void processRegion(const cv::Mat& input, cv::Mat& output, int startRow, int endRow,
                   std::function<void(const cv::Mat&, cv::Mat&, int, int)> processFunc) {
    processFunc(input, output, startRow, endRow);
}

int getOptimalThreadCount() {
    unsigned int hc = std::thread::hardware_concurrency();
    return hc > 0 ? static_cast<int>(hc) : 4;
}

cv::Mat grayscale(const cv::Mat& input, int numThreads) {
    if (input.empty()) return cv::Mat();
    if (input.channels() == 1) return input.clone();
    cv::Mat output(input.rows, input.cols, CV_8UC1);
    auto worker = [&](int s, int e){
        for (int i = s; i < e; ++i) {
            for (int j = 0; j < input.cols; ++j) {
                cv::Vec3b p = input.at<cv::Vec3b>(i, j);
                output.at<uchar>(i, j) = static_cast<uchar>(0.299 * p[2] + 0.587 * p[1] + 0.114 * p[0]);
            }
        }
    };
    runInThreads(input.rows, numThreads, worker);
    return output;
}

static void applyConvolutionMT(const cv::Mat& input, cv::Mat& output, const cv::Mat& kernel, int numThreads) {
    int kRows = kernel.rows, kCols = kernel.cols;
    int kCenterX = kCols / 2, kCenterY = kRows / 2;
    cv::Mat padded;
    cv::copyMakeBorder(input, padded, kCenterY, kCenterY, kCenterX, kCenterX, cv::BORDER_REPLICATE);
    output = cv::Mat::zeros(input.size(), input.type());
    auto worker = [&](int s, int e){
        for (int i = s; i < e; ++i) {
            for (int j = 0; j < input.cols; ++j) {
                if (input.channels() == 1) {
                    double sum = 0.0;
                    for (int ki = 0; ki < kRows; ++ki) {
                        for (int kj = 0; kj < kCols; ++kj) {
                            int pi = i + ki;
                            int pj = j + kj;
                            sum += padded.at<uchar>(pi, pj) * kernel.at<double>(ki, kj);
                        }
                    }
                    output.at<uchar>(i, j) = cv::saturate_cast<uchar>(sum);
                } else {
                    double b=0,g=0,r=0;
                    for (int ki = 0; ki < kRows; ++ki) {
                        for (int kj = 0; kj < kCols; ++kj) {
                            int pi = i + ki; int pj = j + kj; double kv = kernel.at<double>(ki, kj);
                            cv::Vec3b px = padded.at<cv::Vec3b>(pi, pj);
                            b += px[0]*kv; g += px[1]*kv; r += px[2]*kv;
                        }
                    }
                    output.at<cv::Vec3b>(i, j) = cv::Vec3b(cv::saturate_cast<uchar>(b), cv::saturate_cast<uchar>(g), cv::saturate_cast<uchar>(r));
                }
            }
        }
    };
    runInThreads(input.rows, numThreads, worker);
}

cv::Mat blur(const cv::Mat& input, int kernelSize, int numThreads) {
    if (input.empty()) return cv::Mat();
    cv::Mat kernel = utils::getBoxBlurKernel(kernelSize);
    cv::Mat output; applyConvolutionMT(input, output, kernel, numThreads); return output;
}

cv::Mat gaussianBlur(const cv::Mat& input, int kernelSize, int numThreads) {
    if (input.empty()) return cv::Mat();
    cv::Mat kernel = utils::getGaussianKernel(kernelSize);
    cv::Mat output; applyConvolutionMT(input, output, kernel, numThreads); return output;
}

cv::Mat sobel(const cv::Mat& input, int numThreads) {
    if (input.empty()) return cv::Mat();
    cv::Mat gray = input.channels() == 3 ? grayscale(input, numThreads) : input.clone();
    cv::Mat kx = utils::getSobelKernelX(); cv::Mat ky = utils::getSobelKernelY();
    cv::Mat gx, gy; applyConvolutionMT(gray, gx, kx, numThreads); applyConvolutionMT(gray, gy, ky, numThreads);
    cv::Mat out(gray.size(), CV_8UC1);
    auto worker = [&](int s, int e){
        for (int i = s; i < e; ++i) {
            for (int j = 0; j < gray.cols; ++j) {
                int x = gx.at<uchar>(i, j); int y = gy.at<uchar>(i, j);
                out.at<uchar>(i, j) = cv::saturate_cast<uchar>(std::sqrt(x*x + y*y));
            }
        }
    };
    runInThreads(gray.rows, numThreads, worker);
    return out;
}

cv::Mat canny(const cv::Mat& input, double threshold1, double threshold2, int numThreads) {
    if (input.empty()) return cv::Mat();
    cv::Mat gray = input.channels() == 3 ? grayscale(input, numThreads) : input.clone();
    cv::Mat blurred = gaussianBlur(gray, 5, numThreads);
    cv::Mat kx = utils::getSobelKernelX(); cv::Mat ky = utils::getSobelKernelY();
    cv::Mat gx, gy; applyConvolutionMT(blurred, gx, kx, numThreads); applyConvolutionMT(blurred, gy, ky, numThreads);
    cv::Mat mag(gray.size(), CV_8UC1), dir(gray.size(), CV_64F);
    auto gradWorker = [&](int s, int e){
        for (int i = s; i < e; ++i) {
            for (int j = 0; j < gray.cols; ++j) {
                double x = gx.at<uchar>(i, j) - 128, y = gy.at<uchar>(i, j) - 128;
                mag.at<uchar>(i, j) = cv::saturate_cast<uchar>(std::sqrt(x*x + y*y));
                dir.at<double>(i, j) = std::atan2(y, x);
            }
        }
    };
    runInThreads(gray.rows, numThreads, gradWorker);
    cv::Mat out(gray.size(), CV_8UC1, cv::Scalar(0));
    auto nmsWorker = [&](int s, int e){
        int start = std::max(1, s), end = std::min(e, gray.rows - 1);
        for (int i = start; i < end; ++i) {
            for (int j = 1; j < gray.cols - 1; ++j) {
                double angle = dir.at<double>(i, j) * 180.0 / CV_PI; if (angle < 0) angle += 180;
                uchar m = mag.at<uchar>(i, j), q = 255, r = 255;
                if ((angle>=0 && angle<22.5) || (angle>=157.5 && angle<=180)) { q = mag.at<uchar>(i, j+1); r = mag.at<uchar>(i, j-1); }
                else if (angle>=22.5 && angle<67.5) { q = mag.at<uchar>(i+1, j-1); r = mag.at<uchar>(i-1, j+1); }
                else if (angle>=67.5 && angle<112.5) { q = mag.at<uchar>(i+1, j); r = mag.at<uchar>(i-1, j); }
                else if (angle>=112.5 && angle<157.5) { q = mag.at<uchar>(i-1, j-1); r = mag.at<uchar>(i+1, j+1); }
                if (m>=q && m>=r) {
                    if (m>=threshold2) out.at<uchar>(i, j)=255; else if (m>=threshold1) out.at<uchar>(i, j)=128;
                }
            }
        }
    };
    runInThreads(gray.rows, numThreads, nmsWorker);
    // Histerese (sequencial)
    for (int i = 1; i < gray.rows - 1; ++i) {
        for (int j = 1; j < gray.cols - 1; ++j) {
            if (out.at<uchar>(i, j) == 128) {
                bool strong = false;
                for (int di=-1; di<=1 && !strong; ++di) for (int dj=-1; dj<=1 && !strong; ++dj) if (out.at<uchar>(i+di,j+dj)==255) strong=true;
                out.at<uchar>(i, j) = strong ? 255 : 0;
            }
        }
    }
    return out;
}

cv::Mat sharpen(const cv::Mat& input, int numThreads) {
    if (input.empty()) return cv::Mat();
    cv::Mat k = utils::getSharpenKernel(); cv::Mat out; applyConvolutionMT(input, out, k, numThreads); return out;
}

cv::Mat emboss(const cv::Mat& input, int numThreads) {
    if (input.empty()) return cv::Mat();
    cv::Mat k = utils::getEmbossKernel(); cv::Mat out; applyConvolutionMT(input, out, k, numThreads);
    auto worker = [&](int s, int e){
        for (int i = s; i < e; ++i) {
            for (int j = 0; j < out.cols; ++j) {
                if (out.channels()==1) out.at<uchar>(i,j) = cv::saturate_cast<uchar>(out.at<uchar>(i,j)+128);
                else { auto& p = out.at<cv::Vec3b>(i,j); p[0]=cv::saturate_cast<uchar>(p[0]+128); p[1]=cv::saturate_cast<uchar>(p[1]+128); p[2]=cv::saturate_cast<uchar>(p[2]+128); }
            }
        }
    }; runInThreads(out.rows, numThreads, worker);
    return out;
}

cv::Mat negative(const cv::Mat& input, int numThreads) {
    if (input.empty()) return cv::Mat();
    cv::Mat out = input.clone();
    auto worker = [&](int s, int e){
        for (int i = s; i < e; ++i) {
            for (int j = 0; j < input.cols; ++j) {
                if (input.channels()==1) out.at<uchar>(i,j) = 255 - input.at<uchar>(i,j);
                else { auto p = input.at<cv::Vec3b>(i,j); out.at<cv::Vec3b>(i,j) = cv::Vec3b(255-p[0],255-p[1],255-p[2]); }
            }
        }
    }; runInThreads(input.rows, numThreads, worker);
    return out;
}

cv::Mat sepia(const cv::Mat& input, int numThreads) {
    if (input.empty()) return cv::Mat();
    cv::Mat color = input.channels()==1 ? utils::toColor(input) : input.clone();
    cv::Mat out = color.clone();
    auto worker = [&](int s, int e){
        for (int i = s; i < e; ++i) {
            for (int j = 0; j < color.cols; ++j) {
                cv::Vec3b p = color.at<cv::Vec3b>(i,j); int b=p[0], g=p[1], r=p[2];
                int nr = static_cast<int>(0.393*r + 0.769*g + 0.189*b);
                int ng = static_cast<int>(0.349*r + 0.686*g + 0.168*b);
                int nb = static_cast<int>(0.272*r + 0.534*g + 0.131*b);
                out.at<cv::Vec3b>(i,j) = cv::Vec3b(cv::saturate_cast<uchar>(nb), cv::saturate_cast<uchar>(ng), cv::saturate_cast<uchar>(nr));
            }
        }
    }; runInThreads(color.rows, numThreads, worker);
    return out;
}

cv::Mat threshold(const cv::Mat& input, int thresholdValue, int numThreads) {
    if (input.empty()) return cv::Mat();
    cv::Mat gray = input.channels()==3 ? grayscale(input, numThreads) : input.clone();
    cv::Mat out(gray.size(), CV_8UC1);
    auto worker = [&](int s, int e){
        for (int i = s; i < e; ++i) {
            for (int j = 0; j < gray.cols; ++j) out.at<uchar>(i,j) = gray.at<uchar>(i,j) > thresholdValue ? 255 : 0;
        }
    }; runInThreads(gray.rows, numThreads, worker);
    return out;
}

cv::Mat median(const cv::Mat& input, int kernelSize, int numThreads) {
    if (input.empty()) return cv::Mat();
    int k = kernelSize/2; cv::Mat padded; cv::copyMakeBorder(input, padded, k,k,k,k, cv::BORDER_REPLICATE);
    cv::Mat out = cv::Mat::zeros(input.size(), input.type());
    auto worker = [&](int s, int e){
        for (int i = s; i < e; ++i) {
            for (int j = 0; j < input.cols; ++j) {
                if (input.channels()==1) {
                    std::vector<uchar> vals(kernelSize*kernelSize); int idx=0;
                    for (int ki=0; ki<kernelSize; ++ki) for (int kj=0; kj<kernelSize; ++kj) vals[idx++] = padded.at<uchar>(i+ki, j+kj);
                    std::sort(vals.begin(), vals.begin()+idx); out.at<uchar>(i,j) = vals[idx/2];
                } else {
                    std::vector<uchar> vb(kernelSize*kernelSize), vg(kernelSize*kernelSize), vr(kernelSize*kernelSize); int idx=0;
                    for (int ki=0; ki<kernelSize; ++ki) for (int kj=0; kj<kernelSize; ++kj) { auto p=padded.at<cv::Vec3b>(i+ki,j+kj); vb[idx]=p[0]; vg[idx]=p[1]; vr[idx]=p[2]; ++idx; }
                    std::sort(vb.begin(), vb.begin()+idx); std::sort(vg.begin(), vg.begin()+idx); std::sort(vr.begin(), vr.begin()+idx);
                    out.at<cv::Vec3b>(i,j) = cv::Vec3b(vb[idx/2], vg[idx/2], vr[idx/2]);
                }
            }
        }
    }; runInThreads(input.rows, numThreads, worker);
    return out;
}

cv::Mat bilateral(const cv::Mat& input, int d, double sigmaColor, double sigmaSpace, int numThreads) {
    if (input.empty()) return cv::Mat();
    int radius = d/2; cv::Mat padded; cv::copyMakeBorder(input, padded, radius,radius,radius,radius, cv::BORDER_REPLICATE);
    cv::Mat out = cv::Mat::zeros(input.size(), input.type());
    std::vector<std::vector<double>> spatial(d, std::vector<double>(d));
    for (int i=0;i<d;++i) for (int j=0;j<d;++j) { int dx=i-radius, dy=j-radius; spatial[i][j] = std::exp(-(dx*dx+dy*dy)/(2*sigmaSpace*sigmaSpace)); }
    auto worker = [&](int s, int e){
        for (int i = s; i < e; ++i) {
            for (int j = 0; j < input.cols; ++j) {
                if (input.channels()==1) {
                    double sw=0.0, sv=0.0; uchar c = padded.at<uchar>(i+radius, j+radius);
                    for (int ki=0; ki<d; ++ki) for (int kj=0; kj<d; ++kj) { uchar n=padded.at<uchar>(i+ki,j+kj); double cw=std::exp(-((c-n)*(c-n))/(2*sigmaColor*sigmaColor)); double w=spatial[ki][kj]*cw; sw+=w; sv+=w*n; }
                    out.at<uchar>(i,j) = cv::saturate_cast<uchar>(sv/sw);
                } else {
                    double swb=0, swg=0, swr=0, sb=0, sg=0, sr=0; auto c=padded.at<cv::Vec3b>(i+radius,j+radius);
                    for (int ki=0; ki<d; ++ki) for (int kj=0; kj<d; ++kj) { auto n=padded.at<cv::Vec3b>(i+ki,j+kj);
                        for (int ch=0; ch<3; ++ch) { double cd=(c[ch]-n[ch]); double cw=std::exp(-(cd*cd)/(2*sigmaColor*sigmaColor)); double w=spatial[ki][kj]*cw;
                            if (ch==0) { swb+=w; sb+=w*n[0]; } else if (ch==1) { swg+=w; sg+=w*n[1]; } else { swr+=w; sr+=w*n[2]; }
                        }
                    }
                    out.at<cv::Vec3b>(i,j) = cv::Vec3b(cv::saturate_cast<uchar>(sb/swb), cv::saturate_cast<uchar>(sg/swg), cv::saturate_cast<uchar>(sr/swr));
                }
            }
        }
    }; runInThreads(input.rows, numThreads, worker);
    return out;
}

} // namespace multithread
} // namespace pavic
