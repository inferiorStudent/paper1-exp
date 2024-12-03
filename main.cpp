#include <iostream>
#include <vector>
#include "Task.h"
#include "Scheduler.h"
#include "System.h"
using namespace std;

#define MAX_SIMULATION_TIME 100000000

int main() {
    System sys;
    int task_num, schedulable = 0;;
    while (cin >> task_num) {

        /* taskset initialization */

        vector<Task> taskset;
        int sec_num;
        double gpu_ratio, sec_ratio;
        for (int i = 0; i < task_num; i++) {
            Task task;
            cin >> task.id >> task.wcet >> gpu_ratio >> task.period >> sec_num;
            for (int j = 0; j < sec_num; j++) {
                cin >> sec_ratio;
                task.gpu_segment.push_back(sec_ratio);
            }
            task.GPU_section_time = gpu_ratio * task.wcet;
            task.remaining_time = task.wcet;
            task.deadline = task.period;
            task.sm_num = sys.MAX_SM_NUMBER;

            taskset.push_back(task);
        }

        vector<vector<Task>> partitioned_tasksets = sys.smAlocation(taskset, sys.MAX_CORE_NUMBER, sys.MAX_SM_NUMBER);
        /* feasibility test */
        if (partitioned_tasksets.size() != 0) {
            schedulable++;

            // display the partitioned result
            for (int i = 0; i < partitioned_tasksets.size(); i++) {
                cout << "core " << i << ": ";
                for (int j = 0; j < partitioned_tasksets[i].size(); j++) {
                    cout << partitioned_tasksets[i][j].id << " ";
                }
                cout << endl;
            }
            cout << "==========" << endl;
            return 0;
        }
        /* scheduling simulation */
    }
    cout << schedulable << endl;
    return 0;
}