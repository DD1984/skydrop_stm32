#include "buzzer.h"

#include "../uart.h"

#ifndef STM32
Timer buzzer_timer;
#else
#include "stm32f1xx_hal.h"
/* Timer handler declaration */
TIM_HandleTypeDef    TimHandle;
/* Timer Output Compare Configuration Structure declaration */
TIM_OC_InitTypeDef sConfig;
/* Counter Prescaler value */
uint32_t uhPrescalerValue = 0;
#endif

//V = (val / 0xFFF) * 3.3V

#define DAC_MIN		745		//0.6V
#define DAC_MAX		1985	//1.6V
#define DAC_DELTA	(DAC_MAX - DAC_MIN)

volatile uint8_t buzzer_volume = 0;

#ifndef STM32
ISR(BUZZER_TIMER_OVF)
{

}
#endif

void buzzer_set_vol(uint8_t vol)
{
	if (vol == buzzer_volume)
		return;

	uint16_t val = DAC_MIN + (DAC_DELTA / 100) * vol;

	if (val > DAC_MAX)
		val = DAC_MAX;

	if (val < DAC_MIN)
		val = DAC_MIN;

#ifndef STM32
	DacSetCh1(val);

	if (vol == 0)
		buzzer_timer.Stop();
	else
		buzzer_timer.Start();
#else
	if (vol == 0)
		HAL_TIM_PWM_Stop(&TimHandle, TIM_CHANNEL_2);
	else
		HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_2);

#endif

	buzzer_volume = vol;
}

void buzzer_set_freq(uint16_t freq_hz)
{
	if (freq_hz == 0 || buzzer_volume == 0)
	{
#ifndef STM32
		buzzer_timer.Stop();
#else
		 HAL_TIM_PWM_Stop(&TimHandle, TIM_CHANNEL_2);
#endif
		return;
	}

#ifndef STM32
	uint16_t buzzer_period = 31250 / freq_hz;

	buzzer_timer.SetCompare(timer_A, buzzer_period / 5); //20% duty cycle = battery saving
	buzzer_timer.SetTop(buzzer_period);
	if (buzzer_timer.GetValue() > buzzer_period)
		buzzer_timer.SetValue(1);

	buzzer_timer.Start();
#else
#define TIM2CLK 2000000
	uint32_t buzzer_period = (TIM2CLK / freq_hz) - 1;

	TimHandle.Init.Prescaler = (uint32_t)(SystemCoreClock / TIM2CLK) - 1;;
	TimHandle.Init.Period = buzzer_period;
	HAL_TIM_PWM_Init(&TimHandle);

	/* Set the pulse value for channel 2 */
	sConfig.Pulse = buzzer_period / 5; //20% duty cycle = battery saving
	HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, TIM_CHANNEL_2);

	HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_2);
#endif
}

void buzzer_init()
{
#ifndef STM32
	BUZZER_TIMER_PWR_ON;
	BUZZER_VOL_DAC_PWR_ON;

	DacInit(BUZZER_VOL_DAC);
	DacSetReference(dac_avcc);

	//buzzer
	buzzer_timer.Init(BUZZER_TIMER, timer_div1024);
	buzzer_timer.SetMode(timer_pwm);
	buzzer_timer.EnableOutputs(timer_A);
	buzzer_timer.EnableInterrupts(timer_overflow);
#else
	/*##-1- Configure the TIM peripheral #######################################*/
	/* -----------------------------------------------------------------------
	  TIM2 Configuration: generate 4 PWM signals with 4 different duty cycles.

	    In this example TIM2 input clock (TIM2CLK) is set to APB1 clock (PCLK1) x2,
	    since APB1 prescaler is set to 4 (0x100).
	       TIM2CLK = PCLK1*2
	       PCLK1   = HCLK/2
	    => TIM2CLK = PCLK1*2 = (HCLK/2)*2 = HCLK = SystemCoreClock

	    To get TIM2 counter clock at 2.1 MHz, the prescaler is computed as follows:
	       Prescaler = (TIM2CLK / TIM2 counter clock) - 1
	       Prescaler = ((SystemCoreClock) /2.1 MHz) - 1

	    To get TIM2 output clock at 3 KHz, the period (ARR)) is computed as follows:
	       ARR = (TIM2 counter clock / TIM2 output clock) - 1
	           = 699

	    TIM2 Channel1 duty cycle = (TIM2_CCR1/ TIM2_ARR + 1)* 100 = 50%
	    TIM2 Channel2 duty cycle = (TIM2_CCR2/ TIM2_ARR + 1)* 100 = 37.5%
	    TIM2 Channel3 duty cycle = (TIM2_CCR3/ TIM2_ARR + 1)* 100 = 25%
	    TIM2 Channel4 duty cycle = (TIM2_CCR4/ TIM2_ARR + 1)* 100 = 12.5%

	    Note:
	     SystemCoreClock variable holds HCLK frequency and is defined in system_stm32f1xx.c file.
	     Each time the core clock (HCLK) changes, user had to update SystemCoreClock
	     variable value. Otherwise, any configuration based on this variable will be incorrect.
	     This variable is updated in three ways:
	      1) by calling CMSIS function SystemCoreClockUpdate()
	      2) by calling HAL API function HAL_RCC_GetSysClockFreq()
	      3) each time HAL_RCC_ClockConfig() is called to configure the system clock frequency
	  ----------------------------------------------------------------------- */

	/* Initialize TIMx peripheral as follows:
	       + Prescaler = (SystemCoreClock / 2000000) - 1
	       + Period = (700 - 1)
	       + ClockDivision = 0
	       + Counter direction = Up
	 */
	TimHandle.Instance = TIM2;

	TimHandle.Init.ClockDivision     = 0;
	TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
	TimHandle.Init.RepetitionCounter = 0;

	/*##-2- Configure the PWM channels #########################################*/
	/* Common configuration for all channels */
	sConfig.OCMode       = TIM_OCMODE_PWM1;
	sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
	sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
	sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
	sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;
#endif

	buzzer_set_vol(0);
}
