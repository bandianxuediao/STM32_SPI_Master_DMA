#ifndef __DMA_H
#define __DMA_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//DMA ����
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/8
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////

#define SIZE sizeof(TEXT_Buffer)
#define SEND_BUF_SIZE 8200  //�������ݳ���,��õ���sizeof(TEXT_TO_SEND)+2��������.
extern const u8 TEXT_Buffer[];
extern void Wait_Receive_Len(u8 len);

void SPI_DMA_BufferRead(u8 command, u32 ReadAddr,u8 len);

extern u8 ReceiveBuff[100]; //�������ݻ�����


void DMA_Config(void);

void MYDMA_Config(u32 cpar, u32 cmar, u16 cndtr); //����DMA1_CHx

void MYDMA_Enable(void);//ʹ��DMA1_CHx

#endif




