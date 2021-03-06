#include "pwm.h"

u16 comp;

extern double du;
void TIM14_PWM_Init(u32 arr,u32 psc)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//(普通输出模式)
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//shang拉
	GPIO_Init(GPIOF, &GPIO_InitStructure);//初始化GPIO
	
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource9,GPIO_AF_TIM14);//GPIOF9复用映射到TIM14，作为TIM14它的输出引脚；
	
	TIM_TimeBaseInitStructure.TIM_Period = arr;//预装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;//定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数
	TIM_TimeBaseInitStructure.TIM_ClockDivision =TIM_CKD_DIV1;
	
	TIM_TimeBaseInit(TIM14,&TIM_TimeBaseInitStructure);
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;//模式1
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;//比较输出极性(有效的电平)
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//输出使能
//	TIM_OCInitStructure.TIM_Pulse = 100;
	
	TIM_OC1Init(TIM14,&TIM_OCInitStructure);//TIM14通道1初始化函数
	
	TIM_OC1PreloadConfig(TIM14, TIM_OCPreload_Enable);  //使能TIM14在CCR1上的预装载寄存器
 
	TIM_ARRPreloadConfig(TIM14,ENABLE);//ARPE使能预装载
	TIM_Cmd(TIM14,ENABLE);  //使能TIM14
	
}

void TIM10_PWM_Init(u32 arr,u32 psc)
{	 


GPIO_InitTypeDef GPIO_InitStructure;
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;

RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10,ENABLE);  	
RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE); 

GPIO_PinAFConfig(GPIOF,GPIO_PinSource6,GPIO_AF_TIM10); 

GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;         
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;      
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     
GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       
GPIO_Init(GPIOF,&GPIO_InitStructure);             
 
TIM_TimeBaseStructure.TIM_Prescaler=psc;  
TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; 
TIM_TimeBaseStructure.TIM_Period=arr;  
TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 

TIM_TimeBaseInit(TIM10,&TIM_TimeBaseStructure);

 
TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 
TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 
TIM_OC1Init(TIM10, &TIM_OCInitStructure);  

TIM_OC1PreloadConfig(TIM10, TIM_OCPreload_Enable);  
 
  TIM_ARRPreloadConfig(TIM10,ENABLE);

TIM_Cmd(TIM10, ENABLE);  
 
 
}

