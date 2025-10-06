#ifndef SAFETY_TASK_H
#define SAFETY_TASK_H

typedef struct {
    int flame_flag; // 0: no flame, 1: flame
    int flame_switch;// 0: flame off, 1: flame on
    int sr602_flag; // 0: no people, 1: people
    int sr602_switch;// 0: people off, 1: people on
} SafetyData;

void my_flame_exit_callback(void);

void SafetyTask(void* argument);
void Sr602Task(void* argument);
void FlameTask(void* argument);

#endif /* SAFETY_TASK_H */
