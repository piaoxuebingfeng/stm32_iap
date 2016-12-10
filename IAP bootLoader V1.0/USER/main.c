#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "key.h"
#include "stmflash.h"   
#include "iap.h"	 
//FLASH分区: 0X0800 0000~0x0800 8000,给IAP使用,共32K字节,FLASH APP还有256-32=224KB可用.
// SRAM分区: 0X2000 1000处开始,用于存放SRAM IAP代码,共44K字节可用,用户可以自行对44K空间进行ROM和RAM区的分配
//           特别注意，SRAM APP的ROM占用量不能大于41K字节，因为本例程最大是一次接收41K字节，不能超过这个限制。

int main(void)
 { 
	u16 t;
	u8 boot_flag=0;
	u8 update_flag=0;
	u8 key;
	u16 oldcount=0;	//老的串口接收数据值
	u16 applenth=0;	//接收到的app代码长度
	u8 clearflag=0; 
  NVIC_Configuration();
	delay_init();	    	 //延时函数初始化	  
	uart_init(115200);	 	//串口初始化为9600
	LED_Init();		  		//初始化与LED连接的硬件接口
 	KEY_Init();				//按键初始化 	
	printf("IAP boot app\r\n");                                                                                                             
	printf("boot_flag:%d\r\n",boot_flag);
	printf("update_flag:%d\r\n",update_flag);
	while(1)
	{
	 	if(USART_RX_CNT)
		{
			if(oldcount==USART_RX_CNT)//新周期内,没有收到任何数据,认为本次数据接收完成.
			{
				applenth=USART_RX_CNT;
				oldcount=0;
				USART_RX_CNT=0;
				update_flag=1;
				printf("用户程序接收完成!\r\n");
				
				printf("代码长度:%dBytes\r\n",applenth);
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
		if( (key==KEY0_PRES) || (update_flag==1) )			//WK_UP按键按下
		{
			if(applenth)
			{
				printf("开始更新固件...\r\n");
 				if(((*(vu32*)(0X20001000+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
				{	 
					iap_write_appbin(FLASH_APP1_ADDR,USART_RX_BUF,applenth);//更新FLASH代码
					printf("固件更新完成!\r\n");	
				}else 
				{	   
					printf("非FLASH应用程序!\r\n");
				}
 			}else 
			{
				printf("没有可以更新的固件!\r\n");
			}
			clearflag=7;//标志更新了显示,并且设置7*300ms后清除显示	
			update_flag=0;
			boot_flag=1;
		} 
		if( (key==KEY1_PRES)||(boot_flag==1))
		{
			printf("开始执行FLASH用户代码!!\r\n");
			printf("\r\n");
			if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
			{	 
				iap_load_app(FLASH_APP1_ADDR);//执行FLASH APP代码
			}else 
			{
				printf("非FLASH应用程序,无法执行!\r\n");	   
			}									 
			clearflag=7;//标志更新了显示,并且设置7*300ms后清除显示	  
		}
//		if(key==KEY0_PRES)
//		{
//			printf("开始执行SRAM用户代码!!\r\n");
//			if(((*(vu32*)(0X20001000+4))&0xFF000000)==0x20000000)//判断是否为0X20XXXXXX.
//			{	 
//				iap_load_app(0X20001000);//SRAM地址
//			}else 
//			{
//				printf("非SRAM应用程序,无法执行!\r\n");
//			}									 
//			clearflag=7;//标志更新了显示,并且设置7*300ms后清除显示	 
//		}				   
		 
	}   	   
}












