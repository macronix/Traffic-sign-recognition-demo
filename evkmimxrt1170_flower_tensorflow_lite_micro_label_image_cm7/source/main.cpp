/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <iostream>

#include "board_init.h"
#include "demo_config.h"
#include "demo_info.h"
#include "image_from_eiq.h"
#include "image_utils.h"
#include "model.h"
#include "output_postproc.h"
#include "timer.h"

#include "GUI.h"
#include "emwin_support.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_sai.h"
#include "music.h"
#include "fsl_codec_common.h"
#include "fsl_wm8960.h"
#include "fsl_codec_adapter.h"

#include "board.h"
#include "fsl_sai.h"
#include "fsl_codec_common.h"
#include "fsl_wm8960.h"
#include "fsl_codec_adapter.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* SAI instance and clock */
#define DEMO_CODEC_WM8960
#define DEMO_SAI SAI1
#define DEMO_SAI_CHANNEL (0)
#define DEMO_SAI_IRQ SAI1_IRQn
#define DEMO_SAITxIRQHandler  SAI1_IRQHandler
#define DEMO_SAI_TX_SYNC_MODE kSAI_ModeAsync
#define DEMO_SAI_RX_SYNC_MODE kSAI_ModeSync
#define DEMO_SAI_MASTER_SLAVE kSAI_Master

#define DEMO_AUDIO_DATA_CHANNEL (1U)
#define DEMO_AUDIO_BIT_WIDTH    kSAI_WordWidth16bits
#define DEMO_AUDIO_SAMPLE_RATE  (kSAI_SampleRate24KHz)
#define DEMO_AUDIO_MASTER_CLOCK DEMO_SAI_CLK_FREQ

/* Get frequency of sai1 clock */
#define DEMO_SAI_CLK_FREQ CLOCK_GetRootClockFreq(kCLOCK_Root_Sai1)

/* I2C instance and clock */
#define DEMO_I2C LPI2C5

/* Get frequency of lpi2c clock */
#define DEMO_I2C_CLK_FREQ CLOCK_GetRootClockFreq(kCLOCK_Root_Lpi2c5)
#define BOARD_MASTER_CLOCK_CONFIG()
#define BOARD_SAI_RXCONFIG(config, mode)
#ifndef DEMO_CODEC_INIT_DELAY_MS
#define DEMO_CODEC_INIT_DELAY_MS (1000U)
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern void BOARD_SAI_RXConfig(sai_transceiver_t *config, sai_sync_mode_t sync);
/*******************************************************************************
 * Variables
 ******************************************************************************/
wm8960_config_t wm8960Config = {
    .route     = kWM8960_RoutePlaybackandRecord,
    .bus              = kWM8960_BusI2S,
    .format           = {.mclk_HZ    = 24576000U,
               .sampleRate = kWM8960_AudioSampleRate24KHz,
               .bitWidth   = kWM8960_AudioBitWidth16bit},
	.master_slave     = false,
    .leftInputSource  = kWM8960_InputDifferentialMicInput3,
    .rightInputSource = kWM8960_InputDifferentialMicInput2,
    .playSource       = kWM8960_PlaySourceDAC,
    .slaveAddress     = WM8960_I2C_ADDR,
    .i2cConfig = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},

};
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8960, .codecDevConfig = &wm8960Config};
/*
 * AUDIO PLL setting: Frequency = Fref * (DIV_SELECT + NUM / DENOM) / (2^POST)
 *                              = 24 * (32 + 77/100)  / 2
 *                              = 393.24MHZ
 */
const clock_audio_pll_config_t audioPllConfig = {
    .loopDivider = 32,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
    .postDivider = 1,   /* Divider after the PLL, should only be 0, 1, 2, 3, 4, 5 */
    .numerator   = 77,  /* 30 bit numerator of fractional loop divider. */
    .denominator = 100, /* 30 bit denominator of fractional loop divider */
};
static uint64_t g_index           = 0;
static volatile bool isFinished = false;
//extern bool isFinished;
extern codec_config_t boardCodecConfig;
codec_handle_t codecHandle;
/*******************************************************************************
 * Code
 ******************************************************************************/

int result = 0;
int result_new = 0;
uint64_t music_len = 0;

void BOARD_EnableSaiMclkOutput(bool enable)
{
    if (enable)
    {
        IOMUXC_GPR->GPR0 |= IOMUXC_GPR_GPR0_SAI1_MCLK_DIR_MASK;
    }
    else
    {
        IOMUXC_GPR->GPR0 &= (~IOMUXC_GPR_GPR0_SAI1_MCLK_DIR_MASK);
    }
}


void SAI1_IRQHandler111(void)
{
    uint8_t i     = 0;
    uint8_t j     = 0;
    uint32_t data = 0;
    uint32_t temp = 0;
    uint8_t *p;


    if(result == 3) {
  	    p = (uint8_t *)c30_speed;
  	}
    else if(result == 16) {
  	    p = (uint8_t *)attention_general;
  	}
    else if(result == 24) {
  	    p = (uint8_t *)lifted_general;
  	}
    else if(result == 23) {
  	    p = (uint8_t *)give_way;
  	}
    else if(result == 29) {
  	    p = (uint8_t *)no_way_general;
  	}
    else if(result == 30) {
  	    p = (uint8_t *)no_way_one_way;
  	}
    else if(result == 33) {
  	    p = (uint8_t *)right_of_way_general;
  	}
    else if(result == 34) {
  	    p = (uint8_t *)stop;
  	}
    else if(result == 39) {
  	    p = (uint8_t *)turn_right_down;
  	}
    else if(result == 40) {
  	    p = (uint8_t *)turn_straight;
  	}
    else if(result == 42) {
  	    p = (uint8_t *)turn_straight_right;
  	}

    else {
  	    //p = (uint8_t *)zero;
  	}

    /* Clear the FIFO error flag */
    if (SAI_TxGetStatusFlag(DEMO_SAI) & kSAI_FIFOErrorFlag)
    {
        SAI_TxClearStatusFlags(DEMO_SAI, kSAI_FIFOErrorFlag);
    }

    if (SAI_TxGetStatusFlag(DEMO_SAI) & kSAI_FIFOWarningFlag)
    {
        for (i = 0; i < FSL_FEATURE_SAI_FIFO_COUNT; i++)
        {
            data = 0;
            for (j = 0; j < DEMO_AUDIO_BIT_WIDTH / 8U; j++)
            {
                temp = (uint32_t)(p[g_index]);
                data |= (temp << (8U * j));
                g_index++;
            }
            SAI_WriteData(DEMO_SAI, DEMO_SAI_CHANNEL, data);
        }
        DisableIRQ(LCDIFv2_IRQn);
    }

    if (g_index >= music_len)
    {EnableIRQ(LCDIFv2_IRQn);
        isFinished = true;
        g_index = 0;
       // SAI_TxDisableInterrupts(DEMO_SAI, kSAI_FIFOWarningInterruptEnable | kSAI_FIFOErrorInterruptEnable);
        SAI_TxEnable(DEMO_SAI, false);
    }
    SDK_ISR_EXIT_BARRIER;
}

void DelayMS(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++)
    {
        SDK_DelayAtLeastUs(1000, SystemCoreClock);
    }
}

int main()
{

	sai_transceiver_t saiConfig;

    BOARD_Init();
    TIMER_Init();

    DEMO_PrintInfo();

    CLOCK_InitAudioPll(&audioPllConfig);

    /*Clock setting for LPI2C*/
    CLOCK_SetRootClockMux(kCLOCK_Root_Lpi2c5, 1);

    /*Clock setting for SAI1*/
    CLOCK_SetRootClockMux(kCLOCK_Root_Sai1, 4);
    CLOCK_SetRootClockDiv(kCLOCK_Root_Sai1, 16);

    /*Enable MCLK clock*/
    BOARD_EnableSaiMclkOutput(true);

    /* SAI init */
    SAI_Init(DEMO_SAI);
    /* I2S mode configurations */
    SAI_GetClassicI2SConfig(&saiConfig, DEMO_AUDIO_BIT_WIDTH, kSAI_Stereo, 1U << DEMO_SAI_CHANNEL);
    saiConfig.masterSlave = DEMO_SAI_MASTER_SLAVE;
    saiConfig.syncMode    = DEMO_SAI_TX_SYNC_MODE;
    SAI_TxSetConfig(DEMO_SAI, &saiConfig);
    /* set bit clock divider */
    SAI_TxSetBitClockRate(DEMO_SAI, DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH,
                          DEMO_AUDIO_DATA_CHANNEL);

    /* sai rx configurations */
    BOARD_SAI_RXCONFIG(&saiConfig, DEMO_SAI_RX_SYNC_MODE);
    /* master clock configurations */
    BOARD_MASTER_CLOCK_CONFIG();

    /* Use default setting to init codec */
    if (CODEC_Init(&codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        assert(false);
    }

    /* delay for codec output stable */
    DelayMS(DEMO_CODEC_INIT_DELAY_MS);

    /*  Enable interrupt */
    EnableIRQ(DEMO_SAI_IRQ);
    NVIC_SetPriority(DEMO_SAI_IRQ, 1);
    SAI_TxEnableInterrupts(DEMO_SAI, kSAI_FIFOWarningInterruptEnable | kSAI_FIFOErrorInterruptEnable);
    printf("1111111111111111111111\n");

    GUI_Init();
    GUI_SetColor(GUI_RED);
    GUI_SetFont(GUI_LARGE_FONT);
    GUI_SetBkColor(GUI_TRANSPARENT);
    GUI_SetTextMode(GUI_TEXTMODE_TRANS);

    GUI_DispStringAt("Female ", 500, 170);

    if (MODEL_Init() != kStatus_Success)
    {
        std::cerr << "Failed initializing model" << EOL;
        for (;;) {}
    }

    tensor_dims_t inputDims;
    tensor_type_t inputType;
    uint8_t* inputData = MODEL_GetInputTensorData(&inputDims, &inputType);

    tensor_dims_t outputDims;
    tensor_type_t outputType;
    uint8_t* outputData = MODEL_GetOutputTensorData(&outputDims, &outputType);

    while (1)
    {
        /* Expected tensor dimensions: [batches, height, width, channels] */
        if (IMAGE_GetImage(inputData, inputDims.data[2], inputDims.data[1], inputDims.data[3]) != kStatus_Success)
        {
            std::cerr << "Failed retrieving input image" << EOL;
            for (;;) {}
        }

        MODEL_ConvertInput(inputData, &inputDims, inputType);

        auto startTime = TIMER_GetTimeInUS();
        MODEL_RunInference();
        auto endTime = TIMER_GetTimeInUS();

        MODEL_ProcessOutput(outputData, &outputDims, outputType, endTime - startTime, &result);

       //if (result_new != result)
        {
        	result_new = result;

			switch (result) {
				case 3:
					music_len = 51918;        // 30_speed
					SAI_TxEnable(SAI1, true);
			        /* Wait until finished */
			        while (isFinished != true)
			        {
			        }
					break;
				case 16:
					music_len = 65535;        // attention_general
					SAI_TxEnable(SAI1, true);
			        /* Wait until finished */
			        while (isFinished != true)
			        {
			        }
					break;
				case 24:
					music_len = 64014;        // lifted_general
					SAI_TxEnable(SAI1, true);
			        /* Wait until finished */
			        while (isFinished != true)
			        {
			        }
					break;
				case 23:
					music_len = 38094;        // give_way
					SAI_TxEnable(SAI1, true);
			        /* Wait until finished */
			        while (isFinished != true)
			        {
			        }
					break;
				case 29:
					music_len = 65742;       // no_way_general
					SAI_TxEnable(SAI1, true);
			        /* Wait until finished */
			        while (isFinished != true)
			        {
			        }
			        DelayMS(300);
					break;
				case 30:
					music_len = 67470;       // no_way_one_way
					SAI_TxEnable(SAI1, true);
			        /* Wait until finished */
			        while (isFinished != true)
			        {
			        }
					break;
				case 33:
					music_len = 76110;       // right_of_way_general
					SAI_TxEnable(SAI1, true);
			        /* Wait until finished */
			        while (isFinished != true)
			        {
			        }
					break;
				case 34:
					music_len = 46734;       // stop
					SAI_TxEnable(SAI1, true);
			        /* Wait until finished */
			        while (isFinished != true)
			        {
			        }
					break;
				/*case 39:
					music_len = 65742;       // turn_right_down
					SAI_TxEnable(SAI1, true);

			        while (isFinished != true)
			        {
			        }
					break;*/
				case 40:
					music_len = 51918;       // turn_straight
					SAI_TxEnable(SAI1, true);
			        /* Wait until finished */
			        while (isFinished != true)
			        {
			        }
					break;
				case 42:
					music_len = 65742;       // turn_straight_right
					SAI_TxEnable(SAI1, true);
			        /* Wait until finished */
			        while (isFinished != true)
			        {
			        }
					break;
			}
			isFinished = false;
			result = 0;
        }
        /*else
        {
        	result = 0;
        }*/
        //DelayMS(DEMO_CODEC_INIT_DELAY_MS);
    }
}
