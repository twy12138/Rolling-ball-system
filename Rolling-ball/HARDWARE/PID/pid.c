#include "pid.h"

PID pid_x,pid_y;
void PID_Init(PID *p)
{
	(*p).Set = 0.0;
	(*p).Actual = 0.0;
	(*p).err = 0.0;
	(*p).err_last = 0.0;
	(*p).voltage = 0.0;
	(*p).integral = 0.0;
	(*p).Kp = 0.111;
	(*p).Ki = 0;
	(*p).Kd = 54;
}


u16 PID_Calculate_x(float speed)
{
	u8 index;
	
	pid_x.Set = speed;
	pid_x.err = abs(pid_x.Set-pid_x.Actual);
	
	if(abs(pid_x.err)>72) 
	{
		index=0.0;
	}
	else if(abs(pid_x.err)<70)
	{
		index=1.0;
		pid_x.integral+=pid_x.err;
	}
	else
	{
		//index=1.0;
		index=(72-abs(pid_x.err))/12;
		pid_x.integral+=pid_x.err;
	}
	
	pid_x.voltage = pid_x.Kp*pid_x.err+index*pid_x.Ki*pid_x.integral+pid_x.Kd*(pid_x.err-pid_x.err_last);
	pid_x.err_last = pid_x.err;
	
	//pid_x.Actual = pid_x.voltage*1.0;
	return (u16)pid_x.voltage;
}

u16 PID_Calculate_y(float speed)
{
	u8 index;
	
	pid_y.Set = speed;
	pid_y.err = abs(pid_y.Set-pid_y.Actual);
	
	if(abs(pid_y.err)>72) 
	{
		index=0.0;
	}
	else if(abs(pid_y.err)<70)
	{
		index=1.0;
		pid_y.integral+=pid_y.err;
	}
	else
	{
		index=(72-abs(pid_y.err))/12;
		pid_y.integral+=pid_y.err;
	}
	
	pid_y.voltage = pid_y.Kp*pid_y.err+index*pid_y.Ki*pid_y.integral+pid_y.Kd*(pid_y.err-pid_y.err_last);
	pid_y.err_last = pid_y.err;
	
	//pid_y.Actual = pid_y.voltage*1.0;
	return (u16)pid_y.voltage;
}









