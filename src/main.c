#include "Configuration.h"

void initBtn1(void);
void initBtn2(void);
void initBtn3(void);
void initBtn4(void);
void EXTI9_5_IRQHandler(void);
void USART2_Init(void);
void Comm_handler(void);
void ADC_init(void);
void CheckVoltage(float);
void TxStr(char *str);
uint16_t StartConvADC(void);

int page = 0;
int azim = 0;
uint16_t adcResult;

bool Light = false;
bool Recieved = false;
bool SavePF = true;
bool Data = false;
bool But12 = false;

char RxBuffer[RX_BUFF_SIZE];
char charTokens[13][RX_BUFF_SIZE];
char time[6] = {0};
char latitude[11] = {0};
char longitude[12] = {0};
char azimuth[4] = {0};
char time_flash[6] = {0};
char latitude_flash[11] = {0};
char longitude_flash[12] = {0};

float latit = 0, longit = 0;
float latit_2 = 0, longit_2 = 0;
float interval = 0;
float ADCvoltage = 5.9;
float stepADC = 0.000806;

int main() {
  displaySPI2_Init();
  display_Init();
  USART2_Init();
  initBtn1();
  initBtn2();
  initBtn3();
  initBtn4();
  ADC_init();
  display_light_on();
  display_clear();
while (1) {
    if (Recieved) {
      Comm_handler();
    }
  }
}

void initBtn1(void) {
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;

  GPIOB->CRL &= ~GPIO_CRL_MODE6;
  GPIOB->CRL &= ~GPIO_CRL_CNF6_0;
  GPIOB->CRL |= GPIO_CRL_CNF6_1;

  GPIOB->BSRR |= GPIO_BSRR_BS6;

  AFIO->EXTICR[1] |= AFIO_EXTICR2_EXTI6_PB;

  EXTI->FTSR |= EXTI_FTSR_TR6;
  EXTI->PR |= EXTI_PR_PR6;
  EXTI->IMR |= EXTI_IMR_MR6;

  NVIC_EnableIRQ(EXTI9_5_IRQn);
  NVIC_SetPriority(EXTI9_5_IRQn, 7);
}

void initBtn2(void) {
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;

  GPIOB->CRL &= ~GPIO_CRL_MODE5;
  GPIOB->CRL &= ~GPIO_CRL_CNF5_0;
  GPIOB->CRL |= GPIO_CRL_CNF5_1;

  GPIOB->BSRR |= GPIO_BSRR_BS5;

  AFIO->EXTICR[1] |= AFIO_EXTICR2_EXTI5_PB;

  EXTI->FTSR |= EXTI_FTSR_TR5;
  EXTI->PR |= EXTI_PR_PR5;
  EXTI->IMR |= EXTI_IMR_MR5;

  NVIC_EnableIRQ(EXTI9_5_IRQn);
  NVIC_SetPriority(EXTI9_5_IRQn, 1);
}

void initBtn3(void) {
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;

  GPIOB->CRL &= ~GPIO_CRL_MODE4;
  GPIOB->CRL &= ~GPIO_CRL_CNF4_0;
  GPIOB->CRL |= GPIO_CRL_CNF4_1;

  GPIOB->BSRR |= GPIO_BSRR_BS4;

  AFIO->EXTICR[1] |= AFIO_EXTICR2_EXTI4_PB;
  EXTI->FTSR |= EXTI_FTSR_TR4;
  EXTI->PR |= EXTI_PR_PR4;
  EXTI->IMR |= EXTI_IMR_MR4;

  NVIC_EnableIRQ(EXTI4_IRQn);
  NVIC_SetPriority(EXTI4_IRQn, 2);
}

void initBtn4(void) {
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;

  GPIOB->CRL &= ~GPIO_CRL_MODE3;
  GPIOB->CRL &= ~GPIO_CRL_CNF3_0;
  GPIOB->CRL |= GPIO_CRL_CNF3_1;

  GPIOB->BSRR |= GPIO_BSRR_BS3;

  AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
  AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI3_PB;

  EXTI->FTSR |= EXTI_FTSR_TR3;
  EXTI->PR |= EXTI_PR_PR3;
  EXTI->IMR |= EXTI_IMR_MR3;

  NVIC_EnableIRQ(EXTI3_IRQn);
  NVIC_SetPriority(EXTI3_IRQn, 3);
}
void EXTI9_5_IRQHandler(void) {
  if (EXTI->PR & EXTI_PR_PR6) {
    delay(1000);
    display_clear();
    if (But12 == false) {
      int i;
      flash_unlock();
      for (i = 0; i < 5; i++) {
        time_flash[i] = flash_read(ADDRESS_TIME + i);
      }
      for (i = 0; i < 11; i++) {
        latitude_flash[i] = flash_read(ADDRESS_LATITUDE + i);
      }
      for (i = 0; i < 12; i++) {
        longitude_flash[i] = flash_read(ADDRESS_LONGITUDE + i);
      }
      flash_lock();
      for (i = 4; i <= 9; i++) {
        latitude_flash[i] = latitude_flash[i + 1];
      }
      latit_2 = CharToInt(latitude_flash);
      latit_2 = latit_2 / 1000000;
      float fplatit_2 = 0;
      fplatit_2 = modff(latit_2, &latit_2);
      fplatit_2 = fplatit_2 * 100 / 60;
      latit_2 = latit_2 + fplatit_2;

      for (i = 5; i <= 10; i++) {
        longitude_flash[i] = longitude_flash[i + 1];}
longit_2 = CharToInt(longitude_flash);
      longit_2 = longit_2 / 1000000;
      float fplongit_2 = 0;
      fplongit_2 = modff(longit_2, &longit_2);
      fplongit_2 = fplongit_2 * 100 / 60;
      longit_2 = longit_2 + fplongit_2;

      for (i = 8; i >= 4; i--) {
        latitude_flash[i + 1] = latitude_flash[i];
      }
      latitude_flash[4] = '.';
      latitude_flash[10] = '\0';
      for (i = 9; i >= 5; i--) {
        longitude_flash[i + 1] = longitude_flash[i];
      }
      longitude_flash[5] = '.';
      longitude_flash[11] = '\0';
      But12 = true;
    }

    if (Data == false) {
      printstring_at("No signal", 10, 2);
    }

    else {
      if (latit_2 == 0 && longit_2 == 0) {
        printstring_at("Point not", 11, 2);
        printstring_at("saved", 25, 3);
      } else {
        if (latit == latit_2 && longit == longit_2) {
          printstring_at("You come", 15, 2);
        } else {
          float latitda_1 = latit;
          float longitda_1 = longit;

          float *ptrx1 = &latitda_1;
          float *ptry1 = &longitda_1;
          ConvertToXY(latitda_1, longitda_1, ptrx1, ptry1);  

          float latitda_2 = latit_2;
          float longitda_2 = longit_2;

          float *ptrx2 = &latitda_2;
          float *ptry2 = &longitda_2;
          ConvertToXY(latitda_2, longitda_2, ptrx2, ptry2);  

          int DirectAngle;
          DirectAngle = DirectionalAngle(latitda_1, longitda_1, latitda_2, longitda_2);
          if (azim > DirectAngle) {
            DirectAngle = 360 - azim + DirectAngle;
          } else {
            DirectAngle = DirectAngle - azim;
          }
          DirectAngle = DirectAngle / 11.25;
          DirectAngle = round(DirectAngle / 2);
switch (DirectAngle) {
            case 1:
              printArrow(Arrow1);
              break;
            case 2:
              printArrow(Arrow2);
              break;
            case 3:
              printArrow(Arrow3);
              break;
            case 4:
              printArrow(Arrow4);
              break;
            case 5:
              printArrow(Arrow5);
              break;
            case 6:
              printArrow(Arrow6);
              break;
            case 7:
              printArrow(Arrow7);
              break;
            case 8:
              printArrow(Arrow8);
              break;
            case 9:
              printArrow(Arrow9);
              break;
            case 10:
              printArrow(Arrow10);
              break;
            case 11:
              printArrow(Arrow11);
              break;
            case 12:
              printArrow(Arrow12);
              break;
            case 13:
              printArrow(Arrow13);
              break;
            case 14:
              printArrow(Arrow14);
              break;
            case 15:
              printArrow(Arrow15);
              break;
            case 16:
              printArrow(Arrow0);
              break;
            case 0:
              printArrow(Arrow0);
              break;
          }
        }
      }
    }
ADCvoltage = (float)StartConvADC() * stepADC * 2;
    CheckVoltage(ADCvoltage);
    page = 0;
    EXTI->PR |= EXTI_PR_PR6;
  }
  if (EXTI->PR & EXTI_PR_PR5) {
    delay(1000);

    switch (page) {
      case 0:
        display_clear();
        if (Data == false) {
          printstring_at("No signal", 10, 2);
        } else {
          printstring_at(time, 0, 0);
          printstring("(UTC)");
          printstring_at(latitude, 0, 1);
          printstring_at(longitude, 0, 2);
        }
        printstring_at("page 1", 40, 5);
        ADCvoltage = (float)StartConvADC() * stepADC * 2;
        CheckVoltage(ADCvoltage);
        page = page + 1;
        break;

      case 1:
        if (But12 == false) {
          int i;
          flash_unlock();
          for (i = 0; i < 5; i++) {
            time_flash[i] = flash_read(ADDRESS_TIME + i);
          }
          for (i = 0; i < 11; i++) {
            latitude_flash[i] = flash_read(ADDRESS_LATITUDE + i);
          }
          for (i = 0; i < 12; i++) {
            longitude_flash[i] = flash_read(ADDRESS_LONGITUDE + i);
          }
          flash_lock();
          for (i = 4; i <= 9; i++) {
            latitude_flash[i] = latitude_flash[i + 1];
          }
          latit_2 = CharToInt(latitude_flash);
          latit_2 = latit_2 / 1000000;
          float fplatit_2 = 0;
          fplatit_2 = modff(latit_2, &latit_2);
          fplatit_2 = fplatit_2 * 100 / 60;
          latit_2 = latit_2 + fplatit_2;

          for (i = 5; i <= 10; i++) {
            longitude_flash[i] = longitude_flash[i + 1];
          }
          longit_2 = CharToInt(longitude_flash);
          longit_2 = longit_2 / 1000000;
          float fplongit_2 = 0;
          fplongit_2 = modff(longit_2, &longit_2);
          fplongit_2 = fplongit_2 * 100 / 60;
          longit_2 = longit_2 + fplongit_2;

          for (i = 8; i >= 4; i--) {
            latitude_flash[i + 1] = latitude_flash[i];
          }
          latitude_flash[4] = '.';
          latitude_flash[10] = '\0';
          for (i = 9; i >= 5; i--) {
            longitude_flash[i + 1] = longitude_flash[i];
          }
          longitude_flash[5] = '.';
          longitude_flash[11] = '\0';
          But12 = true;
        }

        display_clear();
        if (time_flash[2] == ':') {
          int i;
          printstring_at(time_flash, 0, 0);
          printstring("(UTC)");
          printstring_at(latitude_flash, 0, 1);
          printstring_at(longitude_flash, 0, 2);

          if (latit != 0 && longit != 0 && latit_2 != 0 && longit_2 != 0) {
            interval = distance(latit_2, longit_2, latit, longit);
            char dist[10] = {0};
            interval *= 10000;
            if (interval != 0) {
              IntToChar(interval, dist);

              for (i = (strlen(dist) - 1); i >= (strlen(dist) - 5); i--) {
                dist[i + 1] = dist[i];
              }
              dist[i + 1] = ',';

              for (int i = 0; i < strlen(dist); i++) {
                int j;
                if (dist[0] == '0' && dist[1] != ',') {
                  for (j = 0; j < strlen(dist); j++) {
                    dist[j] = dist[j + 1];
                  }
                  dist[j] = '\0';
                }
              }

              printstring_at(dist, 0, 3);
            } else {
              printstring_at("0", 0, 3);
            }
            printstring("km");
          }
        } else {
          printstring_at("Point not", 11, 2);
          printstring_at("saved", 25, 3);
        }
printstring_at("page 2", 40, 5);
        ADCvoltage = (float)StartConvADC() * stepADC * 2;
        CheckVoltage(ADCvoltage);
        page = 0;
        break;
    }
    EXTI->PR |= EXTI_PR_PR5;
  }
}
void EXTI4_IRQHandler(void) {
  if (EXTI->PR & EXTI_PR_PR4) {
    delay(1000);
    flash_unlock();
    if (SavePF == false) {
      if (Data == true) {
        flash_erase_page(ADDRESS_TIME);
        flash_write(time, ADDRESS_TIME, 5);
        flash_write(latitude, ADDRESS_LATITUDE, 11);
        flash_erase_page(ADDRESS_LONGITUDE);
        flash_write(longitude, ADDRESS_LONGITUDE, 12);
        display_clear();
        printstring_at("Point saved", 2, 2);
        SavePF = true;
        But12 = false;
      } else {
        display_clear();
        printstring_at("No signal", 10, 2);}
    } else {
      flash_erase_page(ADDRESS_TIME);
      flash_erase_page(ADDRESS_LONGITUDE);
      display_clear();
      printstring_at("Point deleted", 0, 2);
      latit_2 = 0;
      longit_2 = 0;
      SavePF = false;
      But12 = false;}
    flash_lock();
    ADCvoltage = (float)StartConvADC() * stepADC * 2;
    CheckVoltage(ADCvoltage);
    page = 0;
    EXTI->PR |= EXTI_PR_PR4;
  }
}
void EXTI3_IRQHandler(void) {
  if (EXTI->PR & EXTI_PR_PR3) {
    delay(1000);
    if (Light == false) {
      display_light_on();
      Light = true;
    } else {
      display_light_off();
      Light = false;
    }
    EXTI->PR |= EXTI_PR_PR3;
  }
}
void USART2_Init(void) {
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;  
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN;  

  GPIOA->CRL &= ~(GPIO_CRL_CNF2 | GPIO_CRL_MODE2);
  GPIOA->CRL |= (GPIO_CRL_MODE2 | GPIO_CRL_CNF2_1);
  GPIOA->CRL &= ~(GPIO_CRL_CNF3 | GPIO_CRL_MODE3);
  GPIOA->CRL |= GPIO_CRL_CNF3_0;  

  USART2->BRR = 0x9C4;  
  USART2->CR1 |= USART_CR1_TE;
  USART2->CR1 |= USART_CR1_RE;  
  USART2->CR1 |= USART_CR1_UE;  
  USART2->CR1 |= USART_CR1_RXNEIE;  
  NVIC_EnableIRQ(USART2_IRQn);  
  NVIC_SetPriority(USART2_IRQn, 0);  
}

void USART2_IRQHandler(void) {
  char tmp;
  if (USART2->SR & USART_SR_RXNE)  
  {
    tmp = USART2->DR;
    if (tmp == '*') {
      Recieved = true;
      return;
    }
    RxBuffer[strlen(RxBuffer)] = tmp;
  }
}
void TxStr(char *str) {
  for (uint16_t i = 0; i < strlen(str); i++) {
    while (!(USART2->SR & USART_SR_TC))
      ;
    USART2->DR = str[i];
  }
}
void Comm_handler(void) {
  Recieved = false;
  if (strncmp(RxBuffer, "$GPRMC", 6) == 0) {
    strcat(RxBuffer, "*");
    int i, j = 0;

    char *token;
    char *token2;

    strcpy(token, RxBuffer);
    char *delimeter = ",";
    TxStr(RxBuffer);
    while (token != NULL) {
      token2 = strpbrk(token + 1, delimeter);
      if (token2 == NULL) {
        delimeter = "*";
        token2 = strpbrk(token + 1, "*");
      }
      j = 0;
memset(charTokens[i], 0, RX_BUFF_SIZE);
      for (char *ch = token + 1; ch < token2; ch++) {
        charTokens[i][j] = *ch;
        j++;
      }
      i++;
      if (delimeter[0] == '*') {
        token = NULL;
      } else {
        token = token2;
      }
    }

    strncpy(time, charTokens[1], 4);  
    strncpy(latitude, charTokens[3], 9);
    latitude[9] = charTokens[4][0];
    strncpy(longitude, charTokens[5], 10);
    longitude[10] = charTokens[6][0];
    strncpy(azimuth, charTokens[8], 3);
    azimuth[3] = '\0';

    if (time[0] == '\0' || latitude[0] == '\0' || longitude[0] == '\0') {
      Data = false;
      latit = 0;
      longit = 0;
    } else {
      Data = true;
      for (i = 3; i >= 2; i--) {
        time[i + 1] = time[i];
      }
      time[2] = ':';
      time[5] = '\0';

      for (i = 4; i < 9; i++) {
        latitude[i] = latitude[i + 1];
      }
      latitude[9] = '\0';
      latit = CharToInt(latitude);
      latit = latit / 1000000;
      float fplatit = 0;
      fplatit = modff(latit, &latit);
      fplatit = fplatit * 100 / 60;
      latit = latit + fplatit;

      for (i = 8; i >= 4; i--) {
        latitude[i + 1] = latitude[i];
      }
      latitude[4] = '.';
      latitude[10] = '\0';

      for (i = 5; i < 10; i++) {
        longitude[i] = longitude[i + 1];
      }
      longitude[10] = '\0';
      longit = CharToInt(longitude);
      longit = longit / 1000000;
      float fplongit = 0;
      fplongit = modff(longit, &longit);
      fplongit = fplongit * 100 / 60;
      longit = longit + fplongit;

      for (i = 9; i >= 5; i--) {
        longitude[i + 1] = longitude[i];
      }
      longitude[5] = '.';
      longitude[11] = '\0';

      if (azimuth[0] != '\0') {
        azim = CharToInt(azimuth);
      }
    }
  }
  memset(RxBuffer, 0, RX_BUFF_SIZE);
}
void ADC_init(void) {
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;
  GPIOA->CRL &= ~(GPIO_CRL_CNF5 | GPIO_CRL_MODE5);

  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN | RCC_CFGR_ADCPRE_DIV2;

  ADC1->CR2 |= ADC_CR2_CAL;
  while (!(ADC1->CR2 & ADC_CR2_CAL)) {};

  ADC1->CR2 |= ADC_CR2_ADON;
  ADC1->CR2 &= ~ADC_CR2_CONT;
  ADC1->CR2 |= ADC_CR2_EXTSEL;
  ADC1->CR2 |= ADC_CR2_EXTTRIG;
  ADC1->SMPR1 &= ~ADC_SMPR1_SMP16;
  ADC1->SQR3 |= ADC_SQR3_SQ1_0;
  ADC1->SQR3 |= ADC_SQR3_SQ1_2;
}
uint16_t StartConvADC(void) {
  ADC1->SR &= ~ADC_SR_EOC;
  ADC1->CR2 |= ADC_CR2_SWSTART;
  while (!(ADC1->SR & ADC_SR_EOC)) {
  };
  return (ADC1->DR);
}
void CheckVoltage(float volt) {
  if (volt >= 5.9) {
    FullVoltage();
  }
  if (volt >= 5.0 && volt < 5.9) {
    MediumVoltage();
  }
  if (volt >= 4.05 && volt < 5.0) {
    LowVoltage();
  }
  if (volt < 4.05) {
    NoVoltage();
  }
}
