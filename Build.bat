@echo off
:: BatchGotAdmin
:-------------------------------------
REM  --> Check for permissions
IF "%PROCESSOR_ARCHITECTURE%" EQU "amd64" (
>nul 2>&1 "%SYSTEMROOT%\SysWOW64\cacls.exe" "%SYSTEMROOT%\SysWOW64\config\system"
) ELSE (
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"
)

REM --> If error flag set, we do not have admin.
if '%errorlevel%' NEQ '0' (
    echo Requesting administrative privileges...
    goto UACPrompt
) else ( goto gotAdmin )

:UACPrompt
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    echo UAC.ShellExecute "%~s0", "", "", "runas", 1 >> "%temp%\getadmin.vbs"

    "%temp%\getadmin.vbs"
    exit /B

:gotAdmin
    if exist "%temp%\getadmin.vbs" ( del "%temp%\getadmin.vbs" )
    pushd "%CD%"
    CD /D "%~dp0"
:--------------------------------------

powershell -Command "New-SelfSignedCertificate -Type Custom -Subject 'CN=Die' -KeyUsage DigitalSignature -FriendlyName 'SelfSignCert' -CertStoreLocation 'Cert:\CurrentUser\My' -TextExtension @('2.5.29.37={text}1.3.6.1.5.5.7.3.3', '2.5.29.19={text}')"
set /p password="Enter a password for the PFX file: "

for /f "tokens=*" %%a in ('powershell -Command "Get-ChildItem -Path Cert:\CurrentUser\My | Where-Object { $_.Subject -eq 'CN=Die' } | Select-Object -ExpandProperty Thumbprint"') do (
    set thumbprint=%%a
)

if not defined thumbprint (
    echo Certificate not found.
    exit /b 1
)

set "batch_dir=%~dp0"
powershell -Command "Export-PfxCertificate -Cert Cert:\CurrentUser\My\%thumbprint% -FilePath '%batch_dir%Die.pfx' -Password (ConvertTo-SecureString -String '%password%' -Force -AsPlainText)"
:: Export the certificate to CER file
powershell -Command "Export-Certificate -Cert Cert:\CurrentUser\My\%thumbprint% -FilePath '%batch_dir%Die.cer'"


:: Run the makeappx.exe command and redirect output to a log file
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\makeappx.exe" pack /d Die/ /p Die.msix /nv

set /p PASSWORD="Enter the password for the .pfx file: "

set DIRECTORY=%~dp0

for /R %DIRECTORY% %%F in (*.pfx) do (
    set PFX_PATH="%%F"
)

for /R %DIRECTORY% %%F in (*.msix) do (
    set MSIX_PATH="%%F"
)

"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22000.0\x64\signtool.exe" sign /fd SHA256 /a /f %PFX_PATH% /p %PASSWORD% %MSIX_PATH%

:: Import the certificate
set "batch_dir=%~dp0"
set "CERT_FILE=%batch_dir%Die.cer"

:: Add the certificate to the "Trusted People" store
certutil -addstore TrustedPeople %CERT_FILE%

echo Certificate added to Trusted People store.
pause
