
#include "stm32g4xx.h"

#include <math.h>
#include "Fusion.h"
#include "INIT_STM32G431_GPIO.h"
#include <CANFD_STM32G431.h>
#include "DMA.h"
#include "Interfaces/SPI.h"
#include "Interfaces/UART1.h"
#include "Interfaces/I2C.h"
#include "Sensors/BMP280.h"
#include "Sensors/IIM_42652.h"
#include "LIS3MD.h"
#include "main.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"


#include <stdio.h>

#define CANID 0x284

uint8_t test = 1,
		trigger = 0,
		command = 0xEF,
		data = 0,
		sendACC = 0, sendMAG = 0,
		sendPs = 0, test_ps = 0,
		test_rf = 0;
volatile uint32_t imu_irq_count = 0;
enum {RF_STATE_WAIT, RF_STATE_MEAS, RF_STATE_GET, RF_STATE_SEND} stateRF = RF_STATE_WAIT;

#define BASE_POLL_TIME 20 // Basic interval of receiving аand sending data
#define PS_POLL_FACTOR 25 // Multiplier of base interval for pressure sensor
#define RF_POLL_FACTOR 10 // Multiplier of base interval for range finder


//uint32_t pause = 500;

I2C_HandleTypeDef hi2c2;
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c);

FusionAhrs ahrs;
FusionQuaternion quaternion;
FusionVector acc,gyro,mag;
float heading;
#define SAMPLE_RATE 1000.0f
#define DELTA_TIME (1.0f / SAMPLE_RATE)
#define ACCEL_SCALE 2048.0f  // ±16g
#define GYRO_SCALE 16.384f   // ±2000 dps
#define MAG_SCALE 0.15f      // Пример (настройте под ваш магнитометр)

uint16_t spi2_rx_buf[30]={0},spi2rxsize = 12,
		 spi2_tx_buf[30]={0},spi2txsize = 12;

uint8_t  spi2_rx_data[30]={0},*fdata, dmaComplete=0, readyINT1 =0;

//static void MX_I2C2_Init(void);
//static void MX_GPIO_Init(void);

//void I2C_SystemClock_Config(void);
int main(void) {

///////  Настройка системного тактирования

	SystemClock_Config();
	HAL_SYSTICK_Config(SystemCoreClock / 1000);
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
	__enable_irq();
	GPIO_INIT();
	//MX_GPIO_Init();
//	HAL_Init();
	//I2C_SystemClock_Config();
	//MX_I2C2_Init();
//MX_GPIO_Init();
	CAN_Config();
	Init_SPI_STM32();
	RS422_UART_init(&dalnomer);


///////  Sensors

	uint8_t imu_init_result = init_iim42652(&imu_iim42652);
	uint8_t bmp_status = init_bmp280(&bmp280_sensor1);
	init_lis3md	 (&mag_lis3md);
	init_bmp280  (&bmp280_sensor1);
	hi2c2.Instance = I2C2;
	hi2c2.Init.Timing = 0x30D0262B;      // 400kHz @ HSI16
	hi2c2.Init.OwnAddress1 = 0;
	hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c2.Init.OwnAddress2 = 0;
	hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c2) != HAL_OK) { Error_Handler(); }
	ssd1306_Init();
	for(volatile uint32_t i = 0; i < 1600000; i++) {
	        __NOP();
	    }
	//
	ssd1306_Init();
	for(volatile uint32_t i = 0; i < 1600000; i++) {
	    __NOP();
	}
	ssd1306_Init();
	ssd1306_Fill(Black);
	ssd1306_SetCursor(0, 2);
	ssd1306_WriteString("BMP280", Font_7x10, White);
	ssd1306_SetCursor(0, 16);
	ssd1306_WriteString("T: --.-C", Font_7x10, White);
	ssd1306_SetCursor(0, 30);
	ssd1306_WriteString("P: -----Pa", Font_7x10, White);
	ssd1306_UpdateScreen();

	float data32[2] = {0};


	while (1) {

	    // iim42652
	    if(*(imu_iim42652.status) & DMA_OK_IIM42xxx) {
	        *(imu_iim42652.status) &= ~DMA_OK_IIM42xxx;

	        sendACC = 1;
	        fdata = (uint8_t*) imu_iim42652.DMA_RX_fifo_buf;
	        spi2_rx_data[0]  = *fdata;
	        spi2_rx_data[1]  = *(fdata+3);
	        spi2_rx_data[2]  = *(fdata+2);
	        spi2_rx_data[3]  = *(fdata+5);
	        spi2_rx_data[4]  = *(fdata+4);
	        spi2_rx_data[5]  = *(fdata+7);
	        spi2_rx_data[6]  = *(fdata+6);
	        spi2_rx_data[7]  = *(fdata+9);
	        spi2_rx_data[8]  = *(fdata+8);
	        spi2_rx_data[9]  = *(fdata+11);
	        spi2_rx_data[10] = *(fdata+10);
	        spi2_rx_data[11] = *(fdata+13);
	        spi2_rx_data[12] = *(fdata+12);
	        spi2_rx_data[13] = *(fdata+15);
	        spi2_rx_data[14] = *(fdata+14);
	        spi2_rx_data[15] = *(fdata+17);
	        spi2_rx_data[16] = *(fdata+16);
	    }

	    // lis3md
	    if(*(mag_lis3md.status) & DMA_OK_LIS3MXX) {
	        *(mag_lis3md.status) &= ~DMA_OK_LIS3MXX;
	        sendMAG = 1;
	    }

	    // bmp280
	    if(*(bmp280_sensor1.status) & DMA_OK_BMP280) {
	        *(bmp280_sensor1.status) &= ~DMA_OK_BMP280;
	        sendPs = 1;
	    } else {
	        if (!systick_pause) {
	            test_ps++;
	            if(test_ps >= PS_POLL_FACTOR) {
	                test_ps = 0;
	                BMP280_Read_Data(&bmp280_sensor1);
	                sendPs = 1;
	            }
	        }
	    }

	    // Range Finder
	    switch (stateRF) {
	        case RF_STATE_WAIT:
	            if (!systick_pause) {
	                test_rf++;
	                if (test_rf < RF_POLL_FACTOR) break;
	                stateRF++;
	                test_rf = 0;
	            } else break;
	        case RF_STATE_MEAS:
	            if (RangeFinder_SingleMeas()) stateRF++;
	            break;
	        case RF_STATE_GET:
	            switch (RangeFinder_GetMessage()) {
	                case RF_OK: stateRF++;
	                case RF_NOREADY: break;
	                default: stateRF = RF_STATE_WAIT;
	            }
	            break;
	        default:;
	    }

	    // CAN + дисплей акселерометр
	    if (sendACC) {
	        CAN_SendMessage(CANID,   (uint8_t*) spi2_rx_data, 8);
	        CAN_SendMessage(CANID+1, (uint8_t*) spi2_rx_data+8, 8);
	        sendACC = 0;
	    }

	    if (!systick_pause) {
	        if (sendMAG) {
	            CAN_SendMessage(CANID+2, (uint8_t*)mag_lis3md.DMA_RX_fifo_buf, 8);
	            sendMAG = 0;
	        }
	        if (stateRF == RF_STATE_SEND) {
	            for (int i=0; i<3; i++)
	                CAN_SendMessage(CANID+4+i, (uint8_t*)(rangeData+i), 4);
	            stateRF = RF_STATE_WAIT;
	        }
	        GPIOA->BSRR = trigger ? GPIO_BSRR_BS12 : GPIO_BSRR_BR12;
	        trigger = trigger ? 0 : 1;
	        systick_pause = BASE_POLL_TIME;
	    }

	    if (sendPs) {
	        *(bmp280_sensor1.raw_p) = ((int32_t)(bmp280_sensor1.DMArx_buf[0]) << 12) |
	                                  ((int32_t)(bmp280_sensor1.DMArx_buf[1]) << 4)  |
	                                  ((bmp280_sensor1.DMArx_buf[2] >> 4) & 0x0F);
	        *(bmp280_sensor1.raw_t) = ((int32_t)(bmp280_sensor1.DMArx_buf[3]) << 12) |
	                                  ((int32_t)(bmp280_sensor1.DMArx_buf[4]) << 4)  |
	                                  ((bmp280_sensor1.DMArx_buf[5] >> 4) & 0x0F);

	        data32[0] = BMP280_Compensate_Temperature(&bmp280_sensor1);
	        data32[1] = BMP280_Compensate_Pressure(&bmp280_sensor1);

	        char buf[32];
	        ssd1306_Fill(Black);
	        ssd1306_SetCursor(0, 2);
	        ssd1306_WriteString("BMP280", Font_7x10, White);
	        ssd1306_SetCursor(0, 16);
	        snprintf(buf, sizeof(buf), "T:%.1fC", data32[0]);
	        ssd1306_WriteString(buf, Font_7x10, White);
	        ssd1306_SetCursor(0, 30);
	        snprintf(buf, sizeof(buf), "P:%.0fPa", data32[1]);
	        ssd1306_WriteString(buf, Font_7x10, White);
	        ssd1306_UpdateScreen();

	        CAN_SendMessage(CANID+3, bmp280_sensor1.DMArx_buf, 8);
	        sendPs = 0;
	    }
	}

}
;
///////////// IRQ block CANFD

//void I2C_SystemClock_Config(void)
//{
//  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
//  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
//
//  /** Configure the main internal regulator output voltage
//  */
//  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
//
//  /** Initializes the RCC Oscillators according to the specified parameters
//  * in the RCC_OscInitTypeDef structure.
//  */
//  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
//  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
//  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
//  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
//  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
//  {
//    Error_Handler();
//  }
//
//  /** Initializes the CPU, AHB and APB buses clocks
//  */
//  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
//                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
//  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
//  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
//  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
//  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
//
//  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
//  {
//    Error_Handler();
//  }
//}
// Message FIFO 0
void FDCAN1_IT1_IRQHandler(void) {

	uint32_t index_rxfifo = 0, rxHeader0, rxHeader1, id, dlc;
	uint32_t *RxBuffer;

	// Проверить, было ли прерывание из FIFO 0 ++
	if (FDCAN1->IR & 1 || FDCAN1->IR & 2) {

		index_rxfifo = (FDCAN1->RXF0S & FDCAN_RXF0S_F0GI)
				>> FDCAN_RXF0S_F0GI_Pos;
		RxBuffer = (uint32_t*) (RAMBaseFDCAN1 + RamFIFO0RX
				+ (index_rxfifo * 18 * 4));
		rxHeader0 = *RxBuffer++;
		rxHeader1 = *RxBuffer++;
		id = (rxHeader0 & XTDbit) ?
				rxHeader0 & 0x1FFFFFFF : (rxHeader0 & 0x1FFFFFFF) >> 18;
		dlc = (rxHeader1 >> 16) & 0xF;

		switch (id) {

		case 0x100:

			if (dlc < 2)
				break;

			test = 1;
			data = (*RxBuffer) & 0xFF;
			command = (*RxBuffer >> 8) & 0xFF;

			break;

		default:
			break;

		};

		FDCAN1->RXF0A = index_rxfifo;

		FDCAN1->IR |= 1; // clearFifo
	}

}

//void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
//{
//    if(hi2c->Instance == I2C2)
//    {
//        __HAL_RCC_GPIOA_CLK_ENABLE();
//        __HAL_RCC_I2C2_CLK_ENABLE();
//
//        GPIO_InitTypeDef GPIO_InitStruct = {0};
//
//        GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
//        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
//        GPIO_InitStruct.Pull = GPIO_PULLUP;
//        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//        GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
//
//        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//    }
//}
///////////// IRQ IIM42652

void DMA1_Channel1_IRQHandler(void) {

    if (DMA1->ISR & DMA_ISR_TCIF1) {

        DMA1->IFCR |= DMA_IFCR_CTCIF1;

        //DMA1_Channel1->CCR &= ~DMA_CCR_EN;

        DMAtx_IIM42XXX->CCR &= ~DMA_CCR_EN;

         IIM42XXX_CS_off

         iim_42652_status &= ~INT_FIFO_IIM42xxx;
         iim_42652_status |= DMA_OK_IIM42xxx;

    }

  DMA1->IFCR |= DMA_IFCR_CGIF1;
}
/*

PB12 IMU_int1  EXTI 12 configuration bits
PB11 IMU int2
*/

void EXTI15_10_IRQHandler(void) {

    if (EXTI->PR1 & (1 << 12)) { // PB12 INT1

        imu_irq_count++;    // ← додай сюди

        if((iim_42652_status & INT_FIFO_IIM42xxx) == 0){

          iim_42652_status |= INT_FIFO_IIM42xxx;

          IIM42XXX_CS_on

          DMA_TX_buf_iim42652[0] = 0xAD00;

          DMAtx_IIM42XXX->CNDTR = 12;
          DMAtx_IIM42XXX->CMAR = (uint32_t)DMA_TX_buf_iim42652;
          DMAtx_IIM42XXX->CCR |= DMA_CCR_EN;
        }

        EXTI->PR1 |= (1 << 12);
    }

    if (EXTI->PR1 & (1 << 11)) {
        EXTI->PR1 |= (1 << 11);
    }
}


///////////// IRQ LISM3D

void DMA1_Channel3_IRQHandler(void) {

    if (DMA1->ISR & DMA_ISR_TCIF3) {

        DMA1->IFCR |= DMA_IFCR_CTCIF3;

        DMAtx_LIS3M->CCR &= ~DMA_CCR_EN;

        LIS3M_CS_off

        *(mag_lis3md.status) &= ~INT_FIFO_LIS3MXX;
        *(mag_lis3md.status) |= DMA_OK_LIS3MXX;

    }

  DMA1->IFCR |= DMA_IFCR_CGIF3;
}



void EXTI1_IRQHandler(void){


	if((*(mag_lis3md.status) & INT_FIFO_LIS3MXX) == 0){

		*(mag_lis3md.status) |= INT_FIFO_LIS3MXX;

		LIS3M_CS_on;

		mag_lis3md.DMA_TX_fifo_buf[0]= ((LIS3M_STATUS_REG|INC_REG_LIS3M |READ_REG_LIS3M)<<8|0);

		 DMAtx_LIS3M->CNDTR = *(mag_lis3md.n_16bit_packet_fifo); //8 byte
		 DMAtx_LIS3M->CMAR = (uint32_t)mag_lis3md.DMA_TX_fifo_buf;
		 DMAtx_LIS3M->CCR |= DMA_CCR_EN;// enable tx dma
	}

	//if(!lis3m)lis3m = 1;

	EXTI->PR1 |= (1 << 1);//PB1

};

//static void MX_GPIO_Init(void)
//{
//  /* USER CODE BEGIN MX_GPIO_Init_1 */
//
//  /* USER CODE END MX_GPIO_Init_1 */
//
//  /* GPIO Ports Clock Enable */
//  __HAL_RCC_GPIOA_CLK_ENABLE();
//
//  /* USER CODE BEGIN MX_GPIO_Init_2 */
//
//  /* USER CODE END MX_GPIO_Init_2 */
//}
//static void MX_I2C2_Init(void)
//{
//
//  /* USER CODE BEGIN I2C2_Init 0 */
//
//  /* USER CODE END I2C2_Init 0 */
//
//  /* USER CODE BEGIN I2C2_Init 1 */
//
//  /* USER CODE END I2C2_Init 1 */
//  hi2c2.Instance = I2C2;
//  hi2c2.Init.Timing = 0x00503D58;
//  hi2c2.Init.OwnAddress1 = 0;
//  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
//  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
//  hi2c2.Init.OwnAddress2 = 0;
//  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
//  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
//  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
//  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
//  {
//    Error_Handler();
//  }
//
//  /** Configure Analogue filter
//  */
//  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
//  {
//    Error_Handler();
//  }
//
//  /** Configure Digital filter
//  */
//  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  /* USER CODE BEGIN I2C2_Init 2 */
//
//  /* USER CODE END I2C2_Init 2 */
//
//}


void Error_Handler(void) {
	// Обработка ошибок
	__disable_irq();
	while (1) {
	}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  // Обработка ошибок assert
}
#endif /* USE_FULL_ASSERT */
/*

    FusionAhrsInitialise(&ahrs);
    FusionAhrsSetSettings(&ahrs, &(FusionAhrsSettings){
        .gain = 0.5f,
        .accelerationRejection = 10.0f,
        .magneticRejection = 20.0f
        // Убрано rejectionTimeout, так как его нет в вашей версии
    });



			 //FusionAhrsUpdateNoMagnetometer(&ahrs, gyro, acc, DELTA_TIME);
			 //heading = FusionCompassCalculateHeading(FusionConventionNwu, acc, mag);
			 //FusionAhrsSetHeading(&ahrs, heading);
			 //quaternion = FusionAhrsGetQuaternion(&ahrs);







			ft32[0] = quaternion.element.w;
			ft32[1] = quaternion.element.x;
			CAN_SendMessage(id+1, (uint8_t*)ft32,8);
			ft32[2] = quaternion.element.y;
			ft32[3] = quaternion.element.z;
			CAN_SendMessage(id+2, (uint8_t*)ft32+2,8);









  */
