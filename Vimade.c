#undef UNICODE
#include <stdio.h>
#include <WinSock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <Shlwapi.h>

#include "Vimade.h"

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "ws2_32.lib")

#include <cfgmgr32.h> // for MAX_DEVICE_ID_LEN
#pragma comment(lib, "Setupapi.lib")
#include <Setupapi.h>

typedef struct 
{
    char sFriendlyName[MAX_PATH];
    char sClass[MAX_PATH];
    char sHardwareId[MAX_PATH];
}HardwareInfo;

BOOLEAN isSandbox()
{

}

BOOLEAN isVm_Drives()
{

	return FALSE;
}

BOOLEAN isVm_Bios(IWbemServices *pSvc)
{
	return FALSE;
}

BOOLEAN isVm_ComputerModel(IWbemServices *pSvc)
{
	IEnumWbemClassObject* pEnumerator = wmiExecQuery(pSvc, L"SELECT * FROM Win32_ComputerSystem");
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
		hr = pclsObj->lpVtbl->Get(pclsObj, L"Model", 0, &vtProp, 0, 0);
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

BOOLEAN isVM_NetworkAdapterName()
{
	DWORD rv, Size = 15000;
	PIP_ADAPTER_ADDRESSES AdapterAddresses = NULL;
	BOOLEAN Result = TRUE;

	rv = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &Size);
	AdapterAddresses = (IP_ADAPTER_ADDRESSES *)malloc(Size);

	rv = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, AdapterAddresses, &Size);
	if (rv != ERROR_SUCCESS) {
		printf("GetAdaptersAddresses() failed...\n");
		free(AdapterAddresses);
		return FALSE;
	}

	printf("Network adapter name:\n");
	while (AdapterAddresses)
	{
		if (Size > 0)
		{
			printf("\t%ls\n", AdapterAddresses->Description);
			// detect if there is no real network adapter.
			if (!StrStrI(AdapterAddresses->Description, L"Virtual") && !StrStrI(AdapterAddresses->Description, L"loopback"))
			{
				Result = FALSE;
			}
		}
		AdapterAddresses = AdapterAddresses->Next;
	}

	free(AdapterAddresses);
	return Result;
}


/*
*
Common Device Class GUIDs
--------------------------
Class	    GUID	                                Device Description
CDROM	    4D36E965-E325-11CE-BFC1-08002BE10318	CD/DVD/Blu-ray drives
DiskDrive	4D36E967-E325-11CE-BFC1-08002BE10318	Hard drives
Display	    4D36E968-E325-11CE-BFC1-08002BE10318	Video adapters
FDC	        4D36E969-E325-11CE-BFC1-08002BE10318	Floppy controllers
FloppyDisk	4D36E980-E325-11CE-BFC1-08002BE10318	Floppy drives
HDC	        4D36E96A-E325-11CE-BFC1-08002BE10318	Hard drive control  lers
HIDClass	745A17A0-74D3-11D0-B6FE-00A0C90F57DA	Some USB devices
1394	    6BDD1FC1-810F-11D0-BEC7-08002BE2092F	IEEE 1394 host controller
Image	    6BDD1FC6-810F-11D0-BEC7-08002BE2092F	Cameras and scanners
Keyboard	4D36E96B-E325-11CE-BFC1-08002BE10318	Keyboards
Modem	    4D36E96D-E325-11CE-BFC1-08002BE10318	Modems
Mouse	    4D36E96F-E325-11CE-BFC1-08002BE10318	Mice and pointing devices
Media	    4D36E96C-E325-11CE-BFC1-08002BE10318	Audio and video devices
Net	        4D36E972-E325-11CE-BFC1-08002BE10318	Network adapters
Ports	    4D36E978-E325-11CE-BFC1-08002BE10318	Serial and parallel ports
SCSIAdapter	4D36E97B-E325-11CE-BFC1-08002BE10318	SCSI and RAID controllers
System	    4D36E97D-E325-11CE-BFC1-08002BE10318	System buses, bridges, etc.
USB	        36FC9E60-C465-11CF-8056-444553540000	USB host controllers and hubs
*/
BOOLEAN isVM_DeviceRegistry()
{

    BOOLEAN Result = FALSE;

    //== This in case we want to get an individual device
    //char DiskDriveGuid[] = { 0x67, 0xE9, 0x36, 0x4D, 0x25, 0xE3, 0xCE, 0x11, 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 }; //4D36E967-E325-11CE-BFC1-08002BE10318
    //HDEVINFO di = SetupDiGetClassDevsA(&DiskDriveGuid, NULL, NULL, DIGCF_ALLCLASSES);

    //== Enumerate all hardware
    HDEVINFO di = SetupDiGetClassDevsA(NULL, NULL, NULL, DIGCF_ALLCLASSES);

    if (di == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }


    int iIdx = 0;
    TCHAR devInstanceId[MAX_DEVICE_ID_LEN];
    HardwareInfo hd;    

    while (TRUE)
    {
        SP_DEVINFO_DATA did;
        did.cbSize = sizeof(SP_DEVINFO_DATA);
        if (SetupDiEnumDeviceInfo(di, iIdx, &did) == FALSE)
        {
            if (GetLastError() == ERROR_NO_MORE_ITEMS)
            {
                break;
            }
            else
            {
                //error, but loop might continue?
            }
        }
        memset(devInstanceId, 0, MAX_DEVICE_ID_LEN);
        if (SetupDiGetDeviceInstanceId(di, &did, devInstanceId, MAX_DEVICE_ID_LEN, NULL) == FALSE)
        {
            //error, but loop might continue?
        }

        BYTE pbuf[MAX_PATH];
        DWORD reqSize = MAX_PATH;
        if (!SetupDiGetDeviceRegistryProperty(di, &did, SPDRP_FRIENDLYNAME, NULL, pbuf, 0, &reqSize))
        {
            //error, but loop might continue?
            int i = GetLastError();
            int j = 0;
        }

        if(!SetupDiGetDeviceRegistryProperty(di, &did, SPDRP_FRIENDLYNAME, NULL, pbuf, reqSize, NULL))
        {
            //error, but loop might continue?
            int i = GetLastError();
            int j = 0;
        }
        else 
        {
            printf("Device: %s\n", pbuf);
            if (StrStrI(pbuf, L"Virtual"))
            {
                Result = TRUE;
            }
        }
         
        iIdx++;
    }

    if (SetupDiDestroyDeviceInfoList(di) == FALSE)
    {
        //error, but should be ignored?
    }


    
    return Result;
}
