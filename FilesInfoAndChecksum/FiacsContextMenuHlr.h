/****************************** Module Header ******************************\
The code creating a Shell context menu handler with C++. 
\***************************************************************************/

#pragma once

#include <windows.h>
#include <shlobj.h>     // For IShellExtInit and IContextMenu
#include <vector>
#include <string>

//  "Files info and checksum" context menu handler
class FiacsContextMenuHlr : public IShellExtInit, public IContextMenu
{
public:
    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // IShellExtInit
    IFACEMETHODIMP Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hKeyProgID);

    // IContextMenu
    IFACEMETHODIMP QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    IFACEMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO pici);
    IFACEMETHODIMP GetCommandString(UINT_PTR idCommand, UINT uFlags, UINT *pwReserved, LPSTR pszName, UINT cchMax);
	
    FiacsContextMenuHlr(void);

protected:
    ~FiacsContextMenuHlr(void);

private:
    // Reference count of component.
    long m_cRef;

    // The list of the names of selected files.
    std::vector<std::wstring> m_SelectedFiles;

	// The list of the names of selected directories.
	std::vector<std::wstring> m_SelectedDirectories;

    // The method that handles the "display" verb.
    void OnMakeLogOfChecksums(HWND hWnd);

    PWSTR m_pszMenuText;
    HANDLE m_hMenuBmp;
    PCSTR m_pszVerb;
    PCWSTR m_pwszVerb;
    PCSTR m_pszVerbCanonicalName;
    PCWSTR m_pwszVerbCanonicalName;
    PCSTR m_pszVerbHelpText;
    PCWSTR m_pwszVerbHelpText;
};