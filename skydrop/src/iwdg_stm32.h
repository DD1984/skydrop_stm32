#ifndef __IWDG_STM32_H__
#define __IWDG_STM32_H__

#ifdef __cplusplus
 extern "C" {
#endif

#define wdt_2s 1

void wdt_init(uint32_t timeout);
void wdt_reset(void);

#ifdef __cplusplus
}
#endif

#endif
