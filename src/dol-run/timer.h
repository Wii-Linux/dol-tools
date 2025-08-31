/*
 * DOL Runner - Timers
 * Copyright (C) 2025 Techflash
 */

#ifndef _DOLTOOLS_DOLRUN_TIMER_H
#define _DOLTOOLS_DOLRUN_TIMER_H

typedef void (*timerFunc_t)(void);
extern void E_Timer_Init(void);
extern void E_Timer_AddHook(timerFunc_t hook);
extern void E_SIGALRM_Handler(int dummy);
#endif
