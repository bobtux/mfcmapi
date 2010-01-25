#include "stdafx.h"
#include "ImportProcs.h"
#include "MAPIFunctions.h"
#include <mapival.h>

// Note: when the user loads MAPI manually, hModMSMAPI and hModMAPI will be the same
HMODULE	hModMSMAPI = NULL; // Address of Outlook's MAPI
HMODULE	hModMAPI = NULL; // Address of MAPI32 in System32
HMODULE	hModMAPIStub = NULL; // Address of the MAPI stub library

HMODULE	hModAclui = NULL;
HMODULE hModRichEd20 = NULL;
HMODULE hModOle32 = NULL;
HMODULE hModUxTheme = NULL;
HMODULE hModInetComm = NULL;
HMODULE hModMSI = NULL;

// For GetComponentPath
typedef BOOL (STDAPICALLTYPE FGETCOMPONENTPATH)
	(LPSTR szComponent,
	LPSTR szQualifier,
	LPSTR szDllPath,
	DWORD cchBufferSize,
	BOOL fInstall);
typedef FGETCOMPONENTPATH FAR * LPFGETCOMPONENTPATH;

// For all the MAPI funcs we use
typedef SCODE (STDMETHODCALLTYPE HRGETONEPROP)(
					LPMAPIPROP lpMapiProp,
					ULONG ulPropTag,
					LPSPropValue FAR *lppProp);
typedef HRGETONEPROP *LPHRGETONEPROP;

typedef void (STDMETHODCALLTYPE FREEPROWS)(
					LPSRowSet lpRows);
typedef FREEPROWS	*LPFREEPROWS;

typedef	LPSPropValue (STDMETHODCALLTYPE PPROPFINDPROP)(
					LPSPropValue lpPropArray, ULONG cValues,
					ULONG ulPropTag);
typedef PPROPFINDPROP *LPPPROPFINDPROP;

typedef SCODE (STDMETHODCALLTYPE SCDUPPROPSET)(
					int cValues, LPSPropValue lpPropArray,
					LPALLOCATEBUFFER lpAllocateBuffer, LPSPropValue FAR *lppPropArray);
typedef SCDUPPROPSET *LPSCDUPPROPSET;

typedef SCODE (STDMETHODCALLTYPE SCCOUNTPROPS)(
					int cValues, LPSPropValue lpPropArray, ULONG FAR *lpcb);
typedef SCCOUNTPROPS *LPSCCOUNTPROPS;

typedef SCODE (STDMETHODCALLTYPE SCCOPYPROPS)(
					int cValues, LPSPropValue lpPropArray, LPVOID lpvDst,
					ULONG FAR *lpcb);
typedef SCCOPYPROPS *LPSCCOPYPROPS;

typedef SCODE (STDMETHODCALLTYPE OPENIMSGONISTG)(
					LPMSGSESS		lpMsgSess,
					LPALLOCATEBUFFER lpAllocateBuffer,
					LPALLOCATEMORE 	lpAllocateMore,
					LPFREEBUFFER	lpFreeBuffer,
					LPMALLOC		lpMalloc,
					LPVOID			lpMapiSup,
					LPSTORAGE 		lpStg,
					MSGCALLRELEASE FAR *lpfMsgCallRelease,
					ULONG			ulCallerData,
					ULONG			ulFlags,
					LPMESSAGE		FAR *lppMsg );
typedef OPENIMSGONISTG *LPOPENIMSGONISTG;

typedef	LPMALLOC (STDMETHODCALLTYPE MAPIGETDEFAULTMALLOC)(
					void);
typedef MAPIGETDEFAULTMALLOC *LPMAPIGETDEFAULTMALLOC;

typedef	void (STDMETHODCALLTYPE CLOSEIMSGSESSION)(
					LPMSGSESS		lpMsgSess);
typedef CLOSEIMSGSESSION *LPCLOSEIMSGSESSION;

typedef	SCODE (STDMETHODCALLTYPE OPENIMSGSESSION)(
					LPMALLOC		lpMalloc,
					ULONG			ulFlags,
					LPMSGSESS FAR	*lppMsgSess);
typedef OPENIMSGSESSION *LPOPENIMSGSESSION;

typedef SCODE (STDMETHODCALLTYPE HRQUERYALLROWS)(
					LPMAPITABLE lpTable,
					LPSPropTagArray lpPropTags,
					LPSRestriction lpRestriction,
					LPSSortOrderSet lpSortOrderSet,
					LONG crowsMax,
					LPSRowSet FAR *lppRows);
typedef HRQUERYALLROWS *LPHRQUERYALLROWS;

typedef SCODE (STDMETHODCALLTYPE MAPIOPENFORMMGR)(
					LPMAPISESSION pSession, LPMAPIFORMMGR FAR * ppmgr);
typedef MAPIOPENFORMMGR *LPMAPIOPENFORMMGR;

typedef HRESULT (STDMETHODCALLTYPE RTFSYNC) (
	LPMESSAGE lpMessage, ULONG ulFlags, BOOL FAR * lpfMessageUpdated);
typedef RTFSYNC *LPRTFSYNC;

typedef SCODE (STDMETHODCALLTYPE HRSETONEPROP)(
					LPMAPIPROP lpMapiProp,
					LPSPropValue lpProp);
typedef HRSETONEPROP *LPHRSETONEPROP;

typedef void (STDMETHODCALLTYPE FREEPADRLIST)(
					LPADRLIST lpAdrList);
typedef FREEPADRLIST *LPFREEPADRLIST;

typedef SCODE (STDMETHODCALLTYPE PROPCOPYMORE)(
					LPSPropValue		lpSPropValueDest,
					LPSPropValue		lpSPropValueSrc,
					ALLOCATEMORE *	lpfAllocMore,
					LPVOID			lpvObject );
typedef PROPCOPYMORE *LPPROPCOPYMORE;

typedef HRESULT (STDMETHODCALLTYPE WRAPCOMPRESSEDRTFSTREAM) (
	LPSTREAM lpCompressedRTFStream, ULONG ulFlags, LPSTREAM FAR * lpUncompressedRTFStream);
typedef WRAPCOMPRESSEDRTFSTREAM *LPWRAPCOMPRESSEDRTFSTREAM;

typedef SCODE (STDMETHODCALLTYPE HRVALIDATEIPMSUBTREE)(
					LPMDB lpMDB, ULONG ulFlags,
					ULONG FAR *lpcValues, LPSPropValue FAR *lppValues,
					LPMAPIERROR FAR *lpperr);
typedef HRVALIDATEIPMSUBTREE *LPHRVALIDATEIPMSUBTREE;

typedef HRESULT (STDAPICALLTYPE MAPIOPENLOCALFORMCONTAINER)(LPMAPIFORMCONTAINER FAR * ppfcnt);
typedef MAPIOPENLOCALFORMCONTAINER *LPMAPIOPENLOCALFORMCONTAINER;

typedef SCODE (STDAPICALLTYPE HRDISPATCHNOTIFICATIONS)(ULONG ulFlags);
typedef HRDISPATCHNOTIFICATIONS *LPHRDISPATCHNOTIFICATIONS;

typedef HRESULT (STDMETHODCALLTYPE WRAPSTOREENTRYID)(
	ULONG ulFlags,
	__in LPTSTR lpszDLLName,
	ULONG cbOrigEntry,
	LPENTRYID lpOrigEntry,
	ULONG *lpcbWrappedEntry,
	LPENTRYID *lppWrappedEntry);
typedef WRAPSTOREENTRYID *LPWRAPSTOREENTRYID;

typedef SCODE (STDMETHODCALLTYPE CREATEIPROP)(
	LPCIID					lpInterface,
	ALLOCATEBUFFER FAR *	lpAllocateBuffer,
	ALLOCATEMORE FAR *		lpAllocateMore,
	FREEBUFFER FAR *		lpFreeBuffer,
	LPVOID					lpvReserved,
	LPPROPDATA FAR *		lppPropData);
typedef CREATEIPROP *LPCREATEIPROP;

typedef SCODE (STDMETHODCALLTYPE CREATETABLE)(
	LPCIID					lpInterface,
	ALLOCATEBUFFER FAR *	lpAllocateBuffer,
	ALLOCATEMORE FAR *		lpAllocateMore,
	FREEBUFFER FAR *		lpFreeBuffer,
	LPVOID					lpvReserved,
	ULONG					ulTableType,
	ULONG					ulPropTagIndexColumn,
	LPSPropTagArray		lpSPropTagArrayColumns,
	LPTABLEDATA FAR *		lppTableData);
typedef CREATETABLE *LPCREATETABLE;

typedef HRESULT (STDMETHODCALLTYPE HRVALIDATEPARAMETERS)(
	METHODS eMethod,
	LPVOID FAR *ppFirstArg);
typedef HRVALIDATEPARAMETERS *LPHRVALIDATEPARAMETERS;

typedef HRESULT (STDMETHODCALLTYPE BUILDDISPLAYTABLE)(
	LPALLOCATEBUFFER	lpAllocateBuffer,
	LPALLOCATEMORE		lpAllocateMore,
	LPFREEBUFFER		lpFreeBuffer,
	LPMALLOC			lpMalloc,
	HINSTANCE			hInstance,
	UINT				cPages,
	LPDTPAGE			lpPage,
	ULONG				ulFlags,
	LPMAPITABLE *		lppTable,
	LPTABLEDATA	*		lppTblData);
typedef BUILDDISPLAYTABLE *LPBUILDDISPLAYTABLE;

typedef int (STDMETHODCALLTYPE MNLS_LSTRLENW)(
	LPCWSTR	lpString);
typedef MNLS_LSTRLENW *LPMNLS_LSTRLENW;

typedef HRESULT (STDMETHODCALLTYPE FAR * LPOPENSTREAMONFILEW) (
	LPALLOCATEBUFFER	lpAllocateBuffer,
	LPFREEBUFFER		lpFreeBuffer,
	ULONG				ulFlags,
	__in LPCWSTR		lpszFileName,
	__in_opt LPCWSTR	lpszPrefix,
	LPSTREAM FAR *		lppStream);

LPEDITSECURITY				pfnEditSecurity = NULL;

LPSTGCREATESTORAGEEX		pfnStgCreateStorageEx = NULL;

LPOPENTHEMEDATA				pfnOpenThemeData = NULL;
LPCLOSETHEMEDATA			pfnCloseThemeData = NULL;
LPGETTHEMEMARGINS			pfnGetThemeMargins = NULL;

// From inetcomm.dll
LPMIMEOLEGETCODEPAGECHARSET pfnMimeOleGetCodePageCharset = NULL;

// From MSI.dll
LPMSIPROVIDEQUALIFIEDCOMPONENT pfnMsiProvideQualifiedComponent = NULL;
LPMSIGETFILEVERSION pfnMsiGetFileVersion = NULL;

// All of these get loaded from a MAPI DLL:
LPFGETCOMPONENTPATH			pfnFGetComponentPath = NULL;
LPLAUNCHWIZARDENTRY			pfnLaunchWizard = NULL;
LPMAPIALLOCATEBUFFER		pfnMAPIAllocateBuffer = NULL;
LPMAPIALLOCATEMORE			pfnMAPIAllocateMore = NULL;
LPMAPIFREEBUFFER			pfnMAPIFreeBuffer = NULL;
LPHRGETONEPROP				pfnHrGetOneProp = NULL;
LPMAPIINITIALIZE			pfnMAPIInitialize = NULL;
LPMAPIUNINITIALIZE			pfnMAPIUninitialize = NULL;
LPFREEPROWS					pfnLPFreeProws = NULL;
LPPPROPFINDPROP				pfnPpropFindProp = NULL;
LPSCDUPPROPSET				pfnScDupPropset = NULL;
LPSCCOUNTPROPS				pfnScCountProps = NULL;
LPSCCOPYPROPS				pfnScCopyProps = NULL;
LPOPENIMSGONISTG			pfnOpenIMsgOnIStg = NULL;
LPMAPIGETDEFAULTMALLOC		pfnMAPIGetDefaultMalloc = NULL;
LPOPENTNEFSTREAMEX			pfnOpenTnefStreamEx = NULL;
LPOPENSTREAMONFILE			pfnOpenStreamOnFile = NULL;
LPOPENSTREAMONFILEW			pfnOpenStreamOnFileW = NULL;
LPCLOSEIMSGSESSION			pfnCloseIMsgSession = NULL;
LPOPENIMSGSESSION			pfnOpenIMsgSession = NULL;
LPHRQUERYALLROWS			pfnHrQueryAllRows = NULL;
LPMAPIOPENFORMMGR			pfnMAPIOpenFormMgr = NULL;
LPRTFSYNC					pfnRTFSync = NULL;
LPHRSETONEPROP				pfnHrSetOneProp = NULL;
LPFREEPADRLIST				pfnFreePadrlist = NULL;
LPPROPCOPYMORE				pfnPropCopyMore = NULL;
LPWRAPCOMPRESSEDRTFSTREAM	pfnWrapCompressedRTFStream = NULL;
LPWRAPSTOREENTRYID			pfnWrapStoreEntryID = NULL;
LPCREATEIPROP				pfnCreateIProp = NULL;
LPCREATETABLE				pfnCreateTable = NULL;
LPHRVALIDATEPARAMETERS		pfnHrValidateParameters = NULL;
LPBUILDDISPLAYTABLE			pfnBuildDisplayTable = NULL;
LPMNLS_LSTRLENW				pfnMNLS_lstrlenW = NULL;
LPMAPILOGONEX				pfnMAPILogonEx = NULL;
LPMAPIADMINPROFILES			pfnMAPIAdminProfiles = NULL;
LPHRVALIDATEIPMSUBTREE		pfnHrValidateIPMSubtree = NULL;
LPWRAPCOMPRESSEDRTFSTREAMEX	pfnWrapEx = NULL;
LPMAPIOPENLOCALFORMCONTAINER pfnMAPIOpenLocalFormContainer = NULL;
LPHRDISPATCHNOTIFICATIONS	pfnHrDispatchNotifications = NULL;

// Exists to allow some logging
HMODULE MyLoadLibrary(LPCTSTR lpLibFileName)
{
	HMODULE hMod = NULL;
	HRESULT hRes = S_OK;
	DebugPrint(DBGLoadLibrary,_T("MyLoadLibrary - loading \"%s\"\n"),lpLibFileName);
	WC_D(hMod,LoadLibrary(lpLibFileName));
	if (hMod)
	{
		DebugPrint(DBGLoadLibrary,_T("MyLoadLibrary - \"%s\" loaded at 0x%08X\n"),lpLibFileName,hMod);
	}
	else
	{
		DebugPrint(DBGLoadLibrary,_T("MyLoadLibrary - \"%s\" failed to load\n"),lpLibFileName);
	}
	return hMod;
}

void LoadGetComponentPath()
{
	HRESULT hRes = S_OK;

	if (pfnFGetComponentPath) return;

	if (!hModMAPI) hModMAPI = LoadFromSystemDir(_T("mapi32.dll")); // STRING_OK
	if (hModMAPI)
	{
		WC_D(pfnFGetComponentPath, (LPFGETCOMPONENTPATH) GetProcAddress(
			hModMAPI,
			"FGetComponentPath")); // STRING_OK
		hRes = S_OK;
	}
	if (!pfnFGetComponentPath)
	{
		if (!hModMAPIStub) hModMAPIStub = LoadFromSystemDir(_T("mapistub.dll")); // STRING_OK
		if (hModMAPIStub)
		{
			WC_D(pfnFGetComponentPath, (LPFGETCOMPONENTPATH) GetProcAddress(
				hModMAPIStub,
				"FGetComponentPath")); // STRING_OK
		}
	}

	DebugPrint(DBGLoadLibrary,_T("FGetComponentPath loaded at 0x%08X\n"),pfnFGetComponentPath);
} // LoadGetComponentPath

BOOL GetComponentPath(
					  LPTSTR szComponent,
					  LPTSTR szQualifier,
					  TCHAR* szDllPath,
					  DWORD cchDLLPath)
{
	HRESULT hRes = S_OK;
	LoadGetComponentPath();

	if (!pfnFGetComponentPath) return false;
#ifdef UNICODE
	int iRet = 0;
	CHAR szAsciiPath[MAX_PATH] = {0};
	char* szAnsiComponent = NULL;
	char* szAnsiQualifier = NULL;
	EC_H(UnicodeToAnsi(szComponent,&szAnsiComponent));
	EC_H(UnicodeToAnsi(szQualifier,&szAnsiQualifier));

	EC_B(pfnFGetComponentPath(
		szAnsiComponent,
		szAnsiQualifier,
		szAsciiPath,
		_countof(szAsciiPath),
		TRUE));
	delete[] szAnsiQualifier;
	delete[] szAnsiComponent;
	// Convert to Unicode.
	EC_D(iRet,MultiByteToWideChar(
		CP_ACP,
		0,
		szAsciiPath,
		_countof(szAsciiPath),
		szDllPath,
		cchDLLPath));
#else
	EC_B(pfnFGetComponentPath(
		szComponent,
		szQualifier,
		szDllPath,
		cchDLLPath,
		TRUE));
#endif
	return SUCCEEDED(hRes);
} // GetComponentPath

// We do this to avoid certain problems Outlook 11's MAPI has when the system RichEd is loaded.
// Note that we don't worry about ever unloading this
void LoadRichEd()
{
	if (hModRichEd20) return;
	HRESULT hRes = S_OK;
	HKEY hCurrentVersion = NULL;

	WC_W32(RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		_T("Software\\Microsoft\\Windows\\CurrentVersion"), // STRING_OK
		NULL,
		KEY_READ,
		&hCurrentVersion));
	hRes = S_OK;

	if (hCurrentVersion)
	{
		DWORD dwKeyType = NULL;
		LPTSTR szCF = NULL;

		WC_H(HrGetRegistryValue(
			hCurrentVersion,
			_T("CommonFilesDir"), // STRING_OK
			&dwKeyType,
			(LPVOID*) &szCF));
		hRes = S_OK;

		if (szCF)
		{
			size_t cchCF = NULL;
			size_t cchOffice11 = NULL;

			LPTSTR szOffice11RichEd = _T("\\Microsoft Shared\\office11\\riched20.dll"); // STRING_OK

			EC_H(StringCchLength(szCF,STRSAFE_MAX_CCH,&cchCF));
			EC_H(StringCchLength(szOffice11RichEd,STRSAFE_MAX_CCH,&cchOffice11));

			LPTSTR szRichEdFullPath = NULL;
			szRichEdFullPath = new TCHAR[cchOffice11+cchCF+1];

			if (szRichEdFullPath)
			{
				EC_H(StringCchPrintf(szRichEdFullPath,cchOffice11+cchCF+1,_T("%s%s"),szCF,szOffice11RichEd)); // STRING_OK
				hModRichEd20 = MyLoadLibrary(szRichEdFullPath);
				delete[] szRichEdFullPath;
			}

			if (!hModRichEd20)
			{
				LPTSTR szOffice11RichEdDebug = _T("\\Microsoft Shared Debug\\office11\\riched20.dll"); // STRING_OK
				EC_H(StringCchLength(szOffice11RichEdDebug,STRSAFE_MAX_CCH,&cchOffice11));

				LPTSTR szRichEdDebugFullPath = NULL;
				szRichEdDebugFullPath = new TCHAR[cchOffice11+cchCF+1];

				if (szRichEdDebugFullPath)
				{
					EC_H(StringCchPrintf(szRichEdDebugFullPath,cchOffice11+cchCF+1,_T("%s%s"),szCF,szOffice11RichEdDebug)); // STRING_OK
					hModRichEd20 = MyLoadLibrary(szRichEdDebugFullPath);
					delete[] szRichEdDebugFullPath;
				}
			}

			delete[] szCF;
		}

		EC_W32(RegCloseKey(hCurrentVersion));
	}
}

HMODULE LoadFromSystemDir(LPTSTR szDLLName)
{
	if (!szDLLName) return NULL;

	HRESULT	hRes = S_OK;
	HMODULE	hModRet = NULL;
	TCHAR	szDLLPath[MAX_PATH] = {0};
	UINT	uiRet = NULL;

	static TCHAR	szSystemDir[MAX_PATH] = {0};
	static BOOL		bSystemDirLoaded = false;

	DebugPrint(DBGLoadLibrary,_T("LoadFromSystemDir - loading \"%s\"\n"),szDLLName);

	if (!bSystemDirLoaded)
	{
		WC_D(uiRet,GetSystemDirectory(szSystemDir, MAX_PATH));
		bSystemDirLoaded = true;
	}

	WC_H(StringCchPrintf(szDLLPath,_countof(szDLLPath),_T("%s\\%s"),szSystemDir,szDLLName)); // STRING_OK
	DebugPrint(DBGLoadLibrary,_T("LoadFromSystemDir - loading from \"%s\"\n"),szDLLPath);
	hModRet = MyLoadLibrary(szDLLPath);

	return hModRet;
}

void LoadAclUI()
{
	HRESULT hRes = S_OK;
	if (!hModAclui) hModAclui = LoadFromSystemDir(_T("aclui.dll")); // STRING_OK
	if (hModAclui)
	{
		WC_D(pfnEditSecurity, (LPEDITSECURITY) GetProcAddress(
			hModAclui,
			"EditSecurity")); // STRING_OK
	}
}

void LoadStg()
{
	HRESULT hRes = S_OK;
	if (!hModOle32) hModOle32 = LoadFromSystemDir(_T("ole32.dll")); // STRING_OK
	if (hModOle32)
	{
		WC_D(pfnStgCreateStorageEx, (LPSTGCREATESTORAGEEX) GetProcAddress(
			hModOle32,
			"StgCreateStorageEx")); // STRING_OK
	}
}

void LoadThemeUI()
{
	HRESULT hRes = S_OK;
	if (!hModUxTheme) hModUxTheme = LoadFromSystemDir(_T("uxtheme.dll")); // STRING_OK
	if (hModUxTheme)
	{
		WC_D(pfnOpenThemeData, (LPOPENTHEMEDATA) GetProcAddress(
			hModUxTheme,
			"OpenThemeData")); // STRING_OK
		WC_D(pfnCloseThemeData, (LPCLOSETHEMEDATA) GetProcAddress(
			hModUxTheme,
			"CloseThemeData")); // STRING_OK
		WC_D(pfnGetThemeMargins, (LPGETTHEMEMARGINS) GetProcAddress(
			hModUxTheme,
			"GetThemeMargins")); // STRING_OK
	}
}

void LoadMimeOLE()
{
	HRESULT hRes = S_OK;
	if (!hModInetComm) hModInetComm = LoadFromSystemDir(_T("inetcomm.dll")); // STRING_OK
	if (hModInetComm)
	{
		WC_D(pfnMimeOleGetCodePageCharset, (LPMIMEOLEGETCODEPAGECHARSET) GetProcAddress(
			hModInetComm,
			"MimeOleGetCodePageCharset")); // STRING_OK
	}
}

void LoadMSI()
{
	HRESULT hRes = S_OK;
	if (!hModMSI) hModMSI = LoadFromSystemDir(_T("msi.dll")); // STRING_OK
	if (hModMSI)
	{
#ifdef _UNICODE
		WC_D(pfnMsiProvideQualifiedComponent, (LPMSIPROVIDEQUALIFIEDCOMPONENT) GetProcAddress(
			hModMSI,
			"MsiProvideQualifiedComponentW" // STRING_OK
			));
		WC_D(pfnMsiGetFileVersion, (LPMSIGETFILEVERSION) GetProcAddress(
			hModMSI,
			"MsiGetFileVersionW" // STRING_OK
			));
#else
		WC_D(pfnMsiProvideQualifiedComponent, (LPMSIPROVIDEQUALIFIEDCOMPONENT) GetProcAddress(
			hModMSI,
			"MsiProvideQualifiedComponentA" // STRING_OK
			));
		WC_D(pfnMsiGetFileVersion, (LPMSIGETFILEVERSION) GetProcAddress(
			hModMSI,
			"MsiGetFileVersionA" // STRING_OK
			));
#endif
	}
} // LoadMSI

// Opens the mail key for the specified MAPI client, such as 'Microsoft Outlook' or 'ExchangeMAPI'
// Pass NULL to open the mail key for the default MAPI client
HKEY GetMailKey(LPTSTR szClient)
{
	DebugPrint(DBGLoadLibrary,_T("Enter GetMailKey(%s)\n"),szClient?szClient:_T("Default"));
	HRESULT hRes = S_OK;
	HKEY hMailKey = NULL;
	BOOL bClientIsDefault = false;

	// If szClient is NULL, we need to read the name of the default MAPI client
	if (!szClient)
	{
		HKEY hDefaultMailKey = NULL;
		WC_W32(RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,
			_T("Software\\Clients\\Mail"), // STRING_OK
			NULL,
			KEY_READ,
			&hDefaultMailKey));
		if (hDefaultMailKey)
		{
			DWORD dwKeyType = NULL;
			WC_H(HrGetRegistryValue(
				hDefaultMailKey,
				_T(""), // get the default value
				&dwKeyType,
				(LPVOID*) &szClient));
			DebugPrint(DBGLoadLibrary,_T("Default MAPI = %s\n"),szClient?szClient:_T("Default"));
			bClientIsDefault = true;
			EC_W32(RegCloseKey(hDefaultMailKey));
		}
	}

	if (szClient)
	{
		TCHAR szMailKey[256];
		EC_H(StringCchPrintf(
			szMailKey,
			_countof(szMailKey),
			_T("Software\\Clients\\Mail\\%s"), // STRING_OK
			szClient));

		if (SUCCEEDED(hRes))
		{
			WC_W32(RegOpenKeyEx(
				HKEY_LOCAL_MACHINE,
				szMailKey,
				NULL,
				KEY_READ,
				&hMailKey));
		}
	}
	if (bClientIsDefault) delete[] szClient;

	return hMailKey;
} // GetMailKey

// Gets MSI IDs for the specified MAPI client, such as 'Microsoft Outlook' or 'ExchangeMAPI'
// Pass NULL to get the IDs for the default MAPI client
// Allocates with new, delete with delete[]
void GetMapiMsiIds(LPTSTR szClient, LPTSTR* lpszComponentID, LPTSTR* lpszAppLCID, LPTSTR* lpszOfficeLCID)
{
	DebugPrint(DBGLoadLibrary,_T("GetMapiMsiIds()\n"));
	HRESULT hRes = S_OK;

	HKEY hKey = GetMailKey(szClient);

	if (hKey)
	{
		DWORD dwKeyType = NULL;

		if (lpszComponentID)
		{
			WC_H(HrGetRegistryValue(
				hKey,
				_T("MSIComponentID"), // STRING_OK
				&dwKeyType,
				(LPVOID*) lpszComponentID));
			DebugPrint(DBGLoadLibrary,_T("MSIComponentID = %s\n"),*lpszComponentID?*lpszComponentID:_T("<not found>"));
			hRes = S_OK;
		}

		if (lpszAppLCID)
		{
			WC_H(HrGetRegistryValue(
				hKey,
				_T("MSIApplicationLCID"), // STRING_OK
				&dwKeyType,
				(LPVOID*) lpszAppLCID));
			DebugPrint(DBGLoadLibrary,_T("MSIApplicationLCID = %s\n"),*lpszAppLCID?*lpszAppLCID:_T("<not found>"));
			hRes = S_OK;
		}

		if (lpszOfficeLCID)
		{
			WC_H(HrGetRegistryValue(
				hKey,
				_T("MSIOfficeLCID"), // STRING_OK
				&dwKeyType,
				(LPVOID*) lpszOfficeLCID));
			DebugPrint(DBGLoadLibrary,_T("MSIOfficeLCID = %s\n"),*lpszOfficeLCID?*lpszOfficeLCID:_T("<not found>"));
			hRes = S_OK;
		}

		EC_W32(RegCloseKey(hKey));
	}
} // GetMapiMsiIds

void GetMAPIPath(LPTSTR szClient, LPTSTR szMAPIPath, ULONG cchMAPIPath)
{
	HRESULT hRes = S_OK;

	szMAPIPath[0] = '\0'; // Terminate String at pos 0 (safer if we fail below)

	// Find some strings:
	LPTSTR szComponentID = NULL;
	LPTSTR szAppLCID = NULL;
	LPTSTR szOfficeLCID = NULL;

	GetMapiMsiIds(szClient,&szComponentID,&szAppLCID,&szOfficeLCID);

	if (szComponentID)
	{
		if (szAppLCID)
		{
			WC_B(GetComponentPath(
				szComponentID,
				szAppLCID,
				szMAPIPath,
				cchMAPIPath));
		}
		if ((FAILED(hRes) || szMAPIPath[0] == _T('\0')) && szOfficeLCID)
		{
			hRes = S_OK;
			WC_B(GetComponentPath(
				szComponentID,
				szOfficeLCID,
				szMAPIPath,
				cchMAPIPath));
		}
		if (FAILED(hRes) || szMAPIPath[0] == _T('\0'))
		{
			hRes = S_OK;
			WC_B(GetComponentPath(
				szComponentID,
				NULL,
				szMAPIPath,
				cchMAPIPath));
		}
	}

	delete[] szComponentID;
	delete[] szOfficeLCID;
	delete[] szAppLCID;
} // GetMAPIPath

void AutoLoadMAPI()
{
	DebugPrint(DBGLoadLibrary,_T("AutoLoadMAPI - loading MAPI exports\n"));

	// Attempt load default MAPI first if we have one
	// This will handle Outlook MAPI
	if (!hModMSMAPI)
	{
		TCHAR szMSMAPI32path[MAX_PATH] = {0};
		GetMAPIPath(NULL,szMSMAPI32path,_countof(szMSMAPI32path));
		if (szMSMAPI32path[0] != NULL)
		{
			hModMSMAPI = MyLoadLibrary(szMSMAPI32path);
		}
	}
	if (hModMSMAPI) LoadMAPIFuncs(hModMSMAPI);

	// In case that fails - load the stub library from system32
	// This will handle Exchange MAPI and rare case where we don't load Outlook MAPI
	if (!hModMAPI) hModMAPI = LoadFromSystemDir(_T("mapi32.dll")); // STRING_OK
	// If hModMAPI is the same as hModMSMAPI, then we don't need to try it again
	if (hModMAPI && hModMAPI != hModMSMAPI) LoadMAPIFuncs(hModMAPI);
} // AutoLoadMAPI

void UnloadMAPI()
{
	DebugPrint(DBGLoadLibrary,_T("UnloadMAPI - unloading MAPI exports\n"));
	// Blank out all the functions we've loaded from MAPI DLLs:
	pfnFGetComponentPath = NULL;
	pfnLaunchWizard = NULL;
	pfnMAPIAllocateBuffer = NULL;
	pfnMAPIAllocateMore = NULL;
	pfnMAPIFreeBuffer = NULL;
	pfnHrGetOneProp = NULL;
	pfnMAPIInitialize = NULL;
	pfnMAPIUninitialize = NULL;
	pfnLPFreeProws = NULL;
	pfnPpropFindProp = NULL;
	pfnScDupPropset = NULL;
	pfnScCountProps = NULL;
	pfnScCopyProps = NULL;
	pfnOpenIMsgOnIStg = NULL;
	pfnMAPIGetDefaultMalloc = NULL;
	pfnOpenTnefStreamEx = NULL;
	pfnOpenStreamOnFile = NULL;
	pfnOpenStreamOnFileW = NULL;
	pfnCloseIMsgSession = NULL;
	pfnOpenIMsgSession = NULL;
	pfnHrQueryAllRows = NULL;
	pfnMAPIOpenFormMgr = NULL;
	pfnRTFSync = NULL;
	pfnHrSetOneProp = NULL;
	pfnFreePadrlist = NULL;
	pfnPropCopyMore = NULL;
	pfnWrapCompressedRTFStream = NULL;
	pfnWrapStoreEntryID = NULL;
	pfnCreateIProp = NULL;
	pfnCreateTable = NULL;
	pfnHrValidateParameters = NULL;
	pfnBuildDisplayTable = NULL;
	pfnMNLS_lstrlenW = NULL;
	pfnMAPILogonEx = NULL;
	pfnMAPIAdminProfiles = NULL;
	pfnHrValidateIPMSubtree = NULL;
	pfnWrapEx = NULL;
	pfnMAPIOpenLocalFormContainer = NULL;
	pfnHrDispatchNotifications = NULL;

	HRESULT hRes = S_OK;
	if (hModMSMAPI) WC_B(FreeLibrary(hModMSMAPI));
	hModMSMAPI = NULL;
	hRes = S_OK;
	if (hModMAPIStub) WC_B(FreeLibrary(hModMAPIStub));
	hModMAPIStub = NULL;
	hRes = S_OK;
	if (hModMAPI) WC_B(FreeLibrary(hModMAPI));
	hModMAPI = NULL;
	hRes = S_OK;
}

// Translation - if pfn isn't already set, GetProcAddress for it from hMod
#define GETPROC(pfn,_TYPE, ProcName) \
if (!(pfn)) \
{ \
	WC_D((pfn), (_TYPE) GetProcAddress(hMod, (ProcName))); \
	if (!(pfn)) DebugPrint(DBGLoadLibrary,_T("Failed to load \"%hs\" from 0x%08X\n"),(ProcName),(hMod)); \
	hRes = S_OK; \
}

void LoadMAPIFuncs(HMODULE hMod)
{
	DebugPrint(DBGLoadLibrary,_T("LoadMAPIFuncs - loading from 0x%08X\n"),hMod);
	HRESULT hRes = S_OK;
	if (!hMod) return;

	GETPROC(pfnLaunchWizard,			LPLAUNCHWIZARDENTRY,		LAUNCHWIZARDENTRYNAME)
	GETPROC(pfnMAPIAllocateBuffer,		LPMAPIALLOCATEBUFFER,		"MAPIAllocateBuffer")
	GETPROC(pfnMAPIAllocateMore,		LPMAPIALLOCATEMORE,			"MAPIAllocateMore")
	GETPROC(pfnMAPIFreeBuffer,			LPMAPIFREEBUFFER,			"MAPIFreeBuffer")
	GETPROC(pfnHrGetOneProp,			LPHRGETONEPROP,				"HrGetOneProp@12")
	GETPROC(pfnHrGetOneProp,			LPHRGETONEPROP,				"HrGetOneProp")
	GETPROC(pfnMAPIInitialize,			LPMAPIINITIALIZE,			"MAPIInitialize")
	GETPROC(pfnMAPIUninitialize,		LPMAPIUNINITIALIZE,			"MAPIUninitialize")
	GETPROC(pfnLPFreeProws,				LPFREEPROWS,				"FreeProws@4")
	GETPROC(pfnLPFreeProws,				LPFREEPROWS,				"FreeProws")
	GETPROC(pfnPpropFindProp,			LPPPROPFINDPROP,			"PpropFindProp@12")
	GETPROC(pfnPpropFindProp,			LPPPROPFINDPROP,			"PpropFindProp")
	GETPROC(pfnScDupPropset,			LPSCDUPPROPSET,				"ScDupPropset@16")
	GETPROC(pfnScDupPropset,			LPSCDUPPROPSET,				"ScDupPropset")
	GETPROC(pfnScCountProps,			LPSCCOUNTPROPS,				"ScCountProps@12")
	GETPROC(pfnScCountProps,			LPSCCOUNTPROPS,				"ScCountProps")
	GETPROC(pfnScCopyProps,				LPSCCOPYPROPS,				"ScCopyProps@16")
	GETPROC(pfnScCopyProps,				LPSCCOPYPROPS,				"ScCopyProps")
	GETPROC(pfnOpenIMsgOnIStg,			LPOPENIMSGONISTG,			"OpenIMsgOnIStg@44")
	GETPROC(pfnOpenIMsgOnIStg,			LPOPENIMSGONISTG,			"OpenIMsgOnIStg")
	GETPROC(pfnMAPIGetDefaultMalloc,	LPMAPIGETDEFAULTMALLOC,		"MAPIGetDefaultMalloc@0")
	GETPROC(pfnMAPIGetDefaultMalloc,	LPMAPIGETDEFAULTMALLOC,		"MAPIGetDefaultMalloc")
	GETPROC(pfnOpenTnefStreamEx,		LPOPENTNEFSTREAMEX,			"OpenTnefStreamEx")
	GETPROC(pfnOpenStreamOnFile,		LPOPENSTREAMONFILE,			"OpenStreamOnFile")
	GETPROC(pfnOpenStreamOnFileW,		LPOPENSTREAMONFILEW,		"OpenStreamOnFileW")
	GETPROC(pfnCloseIMsgSession,		LPCLOSEIMSGSESSION,			"CloseIMsgSession@4")
	GETPROC(pfnCloseIMsgSession,		LPCLOSEIMSGSESSION,			"CloseIMsgSession")
	GETPROC(pfnOpenIMsgSession,			LPOPENIMSGSESSION,			"OpenIMsgSession@12")
	GETPROC(pfnOpenIMsgSession,			LPOPENIMSGSESSION,			"OpenIMsgSession")
	GETPROC(pfnHrQueryAllRows,			LPHRQUERYALLROWS,			"HrQueryAllRows@24")
	GETPROC(pfnHrQueryAllRows,			LPHRQUERYALLROWS,			"HrQueryAllRows")
	GETPROC(pfnMAPIOpenFormMgr,			LPMAPIOPENFORMMGR,			"MAPIOpenFormMgr")
	GETPROC(pfnRTFSync,					LPRTFSYNC,					"RTFSync")
	GETPROC(pfnHrSetOneProp,			LPHRSETONEPROP,				"HrSetOneProp@8")
	GETPROC(pfnHrSetOneProp,			LPHRSETONEPROP,				"HrSetOneProp")
	GETPROC(pfnFreePadrlist,			LPFREEPADRLIST,				"FreePadrlist@4")
	GETPROC(pfnFreePadrlist,			LPFREEPADRLIST,				"FreePadrlist")
	GETPROC(pfnPropCopyMore,			LPPROPCOPYMORE,				"PropCopyMore@16")
	GETPROC(pfnPropCopyMore,			LPPROPCOPYMORE,				"PropCopyMore")
	GETPROC(pfnWrapCompressedRTFStream,	LPWRAPCOMPRESSEDRTFSTREAM,	"WrapCompressedRTFStream")
	GETPROC(pfnWrapStoreEntryID,		LPWRAPSTOREENTRYID,			"WrapStoreEntryID@24")
	GETPROC(pfnWrapStoreEntryID,		LPWRAPSTOREENTRYID,			"WrapStoreEntryID")
	GETPROC(pfnCreateIProp,				LPCREATEIPROP,				"CreateIProp@24")
	GETPROC(pfnCreateIProp,				LPCREATEIPROP,				"CreateIProp")
	GETPROC(pfnCreateTable,				LPCREATETABLE,				"CreateTable@36")
	GETPROC(pfnCreateTable,				LPCREATETABLE,				"CreateTable")
	GETPROC(pfnHrValidateParameters,	LPHRVALIDATEPARAMETERS,		"HrValidateParameters@8")
	GETPROC(pfnHrValidateParameters,	LPHRVALIDATEPARAMETERS,		"HrValidateParameters")
	GETPROC(pfnBuildDisplayTable,		LPBUILDDISPLAYTABLE,		"BuildDisplayTable@40")
	GETPROC(pfnBuildDisplayTable,		LPBUILDDISPLAYTABLE,		"BuildDisplayTable")
	GETPROC(pfnMNLS_lstrlenW,			LPMNLS_LSTRLENW,			"MNLS_lstrlenW")
	GETPROC(pfnMAPILogonEx,				LPMAPILOGONEX,				"MAPILogonEx")
	GETPROC(pfnMAPIAdminProfiles,		LPMAPIADMINPROFILES,		"MAPIAdminProfiles")
	GETPROC(pfnHrValidateIPMSubtree,	LPHRVALIDATEIPMSUBTREE,		"HrValidateIPMSubtree@20")
	GETPROC(pfnHrValidateIPMSubtree,	LPHRVALIDATEIPMSUBTREE,		"HrValidateIPMSubtree")
	GETPROC(pfnWrapEx,					LPWRAPCOMPRESSEDRTFSTREAMEX,"WrapCompressedRTFStreamEx")
	GETPROC(pfnMAPIOpenLocalFormContainer, LPMAPIOPENLOCALFORMCONTAINER,"MAPIOpenLocalFormContainer")
	GETPROC(pfnHrDispatchNotifications,	LPHRDISPATCHNOTIFICATIONS,	"HrDispatchNotifications@4")
	GETPROC(pfnHrDispatchNotifications,	LPHRDISPATCHNOTIFICATIONS,	"HrDispatchNotifications")
}

#define CHECKLOAD(pfn) \
{ \
	if (!(pfn)) AutoLoadMAPI(); \
	if (!(pfn)) \
	{ \
		DebugPrint(DBGLoadLibrary,_T("Function %hs not loaded\n"),#pfn); \
	} \
}

STDMETHODIMP_(SCODE) MAPIAllocateBuffer(ULONG cbSize,
										LPVOID FAR * lppBuffer)
{
	CHECKLOAD(pfnMAPIAllocateBuffer);
	if (pfnMAPIAllocateBuffer) return pfnMAPIAllocateBuffer(cbSize,lppBuffer);
	return MAPI_E_CALL_FAILED;
}

STDMETHODIMP_(SCODE) MAPIAllocateMore(ULONG cbSize,
									  LPVOID lpObject,
									  LPVOID FAR * lppBuffer)
{
	CHECKLOAD(pfnMAPIAllocateMore);
	if (pfnMAPIAllocateMore) return pfnMAPIAllocateMore(cbSize,lpObject,lppBuffer);
	return MAPI_E_CALL_FAILED;
}

STDAPI_(ULONG) MAPIFreeBuffer(LPVOID lpBuffer)
{
	// If we don't have this, the memory's already leaked - don't compound it with a crash
	if (!lpBuffer) return NULL;
	CHECKLOAD(pfnMAPIFreeBuffer);
	if (pfnMAPIFreeBuffer) return pfnMAPIFreeBuffer(lpBuffer);
	return NULL;
}

STDAPI HrGetOneProp(LPMAPIPROP lpMapiProp,
					ULONG ulPropTag,
					LPSPropValue FAR * lppProp)
{
	CHECKLOAD(pfnHrGetOneProp);
	if (pfnHrGetOneProp) return pfnHrGetOneProp(lpMapiProp,ulPropTag,lppProp);
	return MAPI_E_CALL_FAILED;
}

STDMETHODIMP MAPIInitialize(LPVOID lpMapiInit)
{
	CHECKLOAD(pfnMAPIInitialize);
	if (pfnMAPIInitialize) return pfnMAPIInitialize(lpMapiInit);
	return MAPI_E_CALL_FAILED;
}

STDAPI_(void) MAPIUninitialize(void)
{
	CHECKLOAD(pfnMAPIUninitialize);
	if (pfnMAPIUninitialize) pfnMAPIUninitialize();
	return;
}

STDAPI_(void) FreeProws(LPSRowSet lpRows)
{
	CHECKLOAD(pfnLPFreeProws);
	if (pfnLPFreeProws) pfnLPFreeProws(lpRows);
	return;
}

STDAPI_(LPSPropValue) PpropFindProp(LPSPropValue lpPropArray,
									ULONG cValues,
									ULONG ulPropTag)
{
	CHECKLOAD(pfnPpropFindProp);
	if (pfnPpropFindProp) return pfnPpropFindProp(lpPropArray,cValues,ulPropTag);
	return NULL;
}

STDAPI_(SCODE) ScDupPropset(int cValues,
							LPSPropValue lpPropArray,
							LPALLOCATEBUFFER lpAllocateBuffer,
							LPSPropValue FAR *lppPropArray)
{
	CHECKLOAD(pfnScDupPropset);
	if (pfnScDupPropset) return pfnScDupPropset(cValues,lpPropArray,lpAllocateBuffer,lppPropArray);
	return MAPI_E_CALL_FAILED;
}

STDAPI_(SCODE) ScCountProps(int cValues,
							LPSPropValue lpPropArray,
							ULONG FAR *lpcb)
{
	CHECKLOAD(pfnScCountProps);
	if (pfnScCountProps) return pfnScCountProps(cValues,lpPropArray,lpcb);
	return MAPI_E_CALL_FAILED;
}

STDAPI_(SCODE) ScCopyProps(int cValues,
							LPSPropValue lpPropArray,
							LPVOID lpvDst,
							ULONG FAR *lpcb)
{
	CHECKLOAD(pfnScCopyProps);
	if (pfnScCopyProps) return pfnScCopyProps(cValues,lpPropArray,lpvDst,lpcb);
	return MAPI_E_CALL_FAILED;
}

STDAPI_(SCODE) OpenIMsgOnIStg(LPMSGSESS lpMsgSess,
							  LPALLOCATEBUFFER lpAllocateBuffer,
							  LPALLOCATEMORE lpAllocateMore,
							  LPFREEBUFFER lpFreeBuffer,
							  LPMALLOC lpMalloc,
							  LPVOID lpMapiSup,
							  LPSTORAGE lpStg,
							  MSGCALLRELEASE FAR * lpfMsgCallRelease,
							  ULONG ulCallerData,
							  ULONG ulFlags,
							  LPMESSAGE FAR * lppMsg)
{
	CHECKLOAD(pfnOpenIMsgOnIStg);
	if (pfnOpenIMsgOnIStg) return pfnOpenIMsgOnIStg(
		lpMsgSess,
		lpAllocateBuffer,
		lpAllocateMore,
		lpFreeBuffer,
		lpMalloc,
		lpMapiSup,
		lpStg,
		lpfMsgCallRelease,
		ulCallerData,
		ulFlags,
		lppMsg);
	return MAPI_E_CALL_FAILED;
}

STDAPI_(LPMALLOC) MAPIGetDefaultMalloc(VOID)
{
	CHECKLOAD(pfnMAPIGetDefaultMalloc);
	if (pfnMAPIGetDefaultMalloc) return pfnMAPIGetDefaultMalloc();
	return NULL;
}

STDMETHODIMP OpenTnefStreamEx(LPVOID lpvSupport,
							  LPSTREAM lpStream,
							  LPTSTR lpszStreamName,
							  ULONG ulFlags,
							  LPMESSAGE lpMessage,
							  WORD wKeyVal,
							  LPADRBOOK lpAdressBook,
							  LPITNEF FAR * lppTNEF)
{
	CHECKLOAD(pfnOpenTnefStreamEx);
	if (pfnOpenTnefStreamEx) return pfnOpenTnefStreamEx(
		lpvSupport,
		lpStream,
		lpszStreamName,
		ulFlags,
		lpMessage,
		wKeyVal,
		lpAdressBook,
		lppTNEF);
	return MAPI_E_CALL_FAILED;
}

// Since I never use lpszPrefix, I don't convert it
// To make certain of that, I pass NULL for it
// If I ever do need this param, I'll have to fix this
STDMETHODIMP MyOpenStreamOnFile(LPALLOCATEBUFFER lpAllocateBuffer,
								LPFREEBUFFER lpFreeBuffer,
								ULONG ulFlags,
								__in LPCWSTR lpszFileName,
								__in LPCWSTR /*lpszPrefix*/,
								LPSTREAM FAR * lppStream)
{
	CHECKLOAD(pfnOpenStreamOnFile);
	CHECKLOAD(pfnOpenStreamOnFileW);
	HRESULT hRes = S_OK;
	if (!pfnOpenStreamOnFile && !pfnOpenStreamOnFileW) return MAPI_E_CALL_FAILED;
	if (pfnOpenStreamOnFileW)
	{
		hRes = pfnOpenStreamOnFileW(
			lpAllocateBuffer,
			lpFreeBuffer,
			ulFlags,
			lpszFileName,
			NULL,
			lppStream);
	}
	else
	{
		// Convert new file name to Ansi
		LPSTR lpAnsiCharStr = NULL;
		EC_H(UnicodeToAnsi(
			lpszFileName,
			&lpAnsiCharStr));
		if (SUCCEEDED(hRes))
		{
			hRes = pfnOpenStreamOnFile(
				lpAllocateBuffer,
				lpFreeBuffer,
				ulFlags,
				(LPTSTR) lpAnsiCharStr,
				NULL,
				lppStream);
		}
		delete[] lpAnsiCharStr;
	}
	return hRes;
}

STDAPI_(void) CloseIMsgSession(LPMSGSESS lpMsgSess)
{
	CHECKLOAD(pfnCloseIMsgSession);
	if (pfnCloseIMsgSession) pfnCloseIMsgSession(
		lpMsgSess);
	return;
}

STDAPI_(SCODE) OpenIMsgSession(LPMALLOC lpMalloc,
							   ULONG ulFlags,
							   LPMSGSESS FAR * lppMsgSess)
{
	CHECKLOAD(pfnOpenIMsgSession);
	if (pfnOpenIMsgSession) return pfnOpenIMsgSession(
		lpMalloc,
		ulFlags,
		lppMsgSess);
	return MAPI_E_CALL_FAILED;
}

STDAPI HrQueryAllRows(LPMAPITABLE lpTable,
					  LPSPropTagArray lpPropTags,
					  LPSRestriction lpRestriction,
					  LPSSortOrderSet lpSortOrderSet,
					  LONG crowsMax,
					  LPSRowSet FAR *lppRows)
{
	CHECKLOAD(pfnHrQueryAllRows);
	if (pfnHrQueryAllRows) return pfnHrQueryAllRows(
		lpTable,
		lpPropTags,
		lpRestriction,
		lpSortOrderSet,
		crowsMax,
		lppRows);
	return MAPI_E_CALL_FAILED;
}

STDAPI MAPIOpenFormMgr(LPMAPISESSION pSession, LPMAPIFORMMGR FAR * ppmgr)
{
	CHECKLOAD(pfnMAPIOpenFormMgr);
	if (pfnMAPIOpenFormMgr) return pfnMAPIOpenFormMgr(
		pSession,
		ppmgr);
	return MAPI_E_CALL_FAILED;
}

STDAPI_(HRESULT) RTFSync (LPMESSAGE lpMessage, ULONG ulFlags, BOOL FAR * lpfMessageUpdated)
{
	CHECKLOAD(pfnRTFSync);
	if (pfnRTFSync) return pfnRTFSync(lpMessage,ulFlags,lpfMessageUpdated);
	return MAPI_E_CALL_FAILED;
}

STDAPI HrSetOneProp(LPMAPIPROP lpMapiProp,
					LPSPropValue lpProp)
{
	CHECKLOAD(pfnHrSetOneProp);
	if (pfnHrSetOneProp) return pfnHrSetOneProp(lpMapiProp,lpProp);
	return MAPI_E_CALL_FAILED;
}

STDAPI_(void) FreePadrlist(LPADRLIST lpAdrlist)
{
	CHECKLOAD(pfnFreePadrlist);
	if (pfnFreePadrlist) pfnFreePadrlist(lpAdrlist);
	return;
}

HRESULT HrDupPropset(
					 int cprop,
					 LPSPropValue rgprop,
					 LPVOID lpObject,
					 LPSPropValue FAR *	prgprop)
{
	ULONG cb = NULL;
	HRESULT hRes = S_OK;

	//	Find out how much memory we need
	EC_H(ScCountProps(cprop, rgprop, &cb));

	if (SUCCEEDED(hRes) && cb)
	{
		//	Obtain memory
		if (lpObject != NULL)
		{
			EC_H(MAPIAllocateMore(cb, lpObject, (LPVOID FAR *)prgprop));
		}
		else
		{
			EC_H(MAPIAllocateBuffer(cb, (LPVOID FAR *)prgprop));
		}

		if (SUCCEEDED(hRes) && prgprop)
		{
			//	Copy the properties
			EC_H(ScCopyProps(cprop, rgprop, *prgprop, &cb));
		}
	}

	return hRes;
}

typedef ACTIONS FAR *	LPACTIONS;

// swiped from EDK rules sample
STDAPI HrCopyActions(
					LPACTIONS lpActsSrc, // source action ptr
					LPVOID lpObject, // ptr to existing MAPI buffer
					LPACTIONS FAR *	lppActsDst) // ptr to destination ACTIONS buffer
{
	if (!lpActsSrc || !lppActsDst) return MAPI_E_INVALID_PARAMETER;
	if (lpActsSrc->cActions <= 0 || lpActsSrc->lpAction == NULL) return MAPI_E_INVALID_PARAMETER;

	BOOL fNullObject = (lpObject == NULL);
	HRESULT hRes = S_OK;
	ULONG i = 0;
	LPACTION lpActDst = NULL;
	LPACTION lpActSrc = NULL;
	LPACTIONS lpActsDst = NULL;

	*lppActsDst = NULL;

	if (lpObject != NULL)
	{
		WC_H(MAPIAllocateMore(sizeof(ACTIONS), lpObject, (LPVOID FAR *)lppActsDst));
	}
	else
	{
		WC_H(MAPIAllocateBuffer(sizeof(ACTIONS), (LPVOID FAR *)lppActsDst));
		lpObject = *lppActsDst;
	}

	if (FAILED(hRes)) return hRes;
	// no short circuit returns after here

	lpActsDst = *lppActsDst;
	*lpActsDst = *lpActsSrc;
	lpActsDst->lpAction = NULL;

	WC_H(MAPIAllocateMore(sizeof(ACTION) * lpActsDst->cActions,
						  lpObject,
						  (LPVOID FAR *)&(lpActsDst->lpAction)));
	if (SUCCEEDED(hRes) && lpActsDst->lpAction)
	{
		// Initialize acttype values for all members of the array to a value
		// that will not cause deallocation errors should the copy fail.
		for (i = 0; i < lpActsDst->cActions; i++)
			lpActsDst->lpAction[i].acttype = OP_BOUNCE;

		// Now actually copy all the members of the array.
		for (i = 0; i < lpActsDst->cActions; i++)
		{
			lpActDst =	&(lpActsDst->lpAction[i]);
			lpActSrc =	&(lpActsSrc->lpAction[i]);

			*lpActDst = *lpActSrc;

			switch (lpActSrc->acttype)
			{
			case OP_MOVE:			// actMoveCopy
			case OP_COPY:
				if (lpActDst->actMoveCopy.cbStoreEntryId &&
					lpActDst->actMoveCopy.lpStoreEntryId)
				{
					WC_H(MAPIAllocateMore(lpActDst->actMoveCopy.cbStoreEntryId,
						lpObject,
						(LPVOID FAR *)
						&(lpActDst->actMoveCopy.lpStoreEntryId)));
					if (FAILED(hRes)) break;

					memcpy(lpActDst->actMoveCopy.lpStoreEntryId,
						lpActSrc->actMoveCopy.lpStoreEntryId,
						lpActSrc->actMoveCopy.cbStoreEntryId);
				}


				if (lpActDst->actMoveCopy.cbFldEntryId &&
					lpActDst->actMoveCopy.lpFldEntryId)
				{
					WC_H(MAPIAllocateMore(lpActDst->actMoveCopy.cbFldEntryId,
						lpObject,
						(LPVOID FAR *)
						&(lpActDst->actMoveCopy.lpFldEntryId)));
					if (FAILED(hRes)) break;

					memcpy(lpActDst->actMoveCopy.lpFldEntryId,
						lpActSrc->actMoveCopy.lpFldEntryId,
						lpActSrc->actMoveCopy.cbFldEntryId);
				}

				break;

			case OP_REPLY:			// actReply
			case OP_OOF_REPLY:
				if (lpActDst->actReply.cbEntryId &&
					lpActDst->actReply.lpEntryId)
				{
					WC_H(MAPIAllocateMore(lpActDst->actReply.cbEntryId,
						lpObject,
						(LPVOID FAR *)
						&(lpActDst->actReply.lpEntryId)));
					if (FAILED(hRes)) break;

					memcpy(lpActDst->actReply.lpEntryId,
						lpActSrc->actReply.lpEntryId,
						lpActSrc->actReply.cbEntryId);
				}
				break;

			case OP_DEFER_ACTION:	// actDeferAction
				if (lpActSrc->actDeferAction.pbData &&
					lpActSrc->actDeferAction.cbData)
				{
					WC_H(MAPIAllocateMore(lpActDst->actDeferAction.cbData,
						lpObject,
						(LPVOID FAR *)
						&(lpActDst->actDeferAction.pbData)));
					if (FAILED(hRes)) break;

					memcpy(lpActDst->actDeferAction.pbData,
						lpActSrc->actDeferAction.pbData,
						lpActDst->actDeferAction.cbData);
				}
				break;

			case OP_FORWARD:		// lpadrlist
			case OP_DELEGATE:
				lpActDst->lpadrlist = NULL;

				if (lpActSrc->lpadrlist && lpActSrc->lpadrlist->cEntries)
				{
					ULONG j = 0;

					WC_H(MAPIAllocateMore(CbADRLIST(lpActSrc->lpadrlist),
						lpObject,
						(LPVOID FAR *)&(lpActDst->lpadrlist)));
					if (FAILED(hRes)) break;

					lpActDst->lpadrlist->cEntries = lpActSrc->lpadrlist->cEntries;

					// Initialize the new ADRENTRY's and validate cValues.
					for (j = 0; j < lpActSrc->lpadrlist->cEntries; j++)
					{
						lpActDst->lpadrlist->aEntries[j] = lpActSrc->lpadrlist->aEntries[j];
						lpActDst->lpadrlist->aEntries[j].rgPropVals = NULL;

						if (lpActDst->lpadrlist->aEntries[j].cValues == 0)
						{
							hRes = MAPI_E_INVALID_PARAMETER;
							break;
						}
					}

					// Copy the rgPropVals.
					for (j = 0; j < lpActSrc->lpadrlist->cEntries; j++)
					{
						WC_H(HrDupPropset(
							lpActDst->lpadrlist->aEntries[j].cValues,
							lpActSrc->lpadrlist->aEntries[j].rgPropVals,
							lpObject,
							&lpActDst->lpadrlist->aEntries[j].rgPropVals));
						if (FAILED(hRes)) break;
					}
				}

				break;

			case OP_TAG:			// propTag
				WC_H(PropCopyMore(
					&lpActDst->propTag,
					&lpActSrc->propTag,
					MAPIAllocateMore,
					lpObject));
				if (FAILED(hRes)) break;

				break;

			case OP_BOUNCE:			// scBounceCode
			case OP_DELETE:			// union not used
			case OP_MARK_AS_READ:
				break; // Nothing to do!

			default:				// error!
				{
					hRes = MAPI_E_INVALID_PARAMETER;
					break;
				}
			}
		}
	}

	if (FAILED(hRes))
	{
		if (fNullObject)
		    MAPIFreeBuffer(*lppActsDst);
	}

	return hRes;
}

HRESULT HrCopyRestrictionArray(
	LPSRestriction lpResSrc, // source restriction
	LPVOID lpObject, // ptr to existing MAPI buffer
	ULONG cRes, // # elements in array
	LPSRestriction lpResDest // destination restriction
	)
{
	if (!lpResSrc || !lpResDest || !lpObject) return MAPI_E_INVALID_PARAMETER;
	HRESULT hRes = S_OK;
	ULONG i = 0;

	for (i = 0; i < cRes; i++)
	{
		// Copy all the members over
		lpResDest[i] = lpResSrc[i];

		// Now fix up the pointers
		switch (lpResSrc[i].rt)
		{
		// Structures for these two types are identical
		case RES_AND:
		case RES_OR:
			if (lpResSrc[i].res.resAnd.cRes && lpResSrc[i].res.resAnd.lpRes)
			{
				if (lpResSrc[i].res.resAnd.cRes > ULONG_MAX/sizeof(SRestriction))
				{
					hRes = MAPI_E_CALL_FAILED;
					break;
				}
				WC_H(MAPIAllocateMore(sizeof(SRestriction) * lpResSrc[i].res.resAnd.cRes,
					lpObject,
					(LPVOID FAR *)
					&lpResDest[i].res.resAnd.lpRes));
				if (FAILED(hRes)) break;

				WC_H(HrCopyRestrictionArray(
					lpResSrc[i].res.resAnd.lpRes,
					lpObject,
					lpResSrc[i].res.resAnd.cRes,
					lpResDest[i].res.resAnd.lpRes));
				if (FAILED(hRes)) break;
			}
			break;

		// Structures for these two types are identical
		case RES_NOT:
		case RES_COUNT:
			if (lpResSrc[i].res.resNot.lpRes)
			{
				WC_H(MAPIAllocateMore(sizeof(SRestriction),
					lpObject,
					(LPVOID FAR *)
					&lpResDest[i].res.resNot.lpRes));
				if (FAILED(hRes)) break;

				WC_H(HrCopyRestrictionArray(
					lpResSrc[i].res.resNot.lpRes,
					lpObject,
					1,
					lpResDest[i].res.resNot.lpRes));
				if (FAILED(hRes)) break;
			}
			break;

		// Structures for these two types are identical
		case RES_CONTENT:
		case RES_PROPERTY:
			if (lpResSrc[i].res.resContent.lpProp)
			{
				WC_H(HrDupPropset(
					1,
					lpResSrc[i].res.resContent.lpProp,
					lpObject,
					&lpResDest[i].res.resContent.lpProp));
				if (FAILED(hRes)) break;
			}
			break;

		case RES_COMPAREPROPS:
		case RES_BITMASK:
		case RES_SIZE:
		case RES_EXIST:
			break; // Nothing to do.

		case RES_SUBRESTRICTION:
			if (lpResSrc[i].res.resSub.lpRes)
			{
				WC_H(MAPIAllocateMore(sizeof(SRestriction),
					lpObject,
					(LPVOID FAR *)
					&lpResDest[i].res.resSub.lpRes));
				if (FAILED(hRes)) break;

				WC_H(HrCopyRestrictionArray(
					lpResSrc[i].res.resSub.lpRes,
					lpObject,
					1,
					lpResDest[i].res.resSub.lpRes));
				if (FAILED(hRes)) break;
			}
			break;

		// Structures for these two types are identical
		case RES_COMMENT:
		case RES_ANNOTATION:
			if (lpResSrc[i].res.resComment.lpRes)
			{
				WC_H(MAPIAllocateMore(sizeof(SRestriction),
					lpObject,
					(LPVOID FAR *)
					&lpResDest[i].res.resComment.lpRes));
				if (FAILED(hRes)) break;

				WC_H(HrCopyRestrictionArray(
					lpResSrc[i].res.resComment.lpRes,
					lpObject,
					1,
					lpResDest[i].res.resComment.lpRes));
				if (FAILED(hRes)) break;
			}

			if (lpResSrc[i].res.resComment.cValues && lpResSrc[i].res.resComment.lpProp)
			{
				WC_H(HrDupPropset(
					lpResSrc[i].res.resComment.cValues,
					lpResSrc[i].res.resComment.lpProp,
					lpObject,
					&lpResDest[i].res.resComment.lpProp));
				if (FAILED(hRes)) break;
			}
			break;

		default:
			hRes = MAPI_E_INVALID_PARAMETER;
			break;
		}
	}

	return hRes;
}

STDAPI HrCopyRestriction(
	LPSRestriction lpResSrc, // source restriction ptr
	LPVOID lpObject, // ptr to existing MAPI buffer
	LPSRestriction FAR * lppResDest // dest restriction buffer ptr
	)
{
	if (!lppResDest) return MAPI_E_INVALID_PARAMETER;
	*lppResDest = NULL;
	if (!lpResSrc) return S_OK;

	BOOL fNullObject =(lpObject == NULL);
	HRESULT	hRes = S_OK;

	if (lpObject != NULL)
	{
		WC_H(MAPIAllocateMore(sizeof(SRestriction),
							  lpObject,
							  (LPVOID FAR *)lppResDest));
	}
	else
	{
		WC_H(MAPIAllocateBuffer(sizeof(SRestriction),
								(LPVOID FAR *)lppResDest));
		lpObject = *lppResDest;
	}
	if (FAILED(hRes)) return hRes;
	// no short circuit returns after here

	WC_H(HrCopyRestrictionArray(
		lpResSrc,
		lpObject,
		1,
		*lppResDest));

	if (FAILED(hRes))
	{
		if (fNullObject)
			MAPIFreeBuffer(*lppResDest);
	}

	return hRes;
}

// This augmented PropCopyMore is implicitly tied to the built-in MAPIAllocateMore and MAPIAllocateBuffer through
// the calls to HrCopyRestriction and HrCopyActions. Rewriting those functions to accept function pointers is
// expensive for no benefit here. So if you borrow this code, be careful if you plan on using other allocators.
STDAPI_(SCODE) PropCopyMore(LPSPropValue lpSPropValueDest,
							LPSPropValue lpSPropValueSrc,
							ALLOCATEMORE * lpfAllocMore,
							LPVOID lpvObject)
{
	CHECKLOAD(pfnPropCopyMore);
	if (!pfnPropCopyMore) return MAPI_E_CALL_FAILED;
	HRESULT hRes = S_OK;
	switch ( PROP_TYPE(lpSPropValueSrc->ulPropTag) )
	{
	case PT_SRESTRICTION:
	case PT_ACTIONS:
		{
			// It's an action or restriction - we know how to copy those:
			memcpy( (BYTE *) lpSPropValueDest,
				(BYTE *) lpSPropValueSrc,
				sizeof(SPropValue));
			if (PT_SRESTRICTION == PROP_TYPE(lpSPropValueSrc->ulPropTag))
			{
				LPSRestriction lpNewRes = NULL;
				WC_H(HrCopyRestriction(
					(LPSRestriction) lpSPropValueSrc->Value.lpszA,
					lpvObject,
					&lpNewRes));
				lpSPropValueDest->Value.lpszA = (LPSTR) lpNewRes;
			}
			else
			{
				ACTIONS* lpNewAct = NULL;
				WC_H(HrCopyActions(
					(ACTIONS*) lpSPropValueSrc->Value.lpszA,
					lpvObject,
					&lpNewAct));
				lpSPropValueDest->Value.lpszA = (LPSTR) lpNewAct;
			}
			break;
		}
	default:
		hRes = pfnPropCopyMore(lpSPropValueDest,lpSPropValueSrc,lpfAllocMore,lpvObject);
	}
	return hRes;
}

STDAPI_(HRESULT) WrapCompressedRTFStream(LPSTREAM lpCompressedRTFStream,
										 ULONG ulFlags,
										 LPSTREAM FAR * lpUncompressedRTFStream)
{
	CHECKLOAD(pfnWrapCompressedRTFStream);
	if (pfnWrapCompressedRTFStream) return pfnWrapCompressedRTFStream(
		lpCompressedRTFStream,
		ulFlags,
		lpUncompressedRTFStream);
	return MAPI_E_CALL_FAILED;
}

STDMETHODIMP MAPILogonEx(ULONG_PTR ulUIParam,
						 LPTSTR lpszProfileName,
						 LPTSTR lpszPassword,
						 ULONG ulFlags,
						 LPMAPISESSION FAR * lppSession)
{
	CHECKLOAD(pfnMAPILogonEx);
	if (pfnMAPILogonEx) return pfnMAPILogonEx(
		ulUIParam,
		lpszProfileName,
		lpszPassword,
		ulFlags,
		lppSession);
	return MAPI_E_CALL_FAILED;
}

STDMETHODIMP MAPIAdminProfiles(ULONG ulFlags,
							   LPPROFADMIN FAR *lppProfAdmin)
{
	CHECKLOAD(pfnMAPIAdminProfiles);
	if (pfnMAPIAdminProfiles) return pfnMAPIAdminProfiles(
		ulFlags,
		lppProfAdmin);
	return MAPI_E_CALL_FAILED;
}

STDAPI HrValidateIPMSubtree(LPMDB lpMDB,
							ULONG ulFlags,
							ULONG FAR *lpcValues,
							LPSPropValue FAR *lppValues,
							LPMAPIERROR FAR *lpperr)
{
	CHECKLOAD(pfnHrValidateIPMSubtree);
	if (pfnHrValidateIPMSubtree) return pfnHrValidateIPMSubtree(
		lpMDB,
		ulFlags,
		lpcValues,
		lppValues,
		lpperr);
	return MAPI_E_CALL_FAILED;
}

STDAPI MAPIOpenLocalFormContainer(LPMAPIFORMCONTAINER FAR * ppfcnt)
{
	CHECKLOAD(pfnMAPIOpenLocalFormContainer);
	if (pfnMAPIOpenLocalFormContainer) return pfnMAPIOpenLocalFormContainer(
		ppfcnt);
	return MAPI_E_CALL_FAILED;
}

STDAPI HrDispatchNotifications(ULONG ulFlags)
{
	CHECKLOAD(pfnHrDispatchNotifications);
	if (pfnHrDispatchNotifications) return pfnHrDispatchNotifications(
		ulFlags);
	return MAPI_E_CALL_FAILED;
}

STDAPI WrapStoreEntryID(ULONG ulFlags, __in LPTSTR lpszDLLName, ULONG cbOrigEntry,
	LPENTRYID lpOrigEntry, ULONG *lpcbWrappedEntry, LPENTRYID *lppWrappedEntry)
{
	CHECKLOAD(pfnWrapStoreEntryID);
	if (pfnWrapStoreEntryID) return pfnWrapStoreEntryID(ulFlags, lpszDLLName, cbOrigEntry, lpOrigEntry, lpcbWrappedEntry, lppWrappedEntry);
	return MAPI_E_CALL_FAILED;
}
STDAPI_(SCODE)
CreateIProp( LPCIID					lpInterface,
			 ALLOCATEBUFFER FAR *	lpAllocateBuffer,
			 ALLOCATEMORE FAR *		lpAllocateMore,
			 FREEBUFFER FAR *		lpFreeBuffer,
			 LPVOID					lpvReserved,
			 LPPROPDATA FAR *		lppPropData )
{
	CHECKLOAD(pfnCreateIProp);
	if (pfnCreateIProp) return pfnCreateIProp(lpInterface, lpAllocateBuffer, lpAllocateMore, lpFreeBuffer, lpvReserved, lppPropData);
	return MAPI_E_CALL_FAILED;
}

STDAPI_(SCODE)
CreateTable( LPCIID					lpInterface,
			 ALLOCATEBUFFER FAR *	lpAllocateBuffer,
			 ALLOCATEMORE FAR *		lpAllocateMore,
			 FREEBUFFER FAR *		lpFreeBuffer,
			 LPVOID					lpvReserved,
			 ULONG					ulTableType,
			 ULONG					ulPropTagIndexColumn,
			 LPSPropTagArray		lpSPropTagArrayColumns,
			 LPTABLEDATA FAR *		lppTableData )
{
	CHECKLOAD(pfnCreateTable);
	if (pfnCreateTable) return pfnCreateTable(
		lpInterface,
		lpAllocateBuffer,
		lpAllocateMore,
		lpFreeBuffer,
		lpvReserved,
		ulTableType,
		ulPropTagIndexColumn,
		lpSPropTagArrayColumns,
		lppTableData);
	return MAPI_E_CALL_FAILED;
}

STDAPI HrValidateParameters( METHODS eMethod, LPVOID FAR *ppFirstArg)
{
	CHECKLOAD(pfnHrValidateParameters);
	if (pfnHrValidateParameters) return pfnHrValidateParameters(
		eMethod,
		ppFirstArg);
	return MAPI_E_CALL_FAILED;
}

STDAPI
BuildDisplayTable(	LPALLOCATEBUFFER	lpAllocateBuffer,
					LPALLOCATEMORE		lpAllocateMore,
					LPFREEBUFFER		lpFreeBuffer,
					LPMALLOC			lpMalloc,
					HINSTANCE			hInstance,
					UINT				cPages,
					LPDTPAGE			lpPage,
					ULONG				ulFlags,
					LPMAPITABLE *		lppTable,
					LPTABLEDATA	*		lppTblData )
{
	CHECKLOAD(pfnBuildDisplayTable);
	if (pfnBuildDisplayTable) return pfnBuildDisplayTable(
		lpAllocateBuffer,
		lpAllocateMore,
		lpFreeBuffer,
		lpMalloc,
		hInstance,
		cPages,
		lpPage,
		ulFlags,
		lppTable,
		lppTblData);
	return MAPI_E_CALL_FAILED;
}

int	WINAPI MNLS_lstrlenW(LPCWSTR lpString)
{
	CHECKLOAD(pfnMNLS_lstrlenW);
	if (pfnMNLS_lstrlenW) return pfnMNLS_lstrlenW(
		lpString);
	return 0;
}