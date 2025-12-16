#ifndef PERFORMANCE_METRICS_H
#define PERFORMANCE_METRICS_H

#include <chrono>
#include <string>
#include <vector>
#include <map>
#include "ImageProcessor.h"

namespace pavic {

// Estrutura para armazenar métricas
struct Metric {
    FilterType filter;
    ProcessingType processing;
    double executionTimeMs;
    int imageWidth;
    int imageHeight;
    std::chrono::system_clock::time_point timestamp;
};

// Estrutura para comparação
struct ComparisonResult {
    FilterType filter;
    std::map<ProcessingType, double> times;
    ProcessingType fastest;
    double speedupVsSequential;
};

class PerformanceMetrics {
public:
    PerformanceMetrics();
    ~PerformanceMetrics();

    // Timer
    void startTimer();
    double stopTimer();
    double getElapsedMs() const;

    // Registro de métricas
    void recordMetric(FilterType filter, ProcessingType processing, 
                     double timeMs, int width, int height);
    void clearMetrics();

    // Análise
    double getAverageTime(FilterType filter, ProcessingType processing) const;
    double getMinTime(FilterType filter, ProcessingType processing) const;
    double getMaxTime(FilterType filter, ProcessingType processing) const;
    
    // Comparação
    ComparisonResult compareProcessingTypes(FilterType filter) const;
    std::vector<ComparisonResult> getAllComparisons() const;

    // Relatório
    std::string generateReport() const;
    void exportToCSV(const std::string& filename) const;

    // Acesso aos dados
    const std::vector<Metric>& getAllMetrics() const;

private:
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point endTime;
    std::vector<Metric> metrics;

    std::vector<Metric> filterMetrics(FilterType filter, ProcessingType processing) const;
};

} // namespace pavic

#endif // PERFORMANCE_METRICS_H
