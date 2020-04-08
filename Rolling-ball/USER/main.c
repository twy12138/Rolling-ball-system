#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "usmart.h"  
#include "usart2.h"  
#include "timer.h" 
#include "ov2640.h" 
#include "dcmi.h" 
#include "string.h"
#include "pwm.h"
#include "pid.h"

//PD控制 最终调试整合 Made by twy



//ALIENTEK 探索者STM32F407开发板 实验35
//摄像头 实验 -库函数版本
//技术支持：www.openedv.com
//淘宝店铺：http://eboard.taobao.com  
//广州市星翼电子科技有限公司  
//作者：正点原子 @ALIENTEK

static float aimx=0;
static float aimy=0;

//JPEG尺寸支持列表
const u16 jpeg_img_size_tbl[][2]=
{
	176,144,	//QCIF
	160,120,	//QQVGA
	352,288,	//CIF
	320,240,	//QVGA
	640,480,	//VGA
	800,600,	//SVGA
	1024,768,	//XGA
	1280,1024,	//SXGA
	1600,1200,	//UXGA
}; 




void TIM3_Int_Init(u16 arr,u16 psc);
//RGB565测试
//RGB数据直接显示在LCD上面
void rgb565_test(void)
{ 
	u8 key;
	LCD_Clear(WHITE);
  POINT_COLOR=RED; 
	
  OV2640_ImageWin_Set((800-480)/2,(600-600)/2,480,600);
	OV2640_RGB565_Mode();	//RGB565模式
	My_DCMI_Init();			//DCMI配置
	DCMI_DMA_Init((u32)&LCD->LCD_RAM,1,DMA_MemoryDataSize_HalfWord,DMA_MemoryInc_Disable);//DCMI DMA配置  
 	OV2640_OutSize_Set(lcddev.width,600); 
	DCMI_Start(); 		//启动传输
	while(1)
	{ 
		key=KEY_Scan(0); 
		if(key)
		{ 
			DCMI_Stop(); //停止显示
			switch(key)
			{				    
				case KEY0_PRES:	//对比度设置
		
					break;
				case KEY1_PRES:	//饱和度Saturation
				
					break;
				case KEY2_PRES:	//特效设置				 
				
					break;
				case WKUP_PRES:		    
					
					break;
			}
			
			DCMI_Start();//重新开始传输
		} 
		delay_ms(10);		
	}    
} 


static u16 led0pwmval=1400;
static u16 led1pwmval=1300;

u16 rgb_buf[144][144];        
u16 gray;
u16 hang=0;
u8 X_MAX,Y_MAX=0;    //小球的坐标信息
u8 X_MAX_LSAT, X_MIN_LSAT, Y_MAX_LSAT, Y_MIN_LSAT=0;   //上一次小球坐标位置信息
u8 X,Y=0;      //小球的质心信息
u8 X_MIN,Y_MIN=180;
u16 x;
u16 y;

extern PID pid_x,pid_y;	
static u16 increase=0;
static u16 love=0;
static u16 nice=0;
static u16 specialfirst=0;
static u16 keyflag=0;
static u16 finally=0;
int main(void)
{ 


  u16 i,j;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);  //初始化延时函数
	uart_init(115200);		//初始化串口波特率为115200 
	usart2_init(42,115200);		//初始化串口2波特率为115200
	LED_Init();					//初始化LED 
 	LCD_Init();					//LCD初始化  
 	KEY_Init();					//按键初始化 
	TIM3_Int_Init(200-1,8400-1);//10Khz计数,1秒钟中断一次
	TIM14_PWM_Init((20000-1),84-1);
	TIM10_PWM_Init((20000-1),84-1);
	led0pwmval=1435;                    //1420    1620(下)   1220(上)
	TIM_SetCompare1(TIM14,led0pwmval);  //1200 1500 1800
	led1pwmval=1300;                    //1180  1380(上)  980(下)
  TIM_SetCompare1(TIM10,led1pwmval);  //1100 1450 1800

	
 	usmart_dev.init(84);		//初始化USMART
 	POINT_COLOR=BLUE;//设置字体为红色 
	LCD_Set_Window(0,0,200,200); 
	while(OV2640_Init())//初始化OV2640
	{
		LCD_ShowString(30,130,240,16,16,"OV2640 ERR");
		delay_ms(200);
	    LCD_Fill(30,130,239,170,WHITE);
		
		delay_ms(200);
	}
	LCD_ShowString(30,130,200,16,16,"OV2640 OK"); 
  
    
    OV2640_OutSize_Set(144,144); 
    OV2640_RGB565_Mode();	//RGB565模式
    My_DCMI_Init();			//DCMI配置
    DCMI_DMA_Init((u32)rgb_buf,sizeof(rgb_buf)/4,DMA_MemoryDataSize_HalfWord,DMA_MemoryInc_Enable);//DCMI DMA配置
    DCMI_Start(); 		//启动传输
		LCD_DrawLine(145,0,145,145);
	  LCD_DrawLine(0,145,145,145);
	LCD_ShowString(5,150,150,16,16,"X:");
	LCD_ShowString(35,150,150,16,16,"Y:");		
//	  OV2640_Auto_Exposure(2);
	
		PID_Init(&pid_x);
   	PID_Init(&pid_y);
	
    while(1)
    {
		//	increase++;
			
			
            hang=0;
            LCD_SetCursor(0,0);  
            LCD_WriteRAM_Prepare();		//开始写入GRAM
            for(i=0;i<144;i++)
            {
                for(j=0;j<144;j++)
                {
                    if(j==143)
                    {
                        hang++;
                        LCD_SetCursor(0,i+1);  
                        LCD_WriteRAM_Prepare();		//开始写入GRAM
                    }
                    gray=((rgb_buf[i][j]>>11)*19595+((rgb_buf[i][j]>>5)&0x3f)*38469 +(rgb_buf[i][j]&0x1f)*7472)>>16;
                    if(gray>=13)
                    {
											  

                        LCD->LCD_RAM=WHITE;
                    }
                    else
                    {
										if(i>8&&i<136&&j<160&&j>16)
												{
											    if(i>X_MAX) X_MAX=i;
										     	if(i<X_MIN) X_MIN=i;
											 
								 			
									      
											    if(j>Y_MAX) Y_MAX=j;
										     	if(j<Y_MIN) Y_MIN=j;
											 
												}
											
							
											 
                        LCD->LCD_RAM=BLACK;
                    }
                }
            }
					X_MAX_LSAT =	X_MAX;    
				  X_MIN_LSAT =	X_MIN;
				  Y_MAX_LSAT =	Y_MAX;
				  Y_MIN_LSAT =	Y_MIN;    
						
				  X_MAX=0;
				  X_MIN=180;
					Y_MAX=0;
				  Y_MIN=180;
						
					pid_y.Actual=(X_MAX_LSAT+X_MIN_LSAT)/2;
				  pid_x.Actual=(Y_MAX_LSAT+Y_MIN_LSAT)/2;		
						
						
					
	
			
						
						
    }
}


void positionone(void)
{
			  x = PID_Calculate_x(130.0)-2;
			
				y = PID_Calculate_y(13.0)-2;
		
		
				if((pid_y.Actual-13.0)>0)
				{
					led0pwmval+=y;					
				}
				
				if((pid_y.Actual-13.0)<0)
				{
					led0pwmval+=-1*y;		
				}
				
				if((pid_x.Actual-130.0)>0)
				{	
					led1pwmval+=x;
				}
				
				if((pid_x.Actual-130.0)<0)
				{
					led1pwmval+=-1*x;
				}


	
				if(led1pwmval>1366)
				{
					led1pwmval=1366;
				}

				if(led1pwmval<1248)                //TIM14 0 1400 TIM10 1 1300
				{
					led1pwmval=1248;
				}
				if(led0pwmval>=1460)
				{
					led0pwmval=1460;
				}
				if(led0pwmval<=1340)
				{
					led0pwmval=1340;
				}
				
				TIM_SetCompare1(TIM10,led1pwmval);//1300
				TIM_SetCompare1(TIM14,led0pwmval);//1435
				
				LCD_ShowxNum(20,150,x,3,16,0);

				LCD_ShowxNum(50,150,y,3,16,0);	


}

void positiontwo(void)
{
			  x = PID_Calculate_x(120.0)-2;
			
				y = PID_Calculate_y(67.0)-2;
		
		
				if((pid_y.Actual-67.0)>0)
				{
					led0pwmval+=y;					
				}
				
				if((pid_y.Actual-67.0)<0)
				{
					led0pwmval+=-1*y;		
				}
				
				if((pid_x.Actual-120.0)>0)
				{	
					led1pwmval+=x;
				}
				
				if((pid_x.Actual-120.0)<0)
				{
					led1pwmval+=-1*x;
				}


	
				if(led1pwmval>1331)
				{
					led1pwmval=1331;
				}

				if(led1pwmval<1243)                //TIM14 0 1400 TIM10 1 1300
				{
					led1pwmval=1243;
				}
				if(led0pwmval>=1455)
				{
					led0pwmval=1455;
				}
				if(led0pwmval<=1395)
				{
					led0pwmval=1395;
				}
				
				TIM_SetCompare1(TIM10,led1pwmval);//1300
				TIM_SetCompare1(TIM14,led0pwmval);//1435
				
				LCD_ShowxNum(20,150,x,3,16,0);

				LCD_ShowxNum(50,150,y,3,16,0);	


}


void positionthree(void)
{
			  x = PID_Calculate_x(135.0)-2;
			
				y = PID_Calculate_y(124.0)-2;
		
		
				if((pid_y.Actual-124.0)>0)
				{
					led0pwmval+=y;					
				}
				
				if((pid_y.Actual-124.0)<0)
				{
					led0pwmval+=-1*y;		
				}
				
				if((pid_x.Actual-135.0)>0)
				{	
					led1pwmval+=x;
				}
				
				if((pid_x.Actual-135.0)<0)
				{
					led1pwmval+=-1*x;
				}


	
				if(led1pwmval>1368)
				{
					led1pwmval=1368;
				}

				if(led1pwmval<1248)                //TIM14 0 1400 TIM10 1 1300
				{
					led1pwmval=1248;
				}
				if(led0pwmval>=1475)
				{
					led0pwmval=1475;
				}
				if(led0pwmval<=1405)
				{
					led0pwmval=1405;
				}
				
				TIM_SetCompare1(TIM10,led1pwmval);//1300
				TIM_SetCompare1(TIM14,led0pwmval);//1435
				
				LCD_ShowxNum(20,150,x,3,16,0);

				LCD_ShowxNum(50,150,y,3,16,0);	


}


void positionfour(void)
{
			  x = PID_Calculate_x(68.0)-2;
			
				y = PID_Calculate_y(13.0)-2;
		
		
				if((pid_y.Actual-13.0)>0)
				{
					led0pwmval+=y;					
				}
				
				if((pid_y.Actual-13.0)<0)
				{
					led0pwmval+=-1*y;		
				}
				
				if((pid_x.Actual-68.0)>0)
				{	
					led1pwmval+=x;
				}
				
				if((pid_x.Actual-68.0)<0)
				{
					led1pwmval+=-1*x;
				}


	
				if(led1pwmval>1330)
				{
					led1pwmval=1330;
				}

				if(led1pwmval<1209)                //TIM14 0 1400 TIM10 1 1300
				{
					led1pwmval=1209;
				}
				if(led0pwmval>=1460)
				{
					led0pwmval=1460;
				}
				if(led0pwmval<=1365)
				{
					led0pwmval=1365;
				}
				
				TIM_SetCompare1(TIM10,led1pwmval);//1300
				TIM_SetCompare1(TIM14,led0pwmval);//1435
				
				LCD_ShowxNum(20,150,x,3,16,0);

				LCD_ShowxNum(50,150,y,3,16,0);	


}




void positionfive(void)
{
			  x = PID_Calculate_x(63.0)-2;
			
				y = PID_Calculate_y(70.0)-2;
		
		
				if((pid_y.Actual-70.0)>0)
				{
					led0pwmval+=y;					
				}
				
				if((pid_y.Actual-70.0)<0)
				{
					led0pwmval+=-1*y;		
				}
				
				if((pid_x.Actual-63.0)>0)
				{	
					led1pwmval+=x;
				}
				
				if((pid_x.Actual-63.0)<0)
				{
					led1pwmval+=-1*x;
				}


	
				if(led1pwmval>1342)//1332
				{
					led1pwmval=1342;
				}

				if(led1pwmval<1224)              //1229
				{
					led1pwmval=1224;
				}
				if(led0pwmval>=1453)//1460
				{
					led0pwmval=1453;
				}
				if(led0pwmval<=1395)//1380
				{
					led0pwmval=1395;
				}
				
				TIM_SetCompare1(TIM10,led1pwmval);//1300
				TIM_SetCompare1(TIM14,led0pwmval);//1435
				
				LCD_ShowxNum(20,150,x,3,16,0);

				LCD_ShowxNum(50,150,y,3,16,0);	


}


void positionsix(void)
{
			  x = PID_Calculate_x(62.0)-2;
			
				y = PID_Calculate_y(120.0)-2;
		
		
				if((pid_y.Actual-120.0)>0)
				{
					led0pwmval+=y;					
				}
				
				if((pid_y.Actual-120.0)<0)
				{
					led0pwmval+=-1*y;		
				}
				
				if((pid_x.Actual-62.0)>0)
				{	
					led1pwmval+=x;
				}
				
				if((pid_x.Actual-62.0)<0)
				{
					led1pwmval+=-1*x;
				}


	
				if(led1pwmval>1321)//1316
				{
					led1pwmval=1321;
				}

				if(led1pwmval<1228)                
				{
					led1pwmval=1228;
				}
				if(led0pwmval>=1480)
				{
					led0pwmval=1480;
				}
				if(led0pwmval<=1400)//1370
				{
					led0pwmval=1400;
				}
				
				TIM_SetCompare1(TIM10,led1pwmval);//1300
				TIM_SetCompare1(TIM14,led0pwmval);//1435
				
				LCD_ShowxNum(20,150,x,3,16,0);

				LCD_ShowxNum(50,150,y,3,16,0);	


}



void positionseven(void)
{
			  x = PID_Calculate_x(25.0)-2;
			
				y = PID_Calculate_y(13.0)-2;
		
		
				if((pid_y.Actual-13.0)>0)
				{
					led0pwmval+=y;					
				}
				
				if((pid_y.Actual-13.0)<0)
				{
					led0pwmval+=-1*y;		
				}
				
				if((pid_x.Actual-25.0)>0)
				{	
					led1pwmval+=x;
				}
				
				if((pid_x.Actual-25.0)<0)
				{
					led1pwmval+=-1*x;
				}


	
				if(led1pwmval>1306)
				{
					led1pwmval=1306;
				}

				if(led1pwmval<1188)                //TIM14 0 1400 TIM10 1 1300
				{
					led1pwmval=1188;
				}
				if(led0pwmval>=1445)
				{
					led0pwmval=1445;
				}
				if(led0pwmval<=1355)
				{
					led0pwmval=1355;
				}
				
				TIM_SetCompare1(TIM10,led1pwmval);//1300
				TIM_SetCompare1(TIM14,led0pwmval);//1435
				
				LCD_ShowxNum(20,150,x,3,16,0);

				LCD_ShowxNum(50,150,y,3,16,0);	


}


void positioneight(void)
{
			  x = PID_Calculate_x(22.0)-2;
			
				y = PID_Calculate_y(59.0)-2;
		
		
				if((pid_y.Actual-59.0)>0)
				{
					led0pwmval+=y;					
				}
				
				if((pid_y.Actual-59.0)<0)
				{
					led0pwmval+=-1*y;		
				}
				
				if((pid_x.Actual-22.0)>0)
				{	
					led1pwmval+=x;
				}
				
				if((pid_x.Actual-22.0)<0)
				{
					led1pwmval+=-1*x;
				}


	
				if(led1pwmval>1306)
				{
					led1pwmval=1306;
				}

				if(led1pwmval<1158)                //TIM14 0 1400 TIM10 1 1300
				{
					led1pwmval=1158;
				}
				if(led0pwmval>=1445)
				{
					led0pwmval=1445;
				}
				if(led0pwmval<=1365)
				{
					led0pwmval=1365;
				}
				
				TIM_SetCompare1(TIM10,led1pwmval);//1300
				TIM_SetCompare1(TIM14,led0pwmval);//1435
				
				LCD_ShowxNum(20,150,x,3,16,0);

				LCD_ShowxNum(50,150,y,3,16,0);	


}



void positionnine(void)
{
			  x = PID_Calculate_x(20.0)-2;
			
				y = PID_Calculate_y(110.0)-2;
		
		
				if((pid_y.Actual-110.0)>0)
				{
					led0pwmval+=y;					
				}
				
				if((pid_y.Actual-110.0)<0)
				{
					led0pwmval+=-1*y;		
				}
				
				if((pid_x.Actual-20.0)>0)
				{	
					led1pwmval+=x;
				}
				
				if((pid_x.Actual-20.0)<0)
				{
					led1pwmval+=-1*x;
				}


	
				if(led1pwmval>1306)
				{
					led1pwmval=1306;
				}

				if(led1pwmval<1158)                //TIM14 0 1400 TIM10 1 1300
				{
				led1pwmval=1158;
				}
				if(led0pwmval>=1470)
				{
					led0pwmval=1470;
				}
				if(led0pwmval<=1370)
				{
					led0pwmval=1370;
				}
				
				TIM_SetCompare1(TIM10,led1pwmval);//1300
				TIM_SetCompare1(TIM14,led0pwmval);//1435
				
				LCD_ShowxNum(20,150,x,3,16,0);

				LCD_ShowxNum(50,150,y,3,16,0);	


}



//45中间
void specialone(void)
{
			  x = PID_Calculate_x(64.0)-2;
			
				y = PID_Calculate_y(40.0)-2;
		
		
				if((pid_y.Actual-40.0)>0)
				{
					led0pwmval+=y;					
				}
				
				if((pid_y.Actual-40.0)<0)
				{
					led0pwmval+=-1*y;		
				}
				
				if((pid_x.Actual-64.0)>0)
				{	
					led1pwmval+=x;
				}
				
				if((pid_x.Actual-64.0)<0)
				{
					led1pwmval+=-1*x;
				}


	
				if(led1pwmval>1342)//1332
				{
					led1pwmval=1342;
				}

				if(led1pwmval<1220)              //1229
				{
					led1pwmval=1220;
				}
				if(led0pwmval>=1448)//1460
				{
					led0pwmval=1448;
				}
				if(led0pwmval<=1380)//1380
				{
					led0pwmval=1380;
				}
				
				TIM_SetCompare1(TIM10,led1pwmval);//1300
				TIM_SetCompare1(TIM14,led0pwmval);//1435
				
				LCD_ShowxNum(20,150,x,3,16,0);

				LCD_ShowxNum(50,150,y,3,16,0);	


}

//58中间

void specialtwo(void)
{
			  x = PID_Calculate_x(51.0)-2;
			
				y = PID_Calculate_y(76.0)-2;
		
		
				if((pid_y.Actual-76.0)>0)
				{
					led0pwmval+=y;					
				}
				
				if((pid_y.Actual-76.0)<0)
				{
					led0pwmval+=-1*y;		
				}
				
				if((pid_x.Actual-51.0)>0)
				{	
					led1pwmval+=x;
				}
				
				if((pid_x.Actual-51.0)<0)
				{
					led1pwmval+=-1*x;
				}


	
				if(led1pwmval>1322)//1332
				{
					led1pwmval=1322;
				}

				if(led1pwmval<1224)              //1229
				{
					led1pwmval=1224;
				}
				if(led0pwmval>=1463)//1460
				{
					led0pwmval=1463;
				}
				if(led0pwmval<=1395)//1380
				{
					led0pwmval=1395;
				}
				
				TIM_SetCompare1(TIM10,led1pwmval);//1300
				TIM_SetCompare1(TIM14,led0pwmval);//1435
				
				LCD_ShowxNum(20,150,x,3,16,0);

				LCD_ShowxNum(50,150,y,3,16,0);	


}

//15中间


void specialthree(void)
{
			  x = PID_Calculate_x(94.0)-2;
			
				y = PID_Calculate_y(40.0)-2;
		
		
				if((pid_y.Actual-40.0)>0)
				{
					led0pwmval+=y;					
				}
				
				if((pid_y.Actual-40.0)<0)
				{
					led0pwmval+=-1*y;		
				}
				
				if((pid_x.Actual-94.0)>0)
				{	
					led1pwmval+=x;
				}
				
				if((pid_x.Actual-94.0)<0)
				{
					led1pwmval+=-1*x;
				}


	
				if(led1pwmval>1330)//1332
				{
					led1pwmval=1330;
				}

				if(led1pwmval<1224)              //1229
				{
					led1pwmval=1224;
				}
				if(led0pwmval>=1453)//1460
				{
					led0pwmval=1453;
				}
				if(led0pwmval<=1385)//1380
				{
					led0pwmval=1385;
				}
				
				TIM_SetCompare1(TIM10,led1pwmval);//1300
				TIM_SetCompare1(TIM14,led0pwmval);//1435
				
				LCD_ShowxNum(20,150,x,3,16,0);

				LCD_ShowxNum(50,150,y,3,16,0);	


}



//59中间

void specialfour(void)
{
			  x = PID_Calculate_x(44.0)-2;
			
				y = PID_Calculate_y(94.0)-2;
		
		
				if((pid_y.Actual-94.0)>0)
				{
					led0pwmval+=y;					
				}
				
				if((pid_y.Actual-94.0)<0)
				{
					led0pwmval+=-1*y;		
				}
				
				if((pid_x.Actual-44.0)>0)
				{	
					led1pwmval+=x;
				}
				
				if((pid_x.Actual-44.0)<0)
				{
					led1pwmval+=-1*x;
				}


	
				if(led1pwmval>1332)//1332
				{
					led1pwmval=1332;
				}

				if(led1pwmval<1234)              //1229
				{
					led1pwmval=1234;
				}
				if(led0pwmval>=1463)//1460
				{
					led0pwmval=1463;
				}
				if(led0pwmval<=1395)//1380
				{
					led0pwmval=1395;
				}
				
				TIM_SetCompare1(TIM10,led1pwmval);//1300
				TIM_SetCompare1(TIM14,led0pwmval);//1435
				
				LCD_ShowxNum(20,150,x,3,16,0);

				LCD_ShowxNum(50,150,y,3,16,0);	


}


//57中间
void specialfive(void)
{
			  x = PID_Calculate_x(50.0)-2;
			
				y = PID_Calculate_y(50.0)-2;
		
		
				if((pid_y.Actual-50.0)>0)
				{
					led0pwmval+=y;					
				}
				
				if((pid_y.Actual-50.0)<0)
				{
					led0pwmval+=-1*y;		
				}
				
				if((pid_x.Actual-50.0)>0)
				{	
					led1pwmval+=x;
				}
				
				if((pid_x.Actual-50.0)<0)
				{
					led1pwmval+=-1*x;
				}


	
				if(led1pwmval>1337)//1332
				{
					led1pwmval=1337;
				}

				if(led1pwmval<1219)              //1229
				{
					led1pwmval=1219;
				}
				if(led0pwmval>=1468)//1460
				{
					led0pwmval=1468;
				}
				if(led0pwmval<=1380)//1380
				{
					led0pwmval=1380;
				}
				
				TIM_SetCompare1(TIM10,led1pwmval);//1300
				TIM_SetCompare1(TIM14,led0pwmval);//1435
				
				LCD_ShowxNum(20,150,x,3,16,0);

				LCD_ShowxNum(50,150,y,3,16,0);	


}

//35中间


void specialsix(void)
{
			  x = PID_Calculate_x(94.0)-2;
			
				y = PID_Calculate_y(94.0)-2;
		
		
				if((pid_y.Actual-94.0)>0)
				{
					led0pwmval+=y;					
				}
				
				if((pid_y.Actual-94.0)<0)
				{
					led0pwmval+=-1*y;		
				}
				
				if((pid_x.Actual-94.0)>0)
				{	
					led1pwmval+=x;
				}
				
				if((pid_x.Actual-94.0)<0)
				{
					led1pwmval+=-1*x;
				}


	
				if(led1pwmval>1342)//1332
				{
					led1pwmval=1342;
				}

				if(led1pwmval<1254)              //1229
				{
					led1pwmval=1254;
				}
				if(led0pwmval>=1473)//1460
				{
					led0pwmval=1473;
				}
				if(led0pwmval<=1415)//1380
				{
					led0pwmval=1415;
				}
				
				TIM_SetCompare1(TIM10,led1pwmval);//1300
				TIM_SetCompare1(TIM14,led0pwmval);//1435
				
				LCD_ShowxNum(20,150,x,3,16,0);

				LCD_ShowxNum(50,150,y,3,16,0);	


}


//56中间

void specialseven(void)
{
			  x = PID_Calculate_x(63.0)-2;
			
				y = PID_Calculate_y(83.0)-2;
		
		
				if((pid_y.Actual-83.0)>0)
				{
					led0pwmval+=y;					
				}
				
				if((pid_y.Actual-83.0)<0)
				{
					led0pwmval+=-1*y;		
				}
				
				if((pid_x.Actual-63.0)>0)
				{	
					led1pwmval+=x;
				}
				
				if((pid_x.Actual-63.0)<0)
				{
					led1pwmval+=-1*x;
				}


	
				if(led1pwmval>1342)//1332
				{
					led1pwmval=1342;
				}

				if(led1pwmval<1224)              //1229
				{
					led1pwmval=1224;
				}
				if(led0pwmval>=1453)//1460
				{
					led0pwmval=1453;
				}
				if(led0pwmval<=1395)//1380
				{
					led0pwmval=1395;
				}
				
				TIM_SetCompare1(TIM10,led1pwmval);//1300
				TIM_SetCompare1(TIM14,led0pwmval);//1435
				
				LCD_ShowxNum(20,150,x,3,16,0);

				LCD_ShowxNum(50,150,y,3,16,0);	


}

//25中间
void specialeight(void)
{
			  x = PID_Calculate_x(86.0)-2;
			
				y = PID_Calculate_y(72.0)-2;
		
		
				if((pid_y.Actual-72.0)>0)
				{
					led0pwmval+=y;					
				}
				
				if((pid_y.Actual-72.0)<0)
				{
					led0pwmval+=-1*y;		
				}
				
				if((pid_x.Actual-86.0)>0)
				{	
					led1pwmval+=x;
				}
				
				if((pid_x.Actual-86.0)<0)
				{
					led1pwmval+=-1*x;
				}


	
				if(led1pwmval>1320)//1332
				{
					led1pwmval=1320;
				}

				if(led1pwmval<1254)              //1229
				{
					led1pwmval=1254;
				}
				if(led0pwmval>=1466)//1460
				{
					led0pwmval=1466;
				}
				if(led0pwmval<=1395)//1380
				{
					led0pwmval=1395;
				}
				
				TIM_SetCompare1(TIM10,led1pwmval);//1300
				TIM_SetCompare1(TIM14,led0pwmval);//1435
				
				LCD_ShowxNum(20,150,x,3,16,0);

				LCD_ShowxNum(50,150,y,3,16,0);	


}

/**************************************************基础部分************************************************************/

//定点区域2
void question_one()
{

positiontwo();

}


//区域1到区域5
void question_two()
{

	increase++;
	if(increase<=300)
	{
	positionone();
	}
	else if(increase>=300&&increase<=410)
	{
		
				specialthree();
	
	}
	else if(increase>=411)
	{
		increase=411;
	positionfive();
	}


}

//区域1到区域4再到区域5
void question_three()
{


	love++;
	if(love<=300)
	{
	positionone();
	}
	else if(love>=351&&love<=800)
	{
	positionfour();
	}

	else if(love>=801)
	{
		positionfive();
		love=801;
	}



}
	

//区域1到区域9
void question_four()
{

	nice++;
	if(nice<=250)
	{
	positionone();
	}
	else if(nice>=250&&nice<=500)
	{
	specialthree();
	}

	else if(nice>=501&&nice<=750)
	{
		specialone();
		
	}
	else if(nice>=751&&nice<=1000)
	{
		specialfive();
		
	}
	else if(nice>=1001&&nice<=1250)
	{
		specialtwo();
		
	}
		else if(nice>=1251&&nice<=1500)
	{
		specialfour();

	}
			else if(nice>=1501)
	{
		positionnine();
		nice=1501;
	}


}


/*****************************************************发挥部分*********************************************************/
//区域1到区域2到区域6到区域9

void developmentpartfirst()
{

	specialfirst++;
	if(specialfirst<=300)
	{
	positionone();
	}
	else if(specialfirst>=301&&specialfirst<=600)
	{
	positiontwo();
	}

	else if(specialfirst>=601&&specialfirst<=900)
	{
		specialsix();

	}		
	else if(specialfirst>=901&&specialfirst<=1200)
	{
		positionsix();

	}
		else if(specialfirst>=1201)
	{
		positionnine();
		specialfirst=1201;
	}


}

//按键控制自行设置区域ABCD这里区域设置1为A 2为B 5为C 6为D

void developmentpartsecond()
{
	 u8 key;  
	
			key=KEY_Scan(0);		//得到键值
	   	if(key)
	{		
				switch(key)
			{				 
				case WKUP_PRES:	
					keyflag=1;
					break;
				case KEY0_PRES:	
					keyflag=2;
					break;
				case KEY1_PRES:	 
					keyflag=3;
					break;
				case KEY2_PRES:	
					keyflag=4;
					break;
			}
		}else delay_ms(10); 
		
	//	LCD_ShowxNum(20,150,flag,3,16,0);
		
		if(keyflag==0)
		{
		positionone();
		}
		else if(keyflag==1)
		{
		positionone();
		}
		else if(keyflag==2)
		{
		positiontwo();
		}
		else if(keyflag==3)
		{
		positionthree();
		}
		else if(keyflag==4)
		{
		positionsix();
		}
		

}

//控制4区域出发绕5区域三圈后进入区域9

void finalquestion()
{
		finally++;
		
		if(finally<=399)
		{
			positionfour();
		}
		else if(finally>=400&&finally<=600)
		{
			specialone();
		}
		else if(finally>=601&&finally<=800)
		{
		  specialfive();
		}
		else if(finally>=801&&finally<=1000)
		{
		  specialtwo();
		}
		else if(finally>=1001&&finally<=1200)
		{
		  specialfour();
		}
		else if(finally>=1201&&finally<=1400)
		{
		  specialseven();
		}
		else if(finally>=1401&&finally<=1600)//specialeight
		{
		  specialsix();
		}
		else if(finally>=1601&&finally<=1800)
		{
		  specialeight();
		}
		else if(finally>=1801&&finally<=2000)
		{
		  specialthree();
		}
		else if(finally>=2001&&finally<=2200)
		{
			specialone();
		}
		else if(finally>=2201&&finally<=2400)
		{
		  specialfive();
		}
		else if(finally>=2401&&finally<=2600)
		{
		  specialtwo();
		}
		else if(finally>=2601&&finally<=2800)
		{
		  specialfour();
		}
		else if(finally>=2801&&finally<=3000)
		{
		  specialseven();
		}
		else if(finally>=3001&&finally<=3200)//specialeight
		{
		  specialsix();
		}
		else if(finally>=3201&&finally<=3400)
		{
		  specialeight();
		}
		else if(finally>=3401&&finally<=3600)
		{
		  specialthree();
		}
		else if(finally>=3601&&finally<=3800)
		{
			specialone();
		}
		else if(finally>=3801&&finally<=4000)
		{
		  specialfive();
		}
		else if(finally>=4001&&finally<=4200)
		{
		  specialtwo();
		}
		else if(finally>=4201&&finally<=4400)
		{
		  specialfour();
		}
		else if(finally>=4401&&finally<=4600)
		{
		  specialseven();
		}
		else if(finally>=4601&&finally<=4800)//specialeight
		{
		  specialsix();
		}
		else if(finally>=4801&&finally<=5000)
		{
		  specialeight();
		}
		else if(finally>=5001&&finally<=5200)
		{
		  specialthree();
		}
			else if(finally>=5201&&finally<=5400)
		{
			specialone();
		}
		else if(finally>=5401&&finally<=5600)
		{
		  specialfive();
		}
		else if(finally>=5601&&finally<=5800)
		{
		  specialtwo();
		}
		else if(finally>=5801&&finally<=6000)
		{
		  specialfour();
		}
		else if(finally>=6001)
		{
				positionnine();
			  finally=6001;
		}
		
	
	
	
}






void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
	{
		

			

	developmentpartsecond();
		//specialeight();
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位
}








