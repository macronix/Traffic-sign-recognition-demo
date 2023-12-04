/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>

#include "image_utils.h"
#include "eiq_video_worker.h"
#include "eiq_display.h"
#include "eiq_camera.h"
#include "stdbool.h"
#include "fsl_debug_console.h"

#include "30_speed.c"
#include "attention_general.c"
#include "lifted_general.c"
#include "give_way.c"
#include "no_way_general.c"
#include "no_way_one_way.c"
#include "right_of_way_general.c"
#include "stop.c"
#include "turn_right_down.c"
#include "turn_straight.c"
#include "turn_straight_right.c"


#ifndef EIQ_COUNTER_TOP
#define EIQ_COUNTER_TOP 4
#endif

EIQ_VideoWorker_t worker;

EIQ_IWorkerUpdater_t handler;
Rect_t window;
volatile uint8_t counter = EIQ_COUNTER_TOP;

uint8_t *pExtract = NULL;

extern int result;

static status_t start()
{

  if(window.height == 0 || window.width == 0){
    printf("Default capture rate width = %d%%,%d%% is used.\r\n",
           EIQ_DEFAULT_CAPTURE_RATE, EIQ_DEFAULT_CAPTURE_RATE);
#ifdef EIQ_CAPTURE_RECT
    worker.setCaptureWindowRectRate(EIQ_DEFAULT_CAPTURE_RATE,
                                    EIQ_DEFAULT_CAPTURE_RATE);
#else
    worker.setCaptureWindowHeightRate(EIQ_DEFAULT_CAPTURE_RATE);
#endif
  }

  worker.receiver->base.start();
  worker.sender->base.start();
  worker.sender->setReadyCallback(worker.receiver->base.notify);
  worker.receiver->setReadyCallback(worker.base.refresh);
  worker.receiver->base.notify();

  return kStatus_Success;
}

static bool isReady()
{
  return !counter;
}

static void notify()
{
  worker.receiver->base.notify();
}

static Dims_t getResolution(){
  if(pExtract == NULL){
#ifdef EIQ_CAPTURE_RECT
    worker.setCaptureWindowRectRate(EIQ_DEFAULT_CAPTURE_RATE,
                                    EIQ_DEFAULT_CAPTURE_RATE);
#else
    worker.setCaptureWindowHeightRate(EIQ_DEFAULT_CAPTURE_RATE);
#endif
  }

  Dims_t dims;
  dims.width = window.width;
  dims.height = window.height;
  return dims;
}

static uint8_t* getData()
{
  if(pExtract == NULL){
#ifdef EIQ_CAPTURE_RECT
    worker.setCaptureWindowRectRate(EIQ_DEFAULT_CAPTURE_RATE,
                                    EIQ_DEFAULT_CAPTURE_RATE);
#else
    worker.setCaptureWindowHeightRate(EIQ_DEFAULT_CAPTURE_RATE);
#endif
  }

  return (uint8_t*)pExtract;
}

uint8_t buffer=0;
static void refresh(uint32_t bufferAddr)
{
  uint32_t lcdAddr = worker.sender->getEmptyBuffer();
  EIQ_PXP_Rotate(bufferAddr, lcdAddr);
#ifndef EIQ_NO_EXTRACT
  /* Check if camera buffer is extracted for new inference. */
  if (++counter > EIQ_COUNTER_TOP)
  {
    /* Extract image from camera. */
    IMAGE_ExtractRect(pExtract, window.x, window.y, window.width,
                      window.height, (uint16_t*)lcdAddr, DEMO_PANEL_WIDTH);
    /* Draw red rectangle. */
    IMAGE_DrawRect((uint16_t*)lcdAddr, window.x, window.y, window.width,
                   window.height, 255, 0, 0, DEMO_PANEL_WIDTH);

    counter = 0;
    if(handler != NULL){
        handler((EIQ_IWorker_t*)&worker);
    }
  }
  else
  {
    /* Draw white rectangle. */
    IMAGE_DrawRect((uint16_t*)lcdAddr, window.x, window.y, window.width,
                   window.height, 255, 255, 255, DEMO_PANEL_WIDTH);
  }
#endif

  if (buffer)
	  buffer = 0;
  else
	  buffer = 1;

  LCD_Change_Buffer(buffer);

  if (result == 3) {
	  GUI_BMP_DrawScaled(_ac30_speed, 0, 0, 2, 1);
	 // GUI_DispStringAt("30_speed ", 500, 170);
  } else if (result == 16) {
	  GUI_BMP_DrawScaled(_acattention_general, 0, 0, 2, 1);
	 // GUI_DispStringAt("attention_general ", 500, 170);
  } else if (result == 24) {
	  GUI_BMP_DrawScaled(_aclifted_general, 0, 0, 2, 1);
	 // GUI_DispStringAt("lifted_general ", 500, 170);
  } else if (result == 23) {
	  GUI_BMP_DrawScaled(_acgive_way, 0, 0, 2, 1);
	 // GUI_DispStringAt("give_way ", 500, 170);
  } else if (result == 29) {
	  GUI_BMP_DrawScaled(_acno_way_general, 0, 0, 2, 1);
	 // GUI_DispStringAt("no_way_general ", 500, 170);
  } else if (result == 30) {
	  GUI_BMP_DrawScaled(_acno_way_one_way, 0, 0, 2, 1);
	 // GUI_DispStringAt("no_way_one_way ", 500, 170);
  } else if (result == 33) {
	  GUI_BMP_DrawScaled(_acright_of_way_general, 0, 0, 2, 1);
	 // GUI_DispStringAt("right_of_way_general ", 500, 170);
  } else if (result == 34) {
	  GUI_BMP_DrawScaled(_acstop, 0, 0, 2, 1);
	 // GUI_DispStringAt("stop ", 500, 170);
  //} else if (result == 39) {
	//  GUI_BMP_DrawScaled(_acturn_right_down, 0, 0, 2, 1);
	 // GUI_DispStringAt("turn_right_down ", 500, 170);
  } else if (result == 40) {
	  GUI_BMP_DrawScaled(_acturn_straight, 0, 0, 2, 1);
	 // GUI_DispStringAt("turn_straight ", 500, 170);
  } else if (result == 42) {
	  GUI_BMP_DrawScaled(_acturn_straight_right, 0, 0, 2, 1);
	 // GUI_DispStringAt("turn_straight_right ", 500, 170);
  }


  worker.sender->base.notify();

}

static void setReadyCallback(EIQ_IWorkerUpdater_t iworker)
{
  if(iworker != NULL)
  {
      handler = iworker;
  }
}

#ifdef EIQ_CAPTURE_RECT
static status_t setCaptureWindowRectRate(int heightRate, int widthRate)
{
  if(heightRate < 10){
      printf("Incorrect input rate. Set value between 10-100.\r\n");
      return kStatus_Fail;
  }

  Dims_t lcdDims = worker.sender->base.getResolution();
  int dy = (lcdDims.height *(100 - heightRate)) / 200;
  int dx = (lcdDims.width *(100 - widthRate)) / 200;
  window.x = dx;
  window.y = dy;
  window.width = lcdDims.width - 2*dx;
  window.height = lcdDims.height - 2*dy;

  if(pExtract != NULL){
      free(pExtract);
  }

  pExtract = (uint8_t*)malloc(window.width * window.height * 3);
  if(pExtract == NULL){
      printf("Unable to allocate internal buffer");
      return kStatus_Fail;
  }

  return kStatus_Success;
}
#else

static status_t setCaptureWindowHeightRate(int heightRate)
{
  if(heightRate < 10){
      printf("Incorrect input rate. Set value between 10-100.\r\n");
      return kStatus_Fail;
  }

  Dims_t lcdDims = worker.sender->base.getResolution();
  int dy = (lcdDims.height *(100 - heightRate)) / 200;

  window.y = dy;
  window.height = lcdDims.height - 2*dy;
  window.width = window.height;
  window.x = (lcdDims.width - window.width) / 2;

  if(pExtract != NULL){
    free(pExtract);
  }

  pExtract = (uint8_t*)malloc(window.width * window.height * 3);
  if(pExtract == NULL){
    printf("Unable to allocate internal buffer");
    return kStatus_Fail;
  }

  return kStatus_Success;
}
#endif

EIQ_VideoWorker_t* EIQ_VideoWorkerInit()
{
  worker.base.start = start;
  worker.base.isReady = isReady;
  worker.base.getResolution = getResolution;
  worker.base.notify = notify;
  worker.base.getData = getData;
  worker.base.refresh = refresh;
  worker.base.setReadyCallback = setReadyCallback;
#ifdef EIQ_CAPTURE_RECT
  worker.setCaptureWindowRectRate = setCaptureWindowRectRate;
#else
  worker.setCaptureWindowHeightRate = setCaptureWindowHeightRate;
#endif
  EIQ_PXP_Init();
  worker.receiver = EIQ_CameraInit();
  worker.sender = EIQ_DisplayInit();

  return &worker;
}

