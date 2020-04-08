#ifndef __PID_H
#define __PID_H	 
#include "sys.h"
#include "stdlib.h"


typedef struct
{
	float Set;      //定义设定值
	float Actual;   //定义实际值
	float err;      //定义偏差值
	float err_last; //定义上一个偏差值
	float Kp,Ki,Kd; //定义比例、积分、微分系数
	float voltage;  //计算值
	float integral; //累加值
}PID;

void PID_Init(PID *p);
u16 PID_Calculate_x(float speed);
u16 PID_Calculate_y(float speed);
		 				    
#endif
