/*  <The 14th National University Students Intelligent Car Race.>
 *  Copyright (C) <2019>  < github.com:He0L1w  NJUST >
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "system.h"



static void car_direction_control(void);
static void car_direction_control_arcman(void);
static inline void car_direction_control_inductance(void);
static void car_direction_control_circle(void);
static void car_direction_barrier_control(void);


const car_device_t car = {
    .direction_control = car_direction_control,
    .speed_caculate = car_speed_calculate,
};



/* 全局方向控制函数 */
static void car_direction_control(void)
{
  /* 路障，纯开环控制 */
  if ( status.barrier == 1 )
  {
    car_direction_barrier_control();    
  }
  else
  {
    /* 电磁判断、摄像头运行的圆环方向控制 */
    /* adc_circle_check 函数控制此分支入口 */
    if ( adc_roaddata.status > CircleConditon )   /* 圆环开关操作条件满足， */
    {
      car_direction_control_circle();
    }  
    
    /* img.roadtype进行断路检查 或者 电磁偏离过大检查 */
    if ( status.sensor ==  Camera)  
      car_direction_control_arcman();
    else /* 电磁模式运行 */
    {   
      car_direction_control_inductance();  /* 电磁方向控制 */
      img.ops->adc_roadcheck();            /* 道路检查，切换摄像头 */
    }    
  }
  
}


/* 基于数字图像计算阿克曼半径的方向控制 */
static void car_direction_control_arcman(void)
{
  char txt[16];
  double R;
  double arc_err,ud;
  static double arc_err1 = 0;
  static double ud1 = 0;
  uint16_t servo_pwm;
  point_t temp;
  R = calculate_Ackman_R(img.cal_ops->transform(71,midline[71]));
  
  /*
  将Ackman_R和速度建立对应关系
  直线速度最大值
  弯道速度最大值
  */
  arc_err = 980*R;
  ud = 0.4*0.4*(arc_err - arc_err1) + 0.6*ud1;
  if (R<=0.004)  /*直线*/
    servo_pwm = (uint16_t)(SERVO_MID + 20*arc_err);
  else if(R>0.004 && R<0.006)  /*大弯*/
    servo_pwm = (uint16_t)(SERVO_MID + 30*arc_err );
  else if(R>0.006) /*小弯*/
    servo_pwm = (uint16_t)(SERVO_MID + 40*arc_err);
  servo(servo_pwm);
  arc_err1 = arc_err;
  ud1 = ud;
  
  sprintf(txt, "DJP%4d", servo_pwm);
  LCD_P6x8Str(0,0,(uint8_t*)txt); 
  sprintf(txt, "R:%7.5f", R);
  LCD_P6x8Str(0,1,(uint8_t*)txt);  
}



/* 电磁引导的固定打角方向控制，用于大偏差或者断路，对摄像头控制进行验证 */
static inline void car_direction_control_inductance(void)
{
  servo(1500 + (adc_roaddata.err - 3)*40);
}


/* 电磁引导入环 */
static void car_direction_control_circle(void)
{
  /* 条件触发在adc.c内 */
  /*----------------- 右侧入环 ----------------------*/
    
  /* 环内运行 */
  if ( adc_roaddata.status == RightCircleWaitIn && ENC_GetPositionValue(ENC2)>7000 ) 
  {
    adc_roaddata.status = RightCircleRun; /* 此距离判断为已经入环 */
    status.sensor = Camera; /* 切换摄像头 */
    return;
  }
  
  /* 入环修正，距离超过2000 */
  if ( adc_roaddata.status == RightCircleWaitIn && ENC_GetPositionValue(ENC2)>2000)
  {
    if (adc_roaddata.err == 99) /* 99信号 */
      adc_roaddata.err = 5;     /* 转大弯 */
    return;
  }
  
   /* 出环完成 */
  if ( adc_roaddata.status == RightCircleWaitOut && ENC_GetPositionValue(ENC2)>7000 )
  {
    adc_roaddata.status = NoCircle;
    status.sensor = Camera;   /* 切换摄像头 */
    return;
  }  
  
  /* 出环修正，距离超过2000 */
  if ( adc_roaddata.status == RightCircleWaitOut && ENC_GetPositionValue(ENC2)>2000 )
  { /* 99给小左弯 */
    if (adc_roaddata.err == 99) /* 99信号 */
      adc_roaddata.err = 2;     /* 转小弯 */
    return;
  }
  
  /*----------------- 左侧入环 ----------------------*/
    
  /* 环内运行 */
  if ( adc_roaddata.status == LeftCircleWaitIn && ENC_GetPositionValue(ENC1)>7000 ) 
  {
    adc_roaddata.status = LeftCircleRun; /* 此距离判断为已经入环 */
    status.sensor = Camera; /* 切换摄像头 */
    return;
  }
  
  /* 入环修正，距离超过2000 */
  if ( adc_roaddata.status == LeftCircleWaitIn && ENC_GetPositionValue(ENC1)>2000)
  {
    if (adc_roaddata.err == 99) /* 99信号 */
      adc_roaddata.err = 1;     /* 转大弯 */
    return;
  }
  
   /* 出环完成 */
  if ( adc_roaddata.status == LeftCircleWaitOut && ENC_GetPositionValue(ENC1)>7000 )
  {
    adc_roaddata.status = NoCircle;
    status.sensor = Camera;   /* 切换摄像头 */
    return;
  }  
  
  /* 出环修正，距离超过2000 */
  if ( adc_roaddata.status == LeftCircleWaitOut && ENC_GetPositionValue(ENC1)>2000 )
  { /* 99给小左弯 */
    if (adc_roaddata.err == 99) /* 99信号 */
      adc_roaddata.err = 4;     /* 转小弯 */
    return;
  }

}


/* 避障开环控制
     |
      \  避障入左打角
      /   避障出回正
     |   避障出右打角
*/
static void car_direction_barrier_control(void)
{
  if (ENC_GetPositionValue(ENC1)>3000)  /* 入左打角 */ 
  {
    status.barrier = 0; /* 结束避障，进入电磁偏差自动调整模式 */
    //servo(1620); /* 固定左转打角1460 */
    return;
  }
  else if(ENC_GetPositionValue(ENC1)>1800) /* 避障出回正 */
  {
    servo(1500); /* 固定左转打角1460 */
    return;
  }
  else /* >0 */
    servo(1320);/* 固定左转避障打角1580 */
}

