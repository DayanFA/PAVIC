/**
 * PAVIC LAB 2025 - Benchmark
 * Executa todos os filtros em todas as abordagens e exporta CSV
 */

#include "ImageProcessor.h"
#include "PerformanceMetrics.h"

#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <direct.h>  // Para _mkdir no Windows

using namespace pavic;

void runBenchmark(const cv::Mat& image, PerformanceMetrics& metrics, int iterations = 5) {
    if (image.empty()) {
        std::cerr << "Imagem vazia para benchmark!\n";
        return;
    }

    ImageProcessor processor;
    
    std::vector<FilterType> filters = {
        FilterType::GRAYSCALE,
        FilterType::BLUR,
        FilterType::GAUSSIAN_BLUR,
        FilterType::SOBEL,
        FilterType::CANNY,
        FilterType::SHARPEN,
        FilterType::EMBOSS,
        FilterType::NEGATIVE,
        FilterType::SEPIA,
        FilterType::THRESHOLD,
        FilterType::MEDIAN,
        FilterType::BILATERAL
    };

    std::vector<ProcessingType> procs = {
        ProcessingType::SEQUENTIAL,
        ProcessingType::PARALLEL,
        ProcessingType::MULTITHREAD
    };

#if PAVIC_HAVE_CUDA
    procs.push_back(ProcessingType::CUDA);
#endif

    std::cout << "\n========================================\n";
    std::cout << "   PAVIC LAB 2025 - BENCHMARK\n";
    std::cout << "   Imagem: " << image.cols << "x" << image.rows << "\n";
    std::cout << "   Iteracoes: " << iterations << "\n";
    std::cout << "========================================\n\n" << std::flush;

    for (const auto& filter : filters) {
        std::cout << "Filtro: " << ImageProcessor::getFilterName(filter) << "\n";
        std::cout << std::string(50, '-') << "\n";

        for (const auto& proc : procs) {
            double totalTime = 0.0;
            bool success = true;

            for (int i = 0; i < iterations; ++i) {
                auto result = processor.processFrame(image, filter, proc);
                if (result.success) {
                    totalTime += result.executionTimeMs;
                    metrics.recordMetric(filter, proc, result.executionTimeMs,
                                        image.cols, image.rows);
                } else {
                    success = false;
                    if (i == 0) {
                        std::cout << "  " << std::setw(15) << std::left 
                                  << ImageProcessor::getProcessingName(proc)
                                  << ": FALHOU - " << result.errorMessage << "\n";
                    }
                    break;
                }
            }

            if (success) {
                double avgTime = totalTime / iterations;
                std::cout << "  " << std::setw(15) << std::left 
                          << ImageProcessor::getProcessingName(proc)
                          << ": " << std::fixed << std::setprecision(3) 
                          << avgTime << " ms (media)\n";
            }
        }
        std::cout << "\n";
    }
}

void printComparisonTable(const PerformanceMetrics& metrics) {
    std::cout << "\n========================================\n";
    std::cout << "   COMPARACAO DE DESEMPENHO\n";
    std::cout << "========================================\n\n";

    auto comparisons = metrics.getAllComparisons();

    std::cout << std::setw(15) << std::left << "Filtro"
              << std::setw(12) << "Sequential"
              << std::setw(12) << "Parallel"
              << std::setw(12) << "Multithread"
#if PAVIC_HAVE_CUDA
              << std::setw(12) << "CUDA"
#endif
              << std::setw(10) << "Speedup"
              << "\n";
    std::cout << std::string(70, '-') << "\n";

    for (const auto& cmp : comparisons) {
        std::cout << std::setw(15) << std::left << ImageProcessor::getFilterName(cmp.filter);

        for (auto pt : {ProcessingType::SEQUENTIAL, ProcessingType::PARALLEL, ProcessingType::MULTITHREAD}) {
            auto it = cmp.times.find(pt);
            if (it != cmp.times.end()) {
                std::cout << std::setw(12) << std::fixed << std::setprecision(2) << it->second;
            } else {
                std::cout << std::setw(12) << "N/A";
            }
        }

#if PAVIC_HAVE_CUDA
        auto itCuda = cmp.times.find(ProcessingType::CUDA);
        if (itCuda != cmp.times.end()) {
            std::cout << std::setw(12) << std::fixed << std::setprecision(2) << itCuda->second;
        } else {
            std::cout << std::setw(12) << "N/A";
        }
#endif

        std::cout << std::setw(10) << std::fixed << std::setprecision(2) 
                  << cmp.speedupVsSequential << "x\n";
    }
}

int main(int argc, char** argv) {
    try {
        std::cerr << "Benchmark iniciando...\n" << std::flush;
        std::string imgPath;
        std::string outputCSV = "results/benchmark_results.csv";
        int iterations = 5;

        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if ((arg == "--image" || arg == "-i") && i + 1 < argc) {
                imgPath = argv[++i];
            } else if ((arg == "--output" || arg == "-o") && i + 1 < argc) {
                outputCSV = argv[++i];
            } else if ((arg == "--iterations" || arg == "-n") && i + 1 < argc) {
                iterations = std::stoi(argv[++i]);
            } else if (arg == "--help" || arg == "-h") {
                std::cout << "Uso: Benchmark [opcoes]\n"
                          << "  -i, --image <path>       Caminho da imagem\n"
                          << "  -o, --output <path>      Arquivo CSV de saida\n"
                          << "  -n, --iterations <num>   Numero de iteracoes\n"
                          << "  -h, --help               Mostrar ajuda\n";
                return 0;
            }
        }

        cv::Mat image;
        std::cerr << "Preparando imagem...\n" << std::flush;
        if (imgPath.empty()) {
            // Criar imagem de teste Full HD para benchmark realista
            std::cout << "Nenhuma imagem fornecida. Usando imagem de teste 1920x1080 (Full HD).\n" << std::flush;
            image = cv::Mat(1080, 1920, CV_8UC3);
            cv::randu(image, cv::Scalar(0, 0, 0), cv::Scalar(255, 255, 255));
        } else {
            image = cv::imread(imgPath);
            if (image.empty()) {
                std::cerr << "Erro ao carregar imagem: " << imgPath << "\n";
                return 1;
            }
        }
        std::cerr << "Imagem pronta: " << image.cols << "x" << image.rows << "\n" << std::flush;

        PerformanceMetrics metrics;

        runBenchmark(image, metrics, iterations);
        printComparisonTable(metrics);

        // Criar diretorio results se nao existir
        std::cerr << "Preparando para salvar CSV...\n" << std::flush;
        size_t lastSlash = outputCSV.find_last_of("/\\");
        if (lastSlash != std::string::npos && lastSlash > 0) {
            std::string dir = outputCSV.substr(0, lastSlash);
            int mkresult = _mkdir(dir.c_str());
            std::cerr << "_mkdir(" << dir << ") = " << mkresult << "\n" << std::flush;
        }

        std::cerr << "Exportando CSV para: " << outputCSV << "\n" << std::flush;
        metrics.exportToCSV(outputCSV);
        std::cout << "\nResultados exportados para: " << outputCSV << "\n";

        // Gerar relatorio completo
        std::cout << "\n" << metrics.generateReport();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ERRO: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "ERRO desconhecido" << std::endl;
        return 1;
    }
}
