#include "battery.h"
#include "../tasks/tasks.h"

#ifdef STM32
ADC_HandleTypeDef    AdcHandle;
__IO uint16_t   aADCxConvertedValues[1];

static void ADC_Config(void)
{
  ADC_ChannelConfTypeDef   sConfig;

  AdcHandle.Instance = ADC1;

  AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  AdcHandle.Init.ScanConvMode          = ADC_SCAN_DISABLE;              /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */

  AdcHandle.Init.ContinuousConvMode    = ENABLE;                        /* Continuous mode to have maximum conversion speed (no delay between conversions) */
  AdcHandle.Init.NbrOfConversion       = 1;                             /* Parameter discarded because sequencer is disabled */
  AdcHandle.Init.DiscontinuousConvMode = DISABLE;                       /* Parameter discarded because sequencer is disabled */
  AdcHandle.Init.NbrOfDiscConversion   = 1;                             /* Parameter discarded because sequencer is disabled */
  AdcHandle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;            /* Software start to trig the 1st conversion manually, without external event */

  HAL_ADC_Init(&AdcHandle);

  sConfig.Channel      = ADC_CHANNEL_11;
  sConfig.Rank         = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_41CYCLES_5;

  HAL_ADC_ConfigChannel(&AdcHandle, &sConfig);

 // sConfig.Channel      = ADC_CHANNEL_TEMPSENSOR;

  //HAL_ADC_ConfigChannel(&AdcHandle, &sConfig);
}
#endif

void battery_init()
{
#ifndef STM32
	BATTERY_ADC_PWR_ON;
	BATTERY_ADC_ENABLE;

	AdcAInit(adc_int1V);
	AdcASetMode(adc_unsigned);

	AdcPipeSetSource(pipea0, BAT_SNS_ADC);

	GpioSetDirection(BAT_EN, OUTPUT);
#else
	GPIO_InitTypeDef          GPIO_InitStruct;

	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, (GPIO_PinState)1);

	ADC_Config();
	HAL_ADCEx_Calibration_Start(&AdcHandle);
#endif
	bat_en_low(BAT_EN_ADC);
}


#define BATT_THOLD	VDC2ADC(2.2)

#define BATTERY_STATE_IDLE		0
#define BATTERY_STATE_PREPARE	1
#define BATTERY_STATE_START		2
#define BATTERY_STATE_RESULT	3

#define BATTERY_MEAS_AVG 		16
#define BATTERY_MEAS_PERIOD 	10000
//#define BATTERY_MEAS_PERIOD 	1000
#define BATTERY_STABILISE 		100


uint32_t battery_next_meas = 0;
uint8_t  battery_meas_state = BATTERY_STATE_PREPARE;

uint16_t battery_meas_acc = 0;
uint8_t  battery_meas_cnt = 0;

int16_t battery_adc_raw = 0;
int8_t battery_per = 0;

//#define BATT_COEF_A	(0.291950711)
//#define BATT_COEF_B  (-672.1273455619)


#define BATT_COEF_A	(0.2147782473)
#define BATT_COEF_B  (-681.4132446547)

uint8_t battery_get_per()
{
	return battery_per;
}

bool battery_step()
{
	if (battery_next_meas > task_get_ms_tick())
		return false;

	switch (battery_meas_state)
	{
	case(BATTERY_STATE_IDLE):
		battery_adc_raw = battery_meas_acc / BATTERY_MEAS_AVG;
		battery_per = round(((float)battery_adc_raw * BATT_COEF_A) + BATT_COEF_B);
		if (battery_per > 100)
			battery_per = 100;
		if (battery_per < 0)
			battery_per = 0;

//		DEBUG("adc %u (%3d%%)\n", battery_adc_raw, battery_per);

		battery_meas_state = BATTERY_STATE_PREPARE;
		battery_next_meas = task_get_ms_tick() + BATTERY_MEAS_PERIOD;
		battery_meas_acc = 0;
		battery_meas_cnt = 0;

		return true;
	break;

	case(BATTERY_STATE_PREPARE):
		battery_meas_state = BATTERY_STATE_START;
#ifndef STM32
		BATTERY_ADC_PWR_ON;
		BATTERY_ADC_ENABLE;
#endif
		bat_en_high(BAT_EN_ADC);
		battery_next_meas = task_get_ms_tick() + BATTERY_STABILISE;
	break;

	case(BATTERY_STATE_START):
#ifndef STM32
		AdcPipeStart(pipea0);
#else
		HAL_ADC_Start_DMA(&AdcHandle, (uint32_t *)aADCxConvertedValues, 1);
#endif
		battery_meas_state = BATTERY_STATE_RESULT;
	break;

	case(BATTERY_STATE_RESULT):
#ifndef STM32
		if (!AdcPipeReady(pipea0))
		{
			DEBUG("adc not ready\n");
			return false;
		}
		uint16_t tmp = AdcPipeValue(pipea0);
#else
		uint16_t tmp = aADCxConvertedValues[0];
#endif
		battery_meas_acc += tmp;

		battery_meas_cnt++;

		if (battery_meas_cnt >= BATTERY_MEAS_AVG)
		{
			battery_meas_state = BATTERY_STATE_IDLE;

			bat_en_low(BAT_EN_ADC);
#ifndef STM32
			BATTERY_ADC_DISABLE;
			BATTERY_ADC_PWR_OFF;
#endif
		}
		else
			battery_meas_state = BATTERY_STATE_START;
	break;
	}

	return false;
}
