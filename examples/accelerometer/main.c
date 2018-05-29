/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#include <stdio.h>
#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"
#include "nrf_drv_gpiote.h"
#include "SEGGER_RTT.h"
#include "lis3dh.h"

#define TWI_SCL_PIN 8
#define TWI_SDA_PIN 16
#define SHOCK_DETECT_PIN 18

static const nrf_drv_twi_t twi_lis3dh = NRF_DRV_TWI_INSTANCE(0);


/**
 * @brief Read LIS3DH register
 */
uint8_t LIS3DH_read_register(uint8_t reg, bool noStop)
{
	ret_code_t err_code;
	
	err_code = nrf_drv_twi_tx(&twi_lis3dh, LIS3DH_ADDR, &reg, sizeof(reg), noStop);
	APP_ERROR_CHECK(err_code);
	
	nrf_delay_ms(10);
	
	uint8_t data;
	err_code = nrf_drv_twi_rx(&twi_lis3dh, LIS3DH_ADDR, &data, 1, noStop);
	APP_ERROR_CHECK(err_code);
	
	return data;
}

 
/**
 * @brief Write LIS3DH register
 */
void LIS3DH_write_register(uint8_t reg, uint8_t value, bool noStop) 
{
	ret_code_t err_code;
	uint8_t data[2] = {reg, value};

	err_code = nrf_drv_twi_tx(&twi_lis3dh, LIS3DH_ADDR, data, sizeof(data), noStop);
	APP_ERROR_CHECK(err_code);
	
	nrf_delay_ms(10);
}


/**
 * @brief Check TWI connection
 */
void LIS3DH_check_connection(void)
{	
	uint8_t result = LIS3DH_read_register(LIS3DH_WHO_AM_I, false);
	if (result == LIS3DH_WHO_AM_I_RESULT) {
		SEGGER_RTT_printf(0, "LIS3DH connection OK\n");
	}
}


/**
 * @brief Set LIS3DH to shock detection mode
 */
void LIS3DH_set_shock_detection_mode()
{
	LIS3DH_write_register(LIS3DH_CTRL_REG1, 0x27, false);
	LIS3DH_write_register(LIS3DH_CTRL_REG2, 0x00, false);
	LIS3DH_write_register(LIS3DH_CTRL_REG3, 0x40, false);
	LIS3DH_write_register(LIS3DH_CTRL_REG4, 0x00, false);
	LIS3DH_write_register(LIS3DH_CTRL_REG5, 0x08, false);
	LIS3DH_write_register(LIS3DH_INT1_THS, 0x10, false);
	LIS3DH_write_register(LIS3DH_INT1_DURATION, 0x00, false);
	LIS3DH_write_register(LIS3DH_INT1_CFG, 0x0A, false);
}


/**
 * @brief TWI initialization.
 */
void twi_init (void)
{
    ret_code_t err_code;
    
    const nrf_drv_twi_config_t twi_lis3dh_config = {
       .scl = TWI_SCL_PIN,
       .sda = TWI_SDA_PIN,
       .frequency = NRF_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH
    };
    
    err_code = nrf_drv_twi_init(&twi_lis3dh, &twi_lis3dh_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);
    
    nrf_drv_twi_enable(&twi_lis3dh);
}


/**
 * @brief Interrupt handler for wakeup pins
 */
void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	if (pin == SHOCK_DETECT_PIN) {
		
		SEGGER_RTT_printf(0, "Shock detected\n");
		SEGGER_RTT_printf(0, "do something...\n");
		//
		// Do something 
		//
		
		LIS3DH_read_register(LIS3DH_INT1_SOURCE, false); // Clear INT1
	}
}


/**
 * @brief Function for configuring: 
 */
static void gpio_init(void)
{
	ret_code_t err_code;

	err_code = nrf_drv_gpiote_init();
	nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(false);
	in_config.pull = NRF_GPIO_PIN_PULLDOWN;
	err_code = nrf_drv_gpiote_in_init(SHOCK_DETECT_PIN, &in_config, in_pin_handler);
	APP_ERROR_CHECK(err_code);
	nrf_drv_gpiote_in_event_enable(SHOCK_DETECT_PIN, true);
}


/**
 * @brief Function for main application entry.
 */
int main(void)
{
	NRF_CLOCK->XTALFREQ = CLOCK_XTALFREQ_XTALFREQ_32MHz;
	
	//ret_code_t err_code;
	
	SEGGER_RTT_Init();
	SEGGER_RTT_printf(0, "MaBeeeIoTDevelopmentKit TWI Sample\n");
	
	/* TWI setup */
	twi_init();
	LIS3DH_check_connection();
	LIS3DH_set_shock_detection_mode();

	/* GPIO setup */
	gpio_init();

    while(true)
    {
		__WFE();
		__SEV();
		__WFE();
    }
}

/** @} */
