/* CC3200_I2C.h */

#ifndef CC3200_I2CSAMPLE_CC3200_I2C_H_
#define CC3200_I2CSAMPLE_CC3200_I2C_H_

void I2C_Init(void);

int I2C_Transfer(unsigned char ucDevAddr, unsigned char *ucWriteBuffer,
                  unsigned char *ucReadBuffer, unsigned long ulWriteSize,
                  unsigned long ulReadSize);

tBoolean I2C_IsBusy(void);

void I2C_SetAddress(unsigned char address);

#endif /* CC3200_I2CSAMPLE_CC3200_I2C_H_ */
