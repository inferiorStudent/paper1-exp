#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <cstdio>
#include <fstream>
#include <string>
#include <algorithm>
using namespace std;

#define MAX_TASKSETS_NUM 1000  // 生成1000个合法(每个任务的利用率都小于1)任务集

typedef struct Task {
    int id;
    int wcet;
    double GPU_section_time_ratio;
    int period;
    vector<double> gpu_segment;
} Task;

/*
    任务总利用率 生成n个任务
*/
vector<double> UUniFast(double U, int n) {
    double sum = U, next_sum;
    vector<double> utilization;

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> udis(0, 1);
    //e.seed(time(0));

    for (int i = 1; i < n; i++) {
        next_sum = sum * pow(udis(gen), 1.0 / (n - i));
        utilization.push_back(sum - next_sum);
        sum = next_sum;
    }
    utilization.push_back(sum);
    return utilization;
}

vector<Task> generateTasks(double U, int n) {
    vector<double> utilization = UUniFast(U, n);
    for (auto &x : utilization) {
        if (x > 1) return vector<Task>();
    }
    vector<Task> taskset;
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> GPU_dis(0.01, 0.2);
    uniform_int_distribution<> p_dis(100, 500);

    // random section
    uniform_int_distribution<> sec_num_dis(1, 3);
    uniform_real_distribution<> sec_ratio_dis(1, 100);
    for (int i = 0; i < n; i++) {
        Task temp;
        temp.id = i;
        temp.period = p_dis(gen);
        temp.wcet = temp.period * utilization[i];
        temp.GPU_section_time_ratio = GPU_dis(gen);

        int sec_num = sec_num_dis(gen) * 2;
        for (int i = 0; i < sec_num; i++) {
            double j = sec_ratio_dis(gen);
            temp.gpu_segment.push_back(j);
        }
        sort(temp.gpu_segment.begin(), temp.gpu_segment.end());
        double sum = 0;
        for (int i = 0; i < sec_num; i += 2) {
            temp.gpu_segment[i + 1] -= temp.gpu_segment[i];
            sum += temp.gpu_segment[i + 1];
        }
        if (sum == 0) return vector<Task>();
        for (int i = 0; i < sec_num; i += 2) {
            temp.gpu_segment[i] /= 100;
            temp.gpu_segment[i + 1] = temp.gpu_segment[i + 1] / sum * temp.GPU_section_time_ratio;
        }

        taskset.push_back(temp);
    }
    return taskset;
}

void write_into_file(vector<Task> &taskset, const int U, const int n, const int id) {
    fstream f;
    string file_name = to_string(U) + "-" + to_string(n);
    f.open(file_name, ios::out | ios:: app);
    f << n << endl;
    for (int i = 0; i < taskset.size(); i++) {
        f << taskset[i].id << " " << taskset[i].wcet << " " << taskset[i].GPU_section_time_ratio
        << " " << taskset[i].period << " " << taskset[i].gpu_segment.size();
        for (int j = 0; j < taskset[i].gpu_segment.size(); j++) {
            f << " " << taskset[i].gpu_segment[j];
        }
        f << endl;
    }
    f.close();
}

int main() {
    for (int i = 0; i < MAX_TASKSETS_NUM; i++) {
        vector<Task> taskset = generateTasks(4, 10);
        if (taskset.size() < 10) {
            i--;
            continue;
        }
        write_into_file(taskset, 4, 10, i);
    }
    return 0;
}