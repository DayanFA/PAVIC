# PAVIC LAB 2025

Aplicativo de Processamento de Imagens com comparação de desempenho entre diferentes abordagens: **Sequencial**, **Paralelo (OpenMP)**, **Multithread (std::thread)** e **CUDA (GPU)**.

## 🎯 Funcionalidades

- ✅ Exibição de imagens com filtros aplicados lado a lado
- ✅ 12 filtros de processamento de imagens
- ✅ 3 modos de processamento funcionais (Sequential, Parallel/OpenMP, Multithread)
- ⏳ CUDA (requer NVIDIA CUDA Toolkit)
- ✅ Medição de tempo de execução em tempo real
- ✅ Benchmark completo com exportação CSV
- ✅ Interface gráfica com OpenCV HighGUI

## 📋 Filtros Disponíveis

| Tecla | Filtro | Descrição |
|-------|--------|-----------|
| 1 | Grayscale | Conversão para escala de cinza |
| 2 | Blur | Blur de caixa (média) |
| 3 | Gaussian Blur | Blur gaussiano |
| 4 | Sobel | Detecção de bordas Sobel |
| 5 | Canny | Detecção de bordas Canny |
| 6 | Sharpen | Nitidez |
| 7 | Emboss | Efeito relevo |
| 8 | Negative | Negativo da imagem |
| 9 | Sepia | Efeito sépia |
| 0 | Threshold | Limiarização binária |
| b | Bilateral | Filtro bilateral |

## 🎮 Teclas de Atalho (GUI)

| Tecla | Ação |
|-------|------|
| 1-9, 0, b | Seleciona filtro |
| m | Alterna modo (Sequential → Parallel → Multithread → CUDA) |
| s | Salva imagem processada |
| o | Abre nova imagem |
| q / ESC | Sair |

## 🔧 Requisitos

- **CMake** >= 3.18
- **OpenCV** >= 4.5 (instalado: 4.12.0)
- **OpenMP** (incluso em GCC/MSVC)
- **CUDA Toolkit** (opcional, para processamento GPU)

### Windows (MSYS2 MinGW64)

```bash
# Instalar dependências
pacman -S mingw-w64-x86_64-opencv mingw-w64-x86_64-cmake
```

### Ubuntu/Debian

```bash
sudo apt install cmake libopencv-dev
```

## 🚀 Compilação

### Windows (MSYS2)

```bash
# Configurar
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DOpenCV_DIR=C:/msys64/mingw64/lib/cmake/opencv4

# Compilar
cmake --build build --config Release --parallel
```

### Linux

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## 📖 Uso

### Aplicativo Principal

```bash
# Sem argumentos (placeholder)
./build/PAVIC_LAB_2025

# Com imagem
./build/PAVIC_LAB_2025 --image assets/sample.jpg

# Com webcam
./build/PAVIC_LAB_2025 --webcam
```

### Controles

| Tecla | Ação |
|-------|------|
| 1-9, 0, b, n | Selecionar filtro |
| m | Alternar modo de processamento |
| w | Ativar/desativar webcam |
| s | Salvar imagem filtrada |
| o | Abrir nova imagem |
| r | Rodar benchmark |
| q / ESC | Sair |

### Benchmark

```bash
# Benchmark com imagem de teste gerada automaticamente
build\Benchmark.exe -n 5 -o results/benchmark.csv

# Benchmark com imagem específica
build\Benchmark.exe -i assets/test_image.png -n 10 -o results/benchmark.csv
```

## 📊 Resultados de Benchmark (exemplo real)

| Filtro | Sequential | Parallel(OpenMP) | Multithread | Speedup |
|--------|------------|------------------|-------------|---------|
| Grayscale | 0.46ms | 1.62ms | 2.65ms | 1.00x |
| Blur | 9.64ms | 1.98ms | 4.81ms | 4.88x |
| GaussianBlur | 10.58ms | 2.20ms | 5.64ms | 4.80x |
| Sobel | 11.41ms | 2.75ms | 17.35ms | 4.16x |
| Sharpen | 7.47ms | 1.30ms | 2.74ms | 5.76x |
| Bilateral | 518.87ms | 61.03ms | 66.20ms | 8.50x |
| Median | 314.47ms | 115.66ms | 49.67ms | 6.33x |

*Valores medidos em imagem 640x480, MinGW64/Windows*

```
PAVIC_LAB_2025/
├── CMakeLists.txt              # Build config
├── PAVIC_GUI_2024.sln          # Solution VS
├── PAVIC_GUI_2024.vcxproj      # Projeto VS
├── README.md                   # Documentação
├── assets/
│   └── sample.png              # Imagem de teste
├── docs/
│   ├── APRESENTACAO_PAVIC_LAB_2025.pptx  # Slides
│   └── RELATORIO_PAVIC_LAB_2025.pdf       # Relatório
├── forms/
│   ├── Main.cpp                # Entry point
│   ├── MainForm.h              # Windows Forms GUI
│   └── NativeProcessor.h       # Wrapper C++/CLI
├── include/
│   ├── CUDAFilter.h
│   ├── FilterUtils.h
│   ├── GUI.h
│   ├── ImageProcessor.h
│   ├── MultithreadFilter.h
│   ├── ParallelFilter.h
│   ├── PerformanceMetrics.h
│   ├── SequentialFilter.h
│   └── WebcamCapture.h
└── src/
    ├── Benchmark.cpp           # Benchmark automático
    ├── CUDAFilter.cpp/.cu      # Filtros CUDA
    ├── FilterUtils.cpp
    ├── GUI.cpp
    ├── ImageProcessor.cpp
    ├── main.cpp                # App principal
    ├── MultithreadFilter.cpp
    ├── ParallelFilter.cpp
    ├── PerformanceMetrics.cpp
    ├── SequentialFilter.cpp
    └── WebcamCapture.cpp
```

## 📈 Resultados Esperados

| Filtro | Sequential | Parallel | Multithread | CUDA | Speedup |
|--------|------------|----------|-------------|------|---------|
| Grayscale | ~15ms | ~4ms | ~5ms | ~1ms | 15x |
| Gaussian | ~80ms | ~20ms | ~25ms | ~3ms | 27x |
| Sobel | ~120ms | ~30ms | ~35ms | ~5ms | 24x |
| Bilateral | ~500ms | ~125ms | ~140ms | ~15ms | 33x |

*Valores de referência para imagem 1920x1080 em hardware típico*

## 👥 Equipe

- Dayan Freitas Alves

## 📝 Licença

Este projeto foi desenvolvido para fins educacionais no contexto do PAVIC LAB 2025.
