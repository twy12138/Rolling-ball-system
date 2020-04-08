#ifndef __PID_H
#define __PID_H	 
#include "sys.h"
#include "stdlib.h"


typedef struct
{
	float Set;      //�����趨ֵ
	float Actual;   //����ʵ��ֵ
	float err;      //����ƫ��ֵ
	float err_last; //������һ��ƫ��ֵ
	float Kp,Ki,Kd; //������������֡�΢��ϵ��
	float voltage;  //����ֵ
	float integral; //�ۼ�ֵ
}PID;

void PID_Init(PID *p);
u16 PID_Calculate_x(float speed);
u16 PID_Calculate_y(float speed);
		 				    
#endif
