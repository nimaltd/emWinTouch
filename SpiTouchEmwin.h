#ifndef _SPITOUCHEMWIN_H
#define _SPITOUCHEMWIN_H

// 	Spi Touch xpt2046						version 1V02
//	"Nima Askari"								www.github.com/NimaLTD


#include <stdbool.h>
#include "cmsis_os.h"
#include "spi.h"

//###########################################################################################
extern 		uint8_t			SpiTouchEmwin_CalibrateData[88];
//###########################################################################################
bool 			SpiTouchEmwin_Init(osPriority Priority);
bool   		SpiTouchEmwin_CalibrateRun(uint8_t	*StoreCalibrateData);
void		  SpiTouchEmwin_CalibrateLoad(uint8_t	*StoreCalibrateData);
void			SpiTouchEmwin_Test(void);
//###########################################################################################
#endif
