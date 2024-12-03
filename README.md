# Simulation: real-time scheduling with GPU resource contention

1. This is the experiment code of my unfinished *graduation thesis* regarding to real time scheduling. It's going to be perfected in several days, no later than 2024.12.31......
2. Under the directory `${pwd}/`, use the command `make` and you can get the schedulable ratio.
3. File `${pwd}/tasksets/test.cpp` is just for code testing, nothing for the whole project.
4. Change some parameters (utilization `U`, the number of tasks `n` and MAX_TASKSET_NUMBER) in file `${pwd}/tasksets/uunifast.cpp` and generate the tasksets, then write into the file "U-n". The binary executable file needs the input from "U-n" by using command `./main < tasksets/4-10`, taking "4-10" as an example.
5. The sections "generate tasksets" and "allocation & partitioning (feasibility test)" have been finished which was tested correctedly in Dec. 3rd, 2024. The last section "scheduling simulation" will be tested in near future (maybe before Dec. 8th).

# range of parameters for generation
=== to be copied ===

last update time: Dec. 3rd, 2024
