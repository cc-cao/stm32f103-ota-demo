#include "bee.h"

static ADC_HandleTypeDef hadc1;
static ADC_ChannelConfTypeDef s_config;
DMA_HandleTypeDef hdma;
#define CONTINUE_CNOV 0
#define DMA           1

uint16_t adc_dma[2];

void adc_init() {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_ADC_CONFIG(RCC_ADCPCLK2_DIV6);
    __HAL_RCC_DMA1_CLK_ENABLE();

    GPIO_InitTypeDef gpio_def = {
        .Pin = GPIO_PIN_0 | GPIO_PIN_1,
        .Mode = GPIO_MODE_AF_INPUT,
        .Speed = GPIO_SPEED_FREQ_HIGH,
    };
    HAL_GPIO_Init(GPIOA, &gpio_def);

    hadc1.Instance = ADC1;
#if CONTINUE_CNOV || DMA
    hadc1.Init.ContinuousConvMode = ENABLE;
#else
    hadc1.Init.ContinuousConvMode = DISABLE;
#endif
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
#if DMA
    hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
#else
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
#endif
    hadc1.Init.NbrOfConversion = 2;

    HAL_ADC_Init(&hadc1);

    s_config.Channel = ADC_CHANNEL_0;
    s_config.Rank = ADC_REGULAR_RANK_1;
    s_config.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;

    HAL_ADC_ConfigChannel(&hadc1, &s_config);

    s_config.Channel = ADC_CHANNEL_1;
    s_config.Rank = ADC_REGULAR_RANK_2;
    s_config.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;

    HAL_ADC_ConfigChannel(&hadc1, &s_config);

    assert_param(HAL_ADCEx_Calibration_Start(&hadc1) == HAL_OK);

#if CONTINUE_CNOV
    HAL_ADC_Start(&hadc1);
#endif

#if DMA
    hdma.Instance = DMA1_Channel1;
    hdma.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma.Init.MemInc = DMA_MINC_ENABLE;
    hdma.Init.Mode = DMA_CIRCULAR;
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma.Init.Priority = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&hdma);
    __HAL_LINKDMA(&hadc1, DMA_Handle, hdma);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma, 2);
#endif
}

uint32_t adc_read(uint32_t channel) {
#if !CONTINUE_CNOV
    s_config.Channel = channel;
    HAL_ADC_ConfigChannel(&hadc1, &s_config);
    HAL_ADC_Start(&hadc1);
#endif
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    return HAL_ADC_GetValue(&hadc1);
}
