/**
 * PAVIC LAB 2025 - PerformanceMetrics
 */

#include "PerformanceMetrics.h"
#include <fstream>
#include <limits>
#include <sstream>
#include <cstdio>
#include <cerrno>

namespace pavic {

PerformanceMetrics::PerformanceMetrics() {}
PerformanceMetrics::~PerformanceMetrics() {}

void PerformanceMetrics::startTimer() {
    startTime = std::chrono::high_resolution_clock::now();
}

double PerformanceMetrics::stopTimer() {
    endTime = std::chrono::high_resolution_clock::now();
    return getElapsedMs();
}

double PerformanceMetrics::getElapsedMs() const {
    return std::chrono::duration<double, std::milli>(endTime - startTime).count();
}

void PerformanceMetrics::recordMetric(FilterType filter, ProcessingType processing,
                                      double timeMs, int width, int height) {
    Metric m{filter, processing, timeMs, width, height, std::chrono::system_clock::now()};
    metrics.push_back(m);
}

void PerformanceMetrics::clearMetrics() { metrics.clear(); }

std::vector<Metric> PerformanceMetrics::filterMetrics(FilterType filter, ProcessingType processing) const {
    std::vector<Metric> result;
    for (const auto& m : metrics) {
        if (m.filter == filter && m.processing == processing) result.push_back(m);
    }
    return result;
}

double PerformanceMetrics::getAverageTime(FilterType filter, ProcessingType processing) const {
    auto v = filterMetrics(filter, processing);
    if (v.empty()) return 0.0;
    double sum = 0.0;
    for (const auto& m : v) sum += m.executionTimeMs;
    return sum / static_cast<double>(v.size());
}

double PerformanceMetrics::getMinTime(FilterType filter, ProcessingType processing) const {
    auto v = filterMetrics(filter, processing);
    if (v.empty()) return 0.0;
    double mn = std::numeric_limits<double>::max();
    for (const auto& m : v) mn = std::min(mn, m.executionTimeMs);
    return mn;
}

double PerformanceMetrics::getMaxTime(FilterType filter, ProcessingType processing) const {
    auto v = filterMetrics(filter, processing);
    if (v.empty()) return 0.0;
    double mx = 0.0;
    for (const auto& m : v) mx = std::max(mx, m.executionTimeMs);
    return mx;
}

ComparisonResult PerformanceMetrics::compareProcessingTypes(FilterType filter) const {
    ComparisonResult cr{};
    cr.filter = filter;
    // Avaliar apenas tipos presentes
    for (auto pt : {ProcessingType::SEQUENTIAL, ProcessingType::PARALLEL, ProcessingType::MULTITHREAD, ProcessingType::CUDA}) {
        double avg = getAverageTime(filter, pt);
        if (avg > 0.0) cr.times[pt] = avg;
    }
    // Mais rápido
    double best = std::numeric_limits<double>::max();
    cr.fastest = ProcessingType::SEQUENTIAL;
    for (auto& kv : cr.times) {
        if (kv.second < best) {
            best = kv.second;
            cr.fastest = kv.first;
        }
    }
    // Speedup vs SEQUENTIAL
    double seq = getAverageTime(filter, ProcessingType::SEQUENTIAL);
    cr.speedupVsSequential = (seq > 0.0 && best > 0.0) ? (seq / best) : 0.0;
    return cr;
}

std::vector<ComparisonResult> PerformanceMetrics::getAllComparisons() const {
    std::vector<ComparisonResult> out;
    for (int f = 0; f <= static_cast<int>(FilterType::BILATERAL); ++f) {
        out.push_back(compareProcessingTypes(static_cast<FilterType>(f)));
    }
    return out;
}

std::string PerformanceMetrics::generateReport() const {
    std::ostringstream os;
    os << "PAVIC LAB 2025 - Relatório de Desempenho\n";
    os << "Métricas coletadas: " << metrics.size() << "\n\n";
    for (int f = 0; f <= static_cast<int>(FilterType::BILATERAL); ++f) {
        auto filter = static_cast<FilterType>(f);
        auto cr = compareProcessingTypes(filter);
        os << ImageProcessor::getFilterName(filter) << ":\n";
        for (auto& kv : cr.times) {
            os << "  - " << ImageProcessor::getProcessingName(kv.first) << ": " << kv.second << " ms (média)\n";
        }
        os << "  > Mais rápido: " << ImageProcessor::getProcessingName(cr.fastest)
           << ", speedup vs Sequential: " << cr.speedupVsSequential << "x\n\n";
    }
    return os.str();
}

void PerformanceMetrics::exportToCSV(const std::string& filename) const {
    FILE* file = fopen(filename.c_str(), "w");
    if (!file) {
        std::cerr << "Erro ao abrir arquivo CSV: " << filename << " (errno=" << errno << ")" << std::endl;
        return;
    }
    
    fprintf(file, "timestamp,filter,processing,time_ms,width,height\n");
    
    for (const auto& m : metrics) {
        auto tt = std::chrono::system_clock::to_time_t(m.timestamp);
        fprintf(file, "%lld,%s,%s,%.6f,%d,%d\n",
            static_cast<long long>(tt),
            ImageProcessor::getFilterName(m.filter).c_str(),
            ImageProcessor::getProcessingName(m.processing).c_str(),
            m.executionTimeMs,
            m.imageWidth,
            m.imageHeight);
    }
    
    fflush(file);
    fclose(file);
    
    std::cout << "CSV exportado: " << filename << " (" << metrics.size() << " registros)" << std::endl;
}

const std::vector<Metric>& PerformanceMetrics::getAllMetrics() const { return metrics; }

} // namespace pavic
