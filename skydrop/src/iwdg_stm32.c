#include "stm32f1xx_hal.h"

IWDG_HandleTypeDef IwdgHandle;
TIM_HandleTypeDef  TimInputCaptureHandle;

static __IO uint32_t uwLsiFreq = 0;
__IO uint32_t uwCaptureNumber = 0;
__IO uint32_t uwPeriodValue = 0;
__IO uint32_t uwMeasurementDone = 0;

uint16_t tmpCC4[2] = {0, 0};

static uint32_t GetLSIFrequency(void);

static uint32_t wdg_running = 0;

void wdt_init(uint32_t timeout)
{
  /*##-2- Get the LSI frequency: TIM5 is used to measure the LSI frequency ###*/
  uwLsiFreq = GetLSIFrequency();

  /*##-3- Configure the IWDG peripheral ######################################*/
  /* Set counter reload value to obtain 250ms IWDG TimeOut.
     IWDG counter clock Frequency = LsiFreq / 32
     Counter Reload Value = 2000ms / IWDG counter clock period
                          = 2 / (32/LsiFreq)
                          = LsiFreq / 16 */
  IwdgHandle.Instance = IWDG;

  IwdgHandle.Init.Prescaler = IWDG_PRESCALER_32;
  IwdgHandle.Init.Reload    = uwLsiFreq / 16; //16 - 2sec

  if (HAL_IWDG_Init(&IwdgHandle) != HAL_OK)
  {
    /* Initialization Error */
    while (1);
  }

  /*##-4- Start the IWDG #####################################################*/
  if (HAL_IWDG_Start(&IwdgHandle) != HAL_OK)
  {
    while (1);
  }
  wdg_running = 1;
}

void wdt_deinit(void)
{

}

void wdt_reset(void)
{
  if (!wdg_running)
	  return;

  /* Refresh IWDG: reload counter */
  if (HAL_IWDG_Refresh(&IwdgHandle) != HAL_OK)
  {
    /* Refresh Error */
    while(1);
  }
}

/**
  * @brief  Configures TIM5 to measure the LSI oscillator frequency.
  * @param  None
  * @retval LSI Frequency
  */
static uint32_t GetLSIFrequency(void)
{
  uint32_t pclk1 = 0, latency = 0;
  TIM_IC_InitTypeDef timinputconfig = {0};
  RCC_OscInitTypeDef oscinit = {0};
  RCC_ClkInitTypeDef  clkinit =  {0};

  /* Enable LSI Oscillator */
  oscinit.OscillatorType = RCC_OSCILLATORTYPE_LSI;
  oscinit.LSIState = RCC_LSI_ON;
  oscinit.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&oscinit)!= HAL_OK)
  {
    while (1);
  }

  /* Configure the TIM peripheral */
  /* Set TIMx instance */
  TimInputCaptureHandle.Instance = TIM5;

  /* TIMx configuration: Input Capture mode ---------------------
  The LSI clock is connected to TIM5 CH4.
  The Rising edge is used as active edge.
  The TIM5 CCR4 is used to compute the frequency value.
  ------------------------------------------------------------ */
  TimInputCaptureHandle.Init.Prescaler         = 0;
  TimInputCaptureHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TimInputCaptureHandle.Init.Period            = 0xFFFF;
  TimInputCaptureHandle.Init.ClockDivision     = 0;
  TimInputCaptureHandle.Init.RepetitionCounter = 0;
  if (HAL_TIM_IC_Init(&TimInputCaptureHandle) != HAL_OK)
  {
    /* Initialization Error */
    while (1);
  }
  /* Connect internally the  TIM5 CH4 Input Capture to the LSI clock output */
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_AFIO_REMAP_TIM5CH4_ENABLE();

  /* Configure the Input Capture of channel 4 */
  timinputconfig.ICPolarity  = TIM_ICPOLARITY_RISING;
  timinputconfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
  timinputconfig.ICPrescaler = TIM_ICPSC_DIV8;
  timinputconfig.ICFilter    = 0;

  if (HAL_TIM_IC_ConfigChannel(&TimInputCaptureHandle, &timinputconfig, TIM_CHANNEL_4) != HAL_OK)
  {
    /* Initialization Error */
    while (1);
  }

  /* Reset the flags */
  TimInputCaptureHandle.Instance->SR = 0;

  /* Start the TIM Input Capture measurement in interrupt mode */
  if (HAL_TIM_IC_Start_IT(&TimInputCaptureHandle, TIM_CHANNEL_4) != HAL_OK)
  {
    /* Starting Error */
    while (1);
  }

  /* Wait until the TIM5 get 2 LSI edges (refer to TIM5_IRQHandler() in
  stm32f1xx_it.c file) */
  while (uwMeasurementDone == 0)
  {
  }
  uwCaptureNumber = 0;

  /* Deinitialize the TIM5 peripheral registers to their default reset values */
  HAL_TIM_IC_DeInit(&TimInputCaptureHandle);

  /* Compute the LSI frequency, depending on TIM5 input clock frequency (PCLK1)*/
  /* Get PCLK1 frequency */
  pclk1 = HAL_RCC_GetPCLK1Freq();
  HAL_RCC_GetClockConfig(&clkinit, &latency);

  /* Get PCLK1 prescaler */
  if ((clkinit.APB1CLKDivider) == RCC_HCLK_DIV1)
  {
    /* PCLK1 prescaler equal to 1 => TIMCLK = PCLK1 */
    return ((pclk1 / uwPeriodValue) * 8);
  }
  else
  {
    /* PCLK1 prescaler different from 1 => TIMCLK = 2 * PCLK1 */
    return (((2 * pclk1) / uwPeriodValue) * 8) ;
  }
}

/**
  * @brief  Input Capture callback in non blocking mode
  * @param  htim : TIM IC handle
  * @retval None
  */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  /* Get the Input Capture value */
  tmpCC4[uwCaptureNumber++] = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);

  if (uwCaptureNumber >= 2)
  {
    /* Compute the period length */
    uwPeriodValue = (uint16_t)(0xFFFF - tmpCC4[0] + tmpCC4[1] + 1);
    uwMeasurementDone = 1;
    uwCaptureNumber = 0;
  }
}
