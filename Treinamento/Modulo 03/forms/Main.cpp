/**
 * PAVIC LAB 2025 - Windows Forms Application Entry Point
 * Ponto de entrada da aplicação Windows Forms
 */

#include "MainForm.h"
#include "NativeProcessor.h"

using namespace System;
using namespace System::Windows::Forms;

namespace PAVIC_LAB_2025 {

    // Instância global do processador nativo
    static NativeProcessor^ g_processor = nullptr;
    
    // Implementação dos métodos nativos do MainForm
    bool MainForm::StartCameraCapture(int deviceId)
    {
        if (g_processor == nullptr)
            g_processor = gcnew NativeProcessor();
        
        return g_processor->StartCamera(deviceId);
    }
    
    void MainForm::StopCameraCapture()
    {
        if (g_processor != nullptr)
            g_processor->StopCamera();
    }
    
    Bitmap^ MainForm::CaptureFrame()
    {
        if (g_processor == nullptr)
            return nullptr;
        
        return g_processor->CaptureFrame();
    }
    
    Bitmap^ MainForm::ApplyFilter(Bitmap^ input, int filterType, int processingMode, double% timeMs)
    {
        if (g_processor == nullptr)
            g_processor = gcnew NativeProcessor();
        
        return g_processor->ApplyFilter(input, filterType, processingMode, timeMs);
    }
}

[STAThread]
int main(array<String^>^ args)
{
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);
    
    PAVIC_LAB_2025::MainForm^ mainForm = gcnew PAVIC_LAB_2025::MainForm();
    Application::Run(mainForm);
    
    return 0;
}
