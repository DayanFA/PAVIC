#ifndef GUI_H
#define GUI_H

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <functional>
#include "ImageProcessor.h"
#include "WebcamCapture.h"

namespace pavic {

// Estrutura para botão
struct Button {
    cv::Rect rect;
    std::string label;
    cv::Scalar normalColor;
    cv::Scalar hoverColor;
    cv::Scalar pressedColor;
    bool isHovered;
    bool isPressed;
    std::function<void()> onClick;
};

// Classe GUI
class GUI {
public:
    GUI(const std::string& windowName, int width = 1400, int height = 800);
    ~GUI();

    // Inicialização
    void init();
    void run();

    // Callbacks
    static void mouseCallback(int event, int x, int y, int flags, void* userdata);
    void handleMouse(int event, int x, int y, int flags);

private:
    // Janela
    std::string windowName;
    int windowWidth;
    int windowHeight;
    cv::Mat canvas;

    // Componentes
    ImageProcessor processor;
    std::vector<Button> filterButtons;
    std::vector<Button> processingButtons;
    std::vector<Button> controlButtons;

    // Estado
    bool isRunning;
    bool useWebcam;
    WebcamCapture webcam;
    FilterType currentFilter;
    ProcessingType currentProcessing;
    
    // Métricas
    std::vector<ProcessingResult> results;
    double lastExecutionTime;

    // Métodos de desenho
    void drawInterface();
    void drawImage(const cv::Mat& image, const cv::Rect& area, const std::string& title);
    void drawButton(Button& button);
    void drawMetrics();
    void drawComparisonChart();

    // Criação de botões
    void createFilterButtons();
    void createProcessingButtons();
    void createControlButtons();
    Button createButton(int x, int y, int width, int height, const std::string& label,
                        std::function<void()> onClick);

    // Ações
    void loadImageAction();
    void saveImageAction();
    void toggleWebcam();
    void applyCurrentFilter();
    void runBenchmark();
    void clearResults();

    // Utilitários
    cv::Mat resizeToFit(const cv::Mat& image, const cv::Size& maxSize);
};

} // namespace pavic

#endif // GUI_H
