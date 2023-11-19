// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <wrl/module.h>
#include <wrl/implements.h>
#include <shobjidl_core.h>
#include <wil/resource.h>
#include <Shellapi.h>
#include <Shlwapi.h>
#include <Strsafe.h>
#include <VersionHelpers.h>
#include <Shobjidl.h>
#include "ToastNotification.h"
//#include <fstream>

using namespace Microsoft::WRL;
HMODULE g_hModule = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        g_hModule = hModule;
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


class DieCommand : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IExplorerCommand, IObjectWithSite> {
public:
    // IExplorerCommand methods
    IFACEMETHODIMP GetTitle(_In_opt_ IShellItemArray* items, _Outptr_result_nullonfailure_ PWSTR* name) {
        *name = nullptr;
        auto title = wil::make_cotaskmem_string_nothrow(L"Die");
        RETURN_IF_NULL_ALLOC(title);
        *name = title.release();
        return S_OK;
    }

    IFACEMETHODIMP GetIcon(_In_opt_ IShellItemArray* items, _Outptr_result_nullonfailure_ PWSTR* iconPath) {
        *iconPath = nullptr;
        PWSTR itemPath = nullptr;

        if (items) {
            DWORD count;
            RETURN_IF_FAILED(items->GetCount(&count));

            if (count > 0) {
                ComPtr<IShellItem> item;
                RETURN_IF_FAILED(items->GetItemAt(0, &item));

                RETURN_IF_FAILED(item->GetDisplayName(SIGDN_FILESYSPATH, &itemPath));
                wil::unique_cotaskmem_string itemPathCleanup(itemPath);

                WCHAR modulePath[MAX_PATH];
                if (GetModuleFileNameW(g_hModule, modulePath, ARRAYSIZE(modulePath))) {
                    PathRemoveFileSpecW(modulePath);
                    StringCchCatW(modulePath, ARRAYSIZE(modulePath), L"\\Die.ico");

                    auto iconPathStr = wil::make_cotaskmem_string_nothrow(modulePath);
                    if (iconPathStr) {
                        *iconPath = iconPathStr.release();
                    }
                }
            }
        }
        return *iconPath ? S_OK : E_FAIL;
    }

    IFACEMETHODIMP GetToolTip(_In_opt_ IShellItemArray*, _Outptr_result_nullonfailure_ PWSTR* infoTip) { *infoTip = nullptr; return E_NOTIMPL; }
    IFACEMETHODIMP GetCanonicalName(_Out_ GUID* guidCommandName) { *guidCommandName = GUID_NULL; return S_OK; }

    IFACEMETHODIMP GetState(_In_opt_ IShellItemArray* selection, _In_ BOOL okToBeSlow, _Out_ EXPCMDSTATE* cmdState) {
        // Check if the Windows version is 11 or greater
        bool isWindows11OrGreater = IsWindowsVersionOrGreater(10, 0, 0);

        if (isWindows11OrGreater)
        {
            // If the Windows version is 11 or greater and a selection is provided and it's okay to be slow...
            if (selection && okToBeSlow)
            {
                // Enable the context menu item
                *cmdState = ECS_ENABLED;
                return S_OK;
            }
            // Hide the context menu item on Windows 11 or greater
            *cmdState = ECS_HIDDEN;
        }
        else
        {
            // Handle other cases, if necessary.
            // Show the context menu item by default on other versions.
            *cmdState = ECS_ENABLED;
        }

        return S_OK;
    }

    IFACEMETHODIMP Invoke(_In_opt_ IShellItemArray* selection, _In_opt_ IBindCtx*) noexcept try {
        if (!selection) {
            // Debug message
            MessageBox(nullptr, L"Invalid argument", L"Debug Info", MB_OK);
            return E_INVALIDARG;
        }

        DWORD count;
        RETURN_IF_FAILED(selection->GetCount(&count));

        if (count == 0) {
            // Debug message
            MessageBox(nullptr, L"No items to process", L"Debug Info", MB_OK);
            return S_OK; // No items to process
        }

        ComPtr<IShellItem> item;
        RETURN_IF_FAILED(selection->GetItemAt(0, &item));

        PWSTR filePath;
        RETURN_IF_FAILED(item->GetDisplayName(SIGDN_FILESYSPATH, &filePath));
        wil::unique_cotaskmem_string filePathCleanup(filePath);

        // Checking files being downloaded 
       

        // Debug: Display the file path in a message box
        std::wstring message = L"File path: " + std::wstring(filePath);
        // MessageBox(NULL, message.c_str(), L"Debug Information", MB_OK);

        wchar_t dllPath[MAX_PATH];
        GetModuleFileName(g_hModule, dllPath, MAX_PATH);
        PathRemoveFileSpec(dllPath);

        std::wstring dllPathMessage = L"DLL Path: " + std::wstring(dllPath);
        //MessageBox(NULL, dllPathMessage.c_str(), L"Debug Information", MB_OK);
        std::wstring pwszDatabase = std::wstring(dllPath) + L"\\db";
        std::wstring fileInfo = GetFileInformation(filePath, pwszDatabase);

        // Toast notification stuff 
        WinToast::WinToastError error;
        WinToast::instance()->setAppName(L"MyApp");
        const auto aumi = WinToast::configureAUMI(L"company", L"wintoast", L"wintoastexample", L"20201012");
        WinToast::instance()->setAppUserModelId(aumi);

        // Run the toast notification code on the UI thread
        if (!WinToast::instance()->initialize()) {
            std::wcerr << L"Error, your system is not compatible!" << std::endl;
            return S_OK;
        }
        WinToastTemplate templ = CustomTemplate(fileInfo); // Use the custom template
        ToastHandler* toastHandler = new ToastHandler();
        toastHandler->setFilePath(filePath);
         if (WinToast::instance()->showToast(templ, toastHandler) == -1L) {
         std::cerr << "Could not launch your toast notification!" << std::endl;
         }

        if (WinToast::instance()->showToast(templ, new ToastHandler()) == -1L) {
            std::cerr << "Could not launch your toast notification!" << std::endl;
        }

        return S_OK;
    }

    CATCH_RETURN();

    IFACEMETHODIMP GetFlags(_Out_ EXPCMDFLAGS* flags) { *flags = ECF_DEFAULT; return S_OK; }
    IFACEMETHODIMP EnumSubCommands(_COM_Outptr_ IEnumExplorerCommand** enumCommands) { *enumCommands = nullptr; return E_NOTIMPL; }

    // IObjectWithSite methods
    IFACEMETHODIMP SetSite(_In_ IUnknown* site) noexcept { m_site = site; return S_OK; }
    IFACEMETHODIMP GetSite(_In_ REFIID riid, _COM_Outptr_ void** site) noexcept { return m_site.CopyTo(riid, site); }

protected:
    ComPtr<IUnknown> m_site;
};

class __declspec(uuid("7A1E471F-0D43-4122-B1C4-D1AACE76CE9B")) DieCommand1 final : public DieCommand {
    //Testing 
    //public:
    //const wchar_t* Title() override { return L"HelloWorld Command1"; }
    //const EXPCMDSTATE State(_In_opt_ IShellItemArray* selection) override { return ECS_DISABLED; }
};

CoCreatableClass(DieCommand1)

STDAPI DllGetActivationFactory(_In_ HSTRING activatableClassId, _COM_Outptr_ IActivationFactory** factory) {
    return Module<ModuleType::InProc>::GetModule().GetActivationFactory(activatableClassId, factory);
}

STDAPI DllCanUnloadNow() {
    return Module<InProc>::GetModule().GetObjectCount() == 0 ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _COM_Outptr_ void** instance) {
    return Module<InProc>::GetModule().GetClassObject(rclsid, riid, instance);
}
