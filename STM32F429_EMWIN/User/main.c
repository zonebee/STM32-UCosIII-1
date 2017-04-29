/**
 *	Keil project example for emWin
 *
 *	Before you start, select your target, on the right of the "Load" button
 *
 *	@author		Tilen Majerle
 *	@email		tilen@majerle.eu
 *	@website	http://stm32f4-discovery.com
 *	@ide		Keil uVision 5
 *	@packs		STM32F4xx Keil packs version 2.2.0 or greater required
 *	@stdperiph	STM32F4xx Standard peripheral drivers version 1.4.0 or greater required
 */
/* Include core modules */
#include "stm32f4xx.h"
/* Include my libraries here */
#include "defines.h"
#include "tm_stm32f4_delay.h"
#include "tm_stm32f4_disco.h"
#include "tm_stm32f4_emwin.h"
/* GUI modules */
#include "button.h"
#include "DIALOG.h"

#include "includes.h"

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

                                                                /* ----------------- APPLICATION GLOBALS -------------- */
static  OS_TCB   AppTaskStartTCB;//创建一个任务时，必须为该任务分配一个任务控制块(OS_TCB)
static  CPU_STK  AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];//为任务分配一个任务栈 类型必须是CPU_STK

static  OS_TCB   AppTask1TCB;
static  CPU_STK  AppTask1Stk[APP_TASK1_STK_SIZE];

static  OS_TCB   AppTask2TCB;
static  CPU_STK  AppTask2Stk[APP_TASK2_STK_SIZE];

                                                                /* ------------ FLOATING POINT TEST TASK -------------- */
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskStart          (void     *p_arg);//拟创建任务的函数原型声明
static  void  AppTask1							(void     *p_arg);
static  void  AppTask2							(void     *p_arg);
static  void  AppTaskCreate         (void);

int main(void) {
	
	  OS_ERR  err;//声明一个变量  err 用于接受产生的错误
	
    BSP_IntDisAll();                                            /* Disable all interrupts.关闭所有中断                  */
     //大多数CPU最初启动时中断是关闭的，直到应用程序开中断  这里为安全起见  关一次所有中断，确保系统启动期间中断是关闭的                                              
    OSInit(&err);                                               /* Init uC/OS-III.    初始化系统	*/
		if(err != OS_ERR_NONE) {/*检查错误*/
			/* Something didn't get initialized correctly....													*/
			/*... check os.h for the meaning of the error code, see OS_ERR_xxxx       */
		}

    OSTaskCreate((OS_TCB       *)&AppTaskStartTCB,              /* Create the start task                                */
                 (CPU_CHAR     *)"App Task Start",
                 (OS_TASK_PTR   )AppTaskStart,
                 (void         *)0u,
                 (OS_PRIO       )APP_CFG_TASK_START_PRIO,
                 (CPU_STK      *)&AppTaskStartStk[0u],
                 (CPU_STK_SIZE  )AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE / 10u],
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE,
                 (OS_MSG_QTY    )0u,
                 (OS_TICK       )0u,
                 (void         *)0u,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);

    OSStart(&err);                                              /* Start multitasking (i.e. give control to uC/OS-III). */

    (void)&err;

    return (0u);

}

static  void  AppTaskCreate (void)
{
  OS_ERR      err;
	
	/***********************************/
	OSTaskCreate((OS_TCB       *)&AppTask1TCB,             
                 (CPU_CHAR     *)"App Task1",
                 (OS_TASK_PTR   )AppTask1, 
                 (void         *)0,
                 (OS_PRIO       )APP_TASK1_PRIO,
                 (CPU_STK      *)&AppTask1Stk[0],
                 (CPU_STK_SIZE  )APP_TASK1_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_TASK1_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
	
	/***********************************/
	OSTaskCreate((OS_TCB       *)&AppTask2TCB,            
                 (CPU_CHAR     *)"App Task2",
                 (OS_TASK_PTR   )AppTask2, 
                 (void         *)0,
                 (OS_PRIO       )APP_TASK2_PRIO,
                 (CPU_STK      *)&AppTask2Stk[0],
                 (CPU_STK_SIZE  )APP_TASK2_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_TASK2_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
	
}

static  void  AppTaskStart (void *p_arg)
{
    OS_ERR  err;

  (void)p_arg;

  BSP_Init();  																								/* Initialize BSP functions                             */
  
	BSP_Tick_Init();
  CPU_Init();

  Mem_Init();                                                 /* Initialize Memory Management Module                  */

#if OS_CFG_STAT_TASK_EN > 0u
  OSStatTaskCPUUsageInit(&err);                               /* Compute CPU capacity with no task running            */
#endif

  APP_TRACE_INFO(("Creating Application Tasks...\n\r"));
  AppTaskCreate();                                            /* Create Application Tasks                             */


  /* 删除任务 */   	
	OSTaskDel(&AppTaskStartTCB,&err);	
	
}


static  void  AppTask2 (void *p_arg)	
{	
   (void)p_arg;

	BUTTON_Handle hButton, hB1, hB2, hB3, hB4;
	PROGBAR_Handle hProgbar;
	uint8_t i;
	
	/* Initialize system */
	
	
	/* Initialize delay functions */
//	TM_DELAY_Init();
	
	/* Initialize LEDs */
	TM_DISCO_LedInit();
	
	/* Initialize emWin */
	if (TM_EMWIN_Init() != TM_EMWIN_Result_Ok) {
		/* Initialization error */
		while (1) {
			/* Toggle RED led */
			TM_DISCO_LedToggle(LED_RED);
			
			/* Delay */
			//OSTimeDly(100, OS_OPT_TIME_DLY, (OS_ERR *)&err);
			Delayms(100);
		}
	}
	
	/* Create progress bar at location x = 10, y = 10, length = 219, height = 30 */
	hProgbar = PROGBAR_CreateEx(10, 10, 219, 30, 0, WM_CF_SHOW, 0, GUI_ID_PROGBAR0);
	/* Set progress bar font */
	PROGBAR_SetFont(hProgbar, &GUI_Font8x16);
	/* Set progress bar text */
	PROGBAR_SetText(hProgbar, "LOADING..Please wait..");
	
	/* Imitate loading */
	for (i = 1; i <= 100; i++) {
		/* Set bar */
		PROGBAR_SetValue(hProgbar, i);
		/* Little delay, update value on LCD */
		GUI_Delay(20);
	}
	
	/* Create button with GUI_ID_OK ID number */
	hButton = BUTTON_CreateEx(10, 50, 219, 30, 0, WM_CF_SHOW, 0, GUI_ID_BUTTON0);
	/* Set text and font */
	BUTTON_SetText(hButton, "Click me to continue..");
	BUTTON_SetFont(hButton, &GUI_Font8x15B_ASCII);
	
	/* Execute, show button */
	GUI_Exec();
	
	/* Wait till button pressed */
	while (1) {
		/* Check if button was pressed */
		if (GUI_GetKey() == GUI_ID_BUTTON0) {
			/* Led Off */
			TM_DISCO_LedOff(LED_GREEN);
			
			/* Stop while loop */
			break;
		}
		/* Toggle green led */
		TM_DISCO_LedToggle(LED_GREEN);
		/* Delay 100ms */
		GUI_Delay(100);
	}

	/* Delete button functionality */
	BUTTON_Delete(hButton);
	/* Delete button from LCD */
	GUI_ClearRect(10, 50, 269, 90);
	
	/* Create buttons for leds control */
	hB1 = BUTTON_CreateEx(10, 90, 105, 50, 0, WM_CF_SHOW, 0, GUI_ID_BUTTON1);
	hB2 = BUTTON_CreateEx(124, 90, 105, 50, 0, WM_CF_SHOW, 0, GUI_ID_BUTTON2);
	hB3 = BUTTON_CreateEx(10, 150, 105, 50, 0, WM_CF_SHOW, 0, GUI_ID_BUTTON3);
	hB4 = BUTTON_CreateEx(124, 150, 105, 50, 0, WM_CF_SHOW, 0, GUI_ID_BUTTON4);
	
	/* Set font for buttons */
	BUTTON_SetFont(hB1, &GUI_Font13HB_ASCII);
	BUTTON_SetFont(hB2, &GUI_Font13HB_ASCII);
	BUTTON_SetFont(hB3, &GUI_Font13HB_ASCII);
	BUTTON_SetFont(hB4, &GUI_Font13HB_ASCII);
	
	/* Set button text */
	BUTTON_SetText(hB1, "GREEN on");
	BUTTON_SetText(hB2, "GREEN off");
	BUTTON_SetText(hB3, "RED on");
	BUTTON_SetText(hB4, "RED off");
	
	/* Button styling */
	/* Background color when button is not pressed */
	BUTTON_SetBkColor(hB1, BUTTON_CI_UNPRESSED, GUI_DARKGREEN);
	/* Background color when button is pressed */
	BUTTON_SetBkColor(hB1, BUTTON_CI_PRESSED, GUI_GREEN);
	
	/* Background color when button is not pressed */
	BUTTON_SetBkColor(hB3, BUTTON_CI_UNPRESSED, GUI_DARKRED);
	/* Background color when button is pressed */
	BUTTON_SetBkColor(hB3, BUTTON_CI_PRESSED, GUI_RED);
	
	/* Show buttons */
	GUI_Exec();
	
	while (1) {
		/* Get pressed key */
		switch (GUI_GetKey()) {
			case GUI_ID_BUTTON1:
				/* Button 1 pressed */
				TM_DISCO_LedOn(LED_GREEN);
				break;
			case GUI_ID_BUTTON2:
				/* Button 2 pressed */
				TM_DISCO_LedOff(LED_GREEN);
				break;
			case GUI_ID_BUTTON3:
				/* Button 3 pressed */
				TM_DISCO_LedOn(LED_RED);
				break;
			case GUI_ID_BUTTON4:
				/* Button 4 pressed */
				TM_DISCO_LedOff(LED_RED);
				break;
			default:
				break;
		}
	}
}

static void AppTask1 (void *p_arg) 
{

	OS_ERR      err;
(void)p_arg;
while(DEF_TRUE) 
 {
 OSTimeDly(2, OS_OPT_TIME_DLY, &err);
 TM_EMWIN_UpdateTouch(); 
  }
}

/* User handler for 1ms interrupts */
void TM_DELAY_1msHandler(void) {
	/* Call periodically each 1ms */
	TM_EMWIN_UpdateTouch();
}
