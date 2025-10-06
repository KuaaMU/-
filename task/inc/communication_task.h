#ifndef COMMUNICATION_TASK_H
#define COMMUNICATION_TASK_H



void CommunicationTask(void* argument);
void Rc522Task(void *argument);
void Fpm383Task(void *argument);
void Hx1838Task(void *argument);
void Esp8266Task(void *argument);

void app_hx1838_proc(void);
void app_esp8266_proc(void);


#endif /* COMMUNICATION_TASK_H */
