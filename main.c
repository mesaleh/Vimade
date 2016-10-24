#include <Windows.h>
#include <stdio.h>
#include "helper.h"
#include "Vimade.h"


int main()
{
	IWbemServices *pSvc = wmiInit();
	int CheckNum = 0;

	printf("Check %d\n--------\n", ++CheckNum);
	if (isVm_ComputerModel(pSvc))
	{
		printf("DETECTED!");
		dropFile("ComputerModel_contains_Virtual");
	}

	wmiClean(pSvc);
	return 0;
}