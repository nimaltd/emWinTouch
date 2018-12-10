# emWinTouch
xpt2046 Touch drive for STemwin
<br />Example:

<br />SpiTouchEmwin_Init(osPriorityBelowNormal);
<br />uint8_t	IsCalibrated=0xFF;
<br />EEPROM24XX_Load(0,&IsCalibrated,1);
<br />if(IsCalibrated!=1)
<br />{
<br />  if(SpiTouchEmwin_CalibrateRun(SpiTouchEmwin_CalibrateData)==true)
<br />  {
<br />	  for(uint8_t i=0 ; i<128 ; i+=16)
<br />	  EEPROM24XX_Save(1+i,&SpiTouchEmwin_CalibrateData[i],16);		
<br />	  IsCalibrated=1;
<br />	  EEPROM24XX_Save(0,&IsCalibrated,1);			
<br />  }				
<br />}
<br />else
<br />{
<br />  EEPROM24XX_Load(1,SpiTouchEmwin_CalibrateData,sizeof(SpiTouchEmwin_CalibrateData));
<br />  SpiTouchEmwin_CalibrateLoad(SpiTouchEmwin_CalibrateData);		
<br />}	
