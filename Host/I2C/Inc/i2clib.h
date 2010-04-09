#ifndef __I2CLIB_H_
#define __I2CLIB_H_


/***********************************************************************************
* Exported functions in libi2c.a
**********************************************************************************/

void i2cEnableInterrupt (BOOL bIsEnableINT);
void i2cSetIRQHandler(PVOID pvI2CFuncPtr);
UINT32 i2cGetFastI2CStatus (void);
void i2cInitSerialBus (BOOL bIsSIFmode, BOOL bIsEnableClkDivior, UINT32 uFSB_Clk_Divisor);
UINT32 i2cGetSerialBusCtrl (void);
void i2cSetDeviceSlaveAddr (UINT32 uSIF_Slave_Addr);
void i2cWriteI2C (UINT32 uI2C_register_Addr, UINT32 uI2C_register_Data);
UINT32 i2cReadI2C (UINT32 uI2C_register_Addr);
void i2cWriteFastI2C (BOOL bIsSCK_Status, UINT32 uI2C_register_Addr, UINT32 uI2C_register_Data);
//for Micron sensors
void i2cWriteI2C_16b (UINT32 uI2C_register_Addr, UINT32 uI2C_register_Data);
UINT32 i2cReadI2C_16b (UINT32 uI2C_register_Addr);
void i2cWriteFastI2C_16b (BOOL bIsSCK_Status, UINT32 uI2C_register_Addr, UINT32 uI2C_register_Data);
//For Multiple read / wrtie 
void i2cMultiple_WriteI2C (UINT8 *pucI2C_register_Data, UINT32 uParamLen);
void i2cMultiple_ReadI2C (UINT8 *pucI2C_register_Data, UINT32 uParamLen);

//For TV tuner -- Philips TDA9885
UINT8 i2cTV_Tuner_Read_TDA9885 (void);

//For Sony IMX011CQH5
/*
void i2cInitSerialComm_IMX011CQH5 (void);
//For single write
void i2cWriteI2C_IMX011 (UINT32 uI2C_register_Addr, UINT32 uI2C_register_Data);
//For write with continuous addresses
void i2cWriteMultiI2C_IMX011 (UINT32 uI2C_register_Addr, UINT8 *uI2C_multi_register_Data, UINT32 multi_data_count);
UINT8 i2cReadI2C_IMX011 (UINT32 uI2C_register_Addr);	//seems to not support ?!
*/

//For SONY MCB772-Q
void i2c_Write_CMD_MCB77x (UINT8 ucCMD_id, UINT8 ucCMD_param);
void i2c_Read_CMD_EVENT_MCB77x (unsigned char *pucI2C_data);
void i2c_Write_CAM_REG_MCB77x (UINT8 ucI2C_register_Addr, UINT8 *pucI2C_data_Ptr, UINT8 ucI2C_WR_data_len);
void i2c_Read_CAM_REG_MCB77x (UINT8 ucI2C_register_Addr, UINT8 *pucI2C_data_Ptr, UINT8 ucI2C_RD_data_len);
void i2c_Read_CAM_I2C_ERROR_MCB77x (UINT8 *pucI2C_data_Ptr);
void i2c_Read_CMD_EchoBack_MCB77x (unsigned char *pucI2C_data);

//For Biomorphic Bi8921
	//The output format : 8-bit slave addr. + 16-bit register addr. + 8-bit data
void i2cWriteI2C_16bAddr (UINT16 uI2C_register_Addr, UINT8 uI2C_register_Data);
UINT32 i2cReadI2C_16bAddr (UINT16 uI2C_register_Addr);

#endif	//#ifndef __I2CLIB_H_
