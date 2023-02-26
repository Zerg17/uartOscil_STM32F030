#include "sysControl.h"
#include "system.h"

uint16_t adcR[256];

uint32_t volatile tick=0;
uint32_t volatile sec=0;

void SysTick_Handler(void) {
    static uint32_t sec_d=0;
    tick++;
    if(++sec_d==100){
        sec_d=0;
        sec++;
    }
}

union {
    uint8_t buf[0];
    struct {
        uint8_t startSalt;
        uint8_t command;
        uint16_t msgLength;
        uint16_t measNum;
        uint16_t measTime;
        uint16_t values[128];
        uint16_t crc;
        uint16_t endSalt;
    } adcReadingsPack;
} sendMsg;

uint16_t mesNum=0;

void sendReadingsPack(uint16_t* data){
    GPIOA->BSRR |= GPIO_BSRR_BS_10;
    sendMsg.adcReadingsPack.startSalt = 0x55;
    sendMsg.adcReadingsPack.command = 0x00;
    sendMsg.adcReadingsPack.msgLength = 0x0401;
    sendMsg.adcReadingsPack.measNum = __REV16(mesNum);
    sendMsg.adcReadingsPack.measTime = 0x6400;  // 100 мкс
    for(int i=0;i<128;i++)
        sendMsg.adcReadingsPack.values[i] = __REV16(data[i]);
    sendMsg.adcReadingsPack.crc = 0;
    sendMsg.adcReadingsPack.endSalt = 0xA055;

    while(DMA1_Channel2->CNDTR) continue;
    DMA1_Channel2->CCR &=~DMA_CCR_EN;
    DMA1_Channel2->CMAR = (uint32_t)(&sendMsg);
    DMA1_Channel2->CNDTR = 268;
    DMA1_Channel2->CCR |= DMA_CCR_EN;
    GPIOA->BSRR |= GPIO_BSRR_BR_10;
}

void DMA1_Channel1_IRQHandler(void){
    if(DMA1->ISR & DMA_ISR_TCIF1){
        DMA1->IFCR=DMA_IFCR_CTCIF1;
        sendReadingsPack(adcR);
        mesNum++;
    }
    if(DMA1->ISR & DMA_ISR_HTIF1){
        DMA1->IFCR=DMA_IFCR_CHTIF1;
        sendReadingsPack(adcR+128);
        mesNum++;
    }
}