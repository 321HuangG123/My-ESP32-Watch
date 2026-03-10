#include "LSM303.h"
#include <math.h>

unsigned char LSM303_ReadOneReg(unsigned char RegAddr);
void LSM303_ReadMultiReg(unsigned char RegAddr, unsigned char RegNum, unsigned char DataBuff[]);
unsigned char LSM303_Temp_ReadOneReg(unsigned char RegAddr);
void LSM303_WriteOneReg(unsigned char RegAddr, unsigned char dat);

unsigned char LSM303DLH_Init(void);

void LSM303DLH_Sleep(void);
void LSM303DLH_Wakeup(void);

void LSM303_ReadAcceleration(int16_t *Xa, int16_t *Ya, int16_t *Za);
void LSM303_ReadMagnetic(int16_t *Xm, int16_t *Ym, int16_t *Zm);

int LSM303DLH_CalculationZAxisAngle(int16_t Xa, int16_t Ya, int16_t Za);
int LSM303DLH_CalculationXAxisAngle(int16_t Xa, int16_t Ya, int16_t Za);
void LSM303_ReadTemperature(int16_t *Temp);
float Azimuth_Calculate(int16_t Xa, int16_t Ya, int16_t Za, int16_t Xm, int16_t Ym, int16_t Zm);