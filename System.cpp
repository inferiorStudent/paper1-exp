#include <unistd.h>
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include "System.h"
#include "Task.h"
using namespace std;

System::System() {
    this->MAX_CORE_NUMBER = sysconf(_SC_NPROCESSORS_ONLN);
    this->MAX_SM_NUMBER = 14;
}

void display(vector<vector<Task>> &partitioned_tasksets) {
    for (int i = 0; i < partitioned_tasksets.size(); i++) {
        cout << "core " << i << ": ";
        for (int j = 0; j < partitioned_tasksets[i].size(); j++) {
            cout << partitioned_tasksets[i][j].id << " ";
        }
        cout << endl;
    }
    cout << "==========================" << endl;
}

vector<vector<Task>> System::partitionTasks(vector<Task> &taskset, int num_cores, int num_sms) {
    vector<double> utilization(num_cores, 0);
    vector<vector<Task>> partitioned_tasksets(num_cores, vector<Task>());
    double u_sys = 0;
    set<int> unmapped_tasks;
    for (int i = 0; i < taskset.size(); i++) unmapped_tasks.insert(taskset[i].id);
    while (!unmapped_tasks.empty()) {
        // choose task
        double ux = 0;
        int mapped = -1;
        for (auto x : unmapped_tasks) {
            double temp = (taskset[x].wcet + bwEstimate(taskset[x], partitioned_tasksets, num_sms)) / taskset[x].period;
            if (temp > ux) {
                ux = temp;
                mapped = x;
            }
        }
        if (ux > 1 || mapped == -1) return vector<vector<Task>>();
        // choose core
        int m = -1;
        double m_delta = 1;
        for (int i = 0; i < utilization.size(); i++) {
            utilization[i] += ux;
            double u_sys_temp = 0;
            for (int j = 0; j < utilization.size(); j++) {
                u_sys_temp = max(u_sys_temp, utilization[j]);
            }
            double temp_delta = u_sys_temp - u_sys;
            if (temp_delta < m_delta) {
                m_delta = temp_delta;
                m = i;
            }
            utilization[i] -= ux;
        }
        if (m == -1) return vector<vector<Task>>();
        // cout << m << " " << mapped << " " << ux << " " << taskset[mapped].sm_num << endl;
        // update utilization & partition set for every core
        utilization[m] += ux;
        taskset[mapped].cpu_core = m;
        partitioned_tasksets[m].push_back(taskset[mapped]);
        unmapped_tasks.erase(mapped);
        for (int i = 0; i < utilization.size(); i++) {
            u_sys = max(u_sys, utilization[i]);
        }
        if (u_sys > 1) return vector<vector<Task>>();
    }
    return partitioned_tasksets;
}

vector<vector<Task>> System::smAlocation(vector<Task> &taskset, int num_cores, int num_sms) {
    for (int i = 0; i < taskset.size(); i++) {
        int num = 1;
        taskset[i].update_task_info(num++);
        while ((double)taskset[i].wcet / taskset[i].period > 1.0) taskset[i].update_task_info(num++);
    }
    vector<vector<Task>> partitioned_tasksets = this->partitionTasks(taskset, num_cores, num_sms);
    while (partitioned_tasksets.size() == 0) {
        // choose task
        int x;
        double delta = 2;
        for (int i = 0; i < taskset.size(); i++) {
            taskset[i].update_task_info(taskset[i].sm_num + 1);
            double u1 = taskset[i].wcet + bwEstimate(taskset[i], partitioned_tasksets, num_cores);
            taskset[i].update_task_info(taskset[i].sm_num - 1);
            double u2 = taskset[i].wcet + bwEstimate(taskset[i], partitioned_tasksets, num_cores);
            double temp_delta = (u1 - u2) / taskset[i].period;

            if (delta > temp_delta) {
                delta = temp_delta;
                x = i;
            }
        }
        taskset[x].update_task_info(taskset[x].sm_num + 1);
        if (taskset[x].sm_num > num_sms) return vector<vector<Task>>();
        partitioned_tasksets = this->partitionTasks(taskset, num_cores, num_sms);
    }
    return partitioned_tasksets;
}

bool cmp(const vector<int> &a, const vector<int> &b) {
    return a[1] > b[1];
}

double System::bwEstimate(Task &task, vector<vector<Task>> &partitioned_tasksets, int num_sms) {
    double BWest = 0;
    vector<vector<int>> worst_task(partitioned_tasksets.size(), vector<int>(2, 0));
    for (int i = 0; i < partitioned_tasksets.size(); i++) {
        for (int j = 0; j < partitioned_tasksets[i].size(); j++) {
            worst_task[i][0] = max(worst_task[i][0], partitioned_tasksets[i][j].GPU_section_time);
            worst_task[i][1] = max(worst_task[i][1], partitioned_tasksets[i][j].sm_num);
        }
    }
    sort(worst_task.begin(), worst_task.end(), cmp);
    int temp_gpu_wcet = 0, sm_available = num_sms;
    for (int i = 0; i < worst_task.size(); i++) {
        if (sm_available >= worst_task[i][1]) {
            sm_available -= worst_task[i][1];
            temp_gpu_wcet = max(temp_gpu_wcet, worst_task[i][0]);
        } else {
            BWest += temp_gpu_wcet;
            temp_gpu_wcet = 0;
            sm_available = num_sms;
        }
    }
    return BWest;
}

bool System::is_schedulable(vector<vector<Task>> &partitioned_tasksets) {
    for (int i = 0; i < partitioned_tasksets.size(); i++) {
        double u = 0;
        for (int j = 0; j < partitioned_tasksets[i].size(); j++) {
            u += (double)partitioned_tasksets[i][j].wcet / (double)partitioned_tasksets[i][j].period;
        }
        if (u > 1) return false;
    }
    return true;
}