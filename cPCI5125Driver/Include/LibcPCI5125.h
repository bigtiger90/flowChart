/*
 * cPCI5125.h
 *
 *  Created on: Mar 22, 2017
 *      Author: root
 */

#ifndef LIBCPCI5125_H_
#define LIBCPCI5125_H_

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
/*---------------------------------
  Function Declarations
-----------------------------------*/
/*******************************************************************************
* Function Name  : hiCPCI5125Open
* Description    : open device call first
* Input          : board_id: device id should be 0-15
* Return         : Error Code
*******************************************************************************/
HRDrv_ErrCode hiCPCI5125Open(HR_UINT32 board_id);

/*******************************************************************************
* Function Name  : hiCPCI5125Close
* Description    : close device
* Input          : pHDev: device handle
* Output         : None
* Return         : Error Code
*******************************************************************************/
HRDrv_ErrCode hiCPCI5125Close(HR_UINT32 board_id);

/*******************************************************************************
* Function Name  : hiCPCI5125Close
* Description    : close device
* Input          : pHDev: device handle
* Output         : None
* Return         : Error Code
*******************************************************************************/
HRDrv_ErrCode HRWrite5125f(HR_DEVICE_HANDLE board_id,HR_UINT32 regNum,HR_FLOAT fData);

/*******************************************************************************
* Function Name  : hiCPCI5125Close
* Description    : close device
* Input          : pHDev: device handle
* Output         : None
* Return         : Error Code
*******************************************************************************/
HRDrv_ErrCode HRRead5125f(HR_DEVICE_HANDLE board_id,HR_UINT32 regNum,HR_FLOAT *fData);

/*******************************************************************************
* Function Name  : hiCPCI5125Close
* Description    : close device
* Input          : pHDev: device handle
* Output         : None
* Return         : Error Code
*******************************************************************************/
HRDrv_ErrCode HRResolverInit(HR_DEVICE_HANDLE board_id,HR_FLOAT ratio,HR_UINT32 poleNum,HR_UINT32 excEn,HR_FLOAT voltage,HR_FLOAT freq);

/*******************************************************************************
* Function Name  : hiCPCI5125Close
* Description    : close device
* Input          : pHDev: device handle
* Output         : None
* Return         : Error Code
*******************************************************************************/
HRDrv_ErrCode HRResolverStop(HR_DEVICE_HANDLE board_id);

#endif /* CPCI5125_H_ */
