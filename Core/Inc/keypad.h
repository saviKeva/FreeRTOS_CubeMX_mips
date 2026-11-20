/*
 * keypad.h
 *
 *  Created on: 31.07.2026.
 *      Author: SM8536
 */

#ifndef CORE_INC_KEYPAD_H_
#define CORE_INC_KEYPAD_H_

extern char keys[2];
extern int keysChanged;

extern void KEY_Init();
extern void KEY_EXTI_Callback();

#endif /* CORE_INC_KEYPAD_H_ */
