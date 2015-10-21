#include "buzzer.h"

#include "../uart.h"

#ifndef STM32
Timer buzzer_timer;
#else
#define TIM2CLK 2000000
TIM_HandleTypeDef buzzer_timer;
TIM_OC_InitTypeDef sConfig; // Timer Output Compare Configuration Structure declaration
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
		HAL_TIM_PWM_Stop(&buzzer_timer, TIM_CHANNEL_2);
	else
		HAL_TIM_PWM_Start(&buzzer_timer, TIM_CHANNEL_2);

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
		 HAL_TIM_PWM_Stop(&buzzer_timer, TIM_CHANNEL_2);
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
	uint32_t buzzer_period = (TIM2CLK / freq_hz) - 1;

	buzzer_timer.Instance->ARR = buzzer_period;
	buzzer_timer.Instance->CCR2 = buzzer_period / 5; //20% duty cycle = battery saving
	if (buzzer_timer.Instance->CNT > buzzer_period)
		buzzer_timer.Instance->CNT = 1;

	HAL_TIM_PWM_Start(&buzzer_timer, TIM_CHANNEL_2);
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
	buzzer_timer.Instance = TIM2;
	buzzer_timer.Init.ClockDivision = 0;
	buzzer_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
	buzzer_timer.Init.RepetitionCounter = 0;
	buzzer_timer.Init.Prescaler = (uint32_t)(SystemCoreClock / TIM2CLK) - 1;;
	HAL_TIM_PWM_Init(&buzzer_timer);

	sConfig.OCMode       = TIM_OCMODE_PWM1;
	sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
	sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
	sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
	sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	HAL_TIM_PWM_ConfigChannel(&buzzer_timer, &sConfig, TIM_CHANNEL_2);
#endif

	buzzer_set_vol(0);
}
