/*********************************************************************
*                SEGGER Microcontroller GmbH                         *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2020  SEGGER Microcontroller GmbH                *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V6.14 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The software has been licensed to  NXP Semiconductors USA, Inc.  whose
registered  office  is  situated  at 411 E. Plumeria Drive, San  Jose,
CA 95134, USA  solely for  the  purposes  of  creating  libraries  for
NXPs M0, M3/M4 and  ARM7/9 processor-based  devices,  sublicensed  and
distributed under the terms and conditions of the NXP End User License
Agreement.
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
Licensing information
Licensor:                 SEGGER Microcontroller Systems LLC
Licensed to:              NXP Semiconductors, 1109 McKay Dr, M/S 76, San Jose, CA 95131, USA
Licensed SEGGER software: emWin
License number:           GUI-00186
License model:            emWin License Agreement, dated August 20th 2011 and Amendment No. 1, dated October 17th 2017 and Amendment No. 2, dated December 18th 2018
Licensed platform:        NXP's ARM 7/9, Cortex-M0, M3, M4, M7, A7, M33
----------------------------------------------------------------------
Support and Update Agreement (SUA)
SUA period:               2011-08-19 - 2020-09-02
Contact to extend SUA:    sales@segger.com
----------------------------------------------------------------------
File        : LISTBOX_Private.h
Purpose     : Private LISTBOX include
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef LISTBOX_PRIVATE_H
#define LISTBOX_PRIVATE_H

#include "GUI_Private.h"
#include "WM_Intern.h"
#include "LISTBOX.h"
#include "GUI_ARRAY.h"
#include "WIDGET.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define LISTBOX_ITEM_SELECTED (1 << 0)
#define LISTBOX_ITEM_DISABLED (1 << 1)

/*********************************************************************
*
*       Object definition
*
**********************************************************************
*/

typedef struct {
  U16  xSize, ySize;
  I32  ItemPosY;
  U8   Status;
  char acText[1];
} LISTBOX_ITEM;

typedef struct {
  const GUI_FONT * pFont;
  U16              ScrollStepH;
  GUI_COLOR aBackColor[4];
  GUI_COLOR aTextColor[4];
  GUI_COLOR aScrollbarColor[3];
  I16 Align;
} LISTBOX_PROPS;

typedef struct {
  WIDGET Widget;
  GUI_ARRAY ItemArray;
  WIDGET_DRAW_ITEM_FUNC* pfDrawItem;
  WM_SCROLL_STATE ScrollStateV;
  WM_SCROLL_STATE ScrollStateH;
  LISTBOX_PROPS Props;
  WM_HWIN hOwner;
  I16 Sel;                        /* current selection */
  U8  Flags;
  U8  ScrollbarWidth;
  int MotionPosY;
  U16 ItemSpacing;
  U16 ContentSizeX;
  U16 FixedScrollPos;
} LISTBOX_Obj;

/*********************************************************************
*
*       Macros for internal use
*
**********************************************************************
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  #define LISTBOX_INIT_ID(p) p->Widget.DebugId = LISTBOX_ID
#else
  #define LISTBOX_INIT_ID(p)
#endif

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  LISTBOX_Obj * LISTBOX_LockH(LISTBOX_Handle h);
  #define LISTBOX_LOCK_H(h)   LISTBOX_LockH(h)
#else
  #define LISTBOX_LOCK_H(h)   (LISTBOX_Obj *)GUI_LOCK_H(h)
#endif

/*********************************************************************
*
*       Private (module internal) data
*
**********************************************************************
*/

extern LISTBOX_PROPS LISTBOX_DefaultProps;

/*********************************************************************
*
*       Private (module internal) functions
*
**********************************************************************
*/
unsigned     LISTBOX__GetNumItems           (const LISTBOX_Obj * pObj);
const char * LISTBOX__GetpStringLocked      (LISTBOX_Handle hObj, int Index, LISTBOX_ITEM ** ppItem);
void         LISTBOX__InvalidateInsideArea  (LISTBOX_Handle hObj);
void         LISTBOX__InvalidateItem        (LISTBOX_Handle hObj, int Sel);
void         LISTBOX__InvalidateItemAndBelow(LISTBOX_Handle hObj, int Sel);
void         LISTBOX__InvalidateItemSize    (const LISTBOX_Obj * pObj, unsigned Index);
void         LISTBOX__SetScrollbarColor     (LISTBOX_Handle hObj, const LISTBOX_Obj * pObj);
void         LISTBOX__SetScrollbarWidth     (LISTBOX_Handle hObj, const LISTBOX_Obj * pObj);
void         LISTBOX__AddSize               (LISTBOX_Obj * pObj, int Index);

#endif /* GUI_WINSUPPORT */

#else                            /* Avoid problems with empty object modules */
  void LISTBOX_C(void) {}
#endif

/*************************** End of file ****************************/
