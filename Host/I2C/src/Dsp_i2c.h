extern unsigned char _i2c_s_daddr;
extern int _i2c_bEnClockDivisor;
extern int _i2c_FSB_clockDivisor;

unsigned char DSP_ReadI2C (unsigned char addr);
void DSP_WriteI2C (unsigned char addr,unsigned char odata);
void DSP_WriteFastI2C (unsigned char addr,unsigned char odata);
void initSerialBus (unsigned char bIsSerialBusMode);
void i2c_delay (int cnt);
BOOL Check_FastI2C_Status (BOOL bChkFSB_INT);

//For 16-bit data	//Micron sensors
void DSP_WriteI2C_16b (unsigned char addr,UINT16 odata);
UINT16 DSP_ReadI2C_16b (unsigned char addr);
void DSP_WriteFastI2C_16b (unsigned char addr,UINT16 odata);

//For multiple read / write I2C
void DSP_Write_Multi_I2C (unsigned char *odata, int i2c_data_len);
unsigned char DSP_Read_Multi_I2C (unsigned char *i2cData, int i2c_data_len);

//For TV tuner -- Philips TDA9885
unsigned char DSP_Read_TDA9885 (void);

//For Sony IMX011CQH5
void init_XCE (void);
void i2cInitSerialComm_IMX011CQH5 (void);
void DSP_WriteI2C_IMX011 (unsigned char addr,unsigned char odata);
void DSP_Write_Multi_I2C_IMX011 (unsigned char addr, unsigned char *i2c_odata, int i2c_data_len);
unsigned char DSP_ReadI2C_IMX011 (unsigned char addr);
