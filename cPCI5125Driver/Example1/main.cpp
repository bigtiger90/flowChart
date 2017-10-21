#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include "../Include/LibcPCI5125.h"

int status;
#define ErrorCheck(func) {status = func; if(status != HR_STATUS_SUCCESS){printf("Error:%08x\n",status);}}

int main(void)
{
	HR_FLOAT Data = 0;
	printf("-----start------\n");

    ErrorCheck(hiCPCI5125Open(0));

    Data=200;
    printf("HRWrite5125f %f\n",Data);
    ErrorCheck(HRWrite5125f(0,1,Data));

    Data=0;
    ErrorCheck(HRRead5125f(0,1,&Data));
    printf("HRRead5125f %f\n",Data);

    printf("HRResolverInit\n");
    ErrorCheck(HRResolverInit(0, 0.5, 1, 1, 10, 10*1000));

    printf("HRResolverStop\n");
    ErrorCheck( HRResolverStop(0));

    ErrorCheck(hiCPCI5125Close(0));


    return 0;
}
