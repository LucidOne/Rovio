#include <stdio.h>
#include "w99702_reg.h"
#include "wblib.h"
#include "wb_fmi.h"

#ifdef SM_DEVICE

#define _fmi_ECC_BIT7	0x80
#define _fmi_ECC_BIT6	0x40
#define _fmi_ECC_BIT5	0x20
#define _fmi_ECC_BIT4	0x10
#define _fmi_ECC_BIT3	0x08
#define _fmi_ECC_BIT2	0x04
#define _fmi_ECC_BIT1	0x02
#define _fmi_ECC_BIT0	0x01

unsigned char _fmi_ecc1=0;	// LP15 ~ LP08
unsigned char _fmi_ecc2=0;	// LP07 ~ LP00
unsigned char _fmi_ecc3=0;	// CP5 ~ CP0

extern UINT32 _fmi_u2KPageSize;		// 0:page size 512 byte; 1:page size 2K byte


INT correct_data(UINT8 *data, UINT8 ecc1, UINT8 ecc2, UINT8 ecc3, UINT32 ecc)
{
	unsigned int d, l, i, a, add, count, _fmi_ECC_CORRECT;
	unsigned char d1, d2, d3;
	unsigned char b, _fmi_ECC_BIT;

	d1 = ecc1^((ecc >> 8) & 0xff);		// 521
	d2 = ecc2^(ecc & 0xff);				// 520
	d3 = ecc3^((ecc >> 16) & 0xff);		// 522

	if (_fmi_u2KPageSize == 1)
	{
		_fmi_ECC_CORRECT = 0x01555554;
		d = ((unsigned int)d1<<16) + ((unsigned int)d2<<8) + (unsigned int)d3 + (((unsigned int)d3&0x3)<<24);
	}
	else	// 512
	{
		_fmi_ECC_CORRECT = 0x00555554;
		d = ((unsigned int)d1<<16) + ((unsigned int)d2<<8) + (unsigned int)d3;
	}

	if (d == 0)	return FMI_NO_ERR;	// No error

	if (((d^(d>>1)) & _fmi_ECC_CORRECT) == _fmi_ECC_CORRECT)
	{
		// mark LPxx
		add = 0;
		if (_fmi_u2KPageSize == 1)
		{
			l = 0x02000000;
			a = 0x100;		// 512 bytes needs 9 bit
			count = 9;
		}
		else
		{
			l = 0x00800000;
			a = 0x80;
			count = 8;
		}

		for (i=0; i<count; i++)
		{
			if (d & l)	add |= a;
			l >>= 2;
			a >>= 1;
		}

		// mark CPxx
		_fmi_ECC_BIT = 0;
		b = _fmi_ECC_BIT2;
		for (i=0; i<3; i++)
		{
			if (d & l)	_fmi_ECC_BIT |= b;
			l >>= 2;
			b >>= 1;
		}
		b = _fmi_ECC_BIT0;
		data[add] ^= (b << _fmi_ECC_BIT);
		return FMI_SM_ECC_CORRECT;
	}
	return FMI_SM_ECC_UNCORRECT;
}

INT fmiSM_ECC_Correct(UINT32 buf, UINT8 *data)
{
	int volatile status=0;
	unsigned int volatile reg1, reg2;

	if (_fmi_u2KPageSize == 1)
	{
		if (buf == 0)	// field
		{
			// storage area - 1
			reg1 = inpw(REG_SMECC0);
			_fmi_ecc1 = inpw(REG_SMRA_3) & 0xff;			// LP15 ~ LP08	2060
			_fmi_ecc2 = (inpw(REG_SMRA_2) >> 24) & 0xff;	// LP07 ~ LP00	2059
			_fmi_ecc3 = (inpw(REG_SMRA_3) >> 8) & 0xff;		// CP5 ~ CP0	2061
			status = correct_data(data, _fmi_ecc1, _fmi_ecc2, _fmi_ecc3, reg1);
			if (status < 0)
				return status;
		}

		if (buf == 1)
		{
			// storage area - 2
			reg1 = inpw(REG_SMECC1);
			_fmi_ecc1 = inpw(REG_SMRA_7) & 0xff;			// LP15 ~ LP08	2076
			_fmi_ecc2 = (inpw(REG_SMRA_6) >> 24) & 0xff;	// LP07 ~ LP00	2075
			_fmi_ecc3 = (inpw(REG_SMRA_7) >> 8) & 0xff;		// CP5 ~ CP0	2077
			status = correct_data(data+512, _fmi_ecc1, _fmi_ecc2, _fmi_ecc3, reg1);
			if (status < 0)
				return status;
		}

		if (buf == 2)
		{
			// storage area - 3
			reg1 = inpw(REG_SMECC2);
			_fmi_ecc1 = inpw(REG_SMRA_11) & 0xff;			// LP15 ~ LP08	2092
			_fmi_ecc2 = (inpw(REG_SMRA_10) >> 24) & 0xff;	// LP07 ~ LP00	2091
			_fmi_ecc3 = (inpw(REG_SMRA_11) >> 8) & 0xff;	// CP5 ~ CP0	2093
			status = correct_data(data+1024, _fmi_ecc1, _fmi_ecc2, _fmi_ecc3, reg1);
			if (status < 0)
				return status;
		}

		if (buf == 3)
		{
			// storage area - 4
			reg1 = inpw(REG_SMECC3);
			_fmi_ecc1 = inpw(REG_SMRA_15) & 0xff;			// LP15 ~ LP08	2108
			_fmi_ecc2 = (inpw(REG_SMRA_14) >> 24) & 0xff;	// LP07 ~ LP00	2107
			_fmi_ecc3 = (inpw(REG_SMRA_15) >> 8) & 0xff;	// CP5 ~ CP0	2109
			status = correct_data(data+1536, _fmi_ecc1, _fmi_ecc2, _fmi_ecc3, reg1);
			if (status < 0)
				return status;
		}
	}
	else
	{
		if (buf == 0)
		{
			reg1 = inpw(REG_SMRA_3);
			reg2 = inpw(REG_SMRA_2);
		}
		else
		{
			reg1 = inpw(REG_SMRA_7);
			reg2 = inpw(REG_SMRA_6);
		}

		// storage 0 ~ 255
		_fmi_ecc1 = (reg1 >> 16) & 0xff;		// LP15 ~ LP08	521, 526
		_fmi_ecc2 = (reg1 >> 8) & 0xff;			// LP07 ~ LP00	520, 525
		_fmi_ecc3 = (reg1 >> 24) & 0xff;		// CP5 ~ CP0	522, 527
		reg1 = inpw(REG_SMECC0);
		status = correct_data(data, _fmi_ecc1, _fmi_ecc2, _fmi_ecc3, reg1);
		if (status < 0)
			return status;

		// storage 256 ~ 511
		_fmi_ecc1 = (reg2 >> 8) & 0xff;		// LP15 ~ LP08
		_fmi_ecc2 = reg2 & 0xff;			// LP07 ~ LP00
		_fmi_ecc3 = (reg2 >> 16) & 0xff;	// CP5 ~ CP0
		reg1 = inpw(REG_SMECC1);
		status = correct_data(data+256, _fmi_ecc1, _fmi_ecc2, _fmi_ecc3, reg1);
		if (status < 0)
			return status;
	}

	return FMI_NO_ERR;
}

#endif	// SM_DEVICE
