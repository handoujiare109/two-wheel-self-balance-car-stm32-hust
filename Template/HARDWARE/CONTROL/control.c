#include "control.h"
#include "exti.h"
#include "motor.h"
#include "gtim.h"

/* 机械零点 */
#define MECHANICAL_ZERO  -0.90f

/* 安全关停角度 */
#define SAFE_ANGLE_MAX   30.0f

/* 直立环 PD  */
float Balance_Kp = 380.0f;
float Balance_Kd = 23.50f;

/* 速度环 */
int Velocity_Kp = 140;
float Velocity_Ki = 0.70f;

/* 1=只输出调试数据，不实际驱动电机；调零点时使用 */
#define DEBUG_SENSOR_ONLY    0

int debug_balance_pwm = 0;
int debug_speed_pwm = 0;
int debug_final_left = 0;
int debug_final_right = 0;

void Balance_Control(void)
{
    int balance_pwm;
    int speed_pwm;
    int velocity;
    int final_left, final_right;

    if (!balance_enable) {
        debug_balance_pwm = 0;
        debug_speed_pwm = 0;
        debug_final_left = 0;
        debug_final_right = 0;
        turn_Off();
        return;
    }

    /* 安全关停 */
    if (pitch > SAFE_ANGLE_MAX || pitch < -SAFE_ANGLE_MAX) {
        debug_balance_pwm = 0;
        debug_speed_pwm = 0;
        debug_final_left = 0;
        debug_final_right = 0;
        turn_Off();
        return;
    }

    /* 直立环 PD */
    balance_pwm = -(int)(Balance_Kp * (pitch - MECHANICAL_ZERO) + Balance_Kd * gyro_y);

    /* 速度环 PI */
    velocity = speed_left + speed_right;
    speed_pwm = Vel_PI(velocity, target_speed);

    PWM_Limit(&balance_pwm);
    final_left  = balance_pwm + speed_pwm;
    final_right = balance_pwm + speed_pwm;
    PWM_Limit(&final_left);
    PWM_Limit(&final_right);

    debug_balance_pwm = balance_pwm;
    debug_speed_pwm = speed_pwm;
    debug_final_left = final_left;
    debug_final_right = final_right;

    if (motor_enable && !DEBUG_SENSOR_ONLY) {
        motorMove(final_left, final_right);
    } else {
        turn_Off();
    }
}
