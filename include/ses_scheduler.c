/*INCLUDES ************************************************************/
#include "ses_timer.h"
#include "ses_scheduler.h"
#include "util/atomic.h"
#include "ses_led.h"


/* PRIVATE VARIABLES **************************************************/
/** list of scheduled tasks */
static taskDescriptor* taskList = NULL;
taskDescriptor* firstTask = NULL;
bool taskfinished = false;

systemTime_t* time = 0 ;
struct time_t t1;
float MILLISECONDS_TO_HOURS = 0.0000002777777778;
float HRS_TO_MINS = 60.0;
float MINS_TO_SEC = 60.0;
float SEC_TO_MILLISECONDS = 1000.0;

/*FUNCTION DEFINITION *************************************************/

void scheduler_update(void) {
	/* The callback for the timer2 interrupt is used to update the scheduler every
	 *  1 ms by decreasing the expiry time of all tasks by 1 ms and mark expired
	 *  tasks for execution. */
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		taskDescriptor* taskupdate;
		taskupdate = taskList;
		while (taskList != NULL) {
			if (taskList->expire != 0) {
				taskList->expire = (taskList->expire) - 1;
			}
			if (taskList->expire == 0) {
				taskList->execute = 0;
				taskList->expire = taskList->period;
			}
			taskList = taskList->next;
		}
		taskList = taskupdate;
	}
}

void scheduler_init() {
	//Start timer
	timer2_start();
	//Callback the update function to decrease expiry time by 1 ms
	timer2_setCallback(&scheduler_update);
}

void scheduler_run() {
	taskDescriptor* tasktemp;
	while (1) {
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			tasktemp = taskList;
			while (tasktemp != NULL) {
				task_t run = tasktemp->task;
				if (run != NULL) {
					if (tasktemp->execute == 0) {
						if (tasktemp->period == 0) {
							run(tasktemp->param);
							tasktemp->execute = 1;
							scheduler_remove(tasktemp);
						} else {
							run(tasktemp->param);
							tasktemp->execute = 1;
						}
					}
				}
				tasktemp = tasktemp->next;
			}
		}
	}
}

bool added;
bool scheduler_add(taskDescriptor * toAdd) {
	added = false;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if (toAdd == NULL) {                // check if NULL parameter is passed
			return added;
		}
		if (taskList == NULL) {
			taskList = toAdd;
			firstTask = taskList;     // firstTask always points to the 1st task
			added = true;
			return added;
		} else {
			if (duplicateFound(toAdd) == true) {   //Condition 2: Redundant task
				return added;                       // returns false
			} else {
				while (added == false) {
					if (taskList->next != NULL) {
						taskList = taskList->next;
					} else {
						taskList->next = toAdd;
						taskList = firstTask; //taskList points again to the first task
						added = true;
						return added;
					}
				}
			}
		}
	}
	return added;
}

//Searches through the list to check if the to-be-added task is already there
bool duplicateFound(taskDescriptor * toAdd) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		taskDescriptor* current = firstTask;
		while (current != NULL) {
			if (current->task == toAdd->task) {
				return true;
			} else {
				current = current->next;
			}
		}
		return false;
	}
}

void scheduler_remove(taskDescriptor * toRemove) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if (toRemove == NULL) {
			break;
		}
		//case1: first task to be removed
		if (taskList->task == toRemove->task) {
			taskDescriptor* temp = taskList;
			temp = taskList->next;
			taskList->next = NULL;
			taskList = temp;
			firstTask = taskList;
		} else {
			//case2: normal removal
			bool removed = false;
			while (removed == false) {
				if (taskList->next->task == toRemove->task) {
					taskList->next = taskList->next->next;
					removed = true;
				} else {
					taskList = taskList->next;
				}
			}
			taskList = firstTask; //taskList points again to the first task
		}
	}
}
void convert_milliseconds_to_time_structure(uint32_t millis){

	//gets time in millis and converts it to time structure.
	float hrs = MILLISECONDS_TO_HOURS * millis;
	float completedHrs = floor(hrs);
	t1.hour = (uint8_t) completedHrs;

	float remainingMins = hrs - completedHrs;
    float mins = remainingMins * HRS_TO_MINS;
    float completedMins = floor(mins);
    t1.minute = (uint8_t) completedMins;

    float remainingSec = mins - completedMins;
    float sec = remainingSec * MINS_TO_SEC;
    float completedSec = floor(sec);
    t1.second = (uint8_t) completedSec;

    float remainingMS = sec - completedSec;
    float ms = remainingMS * SEC_TO_MILLISECONDS;
    t1.milli = (uint16_t) round(ms);

}

systemTime_t scheduler_getTime(){
	//actual system time
	convert_milliseconds_to_time_structure(timer2_getTime());
	return timer2_getTime() ;
}
void scheduler_setTime(systemTime_t time){
	timer2_setTime(time);

}
