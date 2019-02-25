

#include "GUI.h"
#include "WM.h"
#include "spitouchemwin.h"
#include "spitouchemwinConfig.h"

osThreadId 			TouchTaskHandle;
void 				 		StartTouchTask(void const * argument);
static uint8_t 	_TouchInCalibMode=0;
static int16_t	_TouchX;
static int16_t	_TouchY;
uint8_t					SpiTouchEmwin_CalibrateData[88];

void GUI_TOUCH_X_ActivateX(void) 
{
}
//*********************************************************************
//*********************************************************************
void GUI_TOUCH_X_ActivateY(void) 
{
}
//*********************************************************************
//*********************************************************************
int GUI_TOUCH_X_MeasureX(void) 
{ 
	return 	_TouchX;
}
//*********************************************************************
//*********************************************************************
int GUI_TOUCH_X_MeasureY(void)
{
	return 	_TouchY;
}
//#############################################################################################################################
void StartTouchTask(void const * argument)
{
	uint8_t SpiBuffer[3];
	HAL_GPIO_WritePin(_TOUCHSCREEN_CS_GPIO,_TOUCHSCREEN_CS_PIN,GPIO_PIN_SET);
	osDelay(100);	
	SpiBuffer[0]=0x80;
	SpiBuffer[1]=0x00;
	SpiBuffer[2]=0x00;	
	HAL_GPIO_WritePin(_TOUCHSCREEN_CS_GPIO,_TOUCHSCREEN_CS_PIN,GPIO_PIN_RESET);
	osDelay(2);
	HAL_SPI_TransmitReceive(&_TOUCHSCREEN_SPI,SpiBuffer,SpiBuffer,3,100);
	HAL_GPIO_WritePin(_TOUCHSCREEN_CS_GPIO,_TOUCHSCREEN_CS_PIN,GPIO_PIN_SET);
	osDelay(100);
	for(;;)
	{		
		if(HAL_GPIO_ReadPin(_TOUCHSCREEN_IRQ_GPIO,_TOUCHSCREEN_IRQ_PIN)==GPIO_PIN_RESET)
		{
			uint8_t data_x [] = {0xD0,0x00,0x00};
			uint8_t data_y [] = {0x90,0x00,0x00};	
			
			HAL_GPIO_WritePin(_TOUCHSCREEN_CS_GPIO,_TOUCHSCREEN_CS_PIN,GPIO_PIN_RESET);
			osDelay(2);
			HAL_SPI_TransmitReceive(&_TOUCHSCREEN_SPI,data_x,data_x,3,100);
			HAL_GPIO_WritePin(_TOUCHSCREEN_CS_GPIO,_TOUCHSCREEN_CS_PIN,GPIO_PIN_SET);
			osDelay(10);
			HAL_GPIO_WritePin(_TOUCHSCREEN_CS_GPIO,_TOUCHSCREEN_CS_PIN,GPIO_PIN_RESET);
			osDelay(2);
			HAL_SPI_TransmitReceive(&_TOUCHSCREEN_SPI,data_y,data_y,3,100);
			HAL_GPIO_WritePin(_TOUCHSCREEN_CS_GPIO,_TOUCHSCREEN_CS_PIN,GPIO_PIN_SET);
			osDelay(10);
			
			_TouchX = data_x[1];
			_TouchX = ((_TouchX)<<8)+data_x[2];
			_TouchX>>=4;
			
			_TouchY = data_y[1];
			_TouchY = ((_TouchY)<<8)+data_y[2];
			_TouchY>>=4;	

			if(_TouchInCalibMode==1)
				GUI_TOUCH_StoreState(	_TouchX,_TouchY);
		}
		else
		{
			_TouchX=-1;
			_TouchY=-1;
			GUI_TOUCH_StoreState(	_TouchX,_TouchY);
		}
		GUI_TOUCH_Exec();
		osDelay(20);
	}	
}
//#############################################################################################################################
bool SpiTouchEmwin_Init(osPriority Priority)
{
	HAL_GPIO_WritePin(_TOUCHSCREEN_CS_GPIO,_TOUCHSCREEN_CS_PIN,GPIO_PIN_SET);
	osThreadDef(TouchTask, StartTouchTask, Priority, 0, 128);
	TouchTaskHandle = osThreadCreate(osThread(TouchTask), NULL);	
	if(TouchTaskHandle==0)
		return false;
	else
		return true;
}
//#############################################################################################################################
bool SpiTouchEmwin_CalibrateRun(uint8_t	*StoreCalibrateData)
{
	int16_t NUM_CALIB_POINTS = 5;
	int32_t _aRefX[NUM_CALIB_POINTS];
	int32_t _aRefY[NUM_CALIB_POINTS];
	int32_t _aSamX[NUM_CALIB_POINTS];
	int32_t _aSamY[NUM_CALIB_POINTS];
	
  GUI_PID_STATE State;
  int16_t i;
  int16_t xSize, ySize;
	
	WM_Deactivate();
	GUI_SetBkColor(GUI_BLACK);
	GUI_Clear();
	
	_TouchInCalibMode=1;
  xSize = LCD_GetXSize();
  ySize = LCD_GetYSize();
  //
  // Calculate reference points depending on LCD size
  //
  _aRefX[0] = (xSize * 5) / 100;
  _aRefY[0] = (ySize * 5) / 100;
  _aRefX[1] = xSize - (xSize * 5) / 100;
  _aRefY[1] = _aRefY[0];
  _aRefX[2] = _aRefX[1];
  _aRefY[2] = ySize - (ySize * 5) / 100;
  _aRefX[3] = _aRefX[0];
  _aRefY[3] = _aRefY[2];
  _aRefX[4] = xSize / 2;
  _aRefY[4] = ySize / 2;
  //
  // Draw reference points on LCD
  //
  GUI_TOUCH_GetState(&State);
  State.Pressed = 0;
  GUI_SetPenSize(3);
  for (i = 0; i < NUM_CALIB_POINTS; i++) {
    GUI_Clear();
    GUI_DispStringHCenterAt("Please touch the point", LCD_GetXSize() / 2, LCD_GetYSize() / 2 - 60);
    GUI_DrawCircle(_aRefX[i], _aRefY[i], 5);
    while (State.Pressed != 1) {
      GUI_Delay(250);
      GUI_TOUCH_GetState(&State);
    }
    if (State.Pressed == 1) {
			GUI_Delay(100);
      //
      // Store sample points
      //			
      _aSamX[i] = GUI_TOUCH_GetxPhys();
      _aSamY[i] = GUI_TOUCH_GetyPhys();
			while(State.Pressed==1)
			{
				GUI_TOUCH_GetState(&State);
				GUI_Delay(100);
			}
    }
    State.Pressed = 0;
    GUI_Delay(500);
  }
  //
  // Pass measured points to emWin
  //
  GUI_TOUCH_CalcCoefficients(NUM_CALIB_POINTS,(int*)_aRefX,(int*)_aRefY,(int*)_aSamX,(int*)_aSamY, xSize, ySize);
	uint8_t Bytes=0;
	Bytes = sizeof(_aRefX)+sizeof(_aRefY)+sizeof(_aSamX)+sizeof(_aSamY)+sizeof(xSize)+sizeof(ySize);

	if(StoreCalibrateData!=NULL)
	{
		GUI__memcpy(StoreCalibrateData,&_aRefX,sizeof(_aRefX));
		StoreCalibrateData+=sizeof(_aRefX);
		GUI__memcpy(StoreCalibrateData,&_aRefY,sizeof(_aRefY));
		StoreCalibrateData+=sizeof(_aRefY);
		GUI__memcpy(StoreCalibrateData,&_aSamX,sizeof(_aSamX));
		StoreCalibrateData+=sizeof(_aSamX);
		GUI__memcpy(StoreCalibrateData,&_aSamY,sizeof(_aSamY));
		StoreCalibrateData+=sizeof(_aSamY);
		GUI__memcpy(StoreCalibrateData,&xSize,sizeof(xSize));
		StoreCalibrateData+=sizeof(xSize);
		GUI__memcpy(StoreCalibrateData,&ySize,sizeof(ySize));
		StoreCalibrateData+=sizeof(ySize);
		}
	_TouchInCalibMode=0;
	WM_Activate();
	GUI_Clear();
	if(Bytes>20)
		return true;
	else
		return false;
}
//#############################################################################################################################
void		  SpiTouchEmwin_CalibrateLoad(uint8_t	*StoreCalibrateData)
{
	int16_t  NUM_CALIB_POINTS = 5;
	int32_t _aRefX[NUM_CALIB_POINTS];
	int32_t _aRefY[NUM_CALIB_POINTS];
	int32_t _aSamX[NUM_CALIB_POINTS];
	int32_t _aSamY[NUM_CALIB_POINTS];
	int16_t xSize, ySize;
 
	GUI__memcpy(&_aRefX,StoreCalibrateData,sizeof(_aRefX));
	StoreCalibrateData+=sizeof(_aRefX);
	GUI__memcpy(&_aRefY,StoreCalibrateData,sizeof(_aRefY));
	StoreCalibrateData+=sizeof(_aRefY);
	GUI__memcpy(&_aSamX,StoreCalibrateData,sizeof(_aSamX));
	StoreCalibrateData+=sizeof(_aSamX);
	GUI__memcpy(&_aSamY,StoreCalibrateData,sizeof(_aSamY));
	StoreCalibrateData+=sizeof(_aSamY);
	GUI__memcpy(&xSize,StoreCalibrateData,sizeof(xSize));
	StoreCalibrateData+=sizeof(xSize);
	GUI__memcpy(&ySize,StoreCalibrateData,sizeof(ySize));
	StoreCalibrateData+=sizeof(ySize);
  
  GUI_TOUCH_CalcCoefficients(NUM_CALIB_POINTS,(int*)_aRefX,(int*)_aRefY,(int*)_aSamX,(int*)_aSamY, xSize, ySize);
}
//#############################################################################################################################
void	SpiTouchEmwin_Test(void)
{
		GUI_PID_STATE TouchState;
  int           xPhys;
  int           yPhys;
	GUI_CURSOR_Show();
  GUI_CURSOR_Select(&GUI_CursorCrossL);
  GUI_SetBkColor(GUI_WHITE);
  GUI_SetColor(GUI_BLACK);
	WM_Deactivate();
  GUI_Clear();
  GUI_DispString("Measurement of\nA/D converter values");
  while (1) {
    GUI_TOUCH_GetState(&TouchState);  // Get the touch position in pixel
    xPhys = GUI_TOUCH_GetxPhys();     // Get the A/D mesurement result in x
    yPhys = GUI_TOUCH_GetyPhys();     // Get the A/D mesurement result in y
    //
    // Display the measurement result
    //
    GUI_SetColor(GUI_BLUE);
    GUI_DispStringAt("Analog input:\n", 0, 20);
    GUI_GotoY(GUI_GetDispPosY() + 2);
    GUI_DispString("x:");
    GUI_DispDec(xPhys, 4);
    GUI_DispString(", y:");
    GUI_DispDec(yPhys, 4);
    //
    // Display the according position
    //
    GUI_SetColor(GUI_RED);
    GUI_GotoY(GUI_GetDispPosY() + 4);
    GUI_DispString("\nPosition:\n");
    GUI_GotoY(GUI_GetDispPosY() + 2);
    GUI_DispString("x:");
    GUI_DispDec(TouchState.x,4);
    GUI_DispString(", y:");
    GUI_DispDec(TouchState.y,4);
    //
    // Wait a while
    //
    GUI_Delay(100);
  };	
}
//#############################################################################################################################
