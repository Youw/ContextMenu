/****************************** Module Header ******************************\
The file implements DllMain, and the DllGetClassObject, DllCanUnloadNow, 
DllRegisterServer, DllUnregisterServer functions that are necessary for a COM 
DLL. 
\***************************************************************************/

#include <windows.h>
#include <Guiddef.h>
#include "ClassFactory.h"           // For the class factory
#include "Reg.h"
#include "../Settings.h"


#if UINTPTR_MAX==UINT64_MAX
// GUID for our COM object
// {F9A97231-462A-4DF4-8594-E5761DB8565E}
static const GUID CLSID_FilesInfoAndChecksum =
{ 0xf9a97231, 0x462a, 0x4df4, { 0x64, 0x94, 0xe5, 0x76, 0x1d, 0xb8, 0x56, 0x5e } };
#else
// GUID for our COM object
// {F9A97231-462A-4DF4-8594-E5761DB8565E}
static const GUID CLSID_FilesInfoAndChecksum =
{ 0xf9a97231, 0x462a, 0x4df4, { 0x32, 0x94, 0xe5, 0x76, 0x1d, 0xb8, 0x56, 0x5e } };
#endif


static HINSTANCE   g_hInst     = NULL;
long        g_cDllRef   = 0;


BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
        // Hold the instance of this DLL module, we will use it to get the 
        // path of the DLL to register the component.
        g_hInst = hModule;
		//just optimisation
        DisableThreadLibraryCalls(hModule);
        break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


//
//   FUNCTION: DllGetClassObject
//
//   PURPOSE: Create the class factory and query to the specific interface.
//
//   PARAMETERS:
//   * rclsid - The CLSID that will associate the correct data and code.
//   * riid - A reference to the identifier of the interface that the caller 
//     is to use to communicate with the class object.
//   * ppv - The address of a pointer variable that receives the interface 
//     pointer requested in riid. Upon successful return, *ppv contains the 
//     requested interface pointer. If an error occurs, the interface pointer 
//     is NULL. 
//
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

	if (IsEqualCLSID(CLSID_FilesInfoAndChecksum, rclsid))
    {
        hr = E_OUTOFMEMORY;

        ClassFactory *pClassFactory = new ClassFactory();
        if (pClassFactory)
        {
            hr = pClassFactory->QueryInterface(riid, ppv);
            pClassFactory->Release();
        }
    }

    return hr;
}


//
//   FUNCTION: DllCanUnloadNow
//
//   PURPOSE: Check if we can unload the component from the memory.
//
//   NOTE: The component can be unloaded from the memory when its reference 
//   count is zero (i.e. nobody is still using the component).
// 
STDAPI DllCanUnloadNow(void)
{
	return g_cDllRef > 0 ? S_FALSE : S_OK;
}


//
//   FUNCTION: DllRegisterServer
//
//   PURPOSE: Register the COM server and the context menu handler.
// 
STDAPI DllRegisterServer(void)
{
	using namespace registry;
	HRESULT hr;

    wchar_t szModule[MAX_PATH];
    if (GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule)) == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    // Register the component.
	hr = RegisterInprocServer(szModule, CLSID_FilesInfoAndChecksum,
		MAKEWIDE(HandlerFullName)L" Class",
        L"Apartment");
    if (SUCCEEDED(hr))
    {
        // Register the context menu handler. The context menu handler is 
        // associated with all files.
        hr = RegisterShellExtContextMenuHandler(L"*", 
			CLSID_FilesInfoAndChecksum,
			MAKEWIDE(HandlerFullName));
		if (SUCCEEDED(hr)) {
			hr = RegisterShellExtContextMenuHandler(L"Directory",
				CLSID_FilesInfoAndChecksum,
				MAKEWIDE(HandlerFullName));
		}
    }

    return hr;
}

//
//   FUNCTION: DllUnregisterServer
//
//   PURPOSE: Unregister the COM server and the context menu handler.
// 
STDAPI DllUnregisterServer(void)
{
	using namespace registry;
	
	HRESULT hr = S_OK;

    wchar_t szModule[MAX_PATH];
    if (GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule)) == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    // Unregister the component.
	hr = UnregisterInprocServer(CLSID_FilesInfoAndChecksum);
    if (SUCCEEDED(hr))
    {
        // Unregister the context menu handler.
        hr = UnregisterShellExtContextMenuHandler(L"*", 
			CLSID_FilesInfoAndChecksum);
		if (SUCCEEDED(hr)) {
			hr = UnregisterShellExtContextMenuHandler(L"Directory",
				CLSID_FilesInfoAndChecksum);
		}
    }

    return hr;
}