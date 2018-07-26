#include "dma.h"
#include "w25qxx.h"
#include "spi.h"
#include "delay.h"


DMA_InitTypeDef DMA_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
u16 DMA1_MEM_LEN;//����DMAÿ�����ݴ��͵ĳ���
SPI_InitTypeDef  SPI_InitStructure;
unsigned char SPI2_TX_Buf = 0x55;
unsigned char SPI2_RX_Buf[100];

void DMA_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	/*����ʱ��*/
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	DMA_DeInit(DMA1_Channel4);   //��ѯSTM32�ο��ֲᣬV15���ϰ汾  DMAӳ�����SPI2_RX��ӦDMA1_Channel4  20180720  LYD
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI2->DR;//����DMAͨ��Ҫ�������������
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ReceiveBuff;//����DMAͨ���Ѱ���������ݴ�ŵ��Ķ�  20180720  LYD
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //���˷����Ǵ����赽buffer
	DMA_InitStructure.DMA_BufferSize = 100;//����Ĵ����Ǵ����Ž������������ͽ���DMA���ж�  ���յ�����   20180720  LYD
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//��ͨģʽ������һ��ѭ��ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;//���ȼ�����Ϊ���
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);

//��Ȼֻ��DMA���գ�������SPI��ģʽ�±��뿪�����Ͳ��ܹ���������--�����վ�������  www.wenhemei.com     github.com\bandiandaxiang
	DMA_DeInit(DMA1_Channel5);  //��ѯSTM32�ο��ֲᣬV15���ϰ汾  DMAӳ�����SPI2_TX��ӦDMA1_Channel5
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

	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);//����DMA_Channel4���ж�
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);



}

//ʹ��DMA��Flash��ȡ����
void SPI_DMA_BufferRead(u8 command, u32 ReadAddr,u8 len)
{
	DMA_Cmd(DMA1_Channel4, DISABLE);//�Ƚ���DMA�������ȡ����Ͷ�ȡ��ַ֮���ٿ���DMA
	DMA1_Channel4->CNDTR = len;//CNDTR�Ĵ��������ݸ���   �����Ǹ��ݴ���������ʵ����ȷ��Ҫ��ȡ�����ݸ��������ݸ����ﵽ��ʱ�������ж�   20180720 LYD


	/* FlashƬѡ���ͣ�*/
	W25QXX_CS = 0;

	/* ���Ͷ�ָ�� */
	//SPI_FLASH_SendByte(W25X_ReadData);
	SPI_FLASH_SendByte(command);

	/* ���͵�ַ���ֽ� */
	SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
	/* ���͵�ַ���ֽ� */
	SPI_FLASH_SendByte((ReadAddr & 0xFF00) >> 8);
	/* ���͵�ַ���ֽ� */
	SPI2_ReadWriteByte(ReadAddr & 0xFF);//��ַ�����һ���ֽ���Ҫʹ�÷��ͽ��պ�������SPI�������������ݽ��ջ���   20180725  LYD


	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);
//���պͷ��͵�DMA��һ��Ҫ��			20180725   LYD
	DMA_Cmd (DMA1_Channel4, ENABLE);	
	DMA_Cmd (DMA1_Channel5, ENABLE);


	//while(DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);
	//  /* Deselect the FLASH: Chip Select high */
	//  W25QXX_CS=1;
}

//���ռ�����������DMA�����ж�
void DMA1_Channel4_IRQHandler(void)
{
	if(DMA_GetFlagStatus(DMA1_IT_TC4) == SET)
	{
		W25QXX_CS = 1;//����FlashƬѡ�źţ�ʹ�ӻ�ֹͣ��������
		DMA_Cmd(DMA1_Channel4, DISABLE);//���ý���
		DMA_ClearITPendingBit(DMA1_IT_TC4);
		DMA_Cmd(DMA1_Channel5, DISABLE);//���÷���
		DMA_ClearITPendingBit(DMA1_IT_TC5);				
	}
}


