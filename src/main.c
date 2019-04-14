#include <stm32f4xx_tim.h>
#include <stm32f4xx_rcc.h>
#include <misc.h>
#include "defines.h"
#include "attributes.h"
#include "tm_stm32f4_dac.h"
#include "tm_stm32f4_pwm.h"
#include "tm_stm32f4_nrf24l01.h"
#include "tm_stm32f4_usart.h"
#include "tm_stm32f4_timer_properties.h"


/* Receiver address */
uint8_t TxAddress[] = {
    0xE7,
    0xE7,
    0xE7,
    0xE7,
    0xE7
};
/* My address */
uint8_t MyAddress[] = {
    0x7E,
    0x7E,
    0x7E,
    0x7E,
    0x7E
};

uint8_t dataIn[32];

typedef struct _PPM_Data {
    uint16_t pwms[6];
    uint8_t channel_cnt;
    uint8_t channel_now;
    uint16_t frame_length;
}PPM_Data;

static PPM_Data ppm_data;
void PPM_Modulation() {
    static const int set_time = 50;
    static int time_stamp = 0;
    static int last_start = 0;

    time_stamp++;

    /* Data Frame ends */
    if(time_stamp == ppm_data.frame_length) {
        time_stamp = 0;
        last_start = 0;
        ppm_data.channel_now = 0;
        GPIO_SetBits(GPIOB, GPIO_Pin_8);
        return;
    }

    /* Set period ends */
    if(time_stamp == last_start + set_time) {
        GPIO_ResetBits(GPIOB, GPIO_Pin_8);
        return;
    }

    /* Reset period ends */
    if(ppm_data.channel_now < ppm_data.channel_cnt &&
            time_stamp == last_start + set_time +
            ppm_data.pwms[ppm_data.channel_now] - 50) {
        GPIO_SetBits(GPIOB, GPIO_Pin_8);
        ppm_data.channel_now++;

        last_start = time_stamp;
        return;
    }


}

void TIM2_IRQHandler()
{

    // Checks whether the TIM2 interrupt has occurred or not
    if (TIM_GetITStatus(TIM2, TIM_IT_Update))
    {
        // Clears the TIM2 interrupt pending bit
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        PPM_Modulation();
    }
}

void InitializeGPIOs()
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef gpioStructure;
    gpioStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8;
    gpioStructure.GPIO_Mode = GPIO_Mode_OUT;
    gpioStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpioStructure);

    GPIO_WriteBit(GPIOB, GPIO_Pin_7 | GPIO_Pin_8, Bit_RESET);


}


void InitializeTimer()
{
    TIM_TimeBaseInitTypeDef TIM_BaseStruct;
    TM_TIMER_PROPERTIES_t TIM2_Data;

    /* Set NVIC for timer interrupt */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    /* Get timer properties */
    TM_TIMER_PROPERTIES_GetTimerProperties(TIM2, &TIM2_Data);
    /* Generate settings */
    TM_TIMER_PROPERTIES_GenerateDataForWorkingFrequency(&TIM2_Data, 100000);

    /* Enable clock for Timer */
    TM_TIMER_PROPERTIES_EnableClock(TIM2);

    /* Set timer options */
    TIM_BaseStruct.TIM_Prescaler = TIM2_Data.Prescaler - 1;
    TIM_BaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_BaseStruct.TIM_Period = TIM2_Data.Period - 1;
    TIM_BaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_BaseStruct.TIM_RepetitionCounter = 0;

    /* Initialize timer */
    TIM_TimeBaseInit(TIM2, &TIM_BaseStruct);

    /* Preload enable */
    TIM_ARRPreloadConfig(TIM2, ENABLE);

    /* Enable Timer */
    TIM_ClearFlag(TIM2,TIM_FLAG_Update);
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
    TIM_Cmd(TIM2,ENABLE);
}

void PPM_initialization() {

    for(int i = 0;i < 6; i++) {
        ppm_data.pwms[i] = 100;
        ppm_data.channel_cnt = 6;
        ppm_data.frame_length = 2000;
    }
    TM_USART_Puts(USART1, "RF Initialization ... Done\n");
}

void RF_initialization() {
    /* Initialize NRF24L01+ on channel 15 and 32bytes of payload */
    /* By default 2Mbps data rate and 0dBm output power */
    /* NRF24L01 goes to RX mode by default */
    TM_NRF24L01_Init(15, 32);

    /* Set RF settings, Data rate to 2Mbps, Output power to -18dBm */
    TM_NRF24L01_SetRF(TM_NRF24L01_DataRate_2M, TM_NRF24L01_OutputPower_M18dBm);

    /* Set my address, 5 bytes */
    TM_NRF24L01_SetMyAddress(MyAddress);

    /* Set TX address, 5 bytes */
    TM_NRF24L01_SetTxAddress(TxAddress);
    TM_USART_Puts(USART1, "RF Initialization ... Done\n");
}

TM_PWM_TIM_t TIM3_Data;
void Gimbal_initialization() {

    /* Set PWM to 1kHz frequency on timer TIM3 */
    TM_PWM_InitTimer(TIM3, &TIM3_Data, 1000);
    /* Initialize PWM on TIM3, Channel 1 and PinsPack 1 = PA6 */
    TM_PWM_InitChannel(&TIM3_Data, TM_PWM_Channel_1, TM_PWM_PinsPack_1);

    /* Set channel 1 value, 50% duty cycle */
    TM_PWM_SetChannel(&TIM3_Data, TM_PWM_Channel_1, TIM3_Data.Period / 2);

    /* Initialize DAC1&2, Signal: PA4, PA5, set to midpoint */
    TM_DAC_Init(TM_DAC1);
    TM_DAC_SetValue(TM_DAC1, 2048);
    TM_DAC_Init(TM_DAC2);
    TM_DAC_SetValue(TM_DAC2, 2048);
}

int parseDataIn(uint8_t data[], int st) {
    st *= 4;
    int ret = 0;
    for(int i = 0;i < 3;i++) {
        ret *= 10;
        ret += data[st++]-'0';
    }
    return ret;
}

static int i = 0;
int main()
{
    InitializeGPIOs();
    InitializeTimer();


    /* Initialize USART, TX: PA9, RX: PA10 */
    TM_USART_Init(USART1, TM_USART_PinsPack_1, 115200);
    TM_USART_Puts(USART1, "UART1 Initialization ... Done\n");

    TM_NRF24L01_Transmit_Status_t transmissionStatus;
    RF_initialization();
    PPM_initialization();
    Gimbal_initialization();

    dataIn[0] = 1;
    dataIn[1] = 100;
    dataIn[2] = 100;
    dataIn[3] = 100;
    dataIn[4] = 100;
    dataIn[5] = 100;
    dataIn[6] = 100;
    dataIn[7] = 100;
    dataIn[8] = 100;

    while(1) {

        /* If data is ready on NRF24L01+ */
        if (TM_NRF24L01_DataReady()) {
            /* Get data from NRF24L01+ */
            TM_NRF24L01_GetData(dataIn);
            /* Display on USART */
            uint8_t errmsg[256];
            sprintf(errmsg,"%d %d %d %d %d %d %d %d %d %d\n",
                    i++, dataIn[0], dataIn[1], dataIn[2], dataIn[3], dataIn[4],
                    dataIn[5], dataIn[6], dataIn[7], dataIn[8]);
            TM_USART_Puts(USART1, errmsg);
            int ss = dataIn[0];
            int gimbalx = dataIn[1] * 2048 / 100;
            int gimbaly = dataIn[2] * 2048 / 100;

            for (int ch = 0; ch < 6; ch++) {
                ppm_data.pwms[ch] = dataIn[ch+3];
            }

            TM_DAC_SetValue(TM_DAC1, gimbalx);
            TM_DAC_SetValue(TM_DAC2, gimbaly);

        }
    }

}
