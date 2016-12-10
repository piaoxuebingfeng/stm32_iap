#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "key.h"
#include "stmflash.h"   
#include "iap.h"	 
//FLASH����: 0X0800 0000~0x0800 8000,��IAPʹ��,��32K�ֽ�,FLASH APP����256-32=224KB����.
// SRAM����: 0X2000 1000����ʼ,���ڴ��SRAM IAP����,��44K�ֽڿ���,�û��������ж�44K�ռ����ROM��RAM���ķ���
//           �ر�ע�⣬SRAM APP��ROMռ�������ܴ���41K�ֽڣ���Ϊ�����������һ�ν���41K�ֽڣ����ܳ���������ơ�

int main(void)
 { 
	u16 t;
	u8 boot_flag=0;
	u8 update_flag=0;
	u8 key;
	u16 oldcount=0;	//�ϵĴ��ڽ�������ֵ
	u16 applenth=0;	//���յ���app���볤��
	u8 clearflag=0; 
  NVIC_Configuration();
	delay_init();	    	 //��ʱ������ʼ��	  
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ9600
	LED_Init();		  		//��ʼ����LED���ӵ�Ӳ���ӿ�
 	KEY_Init();				//������ʼ�� 	
	printf("IAP boot app\r\n");                                                                                                             
	printf("boot_flag:%d\r\n",boot_flag);
	printf("update_flag:%d\r\n",update_flag);
	while(1)
	{
	 	if(USART_RX_CNT)
		{
			if(oldcount==USART_RX_CNT)//��������,û���յ��κ�����,��Ϊ�������ݽ������.
			{
				applenth=USART_RX_CNT;
				oldcount=0;
				USART_RX_CNT=0;
				update_flag=1;
				printf("�û�����������!\r\n");
				
				printf("���볤��:%dBytes\r\n",applenth);
			}else oldcount=USART_RX_CNT;			
		}
		t++;
		delay_ms(10);
		if(t%30 == 0)
		{
			LED0=!LED0;
			printf("t:%d\r\n",t);
			if(clearflag)
			{
				clearflag--;
			}
		}	
		else if(t>1000)
		{
			t=0;
			boot_flag=1;
		}
		key=KEY_Scan(0);
		if( (key==KEY0_PRES) || (update_flag==1) )			//WK_UP��������
		{
			if(applenth)
			{
				printf("��ʼ���¹̼�...\r\n");
 				if(((*(vu32*)(0X20001000+4))&0xFF000000)==0x08000000)//�ж��Ƿ�Ϊ0X08XXXXXX.
				{	 
					iap_write_appbin(FLASH_APP1_ADDR,USART_RX_BUF,applenth);//����FLASH����
					printf("�̼��������!\r\n");	
				}else 
				{	   
					printf("��FLASHӦ�ó���!\r\n");
				}
 			}else 
			{
				printf("û�п��Ը��µĹ̼�!\r\n");
			}
			clearflag=7;//��־��������ʾ,��������7*300ms�������ʾ	
			update_flag=0;
			boot_flag=1;
		} 
		if( (key==KEY1_PRES)||(boot_flag==1))
		{
			printf("��ʼִ��FLASH�û�����!!\r\n");
			printf("\r\n");
			if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//�ж��Ƿ�Ϊ0X08XXXXXX.
			{	 
				iap_load_app(FLASH_APP1_ADDR);//ִ��FLASH APP����
			}else 
			{
				printf("��FLASHӦ�ó���,�޷�ִ��!\r\n");	   
			}									 
			clearflag=7;//��־��������ʾ,��������7*300ms�������ʾ	  
		}
//		if(key==KEY0_PRES)
//		{
//			printf("��ʼִ��SRAM�û�����!!\r\n");
//			if(((*(vu32*)(0X20001000+4))&0xFF000000)==0x20000000)//�ж��Ƿ�Ϊ0X20XXXXXX.
//			{	 
//				iap_load_app(0X20001000);//SRAM��ַ
//			}else 
//			{
//				printf("��SRAMӦ�ó���,�޷�ִ��!\r\n");
//			}									 
//			clearflag=7;//��־��������ʾ,��������7*300ms�������ʾ	 
//		}				   
		 
	}   	   
}












