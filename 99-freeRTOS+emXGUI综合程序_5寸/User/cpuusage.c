/* FreeRTOSͷ�ļ� */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
/* ������Ӳ��bspͷ�ļ� */
#include "stm32h7xx.h"
#include "./tim/bsp_basic_tim.h"
#include "string.h"


uint32_t cpu_usage_get(char* task)
{
	TaskStatus_t *TaskStatus;
  UBaseType_t CurrentNumberOfTasks;//current task nums
  uint32_t TotalTime;
  //Get the CurTaskNums
  CurrentNumberOfTasks = uxTaskGetNumberOfTasks();

  //Malloc Mem
  TaskStatus = pvPortMalloc( CurrentNumberOfTasks * sizeof( TaskStatus_t ) );
  
  if( TaskStatus != NULL )
  {
    CurrentNumberOfTasks = uxTaskGetSystemState( TaskStatus, CurrentNumberOfTasks, &TotalTime );
    
    for(int i = 0; i < CurrentNumberOfTasks; i++)
    {
      if(!strcmp(task, TaskStatus[i].pcTaskName))
      {
        return (TaskStatus[i].ulRunTimeCounter / (TotalTime / 100UL));
      }
    }
  }
	vPortFree(TaskStatus);
  return 0;
}  

