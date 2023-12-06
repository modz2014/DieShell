#pragma once
#include <iostream>
#include "wintoastlib.h"
#include <string>

using namespace WinToastLib;
// Initialize the notification template
/**
* @file
* @brief Defines a custom WinToast template and related functions.
*/

/**
* @brief CustomTemplate creates a WinToast template with a specified file information.
*
* This function creates a WinToast template with an image and text fields, using the specified file information.
*
* @param fileInfo The file information to be displayed in the toast notification.
* @return WinToastTemplate instance representing the custom notification template.
* @throw std::runtime_error if there is an error obtaining the DLL path.
*/
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
/**
* @brief Sets the file path and file information.
*
* @param path The file path.
* @param info The file information.
*/
	void setFilePath(const std::wstring& path, const std::wstring& info) {
		filePath = path;
		fileInfo = info;
		std::wcout << L"File path set to: " << filePath << std::endl;
		std::wstring message = L"File path set to: " + filePath;
		//MessageBox(NULL, message.c_str(), L"Debug Message", MB_OK);
	}
/**
* @brief Opens the associated program.
*
* @param g_hModule The module handle.
* @param filePath The file path.
*/
	static void OpenProgram(HMODULE g_hModule, const std::wstring& filePath) {
		// Display the file path
		std::wstring filePathMessage = L"File Path: " + filePath;


		wchar_t dllPath[MAX_PATH];
		GetModuleFileName(g_hModule, dllPath, MAX_PATH);
		PathRemoveFileSpec(dllPath);

		std::wstring dllPathMessage = L"DLL Path: " + std::wstring(dllPath);

		std::wstring dllpath = std::wstring(dllPath) + L"\\die.exe";
		std::wstring parameters = L"\"" + filePath + L"\"";

		std::wstring command = L"Opening: " + dllpath + L" " + parameters;


		SHELLEXECUTEINFO shExInfo = { 0 };
		shExInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		shExInfo.lpFile = dllpath.c_str(); // Corrected variable name
		shExInfo.lpParameters = parameters.c_str();
		shExInfo.nShow = SW_SHOWNORMAL;

		if (ShellExecuteEx(&shExInfo)) {
			// Successfully started the associated program
			// You can add additional handling if needed
		}
		else {
			// Failed to start the associated program
			// You can add error handling here

		}
	}
/**
* @brief Sets the file path.
*
* @param path The file path.
*/
	void setFilePath(const std::wstring& path) {
		filePath = path;
	}
/**
* @brief Handler for when the toast is activated.
*/
	void toastActivated() const override {
		std::wcout << L"Toast Clicked. File Information: " << fileInfo << std::endl;
		OpenProgram(g_hModule, filePath);
	}
/**
* @brief Handler for when a specific action on the toast is activated.
*
* @param actionIndex The index of the activated action.
*/
	void toastActivated(int actionIndex) const override {
		std::cout << "Action Clicked #" << actionIndex << std::endl;
		if (actionIndex == 0) { // Assuming "Open DIE" is the first action
			OpenProgram(g_hModule, filePath); // Call the function to open the program
		}
	}
/**
* @brief Handler for when the toast is dismissed.
*
* @param state The reason for dismissal.
*/
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
/**
* @brief Handler for when the toast activation fails.
*/
	void toastFailed() const {
		std::cout << "Failed" << std::endl;
	}
};
/**
* @brief Retrieves file information using DIE_VB_ScanFile.
*
* @param filePath The path of the file.
* @param pwszDatabase The path to the database.
* @return A wstring containing the file information.
*/
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
