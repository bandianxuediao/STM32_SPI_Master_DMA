#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "w25qxx.h"
#include "dma.h"


/************************************************
 ALIENTEK��ӢSTM32������ʵ��23
 SPI ʵ��
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/


//Ҫд�뵽W25Q64���ַ�������
const u8 TEXT_Buffer[] = {"ELITE STM32 SPI TEST"};

u8 ReceiveBuff[100];    //�������ݻ�����
int main(void)
{
	u8 key;
	u16 i = 0;
	u8 j=0;
	u8 datatemp[SIZE];
	u32 FLASH_SIZE;

	delay_init();            //��ʱ������ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);      //���ڳ�ʼ��Ϊ115200

	DMA_Config();
LED_Init();

	KEY_Init();             //������ʼ��
	W25QXX_Init();          //W25QXX��ʼ��

  while((ReceiveBuff[0]<<8)+(ReceiveBuff[1])!=W25Q128)   //��ⲻ��W25Q128
  {
      delay_ms(500);
      LED1=!LED1;//DS1��˸
  }

	FLASH_SIZE = 128 * 1024 * 1024; //FLASH ��СΪ16M�ֽ�
	for(j=0;j<10;j++)
	{
	ReceiveBuff[j]=0;
	
	}
	SPI_DMA_BufferRead(0x90,(u32)0,2);
	
	while(1)
	{
		key = KEY_Scan(0);

		if(key == KEY1_PRES) //KEY1����,д��W25QXX
		{
			W25QXX_Write((u8*)TEXT_Buffer, FLASH_SIZE - 100, SIZE);     //�ӵ�����100����ַ����ʼ,д��SIZE���ȵ�����
		}

		if(key == KEY0_PRES) //KEY0����,��ȡ�ַ�������ʾ
		{
              
				SPI_DMA_BufferRead(W25X_ReadData,FLASH_SIZE - 100,SIZE);//�ӵ�����100����ַ����ʼ,����SIZE���ֽ�
		}

		i++;
		delay_ms(10);

		if(i == 20)
		{
			LED0 = !LED0; //��ʾϵͳ��������
			i = 0;
		}
	}
}


