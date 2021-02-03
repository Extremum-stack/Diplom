#include "stm32f10x.h"
#include "math.h"
#include "Font.h"
#include "stdbool.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

#define RX_BUFF_SIZE 256
#define FLASH_KEY1 ((uint32_t)0x45670123);
#define FLASH_KEY2 ((uint32_t)0xCDEF89AB);
#define ADDRESS_TIME 0x0801FC00
#define ADDRESS_LATITUDE 0x0801FC06
#define ADDRESS_LONGITUDE 0x0801FBE0
#define RADIUS 6371
#define PI 3.1415
#define A 6367558.4968
void delay(uint32_t time) { 
  for (int i = 0; i < time; i++) { };
}

void displaySPI2_Init(void) {
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;

  GPIOB->CRH &= ~(GPIO_CRH_CNF13_0 | GPIO_CRH_CNF15_0);
  GPIOB->CRH |= (GPIO_CRH_CNF13_1 | GPIO_CRH_CNF15_1);
  GPIOB->CRH &= ~(GPIO_CRH_MODE13_0 | GPIO_CRH_MODE15_0);
  GPIOB->CRH |= (GPIO_CRH_MODE13_1 | GPIO_CRH_MODE15_1);

  GPIOB->CRH &= ~(GPIO_CRH_CNF11 | GPIO_CRH_CNF14);
  GPIOB->CRH &= ~(GPIO_CRH_MODE11_0 | GPIO_CRH_MODE14_0);
  GPIOB->CRH |= (GPIO_CRH_MODE11_1 | GPIO_CRH_MODE14_1);

  
  GPIOB->CRH &= ~GPIO_CRH_CNF10;
  GPIOB->CRH &= ~GPIO_CRH_MODE10;
  GPIOB->CRH |= GPIO_CRH_MODE10_1;

  RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

  SPI2->CR1 |= SPI_CR1_BIDIMODE;
  SPI2->CR1 |= SPI_CR1_BIDIOE;
  SPI2->CR1 &= ~SPI_CR1_DFF;  // 8 	bit
  SPI2->CR1 |= SPI_CR1_CPOL;  // SPI-3
  SPI2->CR1 |= SPI_CR1_CPHA;  // SPI-3
  SPI2->CR1 |= SPI_CR1_BR;    // Baud rate control
  SPI2->CR1 &= ~SPI_CR1_LSBFIRST;
  SPI2->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;

  SPI2->CR1 |= SPI_CR1_MSTR;  // MASTER
  SPI2->CR1 |= SPI_CR1_SPE;   // turn on SPI
}
void SPI2_Write(uint8_t data) {
  SPI2->DR = data;
  while (!(SPI2->SR & SPI_SR_TXE));
  while (SPI2->SR & SPI_SR_BSY);
}
void display_cmd(uint8_t data) {
  GPIOB->BRR |= GPIO_BRR_BR14;
  SPI2_Write(data);
}
void display_data(uint8_t data) {
  GPIOB->BSRR |= GPIO_BSRR_BS14;
  SPI2_Write(data);
}
void display_Init(void) {
  GPIOB->BSRR |= GPIO_BSRR_BR10;
  GPIOB->BSRR |= GPIO_BSRR_BS10;
  display_cmd(0x21);
  display_cmd(0x13);
  display_cmd(0x04);
  display_cmd(0x80);
  display_cmd(0x20);
  display_cmd(0x0C);
}
void display_light_on(void) {
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

  GPIOB->CRH &= ~GPIO_CRH_CNF12;
  GPIOB->CRH |= GPIO_CRH_MODE12_1;
  GPIOB->CRH &= ~GPIO_CRH_MODE12_0;
  GPIOB->BSRR |= GPIO_BSRR_BS12;
}
void display_light_off(void) { GPIOB->BSRR |= GPIO_BSRR_BR12; }
void display_setpos(uint8_t x, uint8_t y) {
  display_cmd(0x80 + x);
  display_cmd(0x40 + y);
}
void display_clear(void) {
  for (uint8_t y = 0; y < 6; y++) {
    for (uint8_t x = 0; x < 84; x++) {
      display_data(0);
    }
  }
}
void flash_unlock(void) {
  FLASH->KEYR = FLASH_KEY1;
  FLASH->KEYR = FLASH_KEY2;
}
void flash_lock(void) { FLASH->CR |= FLASH_CR_LOCK; }

void flash_erase_page(uint32_t address) {
  while (FLASH->SR & FLASH_SR_BSY);
  if (FLASH->SR & FLASH_SR_EOP) {
    FLASH->SR = FLASH_SR_EOP;
  }
  FLASH->CR |= FLASH_CR_PER;
FLASH->AR = address;
  FLASH->CR |= FLASH_CR_STRT;
  while (!(FLASH->SR & FLASH_SR_EOP));
  FLASH->SR = FLASH_SR_EOP;
  FLASH->CR &= ~FLASH_CR_PER;
}

void flash_write(char* data, unsigned int address, unsigned int count) {
  unsigned int i;

  while (FLASH->SR & FLASH_SR_BSY)
    ;
  if (FLASH->SR & FLASH_SR_EOP) {
    FLASH->SR = FLASH_SR_EOP;
  }

  FLASH->CR |= FLASH_CR_PG;

  for (i = 0; i < count; i += 2) {
    *(volatile unsigned short*)(address + i) =
        (((unsigned short)data[i + 1]) << 8) + data[i];
    while (!(FLASH->SR & FLASH_SR_EOP))
      ;
    FLASH->SR = FLASH_SR_EOP;
  }

  FLASH->CR &= ~(FLASH_CR_PG);
}
char flash_read(uint32_t address) { return (*(__IO uint32_t*)address); }

void IntToChar(int d, char* out) {
  out[9] = '\0';
  out[8] = '0' + d % 10;
  out[7] = '0' + (d /= 10) % 10;
  out[6] = '0' + (d /= 10) % 10;
  out[5] = '0' + (d /= 10) % 10;
  out[4] = '0' + (d /= 10) % 10;
  out[3] = '0' + (d /= 10) % 10;
  out[2] = '0' + (d /= 10) % 10;
  out[1] = '0' + (d /= 10) % 10;
  out[0] = '0' + (d /= 10) % 10;
}

int CharToInt(char* in) {
  int i = 0;
  int result = 0;
  while (in[i] >= '0' && in[i] <= '9') {
    result = result + (in[i] - 0x30);
    result = result * 10;
    i++;
  }
  return result / 10;
}
float distance(float x1, float y1, float x2, float y2) { 

  x1 = ((PI * x1) / 180);
  x2 = ((PI * x2) / 180);

  y1 = ((PI * y1) / 180);
  y2 = ((PI * y2) / 180);

  float sx, sy;
  sx = sin((x2 - x1) / 2);
  sx = pow(sx, 2);

  sy = sin((y2 - y1) / 2);
  sy = pow(sy, 2);

  float root;
  root = sx + cos(x1) * cos(x2) * sy;
  root = sqrt(root);

  float length;
  length = 2 * RADIUS * asin(root);

  return length;
}

void ConvertToXY(float latitude, float longitude, float* ptrx, float* ptry) {
  latitude = ((PI * latitude) / 180);  // перевод широты в радианы
  float sin2latit = sin(2 * latitude);
  float sinlatit = sin(latitude);
  float sinlatit2 = pow(sinlatit, 2);
  float sinlatit4 = pow(sinlatit, 4);
  float sinlatit6 = pow(sinlatit, 6);
  float n = floor(((6 + longitude) / 6));  // номер шестиградусной зоны в проекции Гаусса-Крюгера
  float l = (longitude - (3 + 6 * (n - 1))) / 57.2958;
  float l2 = pow(l, 2);
  *ptrx = A * latitude - sin2latit * (16002.89 + 66.9607 * sinlatit2 + 
          0.3515 * sinlatit4 - l2 * (1594561.25 + 5336.535 * sinlatit2 
          + 26.790 * sinlatit4 + 0.149 * sinlatit6 + l2 * (672483.4 - 
          811219.9 * sinlatit2 + 5420 * sinlatit4 - 10.6 * sinlatit6 +
          l2 * (278194 - 830174 * sinlatit2 + 572434 * sinlatit4 -
          16010 * sinlatit6 + l2 * (109500 - 574700 * sinlatit2 +
          863700 * sinlatit4 - 398600 * sinlatit6)))));

  *ptry = (5 + 10 * n) * 100000 + l * cos(latitude) * (6378245 +
          21346.1415 * sinlatit2 + 107.1590 * sinlatit4 +0.5977 *
          sinlatit6 + l2 * (1070204.16 - 2136826.66 * sinlatit2 +
          17.98 * sinlatit4 - 11.99 * sinlatit6 + l2 * (270806 - 
          1523417 * sinlatit2 + 1327645 + sinlatit4 - 21701 * 
          sinlatit6 + l2 * (79690 - 866190 * sinlatit2 +
          1730360 * sinlatit4 - 945360 * sinlatit6))));
}
int DirectionalAngle(float x1, float y1, float x2, float y2) {  // x1, y1 - координаты текущей точки, x2, y2 - сохраненной точки
  float dx, dy, r12;
  dx = x2 - x1;
  dy = y2 - y1;
  int i = 0;
  int change = 0;
  if (dx > 0 && dy > 0) {
    change = 0;
    i = 1;
  }
  if (dx < 0 && dy > 0) {
    change = 180;
    i = -1;
  }
  if (dx < 0 && dy < 0) {
    change = 180;
    i = 1;
  }
  if (dx > 0 && dy < 0) {
    change = 360;
    i = -1;
  }
  r12 = atan(fabs(dy / dx));
  r12 = r12 * 180 / PI;

  float result;
  result = change + i * r12;

  return result;
}

void printchar(uint8_t ch) {
  int i;
  if (ch >= 0x20 && ch <= 0x80) {
    display_data(0x00);
    for (i = 0; i < 5; i++) {
      display_data(ASCII[ch - 0x20][i]);
    }
    display_data(0x00);
  }
}
void printstring(char* str) {
  while (*str) {
    printchar(*str);
    str++;
  }
}
void printstring_at(char* str, uint8_t x, uint8_t y) {
  display_setpos(x, y);
  while (*str) {
    printchar(*str);
    str++;
  }
}
void FullVoltage(void) {
  display_setpos(70, 0);
  display_data(0b11111111);
  display_data(0b10000001);
  display_data(0b10111101);
  display_data(0b10111101);
  display_data(0b10000001);
  display_data(0b10111101);
  display_data(0b10111101);
  display_data(0b10000001);
  display_data(0b10111101);
  display_data(0b10111101);
  display_data(0b11000011);
  display_data(0b01000010);
  display_data(0b01111110);
}

void MediumVoltage(void) {
  display_setpos(70, 0);
  display_data(0b11111111);
  display_data(0b10000001);
  display_data(0b10111101);
  display_data(0b10111101);
  display_data(0b10000001);
  display_data(0b10111101);
  display_data(0b10111101);
  display_data(0b10000001);
  display_data(0b10000001);
  display_data(0b10000001);
  display_data(0b11000011);
  display_data(0b01000010);
  display_data(0b01111110);
}

void LowVoltage(void) {
  display_setpos(70, 0);
  display_data(0b11111111);
  display_data(0b10000001);
  display_data(0b10111101);
  display_data(0b10111101);
  display_data(0b10000001);
  display_data(0b10000001);
  display_data(0b10000001);
  display_data(0b10000001);
  display_data(0b10000001);
  display_data(0b10000001);
  display_data(0b11000011);
  display_data(0b01000010);
  display_data(0b01111110);
}
void NoVoltage(void) {
  display_setpos(70, 0);
  display_data(0b11111111);
  display_data(0b10000001);
  display_data(0b10000001);
  display_data(0b10000001);
  display_data(0b10000001);
  display_data(0b10000001);
  display_data(0b10000001);
  display_data(0b10000001);
  display_data(0b10000001);
  display_data(0b10000001);
  display_data(0b11000011);
  display_data(0b01000010);
  display_data(0b01111110);
}
void printArrow(const uint8_t Arrow[504]) {
  display_setpos(0, 0);
  for (int row = 0; row < 6; row++) {
    for (int column = 0; column < 84; column++) {
      display_data(Arrow[84 * row + column]);
    }
  }
}


