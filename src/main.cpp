/**
 * PAVIC LAB 2025 - Processamento de Imagens GPU vs CPU
 * 
 * Funcionalidades:
 * - Comparação Sequential, Parallel (OpenMP), Multithread, CUDA
 * - Benchmark comparativo mostrando todos os tempos e speedups
 * - Processamento de webcam em tempo real com FPS
 * - Diálogo nativo para seleção de arquivos
 * 
 * Teclas:
 * - 1-9, 0, B: Selecionar filtro
 * - M: Alternar modo de processamento
 * - O: Abrir imagem ou câmera
 * - S: Salvar resultado
 * - C: Benchmark Comparativo (roda nos 4 modos)
 * - Q/ESC: Sair
 */

#include "ImageProcessor.h"
#include "PerformanceMetrics.h"

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <iomanip>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

using namespace pavic;

// Abre diálogo nativo do Windows para selecionar arquivo de imagem
std::string openFileDialog() {
#ifdef _WIN32
    OPENFILENAMEA ofn;
    char szFile[MAX_PATH] = {0};
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Imagens\0*.jpg;*.jpeg;*.png;*.bmp;*.tiff;*.webp\0Todos os arquivos\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = "Selecione uma imagem";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    
    if (GetOpenFileNameA(&ofn) == TRUE) {
        return std::string(szFile);
    }
#endif
    return "";
}

// Mostra menu de seleção e retorna escolha (1=imagem, 2=câmera, 0=cancelar)
int showSourceMenu() {
    cv::Mat menu(300, 500, CV_8UC3, cv::Scalar(40, 40, 40));
    
    // Título
    cv::putText(menu, "SELECIONE A FONTE", {100, 50}, 
                cv::FONT_HERSHEY_DUPLEX, 1.0, {255,255,255}, 2, cv::LINE_AA);
    
    // Linha divisória
    cv::line(menu, {50, 70}, {450, 70}, {100, 100, 100}, 2);
    
    // Opção 1: Imagem
    cv::rectangle(menu, {50, 100}, {450, 160}, {60, 120, 60}, cv::FILLED);
    cv::rectangle(menu, {50, 100}, {450, 160}, {100, 200, 100}, 2);
    cv::putText(menu, "[1]  ABRIR IMAGEM DO PC", {80, 140}, 
                cv::FONT_HERSHEY_DUPLEX, 0.7, {255,255,255}, 2, cv::LINE_AA);
    
    // Opção 2: Câmera
    cv::rectangle(menu, {50, 180}, {450, 240}, {60, 100, 120}, cv::FILLED);
    cv::rectangle(menu, {50, 180}, {450, 240}, {100, 180, 220}, 2);
    cv::putText(menu, "[2]  USAR WEBCAM", {80, 220}, 
                cv::FONT_HERSHEY_DUPLEX, 0.7, {255,255,255}, 2, cv::LINE_AA);
    
    // Cancelar
    cv::putText(menu, "[ESC] Cancelar", {180, 280}, 
                cv::FONT_HERSHEY_DUPLEX, 0.5, {150,150,150}, 1, cv::LINE_AA);
    
    cv::imshow("PAVIC LAB 2025 - Selecionar Fonte", menu);
    
    while (true) {
        int key = cv::waitKey(0);
        if (key == '1') { cv::destroyWindow("PAVIC LAB 2025 - Selecionar Fonte"); return 1; }
        if (key == '2') { cv::destroyWindow("PAVIC LAB 2025 - Selecionar Fonte"); return 2; }
        if (key == 27)  { cv::destroyWindow("PAVIC LAB 2025 - Selecionar Fonte"); return 0; }
    }
}

// Resultado do benchmark comparativo
struct BenchmarkResult {
    double timeSequential = 0;
    double timeParallel = 0;
    double timeMultithread = 0;
    double timeCUDA = 0;
    bool hasResults = false;
};

struct State {
    FilterType filter = FilterType::GRAYSCALE;
    ProcessingType proc = ProcessingType::SEQUENTIAL;
    ProcessingResult last{};
    BenchmarkResult benchmark;
    bool usingCamera = false;
    cv::VideoCapture camera;
    
    // FPS tracking
    int frameCount = 0;
    double fps = 0.0;
    std::chrono::steady_clock::time_point fpsStartTime;
};

// Desenha texto com fundo para melhor legibilidade
static void drawText(cv::Mat& img, const std::string& text, cv::Point pos, 
                     double scale, cv::Scalar color, int thickness = 1, bool withBg = true) {
    int baseline = 0;
    cv::Size sz = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, scale, thickness, &baseline);
    
    if (withBg) {
        cv::rectangle(img, 
                      cv::Point(pos.x - 4, pos.y - sz.height - 4),
                      cv::Point(pos.x + sz.width + 4, pos.y + baseline + 4),
                      cv::Scalar(0, 0, 0), cv::FILLED);
    }
    cv::putText(img, text, pos, cv::FONT_HERSHEY_SIMPLEX, scale, color, thickness, cv::LINE_AA);
}

static void drawSideBySide(const cv::Mat& left, const cv::Mat& right, const State& s) {
    if (left.empty()) return;
    cv::Mat r = right.empty() ? left : right;
    
    // Converter para BGR se necessário
    cv::Mat leftBGR, rightBGR;
    if (left.channels() == 1) cv::cvtColor(left, leftBGR, cv::COLOR_GRAY2BGR);
    else leftBGR = left;
    if (r.channels() == 1) cv::cvtColor(r, rightBGR, cv::COLOR_GRAY2BGR);
    else rightBGR = r;
    
    // Tamanho das imagens
    const int imgW = 450, imgH = 340;
    cv::Mat leftResized, rightResized;
    cv::resize(leftBGR, leftResized, cv::Size(imgW, imgH), 0, 0, cv::INTER_LINEAR);
    cv::resize(rightBGR, rightResized, cv::Size(imgW, imgH), 0, 0, cv::INTER_LINEAR);

    // Canvas com espaço para painel de benchmark
    const int headerH = 55, footerH = 35, gap = 8;
    const int benchmarkW = 280;
    const int totalW = imgW * 2 + gap * 3 + benchmarkW;
    const int totalH = headerH + imgH + footerH;
    
    cv::Mat canvas(totalH, totalW, CV_8UC3, cv::Scalar(30, 30, 30));
    
    // Copiar imagens para o canvas
    leftResized.copyTo(canvas(cv::Rect(gap, headerH, imgW, imgH)));
    rightResized.copyTo(canvas(cv::Rect(imgW + gap * 2, headerH, imgW, imgH)));
    
    // Header - informações
    std::string filterName = ImageProcessor::getFilterName(s.filter);
    std::string procName = ImageProcessor::getProcessingName(s.proc);
    std::string source = s.usingCamera ? "[CAMERA]" : "[IMAGEM]";
    
    drawText(canvas, "Filtro: " + filterName, {gap, 22}, 0.55, {0, 255, 100}, 2);
    drawText(canvas, "Modo: " + procName, {gap + 220, 22}, 0.55, {100, 200, 255}, 2);
    drawText(canvas, source, {gap + 420, 22}, 0.55, {255, 255, 0}, 2);
    
    if (s.last.success) {
        char timeStr[32];
        snprintf(timeStr, sizeof(timeStr), "%.2f ms", s.last.executionTimeMs);
        drawText(canvas, timeStr, {gap + 550, 22}, 0.55, {0, 255, 255}, 2);
    }
    
    // FPS (para câmera)
    if (s.usingCamera && s.fps > 0) {
        char fpsStr[32];
        snprintf(fpsStr, sizeof(fpsStr), "FPS: %.1f", s.fps);
        cv::Scalar fpsColor = s.fps >= 25 ? cv::Scalar(0, 255, 0) : 
                              (s.fps >= 15 ? cv::Scalar(0, 255, 255) : cv::Scalar(0, 0, 255));
        drawText(canvas, fpsStr, {gap + 680, 22}, 0.6, fpsColor, 2);
    }
    
    // Labels das imagens
    drawText(canvas, "ORIGINAL", {gap + imgW/2 - 45, headerH - 5}, 0.5, {200, 200, 200}, 1, false);
    drawText(canvas, "PROCESSADA", {imgW + gap*2 + imgW/2 - 55, headerH - 5}, 0.5, {200, 200, 200}, 1, false);
    
    // === PAINEL DE BENCHMARK COMPARATIVO ===
    int benchX = imgW * 2 + gap * 3;
    cv::rectangle(canvas, cv::Point(benchX, headerH), 
                  cv::Point(benchX + benchmarkW - gap, headerH + imgH),
                  cv::Scalar(40, 40, 45), cv::FILLED);
    cv::rectangle(canvas, cv::Point(benchX, headerH), 
                  cv::Point(benchX + benchmarkW - gap, headerH + imgH),
                  cv::Scalar(80, 80, 80), 1);
    
    drawText(canvas, "BENCHMARK COMPARATIVO", {benchX + 20, headerH + 25}, 0.5, {255, 255, 255}, 1, false);
    drawText(canvas, "[C] para executar", {benchX + 55, headerH + 48}, 0.4, {150, 150, 150}, 1, false);
    
    int yPos = headerH + 75;
    int spacing = 62;
    
    // Sequential
    drawText(canvas, "SEQUENTIAL (CPU)", {benchX + 10, yPos}, 0.45, {100, 150, 255}, 1, false);
    if (s.benchmark.hasResults) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f ms", s.benchmark.timeSequential);
        drawText(canvas, buf, {benchX + 10, yPos + 20}, 0.5, {255, 255, 255}, 1, false);
    } else {
        drawText(canvas, "-- ms", {benchX + 10, yPos + 20}, 0.5, {100, 100, 100}, 1, false);
    }
    
    yPos += spacing;
    
    // Parallel
    drawText(canvas, "PARALLEL (OpenMP)", {benchX + 10, yPos}, 0.45, {100, 255, 150}, 1, false);
    if (s.benchmark.hasResults) {
        char buf[64];
        double speedup = s.benchmark.timeSequential / s.benchmark.timeParallel;
        snprintf(buf, sizeof(buf), "%.2f ms (%.1fx)", s.benchmark.timeParallel, speedup);
        cv::Scalar col = speedup > 1 ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);
        drawText(canvas, buf, {benchX + 10, yPos + 20}, 0.5, col, 1, false);
    } else {
        drawText(canvas, "-- ms", {benchX + 10, yPos + 20}, 0.5, {100, 100, 100}, 1, false);
    }
    
    yPos += spacing;
    
    // Multithread
    drawText(canvas, "MULTITHREAD (std::thread)", {benchX + 10, yPos}, 0.45, {255, 220, 100}, 1, false);
    if (s.benchmark.hasResults) {
        char buf[64];
        double speedup = s.benchmark.timeSequential / s.benchmark.timeMultithread;
        snprintf(buf, sizeof(buf), "%.2f ms (%.1fx)", s.benchmark.timeMultithread, speedup);
        cv::Scalar col = speedup > 1 ? cv::Scalar(0, 255, 255) : cv::Scalar(0, 0, 255);
        drawText(canvas, buf, {benchX + 10, yPos + 20}, 0.5, col, 1, false);
    } else {
        drawText(canvas, "-- ms", {benchX + 10, yPos + 20}, 0.5, {100, 100, 100}, 1, false);
    }
    
    yPos += spacing;
    
    // CUDA
    drawText(canvas, "CUDA (GPU)", {benchX + 10, yPos}, 0.45, {255, 100, 100}, 1, false);
    if (s.benchmark.hasResults) {
        char buf[64];
        double speedup = s.benchmark.timeSequential / s.benchmark.timeCUDA;
        snprintf(buf, sizeof(buf), "%.2f ms (%.1fx)", s.benchmark.timeCUDA, speedup);
        cv::Scalar col = speedup > 1 ? cv::Scalar(255, 0, 255) : cv::Scalar(0, 0, 255);
        drawText(canvas, buf, {benchX + 10, yPos + 20}, 0.5, col, 1, false);
    } else {
        drawText(canvas, "-- ms", {benchX + 10, yPos + 20}, 0.5, {100, 100, 100}, 1, false);
    }
    
    // Footer - teclas de atalho
    int footerY = headerH + imgH + 22;
    drawText(canvas, "[1-9,0,B] Filtros", {gap, footerY}, 0.4, {180, 180, 180}, 1, false);
    drawText(canvas, "[M] Modo", {gap + 150, footerY}, 0.4, {180, 180, 180}, 1, false);
    drawText(canvas, "[C] Benchmark", {gap + 270, footerY}, 0.4, {255, 200, 100}, 1, false);
    drawText(canvas, "[O] Abrir", {gap + 410, footerY}, 0.4, {180, 180, 180}, 1, false);
    drawText(canvas, "[S] Salvar", {gap + 520, footerY}, 0.4, {180, 180, 180}, 1, false);
    drawText(canvas, "[Q] Sair", {gap + 630, footerY}, 0.4, {180, 180, 180}, 1, false);
    
    cv::imshow("PAVIC LAB 2025", canvas);
}

static FilterType keyToFilter(int key, FilterType current) {
    switch (key) {
        case '1': return FilterType::GRAYSCALE;
        case '2': return FilterType::BLUR;
        case '3': return FilterType::GAUSSIAN_BLUR;
        case '4': return FilterType::SOBEL;
        case '5': return FilterType::CANNY;
        case '6': return FilterType::SHARPEN;
        case '7': return FilterType::EMBOSS;
        case '8': return FilterType::NEGATIVE;
        case '9': return FilterType::SEPIA;
        case '0': return FilterType::THRESHOLD;
        case 'b': case 'B': return FilterType::BILATERAL;
        default: return current;
    }
}

// === BENCHMARK COMPARATIVO ===
void runComparativeBenchmark(ImageProcessor& proc, State& s) {
    if (proc.getOriginalImage().empty()) return;
    
    std::cout << "\n=== BENCHMARK COMPARATIVO ===" << std::endl;
    std::cout << "Filtro: " << ImageProcessor::getFilterName(s.filter) << std::endl;
    std::cout << "Imagem: " << proc.getOriginalImage().cols << "x" 
              << proc.getOriginalImage().rows << std::endl;
    std::cout << "----------------------------" << std::endl;
    
    // Sequential
    auto result = proc.applyFilter(s.filter, ProcessingType::SEQUENTIAL);
    s.benchmark.timeSequential = result.executionTimeMs;
    std::cout << "Sequential:  " << std::fixed << std::setprecision(2) 
              << s.benchmark.timeSequential << " ms" << std::endl;
    
    // Parallel
    result = proc.applyFilter(s.filter, ProcessingType::PARALLEL);
    s.benchmark.timeParallel = result.executionTimeMs;
    std::cout << "Parallel:    " << s.benchmark.timeParallel << " ms ("
              << std::setprecision(1) << (s.benchmark.timeSequential / s.benchmark.timeParallel) 
              << "x)" << std::endl;
    
    // Multithread
    result = proc.applyFilter(s.filter, ProcessingType::MULTITHREAD);
    s.benchmark.timeMultithread = result.executionTimeMs;
    std::cout << "Multithread: " << std::setprecision(2) << s.benchmark.timeMultithread << " ms ("
              << std::setprecision(1) << (s.benchmark.timeSequential / s.benchmark.timeMultithread) 
              << "x)" << std::endl;
    
    // CUDA
    result = proc.applyFilter(s.filter, ProcessingType::CUDA);
    s.benchmark.timeCUDA = result.executionTimeMs;
    s.last = result; // Mostrar resultado CUDA
    std::cout << "CUDA:        " << std::setprecision(2) << s.benchmark.timeCUDA << " ms ("
              << std::setprecision(1) << (s.benchmark.timeSequential / s.benchmark.timeCUDA) 
              << "x)" << std::endl;
    
    s.benchmark.hasResults = true;
    std::cout << "==============================\n" << std::endl;
}

static ProcessingType nextProc(ProcessingType p) {
    switch (p) {
        case ProcessingType::SEQUENTIAL: return ProcessingType::PARALLEL;
        case ProcessingType::PARALLEL: return ProcessingType::MULTITHREAD;
        case ProcessingType::MULTITHREAD: return ProcessingType::CUDA;
        case ProcessingType::CUDA: return ProcessingType::SEQUENTIAL;
    }
    return ProcessingType::SEQUENTIAL;
}

int main(int argc, char** argv) {
    std::string imgPath;
    int cameraId = -1;
    
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if ((a == "--image" || a == "-i") && i + 1 < argc) {
            imgPath = argv[++i];
        } else if ((a == "--camera" || a == "-c") && i + 1 < argc) {
            cameraId = std::stoi(argv[++i]);
        }
    }

    ImageProcessor proc;
    State state;
    cv::Mat original;
    
    // Inicializar com argumento de linha de comando
    if (!imgPath.empty()) {
        if (proc.loadImage(imgPath)) {
            original = proc.getOriginalImage();
            state.usingCamera = false;
        } else {
            std::cerr << "Falha ao carregar imagem: " << imgPath << "\n";
        }
    } else if (cameraId >= 0) {
        state.camera.open(cameraId);
        if (state.camera.isOpened()) {
            state.usingCamera = true;
            state.camera >> original;
        } else {
            std::cerr << "Falha ao abrir camera " << cameraId << "\n";
        }
    }

    cv::namedWindow("PAVIC LAB 2025", cv::WINDOW_AUTOSIZE);

    if (original.empty()) {
        // Gerar uma imagem de placeholder
        original = cv::Mat(400, 500, CV_8UC3, cv::Scalar(50, 50, 50));
        drawText(original, "Pressione 'O' para abrir", {100, 180}, 0.7, {200,200,200}, 2, false);
        drawText(original, "[1] Imagem   [2] Webcam", {120, 230}, 0.6, {150,150,150}, 1, false);
    }

    // Inicializar FPS
    state.fpsStartTime = std::chrono::steady_clock::now();
    
    while (true) {
        // Se usando câmera, capturar frame continuamente
        if (state.usingCamera && state.camera.isOpened()) {
            state.camera >> original;
            if (!original.empty()) {
                proc.loadImage(original);
                state.last = proc.applyFilter(state.filter, state.proc);
                
                // Calcular FPS
                state.frameCount++;
                auto now = std::chrono::steady_clock::now();
                double elapsed = std::chrono::duration<double>(now - state.fpsStartTime).count();
                if (elapsed >= 1.0) {
                    state.fps = state.frameCount / elapsed;
                    state.frameCount = 0;
                    state.fpsStartTime = now;
                }
            }
        }
        
        drawSideBySide(original, state.last.image, state);
        int key = cv::waitKey(state.usingCamera ? 1 : 30);
        if (key < 0) continue;

        if (key == 'q' || key == 'Q' || key == 27) break;

        if (key == 'm' || key == 'M') {
            state.proc = nextProc(state.proc);
            state.benchmark.hasResults = false; // Reset benchmark ao mudar modo
        } else if (key == 'c' || key == 'C') {
            // Benchmark comparativo
            if (!proc.getOriginalImage().empty()) {
                runComparativeBenchmark(proc, state);
            }
        } else if (key == 's' || key == 'S') {
            if (state.last.success) {
                std::string out = "output_" + ImageProcessor::getFilterName(state.filter) + "_" + ImageProcessor::getProcessingName(state.proc) + ".png";
                cv::imwrite(out, state.last.image);
                std::cout << "Salvo: " << out << "\n";
            }
        } else if (key == 'o' || key == 'O') {
            int choice = showSourceMenu();
            
            if (choice == 1) {
                // Abrir imagem via diálogo
                std::string path = openFileDialog();
                if (!path.empty() && proc.loadImage(path)) {
                    if (state.usingCamera) {
                        state.camera.release();
                        state.usingCamera = false;
                    }
                    original = proc.getOriginalImage();
                    state.last = ProcessingResult{};
                    state.benchmark.hasResults = false; // Reset benchmark
                    std::cout << "Imagem carregada: " << path << "\n";
                } else if (!path.empty()) {
                    std::cout << "Falha ao abrir: " << path << "\n";
                }
            } else if (choice == 2) {
                // Abrir câmera
                if (state.usingCamera) {
                    state.camera.release();
                }
                
                for (int cam = 0; cam < 5; ++cam) {
                    state.camera.open(cam);
                    if (state.camera.isOpened()) {
                        state.usingCamera = true;
                        state.camera >> original;
                        state.frameCount = 0;
                        state.fps = 0;
                        state.fpsStartTime = std::chrono::steady_clock::now();
                        state.benchmark.hasResults = false;
                        std::cout << "Camera " << cam << " aberta!\n";
                        break;
                    }
                }
                
                if (!state.usingCamera) {
                    std::cout << "Nenhuma camera encontrada!\n";
                }
            }
        } else {
            FilterType maybe = keyToFilter(key, state.filter);
            if (maybe != state.filter) {
                state.filter = maybe;
                state.benchmark.hasResults = false; // Reset benchmark ao mudar filtro
            }
            if (!proc.getOriginalImage().empty() && !state.usingCamera) {
                state.last = proc.applyFilter(state.filter, state.proc);
            }
        }
    }
    
    // Liberar câmera ao sair
    if (state.camera.isOpened()) {
        state.camera.release();
    }

    return 0;
}
