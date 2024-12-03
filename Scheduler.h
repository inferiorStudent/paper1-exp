#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <iostream>
#include <queue>
#include <set>
#include "Task.h"
using namespace std;

class Scheduler {
public:
    queue<Task> waiting;
    set<int> mark;         // 记录队列状态

    bool P_EDF(vector<vector<Task>> &partitioned_tasksets, int simulation_time, int num_sms);
};

#endif