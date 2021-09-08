// /////////////////////////////////////////////////////////////////
// // I2C write bytes to device.
// //
// // Arguments: ucSlaveAdr - slave address
// // ucSubAdr - sub address
// // pBuf - pointer of buffer
// // ucBufLen - length of buffer
// /////////////////////////////////////////////////////////////////
// void i2cBurstWriteBytes(BYTE ucSlaveAdr, BYTE ucSubAdr, BYTE *pBuf, BYTE ucBufLen)
// {
//  BYTE ucDummy; // loop dummy
//  ucDummy = I2C_ACCESS_DUMMY_TIME;
//  while(ucDummy--)
//  {
//  if (i2c_AccessStart(ucSlaveAdr, I2C_WRITE) == FALSE)
//  continue;
//  if (i2c_SendByte(ucSubAdr) == I2C_NON_ACKNOWLEDGE) // check non-acknowledge
//  continue;
//  while(ucBufLen--) // loop of writting data
//  {
//  i2c_SendByte(*pBuf); // send byte
//  pBuf++; // next byte pointer
//  } // while
//  break;
//  } // while
//  i2c_Stop();
// }
// /////////////////////////////////////////////////////////////////
// // I2C read bytes from device.
// //
// // Arguments: ucSlaveAdr - slave address
// // ucSubAdr - sub address
// // pBuf - pointer of buffer
// // ucBufLen - length of buffer
// /////////////////////////////////////////////////////////////////
// void i2cBurstReadBytes(BYTE ucSlaveAdr, BYTE ucSubAdr, BYTE *pBuf, BYTE ucBufLen)
// {
//  BYTE ucDummy; // loop dummy
//  ucDummy = I2C_ACCESS_DUMMY_TIME;
//  while(ucDummy--)
//  {
//  if (i2c_AccessStart(ucSlaveAdr, I2C_WRITE) == FALSE)
//  continue;
//  if (i2c_SendByte(ucSubAdr) == I2C_NON_ACKNOWLEDGE) // check non-acknowledge
//  continue;
//  if (i2c_AccessStart(ucSlaveAdr, I2C_READ) == FALSE)
//  continue;
//  while(ucBufLen--) // loop to burst read
//  {
//  *pBuf = i2c_ReceiveByte(ucBufLen); // receive byte
//  pBuf++; // next byte pointer
//  } // while
//  break;
//  } // while
//  i2c_Stop();
// }
// /////////////////////////////////////////////////////////////////
// // I2C read current bytes from device.
// //
// // Arguments: ucSlaveAdr - slave address
// // pBuf - pointer of buffer
// // ucBufLen - length of buffer
// /////////////////////////////////////////////////////////////////
// void i2cBurstCurrentBytes(BYTE ucSlaveAdr, BYTE *pBuf, BYTE ucBufLen)
// {
//  BYTE ucDummy; // loop dummy

//  ucDummy = I2C_ACCESS_DUMMY_TIME;
//  while(ucDummy--)
//  {
//  if (i2c_AccessStart(ucSlaveAdr, I2C_READ) == FALSE)
//  continue;
//  while(ucBufLen--) // loop to burst read
//  {
//  *pBuf = i2c_ReceiveByte(ucBufLen); // receive byte
//  pBuf++; // next byte pointer
//  } // while
//  break;
//  } // while
//  i2c_Stop();
// }