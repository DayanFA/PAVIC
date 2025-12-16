/**
 * PAVIC LAB 2025 - Windows Forms GUI
 * Interface gráfica para comparação de processamento de imagens
 * CPU (Single/Multi-thread) vs CUDA GPU
 */

#pragma once

#include <msclr/marshal_cppstd.h>
#include <chrono>
#include <vector>
#include <string>

namespace PAVIC_LAB_2025 {

    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections;
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing;
    using namespace System::Drawing::Imaging;
    using namespace System::Threading;
    using namespace System::Diagnostics;

    public ref class MainForm : public System::Windows::Forms::Form
    {
    public:
        MainForm(void)
        {
            InitializeComponent();
            InitializeCustomComponents();
        }

    protected:
        ~MainForm()
        {
            if (components)
            {
                delete components;
            }
            StopCamera();
        }

    private:
        // Componentes da interface
        System::ComponentModel::Container ^components;
        
        // Painéis principais
        Panel^ panelToolbar;
        Panel^ panelImages;
        Panel^ panelResults;
        Panel^ panelStatus;
        
        // Picture Boxes para imagens
        PictureBox^ picOriginal;
        PictureBox^ picProcessed;
        
        // Labels
        Label^ lblOriginal;
        Label^ lblProcessed;
        Label^ lblStatus;
        Label^ lblFPS;
        Label^ lblFilter;
        
        // Botões de controle
        Button^ btnLoadImage;
        Button^ btnStartCamera;
        Button^ btnStopCamera;
        Button^ btnSaveImage;
        Button^ btnBenchmark;
        
        // Combo boxes
        ComboBox^ cmbFilter;
        ComboBox^ cmbProcessingMode;
        
        // Labels de resultados comparativos
        GroupBox^ grpResults;
        Label^ lblTimeSequential;
        Label^ lblTimeParallel;
        Label^ lblTimeMultithread;
        Label^ lblTimeCUDA;
        Label^ lblSpeedupParallel;
        Label^ lblSpeedupMultithread;
        Label^ lblSpeedupCUDA;
        
        // Progress bar
        ProgressBar^ progressBar;
        
        // Timer para câmera
        System::Windows::Forms::Timer^ timerCamera;
        System::Windows::Forms::Timer^ timerFPS;
        
        // Variáveis de estado
        Bitmap^ originalImage;
        Bitmap^ processedImage;
        bool cameraRunning;
        int frameCount;
        double currentFPS;
        System::Diagnostics::Stopwatch^ fpsStopwatch;
        
        // OpenCV VideoCapture (gerenciado via IntPtr)
        IntPtr cameraCapture;

#pragma region Windows Form Designer generated code
        void InitializeComponent(void)
        {
            this->SuspendLayout();
            
            // Configuração do Form
            this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(1400, 800);
            this->Name = L"MainForm";
            this->Text = L"PAVIC LAB 2025 - Processamento de Imagens GPU vs CPU";
            this->StartPosition = FormStartPosition::CenterScreen;
            this->BackColor = Color::FromArgb(45, 45, 48);
            this->ForeColor = Color::White;
            
            this->ResumeLayout(false);
        }
#pragma endregion

        void InitializeCustomComponents()
        {
            // Inicializar variáveis
            cameraRunning = false;
            frameCount = 0;
            currentFPS = 0;
            fpsStopwatch = gcnew System::Diagnostics::Stopwatch();
            
            // === TOOLBAR PANEL ===
            panelToolbar = gcnew Panel();
            panelToolbar->Dock = DockStyle::Top;
            panelToolbar->Height = 80;
            panelToolbar->BackColor = Color::FromArgb(30, 30, 30);
            panelToolbar->Padding = Padding(10);
            
            // Botão Carregar Imagem
            btnLoadImage = gcnew Button();
            btnLoadImage->Text = L"📂 Carregar Imagem";
            btnLoadImage->Location = Point(10, 20);
            btnLoadImage->Size = Drawing::Size(140, 40);
            btnLoadImage->FlatStyle = FlatStyle::Flat;
            btnLoadImage->BackColor = Color::FromArgb(0, 122, 204);
            btnLoadImage->ForeColor = Color::White;
            btnLoadImage->Font = gcnew Drawing::Font("Segoe UI", 9, FontStyle::Bold);
            btnLoadImage->Click += gcnew EventHandler(this, &MainForm::btnLoadImage_Click);
            panelToolbar->Controls->Add(btnLoadImage);
            
            // Botão Iniciar Câmera
            btnStartCamera = gcnew Button();
            btnStartCamera->Text = L"📷 Iniciar Câmera";
            btnStartCamera->Location = Point(160, 20);
            btnStartCamera->Size = Drawing::Size(140, 40);
            btnStartCamera->FlatStyle = FlatStyle::Flat;
            btnStartCamera->BackColor = Color::FromArgb(0, 150, 80);
            btnStartCamera->ForeColor = Color::White;
            btnStartCamera->Font = gcnew Drawing::Font("Segoe UI", 9, FontStyle::Bold);
            btnStartCamera->Click += gcnew EventHandler(this, &MainForm::btnStartCamera_Click);
            panelToolbar->Controls->Add(btnStartCamera);
            
            // Botão Parar Câmera
            btnStopCamera = gcnew Button();
            btnStopCamera->Text = L"⏹ Parar Câmera";
            btnStopCamera->Location = Point(310, 20);
            btnStopCamera->Size = Drawing::Size(130, 40);
            btnStopCamera->FlatStyle = FlatStyle::Flat;
            btnStopCamera->BackColor = Color::FromArgb(180, 50, 50);
            btnStopCamera->ForeColor = Color::White;
            btnStopCamera->Font = gcnew Drawing::Font("Segoe UI", 9, FontStyle::Bold);
            btnStopCamera->Enabled = false;
            btnStopCamera->Click += gcnew EventHandler(this, &MainForm::btnStopCamera_Click);
            panelToolbar->Controls->Add(btnStopCamera);
            
            // Label Filtro
            Label^ lblFilterText = gcnew Label();
            lblFilterText->Text = L"Filtro:";
            lblFilterText->Location = Point(460, 28);
            lblFilterText->AutoSize = true;
            lblFilterText->Font = gcnew Drawing::Font("Segoe UI", 10);
            panelToolbar->Controls->Add(lblFilterText);
            
            // ComboBox Filtro
            cmbFilter = gcnew ComboBox();
            cmbFilter->Location = Point(510, 25);
            cmbFilter->Size = Drawing::Size(150, 30);
            cmbFilter->DropDownStyle = ComboBoxStyle::DropDownList;
            cmbFilter->Items->AddRange(gcnew cli::array<Object^> {
                L"Grayscale", L"Blur", L"Gaussian Blur", L"Sobel",
                L"Canny", L"Sharpen", L"Emboss", L"Negative",
                L"Sepia", L"Threshold", L"Bilateral"
            });
            cmbFilter->SelectedIndex = 0;
            cmbFilter->SelectedIndexChanged += gcnew EventHandler(this, &MainForm::cmbFilter_SelectedIndexChanged);
            panelToolbar->Controls->Add(cmbFilter);
            
            // Label Modo
            Label^ lblModeText = gcnew Label();
            lblModeText->Text = L"Modo:";
            lblModeText->Location = Point(680, 28);
            lblModeText->AutoSize = true;
            lblModeText->Font = gcnew Drawing::Font("Segoe UI", 10);
            panelToolbar->Controls->Add(lblModeText);
            
            // ComboBox Modo de Processamento
            cmbProcessingMode = gcnew ComboBox();
            cmbProcessingMode->Location = Point(735, 25);
            cmbProcessingMode->Size = Drawing::Size(130, 30);
            cmbProcessingMode->DropDownStyle = ComboBoxStyle::DropDownList;
            cmbProcessingMode->Items->AddRange(gcnew cli::array<Object^> {
                L"Sequential", L"Parallel", L"Multithread", L"CUDA"
            });
            cmbProcessingMode->SelectedIndex = 3; // CUDA por padrão
            cmbProcessingMode->SelectedIndexChanged += gcnew EventHandler(this, &MainForm::cmbProcessingMode_SelectedIndexChanged);
            panelToolbar->Controls->Add(cmbProcessingMode);
            
            // Botão Benchmark Comparativo
            btnBenchmark = gcnew Button();
            btnBenchmark->Text = L"⚡ BENCHMARK COMPARATIVO";
            btnBenchmark->Location = Point(890, 20);
            btnBenchmark->Size = Drawing::Size(220, 40);
            btnBenchmark->FlatStyle = FlatStyle::Flat;
            btnBenchmark->BackColor = Color::FromArgb(150, 100, 0);
            btnBenchmark->ForeColor = Color::White;
            btnBenchmark->Font = gcnew Drawing::Font("Segoe UI", 9, FontStyle::Bold);
            btnBenchmark->Click += gcnew EventHandler(this, &MainForm::btnBenchmark_Click);
            panelToolbar->Controls->Add(btnBenchmark);
            
            // Botão Salvar
            btnSaveImage = gcnew Button();
            btnSaveImage->Text = L"💾 Salvar";
            btnSaveImage->Location = Point(1130, 20);
            btnSaveImage->Size = Drawing::Size(100, 40);
            btnSaveImage->FlatStyle = FlatStyle::Flat;
            btnSaveImage->BackColor = Color::FromArgb(80, 80, 80);
            btnSaveImage->ForeColor = Color::White;
            btnSaveImage->Font = gcnew Drawing::Font("Segoe UI", 9, FontStyle::Bold);
            btnSaveImage->Click += gcnew EventHandler(this, &MainForm::btnSaveImage_Click);
            panelToolbar->Controls->Add(btnSaveImage);
            
            // Label FPS
            lblFPS = gcnew Label();
            lblFPS->Text = L"FPS: --";
            lblFPS->Location = Point(1250, 28);
            lblFPS->AutoSize = true;
            lblFPS->Font = gcnew Drawing::Font("Segoe UI", 12, FontStyle::Bold);
            lblFPS->ForeColor = Color::Lime;
            panelToolbar->Controls->Add(lblFPS);
            
            this->Controls->Add(panelToolbar);
            
            // === PANEL DE IMAGENS ===
            panelImages = gcnew Panel();
            panelImages->Location = Point(10, 90);
            panelImages->Size = Drawing::Size(1000, 500);
            panelImages->BackColor = Color::FromArgb(35, 35, 38);
            
            // Label Original
            lblOriginal = gcnew Label();
            lblOriginal->Text = L"IMAGEM ORIGINAL";
            lblOriginal->Location = Point(180, 5);
            lblOriginal->AutoSize = true;
            lblOriginal->Font = gcnew Drawing::Font("Segoe UI", 11, FontStyle::Bold);
            lblOriginal->ForeColor = Color::FromArgb(100, 200, 255);
            panelImages->Controls->Add(lblOriginal);
            
            // PictureBox Original
            picOriginal = gcnew PictureBox();
            picOriginal->Location = Point(10, 30);
            picOriginal->Size = Drawing::Size(480, 360);
            picOriginal->BackColor = Color::FromArgb(25, 25, 28);
            picOriginal->SizeMode = PictureBoxSizeMode::Zoom;
            picOriginal->BorderStyle = BorderStyle::FixedSingle;
            panelImages->Controls->Add(picOriginal);
            
            // Label Processada
            lblProcessed = gcnew Label();
            lblProcessed->Text = L"IMAGEM PROCESSADA";
            lblProcessed->Location = Point(680, 5);
            lblProcessed->AutoSize = true;
            lblProcessed->Font = gcnew Drawing::Font("Segoe UI", 11, FontStyle::Bold);
            lblProcessed->ForeColor = Color::FromArgb(100, 255, 150);
            panelImages->Controls->Add(lblProcessed);
            
            // PictureBox Processada
            picProcessed = gcnew PictureBox();
            picProcessed->Location = Point(510, 30);
            picProcessed->Size = Drawing::Size(480, 360);
            picProcessed->BackColor = Color::FromArgb(25, 25, 28);
            picProcessed->SizeMode = PictureBoxSizeMode::Zoom;
            picProcessed->BorderStyle = BorderStyle::FixedSingle;
            panelImages->Controls->Add(picProcessed);
            
            // Progress Bar
            progressBar = gcnew ProgressBar();
            progressBar->Location = Point(10, 400);
            progressBar->Size = Drawing::Size(980, 20);
            progressBar->Style = ProgressBarStyle::Marquee;
            progressBar->Visible = false;
            panelImages->Controls->Add(progressBar);
            
            // Label Status
            lblStatus = gcnew Label();
            lblStatus->Text = L"Pronto. Carregue uma imagem ou inicie a câmera.";
            lblStatus->Location = Point(10, 430);
            lblStatus->Size = Drawing::Size(980, 60);
            lblStatus->Font = gcnew Drawing::Font("Segoe UI", 10);
            lblStatus->ForeColor = Color::FromArgb(200, 200, 200);
            panelImages->Controls->Add(lblStatus);
            
            this->Controls->Add(panelImages);
            
            // === PANEL DE RESULTADOS COMPARATIVOS ===
            grpResults = gcnew GroupBox();
            grpResults->Text = L"⚡ BENCHMARK COMPARATIVO - Tempos de Processamento";
            grpResults->Location = Point(1020, 90);
            grpResults->Size = Drawing::Size(360, 500);
            grpResults->ForeColor = Color::White;
            grpResults->Font = gcnew Drawing::Font("Segoe UI", 10, FontStyle::Bold);
            grpResults->BackColor = Color::FromArgb(35, 35, 38);
            
            int yPos = 35;
            int spacing = 55;
            
            // Sequential
            Label^ lblSeqTitle = gcnew Label();
            lblSeqTitle->Text = L"🔵 SEQUENTIAL (CPU Single-thread)";
            lblSeqTitle->Location = Point(15, yPos);
            lblSeqTitle->AutoSize = true;
            lblSeqTitle->Font = gcnew Drawing::Font("Segoe UI", 9, FontStyle::Bold);
            lblSeqTitle->ForeColor = Color::FromArgb(100, 150, 255);
            grpResults->Controls->Add(lblSeqTitle);
            
            lblTimeSequential = gcnew Label();
            lblTimeSequential->Text = L"Tempo: -- ms";
            lblTimeSequential->Location = Point(30, yPos + 22);
            lblTimeSequential->AutoSize = true;
            lblTimeSequential->Font = gcnew Drawing::Font("Consolas", 11);
            lblTimeSequential->ForeColor = Color::White;
            grpResults->Controls->Add(lblTimeSequential);
            
            yPos += spacing + 15;
            
            // Parallel
            Label^ lblParTitle = gcnew Label();
            lblParTitle->Text = L"🟢 PARALLEL (OpenMP)";
            lblParTitle->Location = Point(15, yPos);
            lblParTitle->AutoSize = true;
            lblParTitle->Font = gcnew Drawing::Font("Segoe UI", 9, FontStyle::Bold);
            lblParTitle->ForeColor = Color::FromArgb(100, 255, 150);
            grpResults->Controls->Add(lblParTitle);
            
            lblTimeParallel = gcnew Label();
            lblTimeParallel->Text = L"Tempo: -- ms";
            lblTimeParallel->Location = Point(30, yPos + 22);
            lblTimeParallel->AutoSize = true;
            lblTimeParallel->Font = gcnew Drawing::Font("Consolas", 11);
            lblTimeParallel->ForeColor = Color::White;
            grpResults->Controls->Add(lblTimeParallel);
            
            lblSpeedupParallel = gcnew Label();
            lblSpeedupParallel->Text = L"Speedup: --x";
            lblSpeedupParallel->Location = Point(200, yPos + 22);
            lblSpeedupParallel->AutoSize = true;
            lblSpeedupParallel->Font = gcnew Drawing::Font("Consolas", 11, FontStyle::Bold);
            lblSpeedupParallel->ForeColor = Color::Lime;
            grpResults->Controls->Add(lblSpeedupParallel);
            
            yPos += spacing + 15;
            
            // Multithread
            Label^ lblMtTitle = gcnew Label();
            lblMtTitle->Text = L"🟡 MULTITHREAD (std::thread)";
            lblMtTitle->Location = Point(15, yPos);
            lblMtTitle->AutoSize = true;
            lblMtTitle->Font = gcnew Drawing::Font("Segoe UI", 9, FontStyle::Bold);
            lblMtTitle->ForeColor = Color::FromArgb(255, 220, 100);
            grpResults->Controls->Add(lblMtTitle);
            
            lblTimeMultithread = gcnew Label();
            lblTimeMultithread->Text = L"Tempo: -- ms";
            lblTimeMultithread->Location = Point(30, yPos + 22);
            lblTimeMultithread->AutoSize = true;
            lblTimeMultithread->Font = gcnew Drawing::Font("Consolas", 11);
            lblTimeMultithread->ForeColor = Color::White;
            grpResults->Controls->Add(lblTimeMultithread);
            
            lblSpeedupMultithread = gcnew Label();
            lblSpeedupMultithread->Text = L"Speedup: --x";
            lblSpeedupMultithread->Location = Point(200, yPos + 22);
            lblSpeedupMultithread->AutoSize = true;
            lblSpeedupMultithread->Font = gcnew Drawing::Font("Consolas", 11, FontStyle::Bold);
            lblSpeedupMultithread->ForeColor = Color::Yellow;
            grpResults->Controls->Add(lblSpeedupMultithread);
            
            yPos += spacing + 15;
            
            // CUDA
            Label^ lblCudaTitle = gcnew Label();
            lblCudaTitle->Text = L"🔴 CUDA (GPU - RTX 4060)";
            lblCudaTitle->Location = Point(15, yPos);
            lblCudaTitle->AutoSize = true;
            lblCudaTitle->Font = gcnew Drawing::Font("Segoe UI", 9, FontStyle::Bold);
            lblCudaTitle->ForeColor = Color::FromArgb(255, 100, 100);
            grpResults->Controls->Add(lblCudaTitle);
            
            lblTimeCUDA = gcnew Label();
            lblTimeCUDA->Text = L"Tempo: -- ms";
            lblTimeCUDA->Location = Point(30, yPos + 22);
            lblTimeCUDA->AutoSize = true;
            lblTimeCUDA->Font = gcnew Drawing::Font("Consolas", 11);
            lblTimeCUDA->ForeColor = Color::White;
            grpResults->Controls->Add(lblTimeCUDA);
            
            lblSpeedupCUDA = gcnew Label();
            lblSpeedupCUDA->Text = L"Speedup: --x";
            lblSpeedupCUDA->Location = Point(200, yPos + 22);
            lblSpeedupCUDA->AutoSize = true;
            lblSpeedupCUDA->Font = gcnew Drawing::Font("Consolas", 11, FontStyle::Bold);
            lblSpeedupCUDA->ForeColor = Color::Red;
            grpResults->Controls->Add(lblSpeedupCUDA);
            
            yPos += spacing + 30;
            
            // Info da imagem
            Label^ lblInfo = gcnew Label();
            lblInfo->Text = L"Resolução: --\nCanais: --\nTamanho: -- pixels";
            lblInfo->Location = Point(15, yPos);
            lblInfo->Size = Drawing::Size(330, 80);
            lblInfo->Font = gcnew Drawing::Font("Segoe UI", 9);
            lblInfo->ForeColor = Color::FromArgb(180, 180, 180);
            lblInfo->Name = L"lblInfo";
            grpResults->Controls->Add(lblInfo);
            
            this->Controls->Add(grpResults);
            
            // === TIMERS ===
            timerCamera = gcnew System::Windows::Forms::Timer();
            timerCamera->Interval = 33; // ~30 FPS
            timerCamera->Tick += gcnew EventHandler(this, &MainForm::timerCamera_Tick);
            
            timerFPS = gcnew System::Windows::Forms::Timer();
            timerFPS->Interval = 1000; // Atualiza FPS a cada segundo
            timerFPS->Tick += gcnew EventHandler(this, &MainForm::timerFPS_Tick);
        }

        // === EVENT HANDLERS ===
        
        void btnLoadImage_Click(Object^ sender, EventArgs^ e)
        {
            OpenFileDialog^ openDialog = gcnew OpenFileDialog();
            openDialog->Filter = L"Imagens|*.jpg;*.jpeg;*.png;*.bmp;*.tiff;*.webp|Todos os arquivos|*.*";
            openDialog->Title = L"Selecione uma imagem";
            
            if (openDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
            {
                try
                {
                    StopCamera();
                    originalImage = gcnew Bitmap(openDialog->FileName);
                    picOriginal->Image = originalImage;
                    
                    UpdateImageInfo();
                    ProcessCurrentImage();
                    
                    lblStatus->Text = L"Imagem carregada: " + openDialog->FileName;
                }
                catch (Exception^ ex)
                {
                    MessageBox::Show(L"Erro ao carregar imagem: " + ex->Message, L"Erro",
                        MessageBoxButtons::OK, MessageBoxIcon::Error);
                }
            }
        }
        
        void btnStartCamera_Click(Object^ sender, EventArgs^ e)
        {
            StartCamera();
        }
        
        void btnStopCamera_Click(Object^ sender, EventArgs^ e)
        {
            StopCamera();
        }
        
        void btnSaveImage_Click(Object^ sender, EventArgs^ e)
        {
            if (processedImage == nullptr)
            {
                MessageBox::Show(L"Nenhuma imagem processada para salvar.", L"Aviso",
                    MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
            
            SaveFileDialog^ saveDialog = gcnew SaveFileDialog();
            saveDialog->Filter = L"PNG|*.png|JPEG|*.jpg|BMP|*.bmp";
            saveDialog->Title = L"Salvar imagem processada";
            saveDialog->FileName = L"output_" + cmbFilter->SelectedItem->ToString() + L"_" + 
                                   cmbProcessingMode->SelectedItem->ToString();
            
            if (saveDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
            {
                processedImage->Save(saveDialog->FileName);
                lblStatus->Text = L"Imagem salva: " + saveDialog->FileName;
            }
        }
        
        void btnBenchmark_Click(Object^ sender, EventArgs^ e)
        {
            if (originalImage == nullptr)
            {
                MessageBox::Show(L"Carregue uma imagem primeiro!", L"Aviso",
                    MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
            
            RunComparativeBenchmark();
        }
        
        void cmbFilter_SelectedIndexChanged(Object^ sender, EventArgs^ e)
        {
            if (originalImage != nullptr && !cameraRunning)
            {
                ProcessCurrentImage();
            }
        }
        
        void cmbProcessingMode_SelectedIndexChanged(Object^ sender, EventArgs^ e)
        {
            if (originalImage != nullptr && !cameraRunning)
            {
                ProcessCurrentImage();
            }
        }
        
        void timerCamera_Tick(Object^ sender, EventArgs^ e)
        {
            CaptureAndProcessFrame();
        }
        
        void timerFPS_Tick(Object^ sender, EventArgs^ e)
        {
            if (fpsStopwatch->ElapsedMilliseconds > 0)
            {
                currentFPS = frameCount * 1000.0 / fpsStopwatch->ElapsedMilliseconds;
                lblFPS->Text = String::Format(L"FPS: {0:F1}", currentFPS);
                
                // Cor baseada no FPS
                if (currentFPS >= 25)
                    lblFPS->ForeColor = Color::Lime;
                else if (currentFPS >= 15)
                    lblFPS->ForeColor = Color::Yellow;
                else
                    lblFPS->ForeColor = Color::Red;
            }
            
            frameCount = 0;
            fpsStopwatch->Restart();
        }
        
        // === MÉTODOS DE PROCESSAMENTO ===
        
        void StartCamera()
        {
            StopCamera();
            
            // Tentar abrir câmera usando OpenCV nativo via DLL
            if (StartCameraCapture(0))
            {
                cameraRunning = true;
                btnStartCamera->Enabled = false;
                btnStopCamera->Enabled = true;
                btnLoadImage->Enabled = false;
                
                frameCount = 0;
                fpsStopwatch->Restart();
                timerCamera->Start();
                timerFPS->Start();
                
                lblStatus->Text = L"Câmera iniciada. Processando em tempo real...";
            }
            else
            {
                MessageBox::Show(L"Não foi possível abrir a câmera.", L"Erro",
                    MessageBoxButtons::OK, MessageBoxIcon::Error);
            }
        }
        
        void StopCamera()
        {
            timerCamera->Stop();
            timerFPS->Stop();
            StopCameraCapture();
            
            cameraRunning = false;
            btnStartCamera->Enabled = true;
            btnStopCamera->Enabled = false;
            btnLoadImage->Enabled = true;
            lblFPS->Text = L"FPS: --";
            
            if (lblStatus != nullptr)
                lblStatus->Text = L"Câmera parada.";
        }
        
        // Métodos nativos - serão implementados no wrapper
        bool StartCameraCapture(int deviceId);
        void StopCameraCapture();
        Bitmap^ CaptureFrame();
        Bitmap^ ApplyFilter(Bitmap^ input, int filterType, int processingMode, double% timeMs);
        
        void CaptureAndProcessFrame()
        {
            try
            {
                Bitmap^ frame = CaptureFrame();
                if (frame != nullptr)
                {
                    originalImage = frame;
                    picOriginal->Image = originalImage;
                    
                    double timeMs = 0;
                    int filterIdx = cmbFilter->SelectedIndex;
                    int modeIdx = cmbProcessingMode->SelectedIndex;
                    
                    processedImage = ApplyFilter(originalImage, filterIdx, modeIdx, timeMs);
                    picProcessed->Image = processedImage;
                    
                    frameCount++;
                    
                    // Atualizar tempo do modo atual
                    UpdateSingleTime(modeIdx, timeMs);
                }
            }
            catch (Exception^)
            {
                // Ignorar erros de frame
            }
        }
        
        void ProcessCurrentImage()
        {
            if (originalImage == nullptr) return;
            
            try
            {
                progressBar->Visible = true;
                Application::DoEvents();
                
                double timeMs = 0;
                int filterIdx = cmbFilter->SelectedIndex;
                int modeIdx = cmbProcessingMode->SelectedIndex;
                
                processedImage = ApplyFilter(originalImage, filterIdx, modeIdx, timeMs);
                picProcessed->Image = processedImage;
                
                UpdateSingleTime(modeIdx, timeMs);
                
                lblStatus->Text = String::Format(L"Processado com {0} - {1} em {2:F2} ms",
                    cmbFilter->SelectedItem, cmbProcessingMode->SelectedItem, timeMs);
                
                progressBar->Visible = false;
            }
            catch (Exception^ ex)
            {
                progressBar->Visible = false;
                lblStatus->Text = L"Erro: " + ex->Message;
            }
        }
        
        void UpdateSingleTime(int modeIdx, double timeMs)
        {
            String^ timeStr = String::Format(L"Tempo: {0:F2} ms", timeMs);
            
            switch (modeIdx)
            {
                case 0: lblTimeSequential->Text = timeStr; break;
                case 1: lblTimeParallel->Text = timeStr; break;
                case 2: lblTimeMultithread->Text = timeStr; break;
                case 3: lblTimeCUDA->Text = timeStr; break;
            }
        }
        
        void RunComparativeBenchmark()
        {
            if (originalImage == nullptr) return;
            
            lblStatus->Text = L"Executando benchmark comparativo...";
            progressBar->Visible = true;
            Application::DoEvents();
            
            int filterIdx = cmbFilter->SelectedIndex;
            array<double>^ times = gcnew array<double>(4);
            
            // Executar cada modo
            for (int mode = 0; mode < 4; mode++)
            {
                double timeMs = 0;
                Bitmap^ result = ApplyFilter(originalImage, filterIdx, mode, timeMs);
                times[mode] = timeMs;
                
                if (mode == cmbProcessingMode->SelectedIndex)
                {
                    processedImage = result;
                    picProcessed->Image = processedImage;
                }
                
                Application::DoEvents();
            }
            
            // Atualizar UI com resultados
            double baseTime = times[0]; // Sequential como base
            
            lblTimeSequential->Text = String::Format(L"Tempo: {0:F2} ms", times[0]);
            
            lblTimeParallel->Text = String::Format(L"Tempo: {0:F2} ms", times[1]);
            lblSpeedupParallel->Text = String::Format(L"Speedup: {0:F1}x", baseTime / times[1]);
            lblSpeedupParallel->ForeColor = times[1] < baseTime ? Color::Lime : Color::Red;
            
            lblTimeMultithread->Text = String::Format(L"Tempo: {0:F2} ms", times[2]);
            lblSpeedupMultithread->Text = String::Format(L"Speedup: {0:F1}x", baseTime / times[2]);
            lblSpeedupMultithread->ForeColor = times[2] < baseTime ? Color::Yellow : Color::Red;
            
            lblTimeCUDA->Text = String::Format(L"Tempo: {0:F2} ms", times[3]);
            lblSpeedupCUDA->Text = String::Format(L"Speedup: {0:F1}x", baseTime / times[3]);
            lblSpeedupCUDA->ForeColor = times[3] < baseTime ? Color::Cyan : Color::Red;
            
            progressBar->Visible = false;
            lblStatus->Text = String::Format(
                L"Benchmark concluído! Sequential: {0:F2}ms | Parallel: {1:F2}ms | Multithread: {2:F2}ms | CUDA: {3:F2}ms",
                times[0], times[1], times[2], times[3]);
        }
        
        void UpdateImageInfo()
        {
            if (originalImage == nullptr) return;
            
            for each (Control^ ctrl in grpResults->Controls)
            {
                if (ctrl->Name == L"lblInfo")
                {
                    Label^ lblInfo = (Label^)ctrl;
                    lblInfo->Text = String::Format(
                        L"Resolução: {0} x {1}\nCanais: {2}\nPixels: {3:N0}",
                        originalImage->Width, originalImage->Height,
                        Bitmap::GetPixelFormatSize(originalImage->PixelFormat) / 8,
                        originalImage->Width * originalImage->Height);
                    break;
                }
            }
        }
    };
}
