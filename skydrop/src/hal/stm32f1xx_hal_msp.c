/**
  ******************************************************************************
  * @file    UART/UART_Printf/Src/stm32f1xx_hal_msp.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    05-June-2015
  * @brief   HAL MSP module.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"


static DMA_HandleTypeDef hdma_tx;
static DMA_HandleTypeDef hdma_rx;

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;

	if (huart->Instance == USART3) {
		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_USART3_CLK_ENABLE();

		//tx
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pin = GPIO_PIN_10;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		//rx
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pin = GPIO_PIN_11;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}

	if (huart->Instance == USART2) {

		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_USART2_CLK_ENABLE();
		__HAL_RCC_DMA1_CLK_ENABLE();

		//tx
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pin = GPIO_PIN_2;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		//rx
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pin = GPIO_PIN_3;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#if 0
		/*##-3- Configure the DMA ##################################################*/
		/* Configure the DMA handler for Transmission process */
		hdma_tx.Instance                 = DMA1_Channel7;
		hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
		hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
		hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
		hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
		hdma_tx.Init.Mode                = DMA_NORMAL;
		hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;

		HAL_DMA_Init(&hdma_tx);

		/* Associate the initialized DMA handle to the UART handle */
		__HAL_LINKDMA(huart, hdmatx, hdma_tx);
#endif

		/* Configure the DMA handler for reception process */
		hdma_rx.Instance                 = DMA1_Channel6;
		hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
		hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
		hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
		hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
		hdma_rx.Init.Mode                = DMA_CIRCULAR;
		hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;

		HAL_DMA_Init(&hdma_rx);

		/* Associate the initialized DMA handle to the the UART handle */
		__HAL_LINKDMA(huart, hdmarx, hdma_rx);

#if 0
		/*##-4- Configure the NVIC for DMA #########################################*/
		/* NVIC configuration for DMA transfer complete interrupt (USARTx_TX) */
		HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 1);
		HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);
		/* NVIC configuration for DMA transfer complete interrupt (USARTx_RX) */
		HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

		/* NVIC for USART, to catch the TX complete */
		HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);
		HAL_NVIC_EnableIRQ(USART1_IRQn);
#endif
	}
}

/**
  * @brief UART MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO and NVIC configuration to their default state
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3) {
		__HAL_RCC_USART3_FORCE_RESET();
		__HAL_RCC_USART3_RELEASE_RESET();

		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10 | GPIO_PIN_11);
	}

	if (huart->Instance == USART2) {
		__HAL_RCC_USART2_FORCE_RESET();
		__HAL_RCC_USART2_RELEASE_RESET();

		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);

		/*##-3- Disable the DMA Channels ###########################################*/
		/* De-Initialize the DMA Channel associated to transmission process */
		HAL_DMA_DeInit(&hdma_tx);
		/* De-Initialize the DMA Channel associated to reception process */
		HAL_DMA_DeInit(&hdma_rx);

#if 0
		/*##-4- Disable the NVIC for DMA ###########################################*/
		HAL_NVIC_DisableIRQ(DMA1_Channel4_IRQn);
		HAL_NVIC_DisableIRQ(DMA1_Channel5_IRQn);
#endif
	}
}

/**
  * @brief SPI MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  * @param hspi: SPI handle pointer
  * @retval None
  */


void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	if (hspi->Instance == SPI1) {
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_SPI1_CLK_ENABLE();

		GPIO_InitStruct.Pin       = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
		GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
		GPIO_InitStruct.Speed     = GPIO_SPEED_LOW;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}
	else if (hspi->Instance == SPI2) {
		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_SPI2_CLK_ENABLE();

		GPIO_InitStruct.Pin       = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
		GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
		GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}
}

/**
  * @brief SPI MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO configuration to its default state
  * @param hspi: SPI handle pointer
  * @retval None
  */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
	if(hspi->Instance == SPI1)
	{
	}
}


void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
	GPIO_InitTypeDef   GPIO_InitStruct;

	__HAL_RCC_TIM1_CLK_ENABLE();

	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;

	GPIO_InitStruct.Pin = GPIO_PIN_8;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_9;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
  * @brief TIM MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  * @param htim: TIM handle pointer
  * @retval None
  */
/* Definition for TIMx's NVIC */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM3) {
		__HAL_RCC_TIM3_CLK_ENABLE();
		HAL_NVIC_SetPriority(TIM3_IRQn, 3, 0);
		HAL_NVIC_EnableIRQ(TIM3_IRQn);
	}
	if (htim->Instance == TIM4) {
		__HAL_RCC_TIM4_CLK_ENABLE();
		HAL_NVIC_SetPriority(TIM4_IRQn, 3, 0);
		HAL_NVIC_EnableIRQ(TIM4_IRQn);
	}
}

/**
  * @brief I2C MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  *           - DMA configuration for transmission request by peripheral
  *           - NVIC configuration for DMA interrupt request enable
  * @param hi2c: I2C handle pointer
  * @retval None
  */
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
	/*##-1- Enable peripherals and GPIO Clocks #################################*/
	/* Enable GPIO TX/RX clock */
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef  GPIO_InitStruct;

	/*##-2- Configure peripheral GPIO ##########################################*/
	/* I2C TX GPIO pin configuration  */
	GPIO_InitStruct.Pin       = GPIO_PIN_6 //scl
							  | GPIO_PIN_7; //sda
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull      = GPIO_NOPULL;
	GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	__HAL_RCC_I2C1_CLK_ENABLE();
}

/**
  * @brief I2C MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO, DMA and NVIC configuration to their default state
  * @param hi2c: I2C handle pointer
  * @retval None
  */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
	/*##-1- Reset peripherals ##################################################*/
	__HAL_RCC_I2C1_FORCE_RESET();
	__HAL_RCC_I2C1_RELEASE_RESET();

	/*##-2- Disable peripherals and GPIO Clocks #################################*/
	/* Configure I2C Tx as alternate function  */
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7);
}

void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

	/*##-1- Enables the PWR Clock and Enables access to the backup domain ###################################*/
	/* To change the source clock of the RTC feature (LSE, LSI), You have to:
     - Enable the power clock using __HAL_RCC_PWR_CLK_ENABLE()
     - Enable write access using HAL_PWR_EnableBkUpAccess() function before to
       configure the RTC clock source (to be done once after reset).
     - Reset the Back up Domain using __HAL_RCC_BACKUPRESET_FORCE() and
       __HAL_RCC_BACKUPRESET_RELEASE().
     - Configure the needed RTC clock source */
	__HAL_RCC_PWR_CLK_ENABLE();
	HAL_PWR_EnableBkUpAccess();

	/* Enable BKP CLK enable for backup registers */
	__HAL_RCC_BKP_CLK_ENABLE();

	RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.LSIState = RCC_LSI_OFF;

	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;

	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

	/*##-2- Enable the RTC & BKP peripheral Clock ##############################*/
	/* Enable RTC Clock */
	__HAL_RCC_RTC_ENABLE();

}

/**
 * @brief RTC MSP De-Initialization
 *        This function freeze the hardware resources used in this example:
 *          - Disable the Peripheral's clock
 * @param hrtc: RTC handle pointer
 * @retval None
 */
void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc)
{
	/*##-1- Reset peripherals ##################################################*/
	__HAL_RCC_RTC_DISABLE();
}



void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
	static DMA_HandleTypeDef  DmaHandle;
	RCC_PeriphCLKInitTypeDef  PeriphClkInit;

	/* Enable clock of ADCx peripheral */
	__HAL_RCC_ADC1_CLK_ENABLE();

	/* Configure ADCx clock prescaler */
	/* Caution: On STM32F1, ADC clock frequency max is 14MHz (refer to device   */
	/*          datasheet).                                                     */
	/*          Therefore, ADC clock prescaler must be configured in function   */
	/*          of ADC clock source frequency to remain below this maximum      */
	/*          frequency.                                                      */
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
	PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

	/* Enable clock of DMA associated to the peripheral */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* Configure DMA parameters */
	DmaHandle.Instance = DMA1_Channel1;

	DmaHandle.Init.Direction           = DMA_PERIPH_TO_MEMORY;
	DmaHandle.Init.PeriphInc           = DMA_PINC_DISABLE;
	DmaHandle.Init.MemInc              = DMA_MINC_ENABLE;
	DmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;   /* Transfer from ADC by half-word to match with ADC configuration: ADC resolution 10 or 12 bits */
	DmaHandle.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;   /* Transfer to memory by half-word to match with buffer variable type: half-word */
	DmaHandle.Init.Mode                = DMA_CIRCULAR;              /* DMA in circular mode to match with ADC configuration: DMA continuous requests */
	DmaHandle.Init.Priority            = DMA_PRIORITY_HIGH;

	/* Deinitialize  & Initialize the DMA for new transfer */
	HAL_DMA_DeInit(&DmaHandle);
	HAL_DMA_Init(&DmaHandle);

	/* Associate the initialized DMA handle to the ADC handle */
	__HAL_LINKDMA(hadc, DMA_Handle, DmaHandle);
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
{
	__HAL_RCC_ADC1_FORCE_RESET();
	__HAL_RCC_ADC1_RELEASE_RESET();

	/* De-Initialize the DMA associated to the peripheral */
	if(hadc->DMA_Handle != NULL)
	{
		HAL_DMA_DeInit(hadc->DMA_Handle);
	}
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
