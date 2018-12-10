# emWinTouch
xpt2046 Touch drive for STemWin
<br />Example: (use my eeprom Library)
```
SpiTouchEmwin_Init(osPriorityBelowNormal);
uint8_t	IsCalibrated=0xFF;
EEPROM24XX_Load(0,&IsCalibrated,1);
if(IsCalibrated!=1)
{
  if(SpiTouchEmwin_CalibrateRun(SpiTouchEmwin_CalibrateData)==true)
  {
    for(uint8_t i=0 ; i<128 ; i+=16)
	  EEPROM24XX_Save(1+i,&SpiTouchEmwin_CalibrateData[i],16);		
    IsCalibrated=1;
    EEPROM24XX_Save(0,&IsCalibrated,1);			
  }				
}
else
{
  EEPROM24XX_Load(1,SpiTouchEmwin_CalibrateData,sizeof(SpiTouchEmwin_CalibrateData));
  SpiTouchEmwin_CalibrateLoad(SpiTouchEmwin_CalibrateData);		
}	
```
