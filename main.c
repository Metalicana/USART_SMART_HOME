#include "RccConfig.h"
#include "Delay.h"
#include "Gpio.h"
#include "Uart.h"
#include "string.h"

#define LIVING 0
#define DINING 1
#define BEDROOM 2
#define OFFICE 3

void getInputString(uint8_t *data);
void USART2_IRQHandler(void);
void manageTemp(void);
void manageHumidity(void);
void manageWaterLevel(void);
void manageLight(void);

int wow;

struct Configuration
{
	int light;
	int temp;
	int humidity;
	int waterLevel;
}config;

uint8_t currentMessage[100];

void USART2_IRQHandler(void) {
	USART2->CR1 &= ~USART_CR1_RXNEIE;
	getInputString(currentMessage);
	USART2->CR1 |= USART_CR1_RXNEIE;
}


void getInputString(uint8_t *data)
{
	uint8_t nextByte;
	nextByte = getChar();
	int len = 1;
	while(nextByte != '.')
	{
		*data = nextByte;
		data++;
		if(nextByte=='.')break;
		nextByte = getChar();
		if(nextByte=='.')break;
		len++;
	}
	wow = 420;
	*data = '\0'; //terminating character
	for(int q=0;q<len;q++)
	{
		data--;
	}
	wow = 1000;
	for(int q=0;q<len;q++)
	{
		UART2_SendChar(data[q]);
	}
	
	 //char str[80] = "Config Init temp 25 water 50 hum 45 light 1111";
   char tempString[6]="temp";
   char waterString[6]="water";
   char humidityString[6]="hum";
   char lightString[6]="light";
   const char s[2] = " ";
   char *token;
   token = strtok(data, s);
	 char stringToSend[100]="";
   if(strcmp(token,"Read")==0)
   {
     token = strtok(NULL,s);
     token = strtok(NULL,s);
     if(token == NULL)
     {
       //send all
       sprintf(stringToSend, "Configuartion is: ");
       sprintf(stringToSend, "%stemperature %d, humidity %d, water level %d ",stringToSend,config.temp,config.humidity,config.waterLevel);
       sprintf(stringToSend, "%slight %d%d%d%d",stringToSend,(config.light/1000)%10,(config.light/100)%10,(config.light/10)%10,(config.light/1)%10 );
       //printf("%s\n",stringToSend);
     }
     else
     {
       while( token != NULL ) {
         sprintf(stringToSend, "Configuartion is: ");
         if(strcmp(tempString,token)==0)
         {
           sprintf(stringToSend, "%stemperature %d ",stringToSend,config.temp);

         }
         if(strcmp(waterString,token)==0)
         {
           sprintf(stringToSend, "%swaterLevel %d ",stringToSend,config.waterLevel);
         }
         if(strcmp(humidityString,token)==0)
         {
           sprintf(stringToSend, "%shumidity %d",stringToSend,config.humidity);
         }
         if(strcmp(lightString,token)==0)
         {
           sprintf(stringToSend, "%slight %d%d%d%d",stringToSend,(config.light/1000)%10,(config.light/100)%10,(config.light/10)%10,(config.light/1)%10 );
         }
          token = strtok(NULL, s);
       }
       //printf("%s",stringToSend);
     }
		len = (int)strlen(stringToSend);
		for(int q=0;q<len;q++)
		{
			UART2_SendChar(stringToSend[q]);
		}
     return;
   }
   while( token != NULL ) {
      if(strcmp(tempString,token)==0)
      {
        //printf("temperature set korte chacche\n");
        token = strtok(NULL,s);
        sscanf(token,"%d",&config.temp);
        //printf("%d\n",config.temp);
      }
      if(strcmp(waterString,token)==0)
      {
        //printf("water set korte chacche\n");
        token = strtok(NULL,s);
        sscanf(token,"%d",&config.waterLevel);
       // printf("%d\n",config.waterLevel);
      }
      if(strcmp(humidityString,token)==0)
      {
       // printf("humidity set korte chacche\n");
        token = strtok(NULL,s);
        sscanf(token,"%d",&config.humidity);
        //printf("%d\n",config.humidity);
      }
      if(strcmp(lightString,token)==0)
      {
        //printf("light set korte chacche\n");
        token = strtok(NULL,s);
        sscanf(token,"%d",&config.light);
        //printf("%d\n",config.light);
        //printf("%d\n",config.light);
      }
      token = strtok(NULL, s);
   }
	 return;
	
}

void manageTemp(void)
{
	//Temperature input is going to come from GPIOC 0 and 1
	#define temp0 (GPIOC->IDR & (1<<0))
	#define temp1 (GPIOC->IDR & (1<<1))
	//Heater is PA9
	//Cooler is PC7
	#define heaterON GPIOA->BSRR |= (1<<9)
	#define heaterOFF GPIOA->BSRR |= (1<<(9+16))
	
	#define coolerON GPIOC->BSRR |= (1<<7)
	#define coolerOFF GPIOC->BSRR |= (1<<(7+16))
	if( (temp0 && !temp1) || (!temp0 && temp1))//below ambient temp
	{
		heaterON;
		Delay_ms(5000);
		heaterOFF;
	}
	else if(temp0 && temp1)//high temp
	{
		coolerON;
		Delay_ms(5000);
		coolerOFF;	
	}

}
void manageHumidity(void)
{
	//Humidity input is going to come from GPIOB 0 and GPIOA 4
	#define hum0 (GPIOB->IDR & (1<<0))
	#define hum1 (GPIOA->IDR & (1<<4))
	
	//Humidifier is B10
	//Dehumidifier is A8
	#define humidifierON GPIOB->BSRR |= (1<<10)
	#define humidifierOFF GPIOB->BSRR |= (1<< (10+16) )
	
	#define dehumidiferON GPIOA->BSRR |= (1<<8)
	#define dehumidiferOFF GPIOA->BSRR |= (1<< (8+16))

	if( (hum0 && !hum1) || (!hum0 && hum1))//below ambient humidity
	{
		humidifierON;
		Delay_ms(5000);
		humidifierOFF;
	}
	else if(hum0 && hum1)//high temp
	{
		dehumidiferON;
		Delay_ms(5000);
		dehumidiferOFF;	
	}
	
}
void manageWaterLevel(void)
{
	//GPIOB 9 for water level
	#define water (GPIOB->IDR & (1<<9))
	
	//GPIOA 7 for output
	#define pumpON GPIOA->BSRR |= (1<<7)
	#define pumpOFF GPIOA->BSRR |= (1<<(7+16))
	
	if(water)
	{
		pumpON;
		Delay_ms(5000);
		pumpOFF;
	}
}
void manageLight(void)
{
	//Light toggle is going to come from GPIOA 1 0 
	#define light0 (GPIOA->IDR & (1<<1))
	#define light1 (GPIOA->IDR & (1<<0))
	
	//day night defined by GPIOB 8
	#define day_night (GPIOB->IDR & (1<<8))
	
	//4 light rooms are
	//PA10 PB3 PB5 PB4
	#define livingON GPIOA->BSRR |= (1<<10)
	#define livingOFF GPIOA->BSRR |= (1<<(10+16))
	
	
	#define diningON GPIOB->BSRR |= (1<<3)
	#define diningOFF GPIOB->BSRR |= (1<<(3+16))
	
	#define bedON GPIOB->BSRR |= (1<<5)
	#define bedOFF GPIOB->BSRR |= (1<<(5+16))
	
	#define officeON GPIOB->BSRR |= (1<<4)
	#define officeOFF GPIOB->BSRR |= (1<<(4+16))
	
	if(!day_night)//day
	{
		livingOFF;
		diningOFF;
		bedOFF;
		officeOFF;
	}
	else
	{
		if(light0 && light1 && (config.light/1000)%10 == 1)
		{
			livingON;
		}
		else if(light0 && !light1 && (config.light/100)%10 == 1)
		{
			diningON;
		}
		else if(!light0 && light1 && (config.light/10)%10 == 1)
		{
			bedON;
		}
		else if(!light0 && !light1 && (config.light/1)%10 == 1)
		{
			officeON;
		}
	}
}
int main()
{
	SysClockConfig(); // setting clock speed
	TIM6Config(); // timer configuration
	gpio_output_init(); // Gpio port mapping
	
	Uart2Config();//USART configuration
	gpio_usart_init();//Configure GPIO pins for usart
	
	enable_interrupts();
	NVIC_SetPriority(USART2_IRQn, 1);
	NVIC_EnableIRQ(USART2_IRQn);
	//handling interrupts
	
	wow = 0;
	while(1)
	{
		manageTemp();
		manageHumidity();
		manageWaterLevel();
		manageLight();
	}
}
