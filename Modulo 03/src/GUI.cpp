/**
 * PAVIC LAB 2025 - GUI (OpenCV-based)
 * Renderiza botões e imagens lado a lado, aplica filtros e mostra tempos.
 */

#include "GUI.h"
#include "WebcamCapture.h"
#include "PerformanceMetrics.h"

#include <opencv2/opencv.hpp>
#include <iostream>

namespace pavic {

static cv::Scalar colBG(30,30,30), colPanel(50,50,50), colText(230,230,230);

GUI::GUI(const std::string& windowName, int width, int height)
    : windowName(windowName), windowWidth(width), windowHeight(height), isRunning(false), useWebcam(false),
      currentFilter(FilterType::GRAYSCALE), currentProcessing(ProcessingType::SEQUENTIAL), lastExecutionTime(0.0) {}

GUI::~GUI() {}

void GUI::init() {
    canvas = cv::Mat(windowHeight, windowWidth, CV_8UC3, colBG);
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, windowWidth, windowHeight);
    cv::setMouseCallback(windowName, GUI::mouseCallback, this);
    createFilterButtons();
    createProcessingButtons();
    createControlButtons();
}

void GUI::run() {
    isRunning = true;
    drawInterface();
    while (isRunning) {
        if (useWebcam && webcam.isRunning() && webcam.hasNewFrame()) {
            applyCurrentFilter();
        }
        cv::imshow(windowName, canvas);
        int k = cv::waitKey(20);
        if (k == 27 || k == 'q') isRunning = false;
    }
    if (webcam.isRunning()) webcam.stop();
}

void GUI::mouseCallback(int event, int x, int y, int flags, void* userdata) {
    auto* gui = static_cast<GUI*>(userdata);
    if (!gui) return;
    gui->handleMouse(event, x, y, flags);
}

void GUI::handleMouse(int event, int x, int y, int flags) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        auto clickBtn = [&](std::vector<Button>& list){
            for (auto& b : list) {
                if (b.rect.contains(cv::Point(x,y))) { if (b.onClick) b.onClick(); break; }
            }
        };
        clickBtn(filterButtons); clickBtn(processingButtons); clickBtn(controlButtons);
    }
}

void GUI::drawInterface() {
    canvas.setTo(colBG);
    // Painéis
    cv::rectangle(canvas, cv::Rect(0,0,windowWidth,80), colPanel, cv::FILLED);
    cv::putText(canvas, "PAVIC LAB 2025", {20,50}, cv::FONT_HERSHEY_SIMPLEX, 1.0, colText, 2);

    for (auto& b : filterButtons) drawButton(b);
    for (auto& b : processingButtons) drawButton(b);
    for (auto& b : controlButtons) drawButton(b);

    // Imagens
    cv::Mat left = processor.getOriginalImage();
    cv::Mat right = processor.getProcessedImage();
    drawImage(left, cv::Rect(20, 100, (windowWidth-60)/2, windowHeight-140), "Original");
    drawImage(right, cv::Rect(40 + (windowWidth-60)/2, 100, (windowWidth-60)/2, windowHeight-140), "Processada");

    drawMetrics();
}

void GUI::drawImage(const cv::Mat& image, const cv::Rect& area, const std::string& title) {
    cv::rectangle(canvas, area, cv::Scalar(70,70,70), cv::FILLED);
    cv::putText(canvas, title, {area.x+10, area.y+30}, cv::FONT_HERSHEY_SIMPLEX, 0.8, colText, 2);
    if (image.empty()) return;
    double scale = std::min(area.width / static_cast<double>(image.cols), area.height / static_cast<double>(image.rows));
    cv::Mat resized; cv::resize(image, resized, cv::Size(), scale, scale, cv::INTER_AREA);
    cv::Rect dst(area.x + (area.width - resized.cols)/2, area.y + (area.height - resized.rows)/2, resized.cols, resized.rows);
    resized.copyTo(canvas(dst));
}

void GUI::drawButton(Button& button) {
    cv::Scalar color = button.normalColor;
    cv::rectangle(canvas, button.rect, color, cv::FILLED);
    cv::rectangle(canvas, button.rect, cv::Scalar(100,100,100), 1);
    int base = 12;
    cv::putText(canvas, button.label, {button.rect.x+10, button.rect.y+button.rect.height/2+base}, cv::FONT_HERSHEY_SIMPLEX, 0.6, colText, 2);
}

void GUI::drawMetrics() {
    std::string info = "Filtro: " + ImageProcessor::getFilterName(currentFilter) +
                       " | Proc: " + ImageProcessor::getProcessingName(currentProcessing);
    cv::putText(canvas, info, {20, 80}, cv::FONT_HERSHEY_SIMPLEX, 0.7, colText, 2);
}

void GUI::drawComparisonChart() {
    // Placeholder: implementar gráfico posteriormente
}

Button GUI::createButton(int x, int y, int width, int height, const std::string& label,
                         std::function<void()> onClick) {
    Button b;
    b.rect = cv::Rect(x,y,width,height);
    b.label = label;
    b.normalColor = cv::Scalar(60,60,60);
    b.hoverColor = cv::Scalar(80,80,80);
    b.pressedColor = cv::Scalar(100,100,100);
    b.isHovered = false; b.isPressed = false;
    b.onClick = onClick;
    return b;
}

void GUI::createFilterButtons() {
    filterButtons.clear();
    int x = 20, y = 10, w = 120, h = 40, gap = 10;
    auto add = [&](const std::string& label, FilterType ft){
        filterButtons.push_back(createButton(x, y, w, h, label, [this, ft]{ currentFilter = ft; applyCurrentFilter(); drawInterface(); }));
        x += w + gap;
    };
    add("Grayscale", FilterType::GRAYSCALE);
    add("Blur", FilterType::BLUR);
    add("Gaussian", FilterType::GAUSSIAN_BLUR);
    add("Sobel", FilterType::SOBEL);
    add("Canny", FilterType::CANNY);
    add("Sharpen", FilterType::SHARPEN);
    add("Emboss", FilterType::EMBOSS);
    add("Negative", FilterType::NEGATIVE);
    add("Sepia", FilterType::SEPIA);
    add("Threshold", FilterType::THRESHOLD);
    add("Bilateral", FilterType::BILATERAL);
}

void GUI::createProcessingButtons() {
    processingButtons.clear();
    int x = 20, y = 55, w = 150, h = 40, gap = 10;
    auto add = [&](const std::string& label, ProcessingType pt){
        processingButtons.push_back(createButton(x, y, w, h, label, [this, pt]{ currentProcessing = pt; applyCurrentFilter(); drawInterface(); }));
        x += w + gap;
    };
    add("Sequential", ProcessingType::SEQUENTIAL);
    add("Parallel", ProcessingType::PARALLEL);
    add("Multithread", ProcessingType::MULTITHREAD);
    add("CUDA", ProcessingType::CUDA);
}

void GUI::createControlButtons() {
    controlButtons.clear();
    int x = windowWidth - 3*130 - 40, y = 10, w = 120, h = 40, gap = 10;
    controlButtons.push_back(createButton(x, y, w, h, "Open", [this]{ loadImageAction(); })); x += w + gap;
    controlButtons.push_back(createButton(x, y, w, h, "Save", [this]{ saveImageAction(); })); x += w + gap;
    controlButtons.push_back(createButton(x, y, w, h, "Webcam", [this]{ toggleWebcam(); }));
}

void GUI::loadImageAction() {
    std::cout << "Caminho da imagem: ";
    std::string path; std::getline(std::cin, path);
    if (!path.empty() && processor.loadImage(path)) { applyCurrentFilter(); drawInterface(); }
    else { std::cout << "Falha ao abrir: " << path << "\n"; }
}

void GUI::saveImageAction() {
    cv::Mat img = processor.getProcessedImage();
    if (img.empty()) return;
    std::string out = "output_" + ImageProcessor::getFilterName(currentFilter) + "_" + ImageProcessor::getProcessingName(currentProcessing) + ".png";
    processor.saveImage(out, img);
    std::cout << "Salvo: " << out << "\n";
}

void GUI::toggleWebcam() {
    if (!useWebcam) {
        useWebcam = true;
        if (!webcam.start()) { useWebcam = false; std::cout << "Webcam não abriu.\n"; return; }
    } else {
        useWebcam = false; webcam.stop();
    }
}

void GUI::applyCurrentFilter() {
    cv::Mat src;
    if (useWebcam && webcam.isRunning()) {
        src = webcam.getFrame();
    } else {
        src = processor.getOriginalImage();
        if (src.empty()) return;
    }
    auto start = std::chrono::high_resolution_clock::now();
    ProcessingResult r = processor.processFrame(src, currentFilter, currentProcessing);
    auto end = std::chrono::high_resolution_clock::now();
    lastExecutionTime = std::chrono::duration<double, std::milli>(end - start).count();
    if (r.success) {
        // Atualiza imagem processada para visualização
        // Nota: processor guarda processed internamente ao usar applyFilter em imagem carregada;
        // aqui, apenas exibimos o resultado do frame.
        cv::Mat displayed = r.image;
        // Copiar para o canvas na próxima drawInterface()
        processor.saveImage("_temp.png", displayed); // opcional para debug
    }
}

void GUI::runBenchmark() {
    // Implementar numa etapa posterior: iterar sobre filtros e modos e medir tempos.
}

void GUI::clearResults() { results.clear(); }

cv::Mat GUI::resizeToFit(const cv::Mat& image, const cv::Size& maxSize) {
    if (image.empty()) return image;
    double scale = std::min(maxSize.width / static_cast<double>(image.cols), maxSize.height / static_cast<double>(image.rows));
    cv::Mat out; cv::resize(image, out, cv::Size(), scale, scale, cv::INTER_AREA); return out;
}

} // namespace pavic
