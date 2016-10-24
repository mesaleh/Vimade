#include "Vimade.h"
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

BOOLEAN isSandbox()
{

}

BOOLEAN isVm_Drives()
{

	return FALSE;
}

BOOLEAN isVm_ComputerModel(IWbemServices *pSvc)
{
	IEnumWbemClassObject* pEnumerator = wmiExecQuery(pSvc, L"SELECT * FROM Win32_OperatingSystem");
	if (!pEnumerator)
	{
		printf("isVm_ComputerModel failed!\n");
		return FALSE;
	}

	IWbemClassObject *pclsObj = NULL;
	ULONG uReturn = 0;
	BOOLEAN Result = FALSE;
	VARIANT vtProp;

	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->lpVtbl->Next(pEnumerator, WBEM_INFINITE, 1, &pclsObj, &uReturn);
		if (0 == uReturn)
		{
			break;
		}
		hr = pclsObj->lpVtbl->Get(pclsObj, L"Name", 0, &vtProp, 0, 0);
		if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR)
		{
			printf("Computer Model: %ls\n", vtProp.bstrVal);
			if (StrStrI(vtProp.bstrVal, L"Virtual"))
			{
				Result = TRUE;
			}
		}		
	}
	VariantClear(&vtProp);
	if(pclsObj)	pclsObj->lpVtbl->Release(pclsObj);

	return Result;
}
