//***************************************************************************

//

//  MAINDLL.CPP

// 

//  Module: WMI Instance provider sample code

//

//  Purpose: Contains DLL entry points.  Also has code that controls

//           when the DLL can be unloaded by tracking the number of

//           objects and locks as well as routines that support

//           self registration.

//

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
//***************************************************************************

#include "pch.h"
#include <objbase.h>
#include <initguid.h>
#include <strsafe.h>
#include <fstream>
#include "Sample.h"


#pragma warning(disable: 4996) 
HMODULE ghModule;

// TODO, GuidGen should be used to generate a unique number for any 
// providers that are going to be used for anything more extensive 
// than just testing.

DEFINE_GUID(CLSID_instprovider, 0x22cb8761, 0x914a, 0x11cf, 0xb7, 0x5, 0x0, 0xaa, 0x0, 0x62, 0xcb, 0xb7);
// {22CB8761-914A-11cf-B705-00AA0062CBB7}

//Count number of objects and number of locks./Zc:sizedDealloc- 

long       g_cObj = 0;
long       g_cLock = 0;

//***************************************************************************
//
// LibMain32
//
// Purpose: Entry point for DLL.
//
// Return: TRUE if OK.
//
//***************************************************************************


BOOL WINAPI LibMain32(HINSTANCE hInstance, ULONG ulReason
    , LPVOID pvReserved)
{
    if (DLL_PROCESS_ATTACH == ulReason)
        ghModule = hInstance;
    return TRUE;
}

//***************************************************************************
//
//  DllGetClassObject
//
//  Purpose: Called by Ole when some client wants a class factory.  Return 
//           one only if it is the sort of class this DLL supports.
//
//***************************************************************************


STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, PPVOID ppv)
{
    HRESULT hr;
    CProvFactory* pObj;

    if (CLSID_instprovider != rclsid)
        return E_FAIL;

    pObj = new CProvFactory();

    if (NULL == pObj)
        return E_OUTOFMEMORY;

    hr = pObj->QueryInterface(riid, ppv);

    if (FAILED(hr))
        delete pObj;

    return hr;
}

//***************************************************************************
//
// DllCanUnloadNow
//
// Purpose: Called periodically by Ole in order to determine if the
//          DLL can be freed.
//
// Return:  S_OK if there are no objects in use and the class factory 
//          isn't locked.
//
//***************************************************************************

STDAPI DllCanUnloadNow(void)
{
    SCODE   sc;

    //It is OK to unload if there are no objects or locks on the 
    // class factory.

    sc = (0L == g_cObj && 0L == g_cLock) ? S_OK : S_FALSE;
    return sc;
}

//***************************************************************************
//
// DllRegisterServer
//
// Purpose: Called during setup or by regsvr32.
//
// Return:  NOERROR if registration successful, error otherwise.
//***************************************************************************

STDAPI DllRegisterServer(void)
{
    char       szID[128];
    WCHAR		wcID[128];
    char       szCLSID[128];
    TCHAR       szModule[MAX_PATH + 1];
    const char* pName = "WMI Sample Instance Provider";
    const char* pModel = "Both";
    HKEY hKey1, hKey2;
    size_t* intReturnValue = NULL;


    // Create the path.

    memset(wcID, NULL, sizeof(wcID));
    memset(szID, NULL, sizeof(szID));
    StringFromGUID2(CLSID_instprovider, wcID, sizeof(wcID) / sizeof(WCHAR));
    wcstombs_s(intReturnValue, szID, sizeof(szID), wcID, sizeof(szID));
    
    wchar_t wcCLSID[256];
    mbstowcs(wcCLSID, szCLSID, strlen(szCLSID) + 1);
    LPWSTR lpCLSID = wcCLSID;
    wchar_t wszID[128];
    mbstowcs(wszID, szID, strlen(szID) + 1);
    LPWSTR lpsID = wszID;

    //StringCbCopy(szCLSID, sizeof(szCLSID), TEXT("Software\\classes\\CLSID\\"));
    StringCbCopy(lpCLSID, sizeof(lpCLSID), TEXT("Software\\classes\\CLSID\\"));
    StringCbCat(lpCLSID, sizeof(lpCLSID), lpsID);

    // Create entries under CLSID
    std::ofstream fout;



    fout.open("P:\\temp\\fileService.txt");
    int len = WideCharToMultiByte(CP_UTF8, 0, lpCLSID, -1, NULL, 0, 0, 0);
    fout << len << wcID << std::endl;
    if (len > 0)
    {
        LPSTR result = NULL;
        len = len + 1;
        result = new char[len];
        if (result)
        {
            int resLen = WideCharToMultiByte(CP_UTF8, 0, lpCLSID, -1, &result[0], len, 0, 0);

            if (resLen == len)
            {
                fout << result << len << std::endl;

            }

            delete[] result;
        }
    }

    fout << lpsID << std::endl;
    fout << lpCLSID << std::endl;
    HRESULT hr = RegCreateKeyEx(HKEY_LOCAL_MACHINE, lpCLSID, 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
        &hKey1, NULL);
    if (hr != ERROR_SUCCESS)
    {


        fout << "bloack first clsid creation" << GetLastError() << hr << E_FAIL << std::endl;
        fout.close();
        return E_FAIL;
    }


    LONG lRet = RegSetValueEx(hKey1, NULL, 0, REG_SZ, (BYTE*)pName, strlen(pName) + 1);
    if (lRet != ERROR_SUCCESS)
    {
        RegCloseKey(hKey1);
        fout << "default description" << GetLastError() <<E_FAIL << std::endl;
        fout.close();
        return E_FAIL;
    }
    LPCWSTR subKey = TEXT("InprocServer32");
    lRet = RegCreateKeyEx(hKey1, subKey , 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
        &hKey2, NULL);
    if (lRet != ERROR_SUCCESS)
    {
        RegCloseKey(hKey1);
        fout << "InprocServer32" << GetLastError() << E_FAIL << std::endl;
        fout.close();
        return E_FAIL;
    }

    memset(&szModule, NULL, sizeof(szModule));
    GetModuleFileName(ghModule, szModule, sizeof(szModule) / sizeof(TCHAR) - 1);
    lRet = RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE*)szModule,
        sizeof(szModule) + 1);
    if (lRet != ERROR_SUCCESS)
    {
        RegCloseKey(hKey2);
        RegCloseKey(hKey1);
        fout << "key2 module" << GetLastError() << E_FAIL << std::endl;
        fout.close();
        return E_FAIL;
    }
    lRet = RegSetValueEx(hKey2, L"ThreadingModel", 0, REG_SZ,
        (BYTE*)pModel, sizeof(pModel) + 1);
    if (lRet != ERROR_SUCCESS)
    {
        RegCloseKey(hKey2);
        RegCloseKey(hKey1);
        fout << "Threadin model" << E_FAIL << std::endl;
        fout.close();
        return E_FAIL;
    }
    RegCloseKey(hKey1);
    RegCloseKey(hKey2);
    return NOERROR;
}

//***************************************************************************
//
// DllUnregisterServer
//
// Purpose: Called when it is time to remove the registry entries.
//
// Return:  NOERROR if registration successful, error otherwise.
//***************************************************************************

STDAPI DllUnregisterServer(void)
{
    TCHAR	szID[128];
    WCHAR	wcID[128];
    TCHAR	szCLSID[128];
    HKEY	hKey;
    size_t* intReturnValue = NULL;

    // Create the path using the CLSID

    memset(wcID, NULL, sizeof(wcID));
    memset(szID, NULL, sizeof(szID));
    StringFromGUID2(CLSID_instprovider, wcID, sizeof(wcID) / sizeof(WCHAR));
//    wcstombs_s(intReturnValue, szID, sizeof(szID), wcID, sizeof(szID));
    wcstombs((char*)szID, wcID, sizeof(szID));
    StringCbCopy(szCLSID, sizeof(szCLSID), TEXT("Software\\classes\\CLSID\\"));
    StringCbCat(szCLSID, sizeof(szCLSID), (LPCTSTR)szID);


    // First delete the InProcServer subkey.

    DWORD dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szCLSID, 0, KEY_WRITE, &hKey);
    if (dwRet == NO_ERROR)
    {
        RegDeleteKey(hKey, TEXT("InProcServer32"));
        RegCloseKey(hKey);
    }

    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\classes\\CLSID"), 0, KEY_WRITE, &hKey);
    if (dwRet == NO_ERROR)
    {
        RegDeleteKey(hKey, szID);
        RegCloseKey(hKey);
    }

    return NOERROR;
}