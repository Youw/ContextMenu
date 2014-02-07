/****************************** Module Header ******************************\
The code creating a Shell context menu handler with C++.
\***************************************************************************/

#define OEMRESOURCE
#include "ContextMenuHlr.h"
#include <strsafe.h>
#include <Shlwapi.h>
#include <WinUser.h>

#include "../Settings.h"

#pragma comment(lib, "shlwapi.lib")

#include "../JobsExecuting.h"

//extern HINSTANCE g_hInst;
extern long g_cDllRef;


namespace FileInfoAndChecksum {

#define IDM_DISPLAY             0  // The command's identifier offset

	ContextMenuHlr::ContextMenuHlr(void) : m_cRef(1),
		m_pszMenuText(MAKEWIDE(MenuText_CalcChecksum)),
		m_pszVerb(Verb_CalcChecksum),
		m_pwszVerb(MAKEWIDE(Verb_CalcChecksum)),
		m_pszVerbCanonicalName(VerbCanonicalName_CalcChecksum),
		m_pwszVerbCanonicalName(MAKEWIDE(VerbCanonicalName_CalcChecksum)),
		m_pszVerbHelpText(VerbHelpText_CalcChecksum),
		m_pwszVerbHelpText(MAKEWIDE(VerbHelpText_CalcChecksum))
	{
		InterlockedIncrement(&g_cDllRef);

		//   m_hMenuBmp = LoadImage(g_hInst, MAKEINTRESOURCE(IDB_OK), 
		//       IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADTRANSPARENT);

		// Load the bitmap for the menu item. 
		m_hMenuBmp = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_RGARROW));
	}

	ContextMenuHlr::~ContextMenuHlr(void)
	{
		if (m_hMenuBmp)
		{
			DeleteObject(m_hMenuBmp);
			m_hMenuBmp = NULL;
		}
		InterlockedDecrement(&g_cDllRef);
	}


	void ContextMenuHlr::OnMakeLogOfChecksums(HWND hWnd)
	{
		std::wstring szMessage = L"The selected file(s) is:\r\n\r\n";
		for (auto filename : m_SelectedFiles) {
			szMessage += filename + L"\r\n";
		}
		MessageBox(hWnd, szMessage.c_str(), L"Info", MB_OK);
	}


#pragma region IUnknown

	// Query to the interface the component supported.
	IFACEMETHODIMP ContextMenuHlr::QueryInterface(REFIID riid, void **ppv)
	{
		static const QITAB qit[] =
		{
			QITABENT(ContextMenuHlr, IContextMenu),
			QITABENT(ContextMenuHlr, IShellExtInit),
			{ 0 },
		};
		return QISearch(this, qit, riid, ppv);
	}

	// Increase the reference count for an interface on an object.
	IFACEMETHODIMP_(ULONG) ContextMenuHlr::AddRef()
	{
		return InterlockedIncrement(&m_cRef);
	}

	// Decrease the reference count for an interface on an object.
	IFACEMETHODIMP_(ULONG) ContextMenuHlr::Release()
	{
		ULONG cRef = InterlockedDecrement(&m_cRef);
		if (0 == cRef)
		{
			delete this;
		}

		return cRef;
	}

#pragma endregion


#pragma region IShellExtInit

	// Initialize the context menu handler.
	IFACEMETHODIMP ContextMenuHlr::Initialize(
		LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hKeyProgID)
	{
		if (NULL == pDataObj)
		{
			return E_INVALIDARG;
		}

		HRESULT hr = E_FAIL;

		FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM stm;

		// The pDataObj pointer contains the objects being acted upon. In this 
		// example, we get an HDROP handle for enumerating the selected files and 
		// folders.
		if (SUCCEEDED(pDataObj->GetData(&fe, &stm)))
		{
			// Get an HDROP handle.
			HDROP hDrop = static_cast<HDROP>(GlobalLock(stm.hGlobal));
			if (hDrop != NULL)
			{
				wchar_t m_szSelectedPath[MAX_PATH];
				const int a = 23;
				int b = a;
				// Determine how many files are involved in this operation. 
				UINT nFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
				for (unsigned i = 0; i < nFiles; i++)
				{
					hr = S_OK;

					// Store the path of the files and folders.
					if (0 != DragQueryFile(hDrop, i, m_szSelectedPath,
						ARRAYSIZE(m_szSelectedPath)))
					{
						if (!PathIsDirectory(m_szSelectedPath))
							m_SelectedFiles.push_back(m_szSelectedPath);
						else
							m_SelectedDirectories.push_back(m_szSelectedPath);
					}
					else {
						hr = E_FAIL;
						break;
					}
				}

				GlobalUnlock(stm.hGlobal);
			}

			ReleaseStgMedium(&stm);
		}

		// If any value other than S_OK is returned from the method, the context 
		// menu item is not displayed.
		return hr;
	}

#pragma endregion


#pragma region IContextMenu

	//
	//   FUNCTION: fiacsContextMenuHlr::QueryContextMenu
	//
	//   PURPOSE: The Shell calls IContextMenu::QueryContextMenu to allow the 
	//            context menu handler to add its menu items to the menu. It 
	//            passes in the HMENU handle in the hmenu parameter. The 
	//            indexMenu parameter is set to the index to be used for the 
	//            first menu item that is to be added.
	//
	IFACEMETHODIMP ContextMenuHlr::QueryContextMenu(
		HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
	{
		// If uFlags include CMF_DEFAULTONLY then we should not do anything.
		if (CMF_DEFAULTONLY & uFlags)
		{
			return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
		}

		// Use either InsertMenu or InsertMenuItem to add menu items.
		// Learn how to add sub-menu from:
		// http://www.codeproject.com/KB/shell/ctxextsubmenu.aspx

		MENUITEMINFO mii = { sizeof(mii) };
		mii.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
		mii.wID = idCmdFirst + IDM_DISPLAY;
		mii.fType = MFT_STRING;
		mii.dwTypeData = m_pszMenuText;
		mii.fState = MFS_ENABLED;
		mii.hbmpItem = static_cast<HBITMAP>(m_hMenuBmp);
		if (!InsertMenuItem(hMenu, indexMenu, TRUE, &mii))
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}

		// Add a separator.
		MENUITEMINFO sep = { sizeof(sep) };
		sep.fMask = MIIM_TYPE;
		sep.fType = MFT_SEPARATOR;
		if (!InsertMenuItem(hMenu, indexMenu + 1, TRUE, &sep))
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}

		// Return an HRESULT value with the severity set to SEVERITY_SUCCESS. 
		// Set the code value to the offset of the largest command identifier 
		// that was assigned, plus one (1).
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(IDM_DISPLAY + 1));
	}


	//
	//   FUNCTION: fiacsContextMenuHlr::InvokeCommand
	//
	//   PURPOSE: This method is called when a user clicks a menu item to tell 
	//            the handler to run the associated command. The lpcmi parameter 
	//            points to a structure that contains the needed information.
	//
	IFACEMETHODIMP ContextMenuHlr::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
	{
		BOOL fUnicode = FALSE;

		// Determine which structure is being passed in, CMINVOKECOMMANDINFO or 
		// CMINVOKECOMMANDINFOEX based on the cbSize member of lpcmi. Although 
		// the lpcmi parameter is declared in Shlobj.h as a CMINVOKECOMMANDINFO 
		// structure, in practice it often points to a CMINVOKECOMMANDINFOEX 
		// structure. This struct is an extended version of CMINVOKECOMMANDINFO 
		// and has additional members that allow Unicode strings to be passed.
		if (pici->cbSize == sizeof(CMINVOKECOMMANDINFOEX))
		{
			if (pici->fMask & CMIC_MASK_UNICODE)
			{
				fUnicode = TRUE;
			}
		}

		// Determines whether the command is identified by its offset or verb.
		// There are two ways to identify commands:
		// 
		//   1) The command's verb string 
		//   2) The command's identifier offset
		// 
		// If the high-order word of lpcmi->lpVerb (for the ANSI case) or 
		// lpcmi->lpVerbW (for the Unicode case) is nonzero, lpVerb or lpVerbW 
		// holds a verb string. If the high-order word is zero, the command 
		// offset is in the low-order word of lpcmi->lpVerb.

		// For the ANSI case, if the high-order word is not zero, the command's 
		// verb string is in lpcmi->lpVerb. 
		if (!fUnicode && HIWORD(pici->lpVerb))
		{
			// Is the verb supported by this context menu extension?
			if (StrCmpIA(pici->lpVerb, m_pszVerb) == 0)
			{
				OnMakeLogOfChecksums(pici->hwnd);
			}
			else
			{
				// If the verb is not recognized by the context menu handler, it 
				// must return E_FAIL to allow it to be passed on to the other 
				// context menu handlers that might implement that verb.
				return E_FAIL;
			}
		}

		// For the Unicode case, if the high-order word is not zero, the 
		// command's verb string is in lpcmi->lpVerbW. 
		else if (fUnicode && HIWORD(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW))
		{
			// Is the verb supported by this context menu extension?
			if (StrCmpIW(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW, m_pwszVerb) == 0)
			{
				OnMakeLogOfChecksums(pici->hwnd);
			}
			else
			{
				// If the verb is not recognized by the context menu handler, it 
				// must return E_FAIL to allow it to be passed on to the other 
				// context menu handlers that might implement that verb.
				return E_FAIL;
			}
		}

		// If the command cannot be identified through the verb string, then 
		// check the identifier offset.
		else
		{
			// Is the command identifier offset supported by this context menu 
			// extension?
			if (LOWORD(pici->lpVerb) == IDM_DISPLAY)
			{
				OnMakeLogOfChecksums(pici->hwnd);
			}
			else
			{
				// If the verb is not recognized by the context menu handler, it 
				// must return E_FAIL to allow it to be passed on to the other 
				// context menu handlers that might implement that verb.
				return E_FAIL;
			}
		}

		return S_OK;
	}


	//
	//   FUNCTION: CFileContextMenuExt::GetCommandString
	//
	//   PURPOSE: If a user highlights one of the items added by a context menu 
	//            handler, the handler's IContextMenu::GetCommandString method is 
	//            called to request a Help text string that will be displayed on 
	//            the Windows Explorer status bar. This method can also be called 
	//            to request the verb string that is assigned to a command. 
	//            Either ANSI or Unicode verb strings can be requested. This 
	//            example only implements support for the Unicode values of 
	//            uFlags, because only those have been used in Windows Explorer 
	//            since Windows 2000.
	//
	IFACEMETHODIMP ContextMenuHlr::GetCommandString(UINT_PTR idCommand,
		UINT uFlags, UINT *pwReserved, LPSTR pszName, UINT cchMax)
	{
		HRESULT hr = E_INVALIDARG;

		if (idCommand == IDM_DISPLAY)
		{
			switch (uFlags)
			{
			case GCS_HELPTEXTW:
				// Only useful for pre-Vista versions of Windows that have a 
				// Status bar.
				hr = StringCchCopyW(reinterpret_cast<PWSTR>(pszName), cchMax,
					m_pwszVerbHelpText);
				break;

			case GCS_VERBW:
				// GCS_VERBW is an optional feature that enables a caller to 
				// discover the canonical name for the verb passed in through 
				// idCommand.
				hr = StringCchCopyW(reinterpret_cast<PWSTR>(pszName), cchMax,
					m_pwszVerbCanonicalName);
				break;

				// (ANSI)
			case GCS_HELPTEXTA:
				hr = StringCchCopyA(pszName, cchMax, m_pszVerbHelpText);
				break;
			case GCS_VERBA:
				hr = StringCchCopyA(pszName, cchMax, m_pszVerbCanonicalName);
				break;
			default:
				hr = S_OK;
			}
		}

		// If the command (idCommand) is not supported by this context menu 
		// extension handler, return E_INVALIDARG.
		// Too bad...
		return hr;
	}

#pragma endregion

}