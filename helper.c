#include <stdio.h>
#include "helper.h"
#pragma comment(lib, "wbemuuid.lib")
//#pragma comment(lib, "comsupp.lib")

IWbemServices* wmiInit()
{
	HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres))
	{
		printf("Failed to initialize COM library. Error code = 0x%x\n", hres);
		return NULL;                  // Program has failed.
	}

	// Step 2: --------------------------------------------------
	// Set general COM security levels --------------------------

	hres = CoInitializeSecurity(
		NULL,
		-1,                          // COM authentication
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities 
		NULL                         // Reserved
	);


	if (FAILED(hres))
	{
		printf("Failed to initialize security. Error code = 0x%x\n", hres);
		CoUninitialize();
		return NULL;                    // Program has failed.
	}

	// Step 3: ---------------------------------------------------
	// Obtain the initial locator to WMI -------------------------

	IWbemLocator *pLoc = NULL;

	hres = CoCreateInstance(
		&CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		&IID_IWbemLocator, (LPVOID *)&pLoc);

	if (FAILED(hres))
	{
		printf("Failed to create IWbemLocator object. Err code = 0x%x\n", hres);
		CoUninitialize();
		return NULL;                 // Program has failed.
	}

	// Step 4: -----------------------------------------------------
	// Connect to WMI through the IWbemLocator::ConnectServer method

	IWbemServices *pSvc = NULL;

	// Connect to the root\cimv2 namespace with
	// the current user and obtain pointer pSvc
	// to make IWbemServices calls.
	hres = pLoc->lpVtbl->ConnectServer(
		pLoc,
		L"ROOT\\CIMV2", // Object path of WMI namespace
		NULL,                    // User name. NULL = current user
		NULL,                    // User password. NULL = current
		0,                       // Locale. NULL indicates current
		NULL,                    // Security flags.
		0,                       // Authority (for example, Kerberos)
		0,                       // Context object 
		&pSvc                    // pointer to IWbemServices proxy
	);

	if (FAILED(hres))
	{
		printf("Could not connect. Error code = 0x%x\n", hres);
		pLoc->lpVtbl->Release(pLoc);
		CoUninitialize();
		return NULL;                // Program has failed.
	}

	// Step 5: --------------------------------------------------
	// Set security levels on the proxy -------------------------

	hres = CoSetProxyBlanket(
		pSvc,                        // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		NULL,                        // Server principal name 
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                        // client identity
		EOAC_NONE                    // proxy capabilities 
	);

	if (FAILED(hres))
	{
		printf("Could not set proxy blanket. Error code = 0x%x\n", hres);
		pSvc->lpVtbl->Release(pLoc);
		pLoc->lpVtbl->Release(pLoc);
		CoUninitialize();
		return NULL;               // Program has failed.
	}

	pLoc->lpVtbl->Release(pLoc);
	return pSvc;
}

IEnumWbemClassObject* wmiExecQuery(IWbemServices *pSvc, wchar_t* query)
{
	IEnumWbemClassObject* pEnumerator = NULL;
	HRESULT hres;
	hres = pSvc->lpVtbl->ExecQuery(pSvc,
		L"WQL",
		query,
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);

	if (FAILED(hres))
	{
		printf("Query failed. Error code = 0x%x\n", hres);		
		return NULL;
	}

	return pEnumerator;
}

void wmiClean(IWbemServices *pSvc)
{
	pSvc->lpVtbl->Release(pSvc);
	CoUninitialize();
}

void dropFile(char* name)
{
	FILE* fp = fopen(name, "w");
	fclose(fp);
}