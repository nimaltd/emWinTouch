#ifndef _SPITOUCHEMWIN_H
#define _SPITOUCHEMWIN_H

#include <stdbool.h>
#include "cmsis_os.h"
#include "spi.h"

bool 			SpiTouchEmwin_Init(osPriority Priority);
bool   		SpiTouchEmwin_CalibrateRun(uint8_t	*StoreCalibrateData);
void		  SpiTouchEmwin_CalibrateLoad(uint8_t	*StoreCalibrateData);
void			SpiTouchEmwin_Test(void);

#endif
