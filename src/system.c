#include "system.h"

void rccInit() {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
}

void gpioInit() {
    // PA9  - UART_TX        - AF1   - UART1_TX
    // PA10 - UART_RX        - AF1   - UART1_RX

    GPIOA->MODER |= GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1;
    GPIOA->AFR[1] |= 0x00000110;
}

void uart1Write(uint8_t d) {
    while (!(USART1->ISR & USART_ISR_TXE));
    USART1->TDR = d;
}

void uart1Init() {
    USART1->BRR = F_CPU / UART1_BAUD;
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE;
    USART1->CR1 |= USART_CR1_UE;
}

void nvicInit() {
    NVIC_SetPriority(SysTick_IRQn, 3);
    SysTick_Config(F_CPU/100-1);
}

void sysInit() {
    rccInit();
    gpioInit();
    uart1Init();
    nvicInit();
}