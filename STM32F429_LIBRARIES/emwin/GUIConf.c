/*********************************************************************
*          Portions COPYRIGHT 2013 STMicroelectronics                *
*          Portions SEGGER Microcontroller GmbH & Co. KG             *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2012  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.22 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The software  has been  licensed to  STMicroelectronics  International
N.V. whose  registered office  is situated at Plan-les-Ouates, Geneva,
39 Chemin du Champ des Filles,  Switzerland solely for the purposes of
creating libraries for  STMicroelectronics  ARM Cortex™-M-based 32-bit
microcontroller    products,    sublicensed    and    distributed   by
STMicroelectronics  under  the  terms  and  conditions of the End User
License Agreement supplied with  the software. The use of the software
on a stand-alone basis  or for any purpose other  than to operate with
the specified  microcontroller is prohibited and subject to a separate
license agreement.

Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUIConf.c
Purpose     : Display controller initialization
---------------------------END-OF-HEADER------------------------------
*/

/**
  ******************************************************************************
  * @attention
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

#include "GUI.h"
#include "tm_stm32f4_delay.h"
#include "tm_stm32f4_emwin.h"
#include "defines.h"
#include "includes.h"
//
// Define the available number of bytes available for the GUI
//

#if TM_EMWIN_ROTATE_LCD == 1
	#define GUI_NUMBYTES	(1024 * 128 * 4)
#else
	#define GUI_NUMBYTES  	(1024 * 100)    // x KByte
#endif
/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
#if defined ( __CC_ARM   ) 
	U32 HeapMem[1024 * 1024] __attribute__((at(0xD0100000)));
#elif defined ( __ICCARM__ ) 
	#pragma location = 0xD0100000
	static __no_init U32 HeapMem[1024 * 1024];
#elif defined   (  __GNUC__  ) 
	U32 HeapMem[1024 * 1024] __attribute__((section(".HeapMemSection")));
#endif

#if TM_EMWIN_ROTATE_LCD == 1
	#if defined ( __CC_ARM   ) 
		U32 extMem[GUI_NUMBYTES / 4] __attribute__((at(0xD0080000)));
	#elif defined ( __ICCARM__ ) 
		#pragma location = 0xD0080000
		U32 extMem[GUI_NUMBYTES / 4];
	#elif defined   (  __GNUC__  ) 
		U32 extMem[GUI_NUMBYTES / 4] __attribute__((at(0xD0080000)));
	#endif
#else
	U32 extMem[GUI_NUMBYTES / 4];
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*
*********************************************************************************************************
*                                         GLOBAL VARIABLES
*********************************************************************************************************
*/

static  OS_SEM  DispSem;
static  OS_SEM  EventSem;

static  OS_SEM  KeySem;
static  int     KeyPressed;
static  char    KeyIsInited;

/*
*********************************************************************************************************
*                                        TIMING FUNCTIONS
*
* Notes: Some timing dependent routines of uC/GUI require a GetTime and delay funtion. 
*        Default time unit (tick), normally is 1 ms.
*********************************************************************************************************
*/

int  GUI_X_GetTime (void) 
{
	OS_ERR      err;
  return OSTimeGet(&err);
}


void  GUI_X_Delay (int period) 
{
  OS_ERR      err;
	
  OSTimeDly(period, OS_OPT_TIME_DLY, &err);
}


/*********************************************************************
*
*       GUI_X_Config
*
* Purpose:
*   Called during the initialization process in order to set up the
*   available memory for the GUI.
*/
void GUI_X_Config(void)
{
  GUI_ALLOC_AssignMemory(extMem, GUI_NUMBYTES);
}

void GUI_X_ExecIdle (void) 
{
    GUI_X_Delay(1);
}


/*
*********************************************************************************************************
*                                    MULTITASKING INTERFACE FUNCTIONS
*
* Note(1): 1) The following routines are required only if uC/GUI is used in a true multi task environment, 
*             which means you have more than one thread using the uC/GUI API.  In this case the #define 
*             GUI_OS 1   needs to be in GUIConf.h
*********************************************************************************************************
*/

void  GUI_X_InitOS (void)
{ 
	OS_ERR     err;

	/* ÓÃÓÚ×ÊÔ´¹²Ïí cnt = 1*/
    OSSemCreate((OS_SEM    *)&DispSem,
                (CPU_CHAR  *)"DispSem",
                (OS_SEM_CTR )1,
                (OS_ERR    *)&err);
	/* ÓÃÓÚÊÂ¼þ´¥·¢ cnt = 0*/
	OSSemCreate((OS_SEM    *)&EventSem,
                (CPU_CHAR  *)"EventSem",
                (OS_SEM_CTR )0,
                (OS_ERR    *)&err);
}


void  GUI_X_Lock (void)
{ 
  OS_ERR     err;
	   
	OSSemPend((OS_SEM *)&DispSem,
			  (OS_TICK )0,
			  (OS_OPT  )OS_OPT_PEND_BLOCKING,
			  0,
			  (OS_ERR *)&err);
}


void  GUI_X_Unlock (void)
{ 
  OS_ERR     err;

	OSSemPost((OS_SEM *)&DispSem,
	          (OS_OPT  )OS_OPT_POST_1,
	          (OS_ERR *)&err);
}


U32  GUI_X_GetTaskId (void) 
{ 
    return ((U32)(OSTCBCurPtr->Prio));
}

/*
*********************************************************************************************************
*                                        GUI_X_WaitEvent()
*                                        GUI_X_SignalEvent()
*********************************************************************************************************
*/


void GUI_X_WaitEvent (void) 
{
  OS_ERR     err;
	   
	OSSemPend((OS_SEM *)&EventSem,
						(OS_TICK )0,
						(OS_OPT  )OS_OPT_PEND_BLOCKING,
						0,
						(OS_ERR *)&err);
}


void GUI_X_SignalEvent (void) 
{
  OS_ERR     err;

	OSSemPost((OS_SEM *)&EventSem,
	      (OS_OPT  )OS_OPT_POST_1,
	      (OS_ERR *)&err);
}

/*
*********************************************************************************************************
*                                      KEYBOARD INTERFACE FUNCTIONS
*
* Purpose: The keyboard routines are required only by some widgets.
*          If widgets are not used, they may be eliminated.
*
* Note(s): If uC/OS-II is used, characters typed into the log window will be placed	in the keyboard buffer. 
*          This is a neat feature which allows you to operate your target system without having to use or 
*          even to have a keyboard connected to it. (useful for demos !)
*********************************************************************************************************
*/

static  void  CheckInit (void) 
{
   if (KeyIsInited == DEF_FALSE) {
		KeyIsInited = DEF_TRUE;
		GUI_X_Init();
	}
}


void GUI_X_Init (void) 
{
  OS_ERR err;
	
	OSSemCreate((OS_SEM    *)&KeySem,
				(CPU_CHAR  *)"KeySem",
				(OS_SEM_CTR )0,
				(OS_ERR    *)&err);
}


int  GUI_X_GetKey (void) 
{
   int r;

	r          = KeyPressed;
	CheckInit();
	KeyPressed = 0;
	return (r);
}


int  GUI_X_WaitKey (void) 
{
  int    r;
	OS_ERR err;


	CheckInit();
	if (KeyPressed == 0) {   
		OSSemPend((OS_SEM *)&EventSem,
		(OS_TICK )0,
		(OS_OPT  )OS_OPT_PEND_BLOCKING,
		0,
		(OS_ERR *)&err);
	}
	r          = KeyPressed;
	KeyPressed = 0;
	return (r);		   
}


void  GUI_X_StoreKey (int k) 
{
   OS_ERR     err;
	
	KeyPressed = k;
	OSSemPost((OS_SEM *)&KeySem,
	      (OS_OPT  )OS_OPT_POST_1,
	      (OS_ERR *)&err);
}
void GUI_X_Log     (const char *s) { GUI_USE_PARA(s); }
void GUI_X_Warn    (const char *s) { GUI_USE_PARA(s); }
void GUI_X_ErrorOut(const char *s) { GUI_USE_PARA(s); }


/*************************** End of file ****************************/
