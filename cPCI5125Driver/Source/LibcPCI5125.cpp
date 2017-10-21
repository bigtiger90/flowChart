/*
 * cPci5125.c
 *
 *  Created on: Mar 22, 2017
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
#include "LibcPCI5125.h"

#define MAX_CARD_NUM 		15
HR_DEVICE_HANDLE card[MAX_CARD_NUM];

/*int main(void)
{

    int fd;
    TY_IOCTL_DATA val, val1;
    val.data = 0x200;
    val.offset = 0x1000;
    sprintf(dev_name, "/dev/cPCI5125");
    fd = open(dev_name,O_RDWR);
    if (fd <0 )
    {
        perror("open error");
        return -1;
    }
    printf("%s....fd0 = %d.\n", dev_name, fd);

    printf("%s....writebar2 offset %x value %x.\n", dev_name, val.offset , val.data);
    ioctl(fd, WRITEBAR2, &val);
    val.data = 0;
    val.offset = 0x1000;
    ioctl(fd, ReadBAR2, &val);
    printf("%s....readbar2 offset %x value %x.\n", dev_name, val.offset , val.data);
    close(fd);


    return 0;
}*/

HRDrv_ErrCode hiCPCI5125Open(HR_UINT32 board_id)
{
	char dev_name[20];
	char tmpBoardId;
	if (board_id < 0 || board_id > MAX_CARD_NUM)
	{
		return HR_ERR_ILLEGAL_PARAMETER;
	}
	tmpBoardId= board_id;
	sprintf(dev_name, "/dev/cPCI5125%d",tmpBoardId);
	card[board_id] = open(dev_name, O_RDWR);
	printf("%s....fd0 = %d.\n", dev_name, card[board_id]);
	if (card[board_id] > 0)
		return HR_STATUS_SUCCESS;
	else
		return HR_ERR_DEVICE_OPEN_FAILED;
}

HRDrv_ErrCode hiCPCI5125Close(HR_UINT32 board_id)
{
	close(card[board_id]);
	return HR_STATUS_SUCCESS;
}

int regOffset(int regNum)
{
	return 4096+(regNum-1)*4;
}

HRDrv_ErrCode HRWrite5125f(HR_DEVICE_HANDLE board_id,HR_UINT32 regNum,float fData)
{
	HR_UINT32 iData = 0;
	TY_IOCTL_DATA val;
	memcpy(&iData, &fData,sizeof(float));
	val.offset = regOffset(regNum);
	val.data = iData;
	ioctl(card[board_id], WRITEBAR2, &val);
	return HR_STATUS_SUCCESS;
}

HRDrv_ErrCode HRRead5125f(HR_DEVICE_HANDLE board_id,HR_UINT32 regNum,HR_FLOAT *fData)
{
	TY_IOCTL_DATA val;
	val.offset = regOffset(regNum);
	ioctl(card[board_id], ReadBAR2, &val);
	memcpy(fData, &val.data,sizeof(val.data));
	//printf("readdata %x\n",val.data);
	//printf("readdata %f\n",*fData);
	return HR_STATUS_SUCCESS;
}

HRDrv_ErrCode HRResolverInit(HR_DEVICE_HANDLE board_id,HR_FLOAT ratio,HR_UINT32 poleNum,HR_UINT32 excEn,HR_FLOAT voltage,HR_FLOAT freq)
{
	TY_IOCTL_DATA val;
	val.offset = HR5125_REGS_ROTATE_RATIO << 2;
	val.data = (HR_UINT32)(ratio*4.0*32768.0/2.2);
	ioctl(card[board_id], WRITEBAR2, &val);

	val.offset = HR5125_REGS_ROTATE_POLENUM << 2;
	val.data = (HR_UINT32)((poleNum/3.1415926)*(1<<24));
	ioctl(card[board_id], WRITEBAR2, &val);

	val.offset = HR5125_REGS_ROTATE_EN << 2;
	val.data = excEn;
	ioctl(card[board_id], WRITEBAR2, &val);

	if(1==excEn)
	{
		val.offset = HR5125_REGS_ROTATE_FREQ << 2;
		val.data = (HR_UINT32 )(freq*5.36870912);
		ioctl(card[board_id], WRITEBAR2, &val);

		val.offset = HR5125_REGS_ROTATE_AMPL << 2;
		val.data = (HR_UINT32 )(voltage*1489.4545);
		ioctl(card[board_id], WRITEBAR2, &val);
	}

	val.offset = HR5125_RES_EN<<2;
	val.data = 1;
	ioctl(card[board_id], WRITEBAR2, &val);
	return HR_STATUS_SUCCESS;
}

HRDrv_ErrCode HRResolverStop(HR_DEVICE_HANDLE board_id)
{
	TY_IOCTL_DATA val;
	val.offset = HR5125_REGS_ROTATE_EN<<2;
	val.data = 0;
	ioctl(card[board_id], WRITEBAR2, &val);

	val.offset = HR5125_RES_EN<<2;
	val.data = 0;
	ioctl(card[board_id], WRITEBAR2, &val);

	return HR_STATUS_SUCCESS;
}


