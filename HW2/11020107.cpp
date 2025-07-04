#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <fstream>
#include <queue>
#include <algorithm>

struct Process {
    // input data
    int id;
    int cpu_burst;
    int arrival_time;
    int priority;
    // exeuting data
    int finish_time;
    int remain_time;
    float response_ratio;
    // output data
    int waiting_time;
    int turnaround_time;
};

struct Data {
    int method;
    int time_slice;
    std::vector<Process> processes;
};

class Methods {
    private:
        Data data;
        std::string filename;

        struct Result {
            std::vector<Process> results;
            std::string Gantt_chart;
            std::string method_name;
        };

        std::string getOutID(int id) {
            if (id < 10) return std::to_string(id);
            else {
                std::string str(1, static_cast<char>(id+55));
                return str;
            }
        }

        Result FCFS() {
            std::sort(data.processes.begin(), data.processes.end(), [](const Process &a, const Process &b){
                if (a.arrival_time == b.arrival_time) return a.id < b.id;
                return a.arrival_time < b.arrival_time;
            });

            std::string Gantt_chart = "";
            std::queue<Process> unused_q;
            std::vector<Process> results;
            for (Process p: data.processes) unused_q.push(p);
            std::vector<Process> waiting_q;
            Process cur_p;
            cur_p.id = -1; // reset
            for (int time = 0; ; time++) {
                while (! unused_q.empty() && time == unused_q.front().arrival_time) {
                    waiting_q.push_back(unused_q.front());
                    unused_q.pop();
                }
                if (cur_p.id == -1) {
                    if (waiting_q.empty()) { // idle
                        Gantt_chart += "-";
                        continue;
                    }
                    else { // new process start
                        cur_p = waiting_q[0];
                        waiting_q.erase(waiting_q.begin());
                    }
                }

                cur_p.remain_time--;
                Gantt_chart += getOutID(cur_p.id);

                if (cur_p.remain_time == 0) { // current process end
                    cur_p.finish_time = time + 1;
                    cur_p.turnaround_time = cur_p.finish_time - cur_p.arrival_time;
                    cur_p.waiting_time = cur_p.turnaround_time - cur_p.cpu_burst;
                    results.push_back(cur_p);
                    cur_p = Process();
                    cur_p.id = -1;

                    if (unused_q.empty() && waiting_q.empty()) break;
                }
            }

            Result result;
            result.results = results;
            result.Gantt_chart = Gantt_chart;
            result.method_name = "FCFS";
            return result;
        }

        Result RR() {
            std::sort(data.processes.begin(), data.processes.end(), [](const Process &a, const Process &b){
                if (a.arrival_time == b.arrival_time) return a.id < b.id;
                return a.arrival_time < b.arrival_time;
            });

            std::string Gantt_chart = "";
            std::vector<Process> results;
            std::pair<int, int> record_time(0, 0); // previous execution time, waiting time
            std::pair<Process, std::pair<int, int>> cur_p; // process and its record_time
            std::queue<std::pair<Process, std::pair<int, int>>> unused_q;
            for (Process p: data.processes) {
                cur_p = std::pair<Process, std::pair<int, int>>(p, record_time);
                unused_q.push(cur_p);
            }
            cur_p = std::pair<Process, std::pair<int, int>>(); // reset
            cur_p.first.id = -1; // reset
            int time_slice = data.time_slice, time_counter = 0;
            std::queue<std::pair<Process, std::pair<int, int>>> waiting_q;

            for (int time = 0; ; time++) {
                // get arrived processes into waiting queue
                while (! unused_q.empty() && unused_q.front().first.arrival_time == time) {
                    waiting_q.push(unused_q.front());
                    unused_q.pop();
                }

                // enqueue unfinished process and reset
                if (time_counter == time_slice && cur_p.first.remain_time != 0) {
                    waiting_q.push(cur_p);
                    // reset
                    cur_p = std::pair<Process, std::pair<int, int>>();
                    cur_p.first.id = -1;
                    time_counter = 0;
                }

                // judge current process
                if (cur_p.first.id == -1) {
                    if (waiting_q.empty() && time < unused_q.front().first.arrival_time) { // idle
                        Gantt_chart += "-";
                        continue;
                    }
                    else if (! waiting_q.empty()) {
                        cur_p = waiting_q.front();
                        waiting_q.pop();
                    }
                    else std::cerr << "ERROR" << std::endl;
                }

                // execute current process
                if (cur_p.first.cpu_burst == cur_p.first.remain_time) cur_p.second.second = time - cur_p.first.arrival_time;
                else cur_p.second.second += (time - cur_p.second.first - 1); // wait += (cur_exec_time - prev_exec_time - 1);
                cur_p.first.remain_time--;
                cur_p.second.first = time; // update previous execute time
                time_counter++;
                Gantt_chart += getOutID(cur_p.first.id);

                // judge end
                if (cur_p.first.remain_time == 0) { // finish
                    cur_p.first.finish_time = time + 1;
                    cur_p.first.turnaround_time = cur_p.first.finish_time - cur_p.first.arrival_time;
                    cur_p.first.waiting_time = cur_p.second.second;
                    results.push_back(cur_p.first);
                    // reset
                    cur_p = std::pair<Process, std::pair<int, int>>();
                    cur_p.first.id = -1;
                    time_counter = 0;
                    // break condition
                    if (unused_q.empty() && waiting_q.empty()) break;
                }
            }

            Result result;
            result.results = results;
            result.Gantt_chart = Gantt_chart;
            result.method_name = "RR";
            return result;
        }

        Result SJF() {
            std::sort(data.processes.begin(), data.processes.end(), [](const Process &a, const Process &b){
                if (a.arrival_time == b.arrival_time && a.cpu_burst == b.cpu_burst) return a.id < b.id;
                if (a.arrival_time == b.arrival_time && a.cpu_burst != b.cpu_burst) return a.cpu_burst < b.cpu_burst;
                return a.arrival_time < b.arrival_time;
            });

            std::string Gantt_chart = "";
            std::vector<Process> results;
            Process cur_p; // process and its record_time
            std::queue<Process> unused_q;
            for (Process p: data.processes) unused_q.push(p);
            std::vector<Process> waiting_q;
            cur_p = Process(); // reset
            cur_p.id = -1; // reset

            for (int time = 0; ; time++) {
                // get arrived processes into waiting queue
                while (! unused_q.empty() && unused_q.front().arrival_time == time) {
                    waiting_q.push_back(unused_q.front());
                    unused_q.pop();
                }

                // judge current process
                if (cur_p.id == -1) {
                    if (waiting_q.empty()) { // idle
                        Gantt_chart += "-";
                        continue;
                    }
                    else {
                        // sort waiting queue
                        std::sort(waiting_q.begin(), waiting_q.end(), [](const Process &a, const Process &b){
                            if (a.cpu_burst == b.cpu_burst && a.arrival_time == b.arrival_time) return a.id < b.id;
                            if (a.cpu_burst == b.cpu_burst && a.arrival_time != b.arrival_time) return a.arrival_time < b.arrival_time;
                            return a.cpu_burst < b.cpu_burst;
                        });
                        // get process
                        cur_p = waiting_q[0];
                        waiting_q.erase(waiting_q.begin());
                    }
                }

                cur_p.remain_time--;
                Gantt_chart += getOutID(cur_p.id);

                if (cur_p.remain_time == 0) { // current process end
                    cur_p.finish_time = time + 1;
                    cur_p.turnaround_time = cur_p.finish_time - cur_p.arrival_time;
                    cur_p.waiting_time = cur_p.turnaround_time - cur_p.cpu_burst;
                    results.push_back(cur_p);
                    cur_p = Process();
                    cur_p.id = -1;

                    if (unused_q.empty() && waiting_q.empty()) break;
                }
            }

            Result result;
            result.results = results;
            result.Gantt_chart = Gantt_chart;
            result.method_name = "SJF";
            return result;
        }

        Result SRTF() {
            std::sort(data.processes.begin(), data.processes.end(), [](const Process &a, const Process &b){
                if (a.arrival_time == b.arrival_time && a.cpu_burst == b.cpu_burst) return a.id < b.id;
                if (a.arrival_time == b.arrival_time && a.cpu_burst != b.cpu_burst) return a.cpu_burst < b.cpu_burst;
                return a.arrival_time < b.arrival_time;
            });

            std::string Gantt_chart = "";
            std::vector<Process> results;
            Process cur_p; // process and its record_time
            std::queue<Process> unused_q;
            for (Process p: data.processes) unused_q.push(p);
            std::vector<Process> waiting_q;
            cur_p = Process(); // reset
            cur_p.id = -1; // reset

            for (int time = 0; ; time++) {
                // get arrived processes into waiting queue
                while (! unused_q.empty() && unused_q.front().arrival_time == time) {
                    waiting_q.push_back(unused_q.front());
                    unused_q.pop();
                }
                
                if (cur_p.id != -1) {
                    waiting_q.insert(waiting_q.begin(), cur_p);
                    // reset
                    cur_p = Process();
                    cur_p.id = -1;
                }

                // sort waiting queue
                std::sort(waiting_q.begin(), waiting_q.end(), [](const Process &a, const Process &b){
                    if (a.remain_time == b.remain_time && a.arrival_time == b.arrival_time) return a.id < b.id;
                    if (a.remain_time == b.remain_time && a.arrival_time != b.arrival_time) return a.arrival_time < b.arrival_time;
                    return a.remain_time < b.remain_time;
                });

                // judge current process
                if (cur_p.id == -1) {
                    if (waiting_q.empty()) { // idle
                        Gantt_chart += "-";
                        continue;
                    }
                    else {
                        // get process
                        cur_p = waiting_q[0];
                        waiting_q.erase(waiting_q.begin());
                    }
                }

                cur_p.remain_time--;
                Gantt_chart += getOutID(cur_p.id);

                if (cur_p.remain_time == 0) { // current process end
                    cur_p.finish_time = time + 1;
                    cur_p.turnaround_time = cur_p.finish_time - cur_p.arrival_time;
                    cur_p.waiting_time = cur_p.turnaround_time - cur_p.cpu_burst;
                    results.push_back(cur_p);
                    cur_p = Process();
                    cur_p.id = -1;

                    if (unused_q.empty() && waiting_q.empty()) break;
                }
            }

            Result result;
            result.results = results;
            result.Gantt_chart = Gantt_chart;
            result.method_name = "SRTF";
            return result;
        }

        Result HRRN() {
            std::sort(data.processes.begin(), data.processes.end(), [](const Process &a, const Process &b){
                if (a.arrival_time == b.arrival_time) return a.id < b.id;
                return a.arrival_time < b.arrival_time;
            });

            std::string Gantt_chart = "";
            std::vector<Process> results;
            Process cur_p; // process and its record_time
            std::queue<Process> unused_q;
            for (Process p: data.processes) unused_q.push(p);
            std::vector<Process> waiting_q;
            cur_p = Process(); // reset
            cur_p.id = -1; // reset

            for (int time = 0; ; time++) {
                // get arrived processes into waiting queue
                while (! unused_q.empty() && unused_q.front().arrival_time == time) {
                    waiting_q.push_back(unused_q.front());
                    unused_q.pop();
                }

                // judge current process
                if (cur_p.id == -1) {
                    if (waiting_q.empty()) { // idle
                        Gantt_chart += "-";
                        continue;
                    }
                    else {
                        cur_p = waiting_q[0];
                        waiting_q.erase(waiting_q.begin());
                    }
                }

                cur_p.remain_time--;
                Gantt_chart += getOutID(cur_p.id);

                if (cur_p.remain_time == 0) { // current process end
                    cur_p.finish_time = time + 1;
                    cur_p.turnaround_time = cur_p.finish_time - cur_p.arrival_time;
                    cur_p.waiting_time = cur_p.turnaround_time - cur_p.cpu_burst;
                    results.push_back(cur_p);
                    cur_p = Process();
                    cur_p.id = -1;

                    if (unused_q.empty() && waiting_q.empty()) break;
                    if (waiting_q.empty()) continue;

                    // calculate response ratio
                    for (Process &p:waiting_q) p.response_ratio = float(((time - p.arrival_time) + p.cpu_burst)) / float(p.cpu_burst);
                    // sort waiting queue
                    std::sort(waiting_q.begin(), waiting_q.end(), [](const Process &a, const Process &b){
                        if (a.response_ratio == b.response_ratio && a.arrival_time == b.arrival_time) return a.id < b.id;
                        if (a.response_ratio == b.response_ratio && a.arrival_time != b.arrival_time) return a.arrival_time < b.arrival_time;
                        return a.response_ratio > b.response_ratio;
                    });
                    // get process
                    cur_p = waiting_q[0];
                    waiting_q.erase(waiting_q.begin());
                }
            }

            Result result;
            result.results = results;
            result.Gantt_chart = Gantt_chart;
            result.method_name = "HRRN";
            return result;
        }

        Result PPRR() {
            std::sort(data.processes.begin(), data.processes.end(), [](const Process &a, const Process &b){
                if (a.priority == b.priority && a.arrival_time == b.arrival_time) return a.id < b.id;
                if (a.arrival_time == b.arrival_time) return a.priority < b.priority;
                return a.arrival_time < b.arrival_time;
            });

            std::string Gantt_chart = "";
            std::vector<Process> results;
            std::pair<int, int> record_time(0, 0); // previous execution time, waiting time
            std::pair<Process, std::pair<int, int>> cur_p; // process and its record_time
            std::queue<std::pair<Process, std::pair<int, int>>> unused_q;
            for (Process p: data.processes) {
                cur_p = std::pair<Process, std::pair<int, int>>(p, record_time);
                unused_q.push(cur_p);
            }
            cur_p = std::pair<Process, std::pair<int, int>>(); // reset
            cur_p.first.id = -1; // reset
            int time_slice = data.time_slice, time_counter = 0;
            std::vector<std::pair<Process, std::pair<int, int>>> waiting_q;

            for (int time = 0; ; time++) {
                while (! unused_q.empty() && unused_q.front().first.arrival_time == time) {
                    auto to_insert = unused_q.front();
                    unused_q.pop();
                    auto it = waiting_q.begin();
                    while (it != waiting_q.end() &&
                           (it->first.priority < to_insert.first.priority
                            || (it->first.priority == to_insert.first.priority
                                && it->first.arrival_time <= to_insert.first.arrival_time))) ++it;
                    waiting_q.insert(it, to_insert);
                }

                // judge current process
                if (cur_p.first.id == -1 || time_counter == time_slice) {
                    if (waiting_q.empty() && cur_p.first.id == -1) { // idle
                        Gantt_chart += "-";
                        continue;
                    }
                    else {
                        if (cur_p.first.id == -1) {
                            cur_p = waiting_q[0];
                            waiting_q.erase(waiting_q.begin());
                        }
                        else if (! waiting_q.empty() || time_counter == time_slice) { // get process
                            if (! waiting_q.empty()) { // time_counter == 0 , time_counter != 0, 2 conditions
                                if (cur_p.first.priority < waiting_q[0].first.priority) { // current process is the most significant
                                    if (time_counter == time_slice) time_counter = 0;
                                }
                                else {
                                    // find position to insert
                                    auto pos = std::find_if(waiting_q.begin(), waiting_q.end(), [&](const std::pair<Process, std::pair<int, int>> &p) {
                                        return p.first.priority > cur_p.first.priority;
                                    });
                                    // judge
                                    if (cur_p.first.priority > waiting_q[0].first.priority) { // more significant process exist, break RR
                                        time_counter = 0;
                                        // insert back
                                        waiting_q.insert(pos, cur_p);
                                        cur_p = waiting_q[0];
                                        waiting_q.erase(waiting_q.begin());
                                    }
                                    else { // same priority
                                        if (time_counter == time_slice) {
                                            time_counter = 0;
                                            // insert back
                                            waiting_q.insert(pos, cur_p);
                                            cur_p = waiting_q[0];
                                            waiting_q.erase(waiting_q.begin());
                                        }
                                        // else keep executing current process
                                    }
                                }
                            }
                            else time_counter = 0; // waiting_q.empty() && time_conter == time_slice
                        }
                        else std::cerr << "ERROR" << std::endl;
                    }
                }
                else { // (cur_p.first.id != -1 && time_counter != time_slice)
                    if (! waiting_q.empty()) {
                        if (! (cur_p.first.priority < waiting_q[0].first.priority)) {
                            // find position to insert
                            auto pos = std::find_if(waiting_q.begin(), waiting_q.end(), [&](const std::pair<Process, std::pair<int, int>> &p) {
                                return p.first.priority > cur_p.first.priority;
                            });
                            // judge
                            if (cur_p.first.priority > waiting_q[0].first.priority) { // more significant process exist, break RR
                                time_counter = 0;
                                // insert back
                                waiting_q.insert(pos, cur_p);
                                cur_p = waiting_q[0];
                                waiting_q.erase(waiting_q.begin());
                            }
                            else { // same priority
                                if (time_counter == time_slice) {
                                    time_counter = 0;
                                    // insert back
                                    waiting_q.insert(pos, cur_p);
                                    cur_p = waiting_q[0];
                                    waiting_q.erase(waiting_q.begin());
                                }
                                // else keep executing current process
                            }
                        }
                    }
                }

                // execute current process
                if (cur_p.first.cpu_burst == cur_p.first.remain_time) cur_p.second.second = time - cur_p.first.arrival_time;
                else cur_p.second.second += (time - cur_p.second.first - 1); // wait += (cur_exec_time - prev_exec_time - 1);
                cur_p.first.remain_time--;
                cur_p.second.first = time; // update previous execute time
                time_counter++;
                Gantt_chart += getOutID(cur_p.first.id);

                // judge end
                if (cur_p.first.remain_time == 0) { // finish
                    cur_p.first.finish_time = time + 1;
                    cur_p.first.turnaround_time = cur_p.first.finish_time - cur_p.first.arrival_time;
                    cur_p.first.waiting_time = cur_p.second.second;
                    results.push_back(cur_p.first);
                    // reset
                    cur_p = std::pair<Process, std::pair<int, int>>();
                    cur_p.first.id = -1;
                    time_counter = 0;
                    // break condition
                    if (unused_q.empty() && waiting_q.empty()) break;
                }
            }

            Result result;
            result.results = results;
            result.Gantt_chart = Gantt_chart;
            result.method_name = "Priority RR";
            return result;
        }

        void writeFile(Result result) {
            // sort by id
            std::sort(result.results.begin(), result.results.end(), [](const Process &a, const Process &b) {
                return a.id < b.id;
            });
            // write file
            std::ofstream ofs("out_" + filename + ".txt");
            if (ofs.is_open()) {
                ofs << result.method_name << std::endl;
                if (result.method_name == "Priority RR") result.method_name = "PPRR";
                ofs << "==        ";
                int blank_num = 4 - result.method_name.size();
                if (blank_num != 0) for (int i = 0; i < blank_num; i++) ofs << " ";
                ofs << result.method_name << "==" << std::endl;
                ofs << result.Gantt_chart << std::endl;
                ofs << "===========================================================" << std::endl << std::endl;
                ofs << "Waiting Time" << std::endl;
                ofs << "ID\t" << result.method_name << std::endl;
                ofs << "===========================================================" << std::endl;
                for (Process p: result.results) ofs << p.id << "\t" << p.waiting_time << std::endl;
                ofs << "===========================================================" << std::endl << std::endl;
                ofs << "Turnaround Time" << std::endl;
                ofs << "ID\t" << result.method_name << std::endl;
                ofs << "===========================================================" << std::endl;
                for (Process p: result.results) ofs << p.id << "\t" << p.turnaround_time << std::endl;
                ofs << "===========================================================" << std::endl << std::endl;
                ofs.close();
            }
        }

        void writeFile(std::vector<Result> results) {
            // sort by id
            for (auto &r: results) {
                std::sort(r.results.begin(), r.results.end(), [](const Process &a, const Process &b) {
                    return a.id < b.id;
                });
            }
            // write file
            std::ofstream ofs("out_" + filename + ".txt");
            if (ofs.is_open()) {
                ofs << "All" << std::endl;
                ofs << "==        FCFS==" << std::endl;
                ofs << results[0].Gantt_chart << std::endl;
                ofs << "==          RR==" << std::endl;
                ofs << results[1].Gantt_chart << std::endl;
                ofs << "==         SJF==" << std::endl;
                ofs << results[2].Gantt_chart << std::endl;
                ofs << "==        SRTF==" << std::endl;
                ofs << results[3].Gantt_chart << std::endl;
                ofs << "==        HRRN==" << std::endl;
                ofs << results[4].Gantt_chart << std::endl;
                ofs << "==        PPRR==" << std::endl;
                ofs << results[5].Gantt_chart << std::endl;
                ofs << "===========================================================" << std::endl << std::endl;
                
                ofs << "Waiting Time" << std::endl;
                ofs << "ID\tFCFS\tRR\tSJF\tSRTF\tHRRN\tPPRR" << std::endl;
                ofs << "===========================================================" << std::endl;
                for (int i = 0; i < data.processes.size(); i++) {
                    ofs << results[0].results[i].id; // ID
                    ofs << "\t" << results[0].results[i].waiting_time; // FCFS
                    ofs << "\t" << results[1].results[i].waiting_time; // RR
                    ofs << "\t" << results[2].results[i].waiting_time; // SJF
                    ofs << "\t" << results[3].results[i].waiting_time; // SRTF
                    ofs << "\t" << results[4].results[i].waiting_time; // HRRN
                    ofs << "\t" << results[5].results[i].waiting_time; // PPRR
                    ofs << std::endl;
                }
                ofs << "===========================================================" << std::endl << std::endl;

                ofs << "Turnaround Time" << std::endl;
                ofs << "ID\tFCFS\tRR\tSJF\tSRTF\tHRRN\tPPRR" << std::endl;
                ofs << "===========================================================" << std::endl;
                for (int i = 0; i < data.processes.size(); i++) {
                    ofs << results[0].results[i].id; // ID
                    ofs << "\t" << results[0].results[i].turnaround_time; // FCFS
                    ofs << "\t" << results[1].results[i].turnaround_time; // RR
                    ofs << "\t" << results[2].results[i].turnaround_time; // SJF
                    ofs << "\t" << results[3].results[i].turnaround_time; // SRTF
                    ofs << "\t" << results[4].results[i].turnaround_time; // HRRN
                    ofs << "\t" << results[5].results[i].turnaround_time; // PPRR
                    ofs << std::endl;
                }
                ofs << "===========================================================" << std::endl << std::endl;

                ofs.close();
            }
        }

    public:
        Methods(Data cur_data, std::string cur_filename) {
            data = cur_data;
            filename = cur_filename;
        }

        void exec() {
            switch(data.method) {
                case 1:
                    writeFile(FCFS());
                    break;
                case 2:
                    writeFile(RR());
                    break;
                case 3:
                    writeFile(SJF());
                    break;
                case 4:
                    writeFile(SRTF());
                    break;
                case 5:
                    writeFile(HRRN());
                    break;
                case 6:
                    writeFile(PPRR());
                    break;
                case 7: {
                        std::vector<Result> results;
                        results.push_back(FCFS());
                        results.push_back(RR());
                        results.push_back(SJF());
                        results.push_back(SRTF());
                        results.push_back(HRRN());
                        results.push_back(PPRR());
                        writeFile(results);
                    }

                    break;
                default:
                    std::cerr << "Error input format!" << std::endl;
                    break;
            }
        }
};

std::vector<int> split(std::string line) {
    std::vector<int> tempTokens;
    std::stringstream ss(line);
    std::string token;
    while (ss >> token) tempTokens.push_back(stoi(token));
    return tempTokens;
}

bool hasInt(const std::string &line) {
    std::istringstream iss(line);
    int num;
    if (iss >> num) return true;
    else return false;
}

int main() {
    std::string filename;
    std::cout << "Please enter File Name (eg. input1 ¡B input1.txt) :";
    std::cin >> filename;
    size_t pos = filename.find(".txt");
    if (pos != std::string::npos) filename.erase(pos, 4);
    std::ifstream ifs(filename + ".txt", std::ios::in | std::ios::binary);
    if (ifs.good()) {
        Data data;
        std::vector<int> temp;
        std::string line;

        std::getline(ifs, line); // get method, time_slice
        temp = split(line);
        data.method = temp[0];
        data.time_slice = temp[1];
        std::getline(ifs, line); // skip column names
        for (int i = 0; std::getline(ifs, line); i++) { // read columns
            if (! hasInt(line)) break;
            temp = split(line);
            Process p;
            p.id = temp[0];
            p.cpu_burst = temp[1];
            p.remain_time = p.cpu_burst;
            p.arrival_time = temp[2];
            p.priority = temp[3];
            data.processes.push_back(p);
        }

        Methods methods(data, filename);
        methods.exec();
    }
    else std::cout << "File " << filename << ".txt doesn\'t exist!" << std::endl;
    
    return 0;
}