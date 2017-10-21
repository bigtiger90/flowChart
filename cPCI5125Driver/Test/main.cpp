/*
 * main.cpp
 *
 *  Created on: Mar 23, 2017
 *      Author: root
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


//char dev_name[20];

typedef struct ioctl_data
{
    unsigned int data;
    unsigned int offset;
} TY_IOCTL_DATA;

#define ReadBAR2	 0xc
#define WRITEBAR2 	 0xd

/*---------------------------------
  Type Definenation
-----------------------------------*/
typedef unsigned int		HR_DEVICE_HANDLE;
typedef long				HR_DWORD;
typedef unsigned int        HR_UINT32;
typedef int					HR_INT32;
typedef int                 HR_BOOL;
typedef float               HR_FLOAT;
typedef double				HR_DOUBLE;

/*---------------------------------
  HR Error Code
-----------------------------------*/
typedef enum {

	HR_STATUS_SUCCESS = 0x0L,
	HR_ERR_STATUS_INVALID_HANDLE,

	HR_ERR_LIB_OPENFAILED ,
	HR_ERR_LIB_CLOSEFAILED ,

	HR_ERR_READWRITE_MEMIO_FAILED,

	HR_ERR_DEVICE_NOT_FOUNDOROVERLAP,
	HR_ERR_DEVICE_OPEN_FAILED,
	HR_ERR_DEV_CLOSEFAILED,

	HR_ERR_ILLEGAL_PARAMETER,

} HRDrv_ErrCode;

#define HR5125_REGS_ROTATE_EN			(0x360)
#define HR5125_REGS_ROTATE_FREQ			(0x362)
#define HR5125_REGS_ROTATE_AMPL			(0x364)
#define HR5125_REGS_ROTATE_RATIO		(0x366)
#define HR5125_REGS_ROTATE_POLENUM		(0x372)
#define HR5125_RES_EN					(0x370)

int main(void)
{

    int fd;
    char tmpBoardId;
    float fval;
    char dev_name[sizeof("/dev/cPCI5125")+1];
    TY_IOCTL_DATA val, val1;
    printf("----start-------\n");
    sprintf(dev_name, "/dev/cPCI5125%d",0);
    printf("devname :%s\n",dev_name);
    fd = open(dev_name,O_RDWR);
    if (fd <0 )
    {
        perror("open error");
        return -1;
    }
    printf("%s....fd0 = %d.\n", dev_name, fd);
    printf("sizeof(HR_UINT32) %d.\n",sizeof(HR_UINT32));
    printf("sizeof(float) %d.\n",sizeof(float));
    fval=3.3;
    memcpy(&val.data,&fval,sizeof(fval));
    val.offset = 0x1000;
    printf("%s....writebar2 offset %x value %x.\n", dev_name, val.offset , val.data);
    ioctl(fd, WRITEBAR2, &val);
    val.data = 0;
    val.offset = 0x1000;
    ioctl(fd, ReadBAR2, &val);
    memcpy(&fval,&val.data,sizeof(val.data));
    printf("%s....readbar2 offset %x value %x.\n", dev_name, val.offset , val.data);

    close(fd);

    return 0;
}
