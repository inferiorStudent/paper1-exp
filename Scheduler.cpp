#include <iostream>
#include <queue>
#include <set>
#include <algorithm>
#include <vector>
#include "Scheduler.h"
using namespace std;

bool CompareDeadline (const Task &a, const Task &b) {
    return a.deadline < b.deadline;
}

/* 离散时间模拟 */
bool Scheduler::P_EDF(vector<vector<Task>> &partitioned_tasksets, int simulation_time, const int num_sms) {
    const int core_number = partitioned_tasksets.size();
    int sm_available = num_sms;

    for (int current_time = 0; current_time < simulation_time; current_time++) {
        for (int core_id = 0; core_id < core_number; core_id++) {
            auto& core_tasksets = partitioned_tasksets[core_id];

            // 根据 最小截止时间 排序
            sort(core_tasksets.begin(), core_tasksets.end(), CompareDeadline);

            // 确定在CPU上运行的任务 先选取第一个还没完成的任务
            Task task = core_tasksets[0];
            bool idle = true;
            for (auto &task_temp : core_tasksets) {
                // 找第一个 开始时间 不超过当前时间的任务 作为调度的任务
                if (current_time >= task_temp.deadline - task_temp.period) {
                    task = task_temp;
                    idle = false;
                    break;
                }
            }
            if (idle) continue;     // 没找到task 说明该核进入空闲状态
            for (auto &task_temp : core_tasksets) {
                if (task_temp.sm_locked) {
                    task = task_temp;
                    break;
                }
            }

            // 检验是否调度失败
            if ((current_time > task.deadline) || 
                (current_time == task.deadline && task.remaining_time != 0)) return false;

            // task execution & access SMs with FIFO queue
            if (task.remaining_time == 0) {
                // 完成执行
                task.Reset();
            } else {
                if (task.sm_locked) {
                    // 释放资源
                    if (task.sm_locked && task.release_SMs()) {
                        task.sm_locked = false;
                        sm_available += task.sm_num;
                    }
                    task.remaining_time--;
                } else {
                    if (task.is_on_cpu()) {                     // 如果在CPU上则正常运行
                        task.remaining_time--;
                    } else if (task.get_SMs()) {
                        if (this->mark.count(task.id) == 1) {   // 如果在队列中
                            // 如果是队头元素
                            if (this->waiting.front().id == task.id && task.sm_num <= sm_available) {
                                this->waiting.pop();
                                this->mark.erase(task.id);
                                sm_available -= task.sm_num;
                                task.sm_locked = true;
                                task.remaining_time--;
                                /* 插队 */
                            }
                        } else {                                // 如果不在队列里 即刚来的
                            if (!this->waiting.empty() || task.sm_num > sm_available) {
                                this->waiting.push(task);
                                this->mark.insert(task.id);
                            } else {
                                // 队列空 且资源够
                                sm_available -= task.sm_num;
                                task.sm_locked = true;
                                task.remaining_time--;
                            }
                        }
                    }
                }
            }
        }
        if (!this->waiting.empty() && this->waiting.front().sm_num <= sm_available) {
            Task temp = this->waiting.front();
            temp.sm_locked = true;
            this->waiting.pop();
            this->mark.erase(temp.id);
            sm_available -= temp.sm_num;
            /* 插队 */
        }
    }
    return true;
}