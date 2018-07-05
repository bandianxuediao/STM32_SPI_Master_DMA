#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "w25qxx.h"
#include "dma.h"


/************************************************
 ALIENTEK精英STM32开发板实验23
 SPI 实验
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司
 作者：正点原子 @ALIENTEK
************************************************/


//要写入到W25Q64的字符串数组
const u8 TEXT_Buffer[] = {"ELITE STM32 SPI TEST"};

u8 ReceiveBuff[100];    //发送数据缓冲区
int main(void)
{
	u8 key;
	u16 i = 0;
	u8 j=0;
	u8 datatemp[SIZE];
	u32 FLASH_SIZE;

	delay_init();            //延时函数初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);      //串口初始化为115200

	DMA_Config();
LED_Init();

	KEY_Init();             //按键初始化
	W25QXX_Init();          //W25QXX初始化

  while((ReceiveBuff[0]<<8)+(ReceiveBuff[1])!=W25Q128)   //检测不到W25Q128
  {
      delay_ms(500);
      LED1=!LED1;//DS1闪烁
  }

	FLASH_SIZE = 128 * 1024 * 1024; //FLASH 大小为16M字节
	for(j=0;j<10;j++)
	{
	ReceiveBuff[j]=0;
	
	}
	SPI_DMA_BufferRead(0x90,(u32)0,2);
	
	while(1)
	{
		key = KEY_Scan(0);

		if(key == KEY1_PRES) //KEY1按下,写入W25QXX
		{
			W25QXX_Write((u8*)TEXT_Buffer, FLASH_SIZE - 100, SIZE);     //从倒数第100个地址处开始,写入SIZE长度的数据
		}

		if(key == KEY0_PRES) //KEY0按下,读取字符串并显示
		{
              
				SPI_DMA_BufferRead(W25X_ReadData,FLASH_SIZE - 100,SIZE);//从倒数第100个地址处开始,读出SIZE个字节
		}

		i++;
		delay_ms(10);

		if(i == 20)
		{
			LED0 = !LED0; //提示系统正在运行
			i = 0;
		}
	}
}


