#include "dma.h"
#include "w25qxx.h"
#include "spi.h"
#include "delay.h"


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

	DMA_DeInit(DMA1_Channel4);   //查询STM32参考手册，V15以上版本  DMA映射表内SPI2_RX对应DMA1_Channel4  20180720  LYD
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI2->DR;//告诉DMA通道要从哪里搬运数据
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ReceiveBuff;//告诉DMA通道把搬过来的数据存放到哪儿  20180720  LYD
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //搬运方向是从外设到buffer
	DMA_InitStructure.DMA_BufferSize = 100;//这个寄存器是代表着接收满几个数就进入DMA的中断  接收倒计数   20180720  LYD
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//普通模式，还有一个循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;//优先级设置为最高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);

//虽然只用DMA接收，但是在SPI主模式下必须开启发送才能够正常接收--详见网站相关文章  www.wenhemei.com     github.com\bandiandaxiang
	DMA_DeInit(DMA1_Channel5);  //查询STM32参考手册，V15以上版本  DMA映射表内SPI2_TX对应DMA1_Channel5
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

	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);//配置DMA_Channel4的中断
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);



}

//使用DMA从Flash读取数据
void SPI_DMA_BufferRead(u8 command, u32 ReadAddr,u8 len)
{
	DMA_Cmd(DMA1_Channel4, DISABLE);//先禁用DMA，发完读取命令和读取地址之后再开启DMA
	DMA1_Channel4->CNDTR = len;//CNDTR寄存器是数据个数   这里是根据传过来的真实数据确定要读取的数据个数，数据个数达到的时候会进入中断   20180720 LYD


	/* Flash片选拉低，*/
	W25QXX_CS = 0;

	/* 发送读指令 */
	//SPI_FLASH_SendByte(W25X_ReadData);
	SPI_FLASH_SendByte(command);

	/* 发送地址高字节 */
	SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
	/* 发送地址中字节 */
	SPI_FLASH_SendByte((ReadAddr & 0xFF00) >> 8);
	/* 发送地址低字节 */
	SPI2_ReadWriteByte(ReadAddr & 0xFF);//地址的最后一个字节需要使用发送接收函数，将SPI缓冲区残留数据接收回来   20180725  LYD


	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);
//接收和发送的DMA都一定要打开			20180725   LYD
	DMA_Cmd (DMA1_Channel4, ENABLE);	
	DMA_Cmd (DMA1_Channel5, ENABLE);


	//while(DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);
	//  /* Deselect the FLASH: Chip Select high */
	//  W25QXX_CS=1;
}

//接收计数触发进入DMA接收中断
void DMA1_Channel4_IRQHandler(void)
{
	if(DMA_GetFlagStatus(DMA1_IT_TC4) == SET)
	{
		W25QXX_CS = 1;//拉高Flash片选信号，使从机停止发送数据
		DMA_Cmd(DMA1_Channel4, DISABLE);//禁用接收
		DMA_ClearITPendingBit(DMA1_IT_TC4);
		DMA_Cmd(DMA1_Channel5, DISABLE);//禁用发送
		DMA_ClearITPendingBit(DMA1_IT_TC5);				
	}
}


