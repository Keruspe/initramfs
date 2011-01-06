#ifndef __RUN_BG_H__
#define __RUN_BG_H__

void run_bg(void (*fun)(), int wait);
void run_bg_with_argv(void (*fun)(void *), void * argv, int wait);
void exec_bg_and_wait(char* path, char * arg, ...);

#endif /* __RUN_BG_H__ */

