@echo off
REM ===================================
REM PAVIC LAB 2025 - Script de Upload para GitHub
REM ===================================

echo.
echo === PAVIC LAB 2025 - Upload para GitHub ===
echo.

REM Verificar se Git existe
where git >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERRO: Git nao encontrado!
    echo Instale o Git de: https://git-scm.com/download/win
    echo Depois execute este script novamente.
    pause
    exit /b 1
)

echo Git encontrado!
echo.

REM Configurar diretorio
cd /d C:\PAVIC_LAB_2025

REM Inicializar Git se necessario
if not exist ".git" (
    echo Inicializando repositorio Git...
    git init
)

REM Configurar usuario (ajuste se necessario)
echo Configurando usuario Git...
git config user.email "seu-email@exemplo.com"
git config user.name "DayanFA"

REM Adicionar arquivos
echo Adicionando arquivos...
git add .

REM Commit
echo Fazendo commit...
git commit -m "PAVIC LAB 2025 - Processamento de Imagens GPU vs CPU"

REM Adicionar remote
echo Configurando remote...
git remote remove origin 2>nul
git remote add origin https://github.com/DayanFA/PAVIC.git

REM Criar branch se necessario
git branch -M main

REM Push
echo.
echo Fazendo push para GitHub...
echo Voce sera solicitado a fazer login no GitHub.
echo.
git push -u origin main

echo.
echo === Processo concluido! ===
echo Verifique seu repositorio em: https://github.com/DayanFA/PAVIC
pause
