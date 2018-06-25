#include "../include/UtilsFunctions.h"
#include <CAENDigitizer.h>
#include <sys/time.h>
#include <time.h>

/*! \fn      int WriteRegisterBitmask(int32_t handle, uint32_t address, uint32_t data, uint32_t mask)
*   \brief   writes 'data' on register at 'address' using 'mask' as bitmask
*
*   \param   handle :   Digitizer handle
*   \param   address:   Address of the Register to write
*   \param   data   :   Data to Write on the Register
*   \param   mask   :   Bitmask to use for data masking
*   \return  0 = Success; negative numbers are error codes
*/
int WriteRegisterBitmask(int32_t handle, uint32_t address, uint32_t data, uint32_t mask) {
  int32_t ret = CAEN_DGTZ_Success;
  uint32_t d32 = 0xFFFFFFFF;
  
  ret = CAEN_DGTZ_ReadRegister(handle, address, &d32);
  if(ret != CAEN_DGTZ_Success)
    return ret;
  
  data &= mask;
  d32 &= ~mask;
  d32 |= data;
  
  ret = CAEN_DGTZ_WriteRegister(handle, address, d32);
 
  return ret;
}



double getTime() {
  double returnTime;
  struct timespec currTime;
  clock_gettime(CLOCK_REALTIME, &currTime);
  returnTime = ((double) (currTime.tv_sec))+((double) (currTime.tv_nsec)/1000000000.);
  return returnTime;
}

