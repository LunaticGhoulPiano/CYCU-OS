import matplotlib.pyplot as plt

"""
out_input1.txt
IDs = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 13, 20, 27, 29]
FCFS_turnaround_time = [23, 15, 25, 22, 16, 26, 5, 16, 23, 9, 16, 19, 16, 22, 20]
RR_turnaround_time = [22, 10, 22, 29, 22, 33, 20, 3, 16, 17, 45, 4, 20, 34, 37]
SJF_turnaround_time = [9, 7, 5, 10, 10, 25, 10, 3, 2, 4, 57, 5, 8, 15, 21]
SRTF_turnaround_time = [4, 2, 5, 10, 3, 25, 11, 1, 2, 5, 57, 1, 3, 25, 25]
HRRN_turnaround_time = [23, 7, 19, 18, 16, 29, 5, 4, 13, 10, 26, 5, 16, 15, 26]
PPRR_turnaround_time = [4, 2, 17, 4, 14, 27, 16, 56, 11, 4, 53, 1, 43, 16, 10]
"""
"""
out_input2.txt
IDs = [1, 2, 3, 4, 5]
FCFS_turnaround_time = [11, 12, 13, 13, 17]
RR_turnaround_time = [24, 4, 5, 8, 15]
SJF_turnaround_time = [11, 12, 15, 10, 17]
SRTF_turnaround_time = [24, 2, 3, 3, 7]
HRRN_turnaround_time = [11, 12, 15, 10, 17]
PPRR_turnaround_time = [11, 23, 11, 11, 15]
"""
"""
out_input3.txt
IDs = [1, 2, 3, 4, 5, 6]
FCFS_turnaround_time = [20, 25, 45, 30, 10, 15]
RR_turnaround_time = [20, 45, 55, 30, 10, 15]
SJF_turnaround_time = [20, 25, 45, 30, 10, 15]
SRTF_turnaround_time = [20, 25, 45, 30, 10, 15]
HRRN_turnaround_time = [20, 25, 45, 30, 10, 15]
PPRR_turnaround_time = [20, 55, 60, 15, 20, 10]
"""
"""
out_input4.txt
IDs = [1, 2, 3, 4]
FCFS_turnaround_time = [6, 7, 10, 12]
RR_turnaround_time = [11, 5, 14, 12]
SJF_turnaround_time = [6, 7, 15, 6]
SRTF_turnaround_time = [9, 3, 15, 6]
HRRN_turnaround_time = [6, 7, 10, 12]
PPRR_turnaround_time = [15, 3, 15, 5]
"""
plt.figure(figsize=(10, 6))
bar_width = 0.1
index = range(len(IDs))

plt.bar(index, FCFS_turnaround_time, bar_width, label='FCFS')
plt.bar([i + bar_width for i in index], RR_turnaround_time, bar_width, label='RR')
plt.bar([i + 2 * bar_width for i in index], SJF_turnaround_time, bar_width, label='SJF')
plt.bar([i + 3 * bar_width for i in index], SRTF_turnaround_time, bar_width, label='SRTF')
plt.bar([i + 4 * bar_width for i in index], HRRN_turnaround_time, bar_width, label='HRRN')
plt.bar([i + 5 * bar_width for i in index], PPRR_turnaround_time, bar_width, label='PPRR')

plt.xlabel('ID')
plt.ylabel('Turnaround Time')
plt.title('Turnaround Time Comparison of out_input.txt')
plt.xticks([i + 2 * bar_width for i in index], IDs)
plt.legend()
plt.tight_layout()

plt.show()
