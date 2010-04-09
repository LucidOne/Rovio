#ifndef __I2CLIB_H_
#define __I2CLIB_H_


/***********************************************************************************
* Exported functions in libi2c.a
**********************************************************************************/

void i2cEnableInterrupt (bool bIsEnableINT);
void i2cSetIRQHandler(void* pvI2CFuncPtr);
cyg_uint32 i2cGetFastI2CStatus (void);
void i2cInitSerialBus (bool bIsSIFmode, bool bIsEnableClkDivior, cyg_uint32 uFSB_Clk_Divisor);
cyg_uint32 i2cGetSerialBusCtrl (void);
void i2cSetDeviceSlaveAddr (cyg_uint32 uSIF_Slave_Addr);
void i2cWriteI2C (cyg_uint32 uI2C_register_Addr, cyg_uint32 uI2C_register_Data);
cyg_uint32 i2cReadI2C (cyg_uint32 uI2C_register_Addr);
void i2cWriteFastI2C (bool bIsSCK_Status, cyg_uint32 uI2C_register_Addr, cyg_uint32 uI2C_register_Data);
//for Micron sensors
void i2cWriteI2C_16b (cyg_uint32 uI2C_register_Addr, cyg_uint32 uI2C_register_Data);
cyg_uint32 i2cReadI2C_16b (cyg_uint32 uI2C_register_Addr);
void i2cWriteFastI2C_16b (bool bIsSCK_Status, cyg_uint32 uI2C_register_Addr, cyg_uint32 uI2C_register_Data);
//For Multiple read / wrtie 
void i2cMultiple_WriteI2C (cyg_uint8 *pucI2C_register_Data, cyg_uint32 uParamLen);
void i2cMultiple_ReadI2C (cyg_uint8 *pucI2C_register_Data, cyg_uint32 uParamLen);

//For TV tuner -- Philips TDA9885
cyg_uint8 i2cTV_Tuner_Read_TDA9885 (void);


#endif	//#ifndef __I2CLIB_H_
