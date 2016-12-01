#include <Windows.h>
#include <stdio.h>
#include "helper.h"
#include "Vimade.h"


int main()
{
	IWbemServices *pSvc = wmiInit();
	int CheckNum = 0;

	printf("\nCheck %d\n--------\n", ++CheckNum);
	if (isVm_ComputerModel(pSvc))
	{
		printf("DETECTED!\n");
		dropFile("ComputerModel_contains_Virtual");
	}

	printf("\nCheck %d\n--------\n", ++CheckNum);
	if (isVM_NetworkAdapterName())
	{
		printf("DETECTED!\n");
		dropFile("NetworkAdapterName_contains_Virtual");
	}	

    printf("\nCheck %d\n--------\n", ++CheckNum);
    if (isVM_DeviceRegistry())
    {
        printf("DETECTED!\n");
        dropFile("SystemDevicesName_contains_Virtual");
    }

	printf("\n\n");
	wmiClean(pSvc);

	printf("PRESS ANY KEY TO CONTINUE...\n\n");
	getchar();
	return 0;
}