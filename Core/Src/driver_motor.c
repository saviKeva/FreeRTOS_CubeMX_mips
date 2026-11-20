/*
 * driver_motor.c
 *
 *  Created on: Jan 6, 2022
 *      Author: Marko Micovic
 */

#include "tim.h"
#define ARR 9
#define STEP 2
#define INC ((ARR+1)/STEP)

static uint32_t compReg = 0;
void MOTOR_SpeedIncrease(){

	if(compReg<(ARR+1)){
		compReg+=INC;
	}
	htim3.Instance->CCR1 = compReg;

}
void MOTOR_SpeedDecrease(){

	if(compReg>0){
		compReg-=INC;
	}
	htim3.Instance->CCR1 = compReg;
}

void MOTOR_Init(){

	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

}

