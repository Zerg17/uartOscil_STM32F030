#include "system.h"

void rccInit() {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_DMA1EN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_ADC1EN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
}

void gpioInit() {
    // PA9  - UART_TX        - AF1   - UART1_TX
    // PA10 - UART_RX        - AF1   - UART1_RX

    GPIOA->MODER |= GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_0;
    GPIOA->AFR[1] |= 0x00000110;
}

void uart1Write(uint8_t d) {
    while (!(USART1->ISR & USART_ISR_TXE));
    USART1->TDR = d;
}

void uart1Init() {
    USART1->BRR = F_CPU / UART1_BAUD;
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE;
    USART1->CR3 = USART_CR3_DMAT;
    USART1->CR1 |= USART_CR1_UE;

    DMA1_Channel2->CPAR = (uint32_t) (&(USART1->TDR));
    DMA1_Channel2->CCR = DMA_CCR_MINC | DMA_CCR_DIR;
}

void tim3Init(){
    TIM3->CR2 = TIM_CR2_MMS_1;
    TIM3->ARR = F_CPU/10000 - 1;
    TIM3->CR1 |= TIM_CR1_CEN;
}

void adcInit(){
    // Сброс ADC если был запущен
    if ((ADC1->CR & ADC_CR_ADEN) != 0) ADC1->CR |= ADC_CR_ADDIS;
    while ((ADC1->CR & ADC_CR_ADEN) != 0);
    ADC1->CFGR1 &= ~ADC_CFGR1_DMAEN;

    // Калибровка ADC
    ADC1->CR = ADC_CR_ADCAL;
    while(ADC1->CR & ADC_CR_ADCAL);
    if ((ADC1->ISR & ADC_ISR_ADRDY) != 0);

    // Запуск ADC
    ADC1->CR = ADC_CR_ADEN;
    while((ADC1->ISR & ADC_ISR_ADRDY) == 0);

    ADC1->SMPR = 0b111;  // Время зарядки емкости ADC 239.5 тактов

    ADC1->CFGR1
        =  ADC_CFGR1_EXTEN_0 // Работа по фронту триггера
        | ADC_CFGR1_EXTSEL_1 | ADC_CFGR1_EXTSEL_0 // Выбираем 3 таймер в качестве триггера
        | ADC_CFGR1_DMACFG  // Циклический режим работы с ДМА
        | ADC_CFGR1_DMAEN;   // Разрешение работы с ДМА

    ADC1->CHSELR = 0b11;  // Выбираем каналы для преобразования 0,1

    DMA1_Channel1->CNDTR = 256;
    DMA1_Channel1->CPAR = (uint32_t)(&(ADC1->DR));
    DMA1_Channel1->CMAR = (uint32_t)adcR;
    DMA1_Channel1->CCR
        = DMA_CCR_PL_0
        | DMA_CCR_MSIZE_0
        | DMA_CCR_PSIZE_0
        | DMA_CCR_MINC
        | DMA_CCR_CIRC
        | DMA_CCR_HTIE
        | DMA_CCR_TCIE;
    DMA1_Channel1->CCR |= DMA_CCR_EN;

    ADC1->CR |= ADC_CR_ADSTART;  // Запускаем ADC для отслеживания триггера
}

void nvicInit() {
    NVIC_SetPriority(SysTick_IRQn, 3);

    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
    SysTick_Config(F_CPU/100-1);
}

void sysInit() {
    rccInit();
    gpioInit();
    uart1Init();
    tim3Init();
    adcInit();
    nvicInit();
}