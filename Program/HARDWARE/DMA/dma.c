#include "dma.h"
#include "w25qxx.h"
#include "spi.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK精英STM32开发板
//DMA 代码
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/8
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////

DMA_InitTypeDef DMA_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
u16 DMA1_MEM_LEN;//保存DMA每次数据传送的长度
SPI_InitTypeDef  SPI_InitStructure;
unsigned char SPI2_TX_Buf = 0x55;
unsigned char SPI2_RX_Buf[100];

void DMA_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	/*开启时钟*/
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);



	DMA_DeInit(DMA1_Channel4);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI2->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ReceiveBuff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 100;//这个寄存器是代表着接收满几个数就进入DMA的中断
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);


	DMA_DeInit(DMA1_Channel5);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI2->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&SPI2_TX_Buf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = 256;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);

	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);



}


void SPI_DMA_BufferRead(u8 command, u32 ReadAddr,u8 len)
{
	DMA_Cmd(DMA1_Channel4, DISABLE);
	DMA1_Channel4->CNDTR = len;//CNDTR寄存器是等待发送的数据个数


	/* Select the FLASH: Chip Select low */
	W25QXX_CS = 0;

	/* Send "Read from Memory " instruction */
	//SPI_FLASH_SendByte(W25X_ReadData);
	SPI_FLASH_SendByte(command);

	/* Send ReadAddr high nibble address byte to read from */
	SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
	/* Send ReadAddr medium nibble address byte to read from */
	SPI_FLASH_SendByte((ReadAddr & 0xFF00) >> 8);
	/* Send ReadAddr low nibble address byte to read from */
	SPI2_ReadWriteByte(ReadAddr & 0xFF);//地址的最后一个字节使用发送接收函数，将SPI缓冲区残留数据接收回来


	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);

	DMA_Cmd (DMA1_Channel4, ENABLE);	
	DMA_Cmd (DMA1_Channel5, ENABLE);




	//while(DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);

	//  /* Deselect the FLASH: Chip Select high */
	//  W25QXX_CS=1;
}

void DMA1_Channel4_IRQHandler(void)
{
	if(DMA_GetFlagStatus(DMA1_IT_TC4) == SET)
	{
		W25QXX_CS = 1;
		DMA_Cmd(DMA1_Channel4, DISABLE);
		DMA_ClearITPendingBit(DMA1_IT_TC4);
		DMA_Cmd(DMA1_Channel5, DISABLE);
		DMA_ClearITPendingBit(DMA1_IT_TC5);				
	}
}


