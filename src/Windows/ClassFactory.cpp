/****************************** Module Header ******************************\
The file implements the class factory for the ContextMenuHlr COM class. 
\***************************************************************************/

#include "ClassFactory.h"
#include "ContextMenuHlr.h"
#include <new>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")


extern long g_cDllRef;


ClassFactory::ClassFactory() : m_cRef(1)
{
    InterlockedIncrement(&g_cDllRef);
}

ClassFactory::~ClassFactory()
{
    InterlockedDecrement(&g_cDllRef);
}


//
// IUnknown
//

IFACEMETHODIMP ClassFactory::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(ClassFactory, IClassFactory),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

IFACEMETHODIMP_(ULONG) ClassFactory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

IFACEMETHODIMP_(ULONG) ClassFactory::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
    {
        delete this;
    }
    return cRef;
}


// 
// IClassFactory
//

IFACEMETHODIMP ClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv)
{
    HRESULT hr = CLASS_E_NOAGGREGATION;

    // pUnkOuter is used for aggregation. We do not support it here
    if (pUnkOuter == NULL)
    {
		using FileInfoAndChecksum::ContextMenuHlr;
		hr = E_OUTOFMEMORY;
		// Create the COM component.
        ContextMenuHlr *MenuHandler = new (std::nothrow) ContextMenuHlr();
		if (MenuHandler)
        {
            // Query the specified interface.
			hr = MenuHandler->QueryInterface(riid, ppv);
			MenuHandler->Release();
        }
    }

    return hr;
}

IFACEMETHODIMP ClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
    {
        InterlockedIncrement(&g_cDllRef);
    }
    else
    {
        InterlockedDecrement(&g_cDllRef);
    }
    return S_OK;
}