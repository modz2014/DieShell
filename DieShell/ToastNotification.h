#pragma once
#include <iostream>
#include "wintoastlib.h"
#include <string>

using namespace WinToastLib;
// Initialize the notification template

extern HMODULE g_hModule;
extern "C" __declspec(dllimport) int __cdecl DIE_VB_ScanFile(const wchar_t* pwszFileName, int nFlags, const wchar_t* pwszDatabase, wchar_t* pwszBuffer, int nBufferSize);

WinToastTemplate CustomTemplate(const std::wstring& fileInfo) {
    HMODULE hModule = GetModuleHandle(L"DieShell.dll");

    WCHAR dllPath[MAX_PATH];
    if (GetModuleFileName(hModule, dllPath, MAX_PATH) == 0) {
        // Handle error (e.g., call GetLastError())
        throw std::runtime_error("Failed to get DLL path");
    }
    // Remove the filename from the full path
    PathRemoveFileSpec(dllPath);
    // Construct the full path to the image file
    std::wstring imagePath = std::wstring(dllPath) + L"\\die.png";

    WinToastTemplate templ(WinToastTemplate::ImageAndText04);

    // Set the image path to the constructed image path

    templ.setImagePath(imagePath.c_str());

   // Set text fields
    templ.setTextField(fileInfo, WinToastTemplate::FirstLine);


    return templ;
}


class ToastHandler : public WinToastLib::IWinToastHandler {
private:
    std::wstring filePath; // Add a member variable to store the file path
    std::wstring fileInfo;   // Add a member variable to store file information

public:
    ToastHandler() : filePath(L""), fileInfo(L"") {}
    void setFilePath(const std::wstring& path, const std::wstring& info) {
        filePath = path;
        fileInfo = info;
        std::wcout << L"File path set to: " << filePath << std::endl;
        std::wstring message = L"File path set to: " + filePath;
        //MessageBox(NULL, message.c_str(), L"Debug Message", MB_OK);
    }

    void OpenProgram(HMODULE g_hModule, const std::wstring& filePath) const {

        wchar_t dllPath[MAX_PATH];
        GetModuleFileName(g_hModule, dllPath, MAX_PATH);
        PathRemoveFileSpec(dllPath);

        std::wstring dllPathMessage = L"DLL Path: " + std::wstring(dllPath);
        // MessageBox(NULL, dllPathMessage.c_str(), L"Debug Information", MB_OK);
        std::wstring dllpath = std::wstring(dllPath) + L"\\die.exe";
        std::wstring parameters = L"\"" + filePath;

        std::wstring command = L"Opening: " + dllpath + L" " + parameters;
        //MessageBox(NULL, command.c_str(), L"Debug Message", MB_OK);

        HINSTANCE result = ShellExecute(NULL, L"open", dllpath.c_str(), parameters.c_str(), NULL, SW_SHOW);

        if ((int)result <= 32) {
            std::wstring message = L"ShellExecute failed, error: " + std::to_wstring((int)result);
            MessageBox(NULL, message.c_str(), L"Debug Message", MB_OK);
        }
    }




    void toastActivated() const override {
        std::wcout << L"Toast Clicked. File Information: " << fileInfo << std::endl;
        OpenProgram(g_hModule, filePath);
    }

    void toastActivated(int actionIndex) const override {
        std::cout << "Action Clicked #" << actionIndex << std::endl;
        if (actionIndex == 0) { // Assuming "Open DIE" is the first action
            OpenProgram(g_hModule, filePath); // Call the function to open the program
        }
    }

    void toastDismissed(WinToastDismissalReason state) const {
        switch (state) {

        case UserCanceled:
            std::cout << "Dissmissed" << std::endl;
            break;

        case TimedOut:
            std::cout << "Timed out" << std::endl;
            break;

        case ApplicationHidden:
            std::cout << "Hidden from app" << std::endl;
            break;

        default:
            std::cout << "Toast not activated" << std::endl;
            break;
        }
    }

    void toastFailed() const {
        std::cout << "Failed" << std::endl;
    }
};

std::wstring GetFileInformation(const std::wstring& filePath, const std::wstring& pwszDatabase)
{
    try {
        const int nBufferSize = 10000;
        std::wstring sBuffer(nBufferSize, L' ');  // Allocate buffer

        // Use DIE_VB_ScanFile to get the file information
        int nResult = DIE_VB_ScanFile(filePath.c_str(), 0, pwszDatabase.c_str(), &sBuffer[0], nBufferSize - 1);

        std::wstring result;
        if (nResult > 0) {
            // Trim the buffer to the actual length of the result
            sBuffer.resize(nResult);
            result = sBuffer;
        }
        else {
            result = L"Scan failed or no result.";
        }

        return result;
    }
    catch (...) {
        // Handle the exception here if needed
        return L"Exception occurred while retrieving file information";
    }
}