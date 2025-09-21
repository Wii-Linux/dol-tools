/*
 * DOL Runner - Timers
 * Copyright (C) 2025 Techflash
 */

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "emu.h"
#include "timer.h"

#define MAX_HOOKS 32
static timerFunc_t timersHooks[MAX_HOOKS];
static int hookIdx = 0;
static struct itimerval timer;

void E_Timer_Init(void) {
	memset(timersHooks, 0, sizeof(timerFunc_t) * MAX_HOOKS);
	hookIdx = 0;

	signal(SIGALRM, E_SIGALRM_Handler);
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 250000;  /* first expiration */
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 16000;  /* subsequent expirations */
	if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
		perror("FATAL: settimer() failed");
		E_State.fatalError = true;
		return;
	}
	return;
}

void E_Timer_AddHook(timerFunc_t hook) {
	if (hookIdx >= MAX_HOOKS) {
		printf("FATAL: Timer: Tried to add more than %d (MAX_HOOKS) timer hooks, something has gone horribly wrong\n", MAX_HOOKS);
		E_State.fatalError = true;
		return;
	}
	timersHooks[hookIdx] = hook;
	hookIdx++;
	return;
}

/* we got here via signal */
void E_SIGALRM_Handler(int dummy) {
	int i;
	(void)dummy;
	for (i = 0; i < hookIdx; i++) {
		timersHooks[i]();
	}

	return;
}
