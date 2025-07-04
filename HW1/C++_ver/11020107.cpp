#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <unistd.h>
#include <queue>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <iconv.h>
#include <thread>
#include <mutex>
#include <future>
using namespace std;

string convBig5(string &big5String) {
    iconv_t cd = iconv_open("UTF-8", "BIG5");
    
    const char *in = big5String.c_str();
    size_t len = big5String.length();

    string output;
    output.resize(len*2);

    char *output_ptr = &output[0];
    size_t output_len = output.size();

    if (iconv(cd, (char**)&in, &len, &output_ptr, &output_len) == (size_t) - 1) {
        cerr << "Error" << endl;
        iconv_close(cd);
        return "";
    }

    iconv_close(cd);
    output.resize(output.size() - output_len);
    return output;
}

struct Core {
    string filename;
    int k;
    int n;
    double time;
    int method;
    vector<int> data;
    vector<vector<int>> sublists;
};

class Method1 {
    public:
        Core core;

        Method1(string filename, int k, int method) {
            core.k = k;
            core.method = method;
            core.sublists.clear();
            ifstream file(filename + ".txt");
            int temp;
            while (file >> temp) core.data.push_back(temp);
            core.n = core.data.size();
            core.filename = filename + "_output" + to_string(core.method) + ".txt";
        }

        void bubbleSort(vector<int> &arr) {
            int len = arr.size();
            bool swapped = true;
            while (swapped) {
                swapped = false;
                int last_swap_index = 0;
                for (int i = 0; i < len; i++) {
                    if (arr[i-1] > arr[i]) {
                        int temp = arr[i-1];
                        arr[i-1] = arr[i];
                        arr[i] = temp;
                        swapped = true;
                        last_swap_index = i;
                    }
                }
                len = last_swap_index;
            }
        }

        void writeFile() {
            ofstream file(core.filename);
            file << "Sort : " << endl;
            for (int temp: core.data) file << temp << endl;
            file << "CPU Time : " << core.time << endl;

            auto now = chrono::system_clock::now();
            auto now_time = chrono::system_clock::to_time_t(now);
            file << "Output Time : " << put_time(localtime(&now_time), "%Y-%m-%d %H:%M:%S") << ".";
            auto us = chrono::duration_cast<chrono::microseconds>(now.time_since_epoch()) % 1000000;
            file << setw(6) << setfill('0') << us.count() << "+08:00" << endl;
            file.close();
        }

        virtual void exec() {
            chrono::steady_clock::time_point start = chrono::steady_clock::now();
            
            bubbleSort(core.data);
            
            chrono::steady_clock::time_point end = chrono::steady_clock::now();
            core.time = chrono::duration<double, milli> (end - start).count();
            writeFile();
        }
};

class Method2: public Method1 {
    public:
        Method2(string filename, int k, int method): Method1(filename, k, method) {
        }

        vector<int> merge(vector<int> &left, vector<int> &right) {
            vector<int> result;
            int i = 0, j = 0, len_left = left.size(), len_right = right.size();
            result.reserve(len_left + len_right);
            while (i < len_left && j < len_right) {
                if (left[i] < right[j]) {
                    result.push_back(left[i]);
                    i++;
                }
                else {
                    result.push_back(right[j]);
                    j++;
                }
            }

            for (; i < len_left; i++) result.push_back(left[i]);
            for (; j < len_right; j++) result.push_back(right[j]);

            return result;
        }

        vector<int> mergeSort(vector<int> &arr) {
            if (arr.size() <= 1) return arr;
            int mid = arr.size()/ 2;
            vector<int> left(arr.begin(), arr.begin() + mid), right(arr.begin() + mid, arr.end());
            left = mergeSort(left);
            right = mergeSort(right);
            return merge(left, right);
        }

        void splitData() {
            int a = core.data.size() / core.k;
            int b = core.data.size() % core.k;
            for (int i = 0; i < core.k; i++) {
                int start = i * a + min(i, b);
                int end = (i + 1) * a + min(i + 1, b);
                core.sublists.insert(core.sublists.begin(), vector<int>(core.data.begin() + start, core.data.begin() + end));
            }
        }

        void exec() override { // Shared Memory Model
            splitData();
            chrono::steady_clock::time_point start = chrono::steady_clock::now(); // start

            // create shared memory block
            int shm_fd = shm_open("/shared_memory", O_CREAT | O_RDWR, 0666); // 0666: r/w mode
            ftruncate(shm_fd, core.n * sizeof(int)); // set size
            // NULL: addr. choose by sys, shm's size, permission: r/w, share for all processes, to-share data, share from beginning
            int *shared_data = (int *) mmap(NULL, core.n *sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
            
            pid_t pid = fork();
            if (pid < 0) {
                cerr << "Fork failed." << endl;
                exit(1);
            }
            else if (pid == 0) { // child
                for (auto &sublist: core.sublists) bubbleSort(sublist);
                vector<int> sorted_data;
                for (auto &sublist: core.sublists)
                    for (auto &line: sublist)
                        sorted_data.push_back(line);
                sorted_data = mergeSort(sorted_data);
                // copy to shared memory
                memcpy(shared_data, sorted_data.data(), sorted_data.size() * sizeof(int));
                exit(0);
            }
            else { // parent
                wait(NULL); // wait for child process to finish

                // read data from shared memory
                core.data.resize(core.n);
                memcpy(core.data.data(), shared_data, core.n * sizeof(int));

                // clean shared memory
                munmap(shared_data, core.n * sizeof(int));
                close(shm_fd);
                shm_unlink("/shared_memory");
                
                chrono::steady_clock::time_point end = chrono::steady_clock::now(); // end
                core.time = chrono::duration<double, milli> (end - start).count();
                writeFile();
            }
        }
};

class Method3: public Method2 {
    public:
        Method3(string filename, int k, int method): Method2(filename, k, method) {
        }

        void exec() override {
            splitData();
            chrono::steady_clock::time_point start = chrono::steady_clock::now(); // start

            // core.k processes do bubble sort
            vector<int *> bubble_shared_data(core.k, nullptr); // store pointer points to shared memories
            for (int i = 0; i < core.k; i++) {
                string shm_name = "/bubble_shared_memory_" + to_string(i); // to differ shared memories
                int shm_fd = shm_open(shm_name.c_str(), O_CREAT | O_RDWR, 0666);
                int shm_size = core.sublists[i].size() * sizeof(int);
                ftruncate(shm_fd, shm_size);
                bubble_shared_data[i] = (int *) mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

                pid_t pid = fork();
                if (pid < 0) {
                    cerr << "Child process " << i << " while bubble sort failed." << endl;
                    exit(1);
                }
                else if (pid == 0) { // child processes
                    bubbleSort(core.sublists[i]); // bubble sort

                    memcpy(bubble_shared_data[i], core.sublists[i].data(), shm_size); // copy to shared memory
                    exit(0);
                }
            }

            for (int i = 0; i < core.k; i++) wait(NULL); // main process wait for k processes finish
            for (int i = 0; i < core.k; i++) {
                int shm_size = core.sublists[i].size() * sizeof(int);
                memcpy(core.sublists[i].data(), bubble_shared_data[i], shm_size); // copy data from shared memories to core.data
                munmap(bubble_shared_data[i], shm_size);
                string shm_name = "/bubble_shared_memory_" + to_string(i);
                shm_unlink(shm_name.c_str());
            }

            // core.k-1 processes do merge sort
            vector<int *> merge_shared_data(core.k, nullptr);
            // prepare first data
            string first_shm_name = "/merge_shared_memory_0";
            int first_shm_fd = shm_open(first_shm_name.c_str(), O_CREAT | O_RDWR, 0666);
            int first_shm_size = core.sublists[0].size() * sizeof(int);
            ftruncate(first_shm_fd, first_shm_size);
            merge_shared_data[0] = (int *) mmap(NULL, first_shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, first_shm_fd, 0);
            memcpy(merge_shared_data[0], core.sublists[0].data(), first_shm_size);

            int arr_size = core.sublists[0].size();
            for (int i = 1; i < core.k; i++) {
                // read prev data from previous shared memory to prev_arr
                string prev_shm_name = "/merge_shared_memory_" + to_string(i-1);
                int prev_shm_fd = shm_open(prev_shm_name.c_str(), O_CREAT | O_RDWR, 0666);
                int prev_shm_size = arr_size * sizeof(int);
                ftruncate(prev_shm_fd, prev_shm_size);
                merge_shared_data[i-1] = (int *) mmap(NULL, prev_shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, prev_shm_fd, 0);
                vector<int> prev_arr(arr_size);
                memcpy(prev_arr.data(), merge_shared_data[i-1], prev_shm_size);

                // create shared memory after merged
                arr_size += core.sublists[i].size(); // update size that after being merged
                int arr_size_after_merge = arr_size * sizeof(int);
                string shm_name = "/merge_shared_memory_" + to_string(i);
                int shm_fd = shm_open(shm_name.c_str(), O_CREAT | O_RDWR, 0666);
                ftruncate(shm_fd, arr_size_after_merge);
                merge_shared_data[i] = (int *) mmap(NULL, arr_size_after_merge, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

                pid_t pid = fork();
                if (pid < 0) {
                    cerr << "Child process " << i << " while merge sort failed." << endl;
                    exit(1);
                }
                else if (pid == 0) { // child process (process i)
                    vector<int> merged = merge(prev_arr, core.sublists[i]);
                    memcpy(merge_shared_data[i], merged.data(), arr_size_after_merge);
                    exit(0);
                }
                else { // parent process, wait the previous process finish, IMPORTANT !!!
                    int status;
                    waitpid(pid, &status, 0);
                }
            }

            core.data.resize(core.n);
            memcpy(core.data.data(), merge_shared_data[core.k-1], core.n * sizeof(int));
            for (int i = 0, temp_size = core.sublists[0].size() * sizeof(int); i < core.k; i++) {
                munmap(merge_shared_data[i], first_shm_size);
                string shm_name = "/merge_shared_memory_" + to_string(i);
                shm_unlink(shm_name.c_str());
                if (i != core.k-1) temp_size += core.sublists[i+1].size() * sizeof(int);
            }
            
            chrono::steady_clock::time_point end = chrono::steady_clock::now(); // end
            core.time = chrono::duration<double, milli> (end - start).count();
            writeFile();
        }
};

class Method4: public Method3 {
    public:
        Method4(string filename, int k, int method): Method3(filename, k, method) {
        }

        void exec() override {
            splitData();
            chrono::steady_clock::time_point start = chrono::steady_clock::now(); // start

            // bubble sort with core.k threads
            vector<thread> bubble_threads;
            for (int i = 0; i < core.k; i++) {
                bubble_threads.emplace_back([this, i]() {
                    bubbleSort(core.sublists[i]);
                });
            }

            for (auto &thread : bubble_threads) thread.join();

            // merge sort with core.k-1 threads
            vector<thread> merge_threads;
            for (int i = 1; i < core.k; i++) {
                merge_threads.emplace_back([this, i]() {
                    vector<int> merged = merge(core.sublists[i-1], core.sublists[i]);
                    core.sublists[i] = merged;
                });

                merge_threads.back().join(); // wait the previous thread finish
            }

            core.data = core.sublists.back();

            chrono::steady_clock::time_point end = chrono::steady_clock::now(); // end
            core.time = chrono::duration<double, milli> (end - start).count();
            writeFile();
        }
};

void exp() {
    vector<string> names = {"input_1w", "input_10w", "input_50w", "input_100w"};
    vector<int> ks = {100, 200, 300, 400};
    vector<int> methods = {1, 2, 3, 4};
    Method1 *obj;

    for (string filename: names) {
        for (int k: ks) {
            for (int method: methods) {
            switch(method) {
                case 1:
                    obj = new Method1(filename, k, method);
                    break;
                case 2:
                    obj = new Method2(filename, k, method);
                    break;
                case 3: // pass
                    obj = new Method3(filename, k, method);
                    break;
                case 4: // pass
                    obj = new Method4(filename, k, method);
                    break;
                default:
                    cout << "NOT SUCH METHOD!!!" << endl;
                    break;
            }

            cout << "Executing: " << filename << ", n = " << obj->core.n << ", k = " << obj->core.k << ", method = " << method << endl;

            obj->exec();
            
            ofstream file("exp.txt", ios_base::app);
            file << "n = " << obj->core.n << ", k = " << obj->core.k;
            file << ", method = " << method << ", time: " << obj->core.time << " ms" << endl;
            
            delete obj;
            obj = nullptr;
            }
        }
    }
}

int main() {

    // exp();

    string filename, str1 = "請輸入檔案名稱:", str2 = "請輸入要切成幾份:", str3 = "請輸入方法編號:(方法1, 方法2, 方法3, 方法4)";
    cout << convBig5(str1) << endl;
    cin >> filename;
    int k;
    cout << convBig5(str2) << endl;
    cin >> k;
    int method;
    cout << convBig5(str3) << endl;
    cin >> method;

    Method1 *obj;
    switch(method) {
        case 1:
            obj = new Method1(filename, k, method);
            break;
        case 2:
            obj = new Method2(filename, k, method);
            break;
        case 3: // pass
            obj = new Method3(filename, k, method);
            break;
        case 4: // pass
            obj = new Method4(filename, k, method);
            break;
        default:
            cout << "NOT SUCH METHOD!!!" << endl;
            break;
    }

    obj->exec();

    delete obj;
    obj = nullptr;

    return 0;
}