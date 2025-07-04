#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

// structures -------------------------------------
struct Data { // single line of a method
    int ref; // page reference
    std::string frame; // page frame
    std::string fault; // page fault
    Data (int r): ref(r), frame(""), fault("") {} // init
};

struct SUM_UP { // last line of a method (counts in int)
    int faults; // page faults' count
    int replaces; // page replacements' count
    int frames; // limited frames
    SUM_UP (int f): faults(0), replaces(0), frames(f) {} // init
};

// define function type ---------------------------
using pageReplacementFunction = void(*)(std::vector<Data>&, SUM_UP&);

// methods ----------------------------------------
void FIFO(std::vector<Data> &data, SUM_UP &sum_up) {
    std::vector<int> q;
    for (Data &d: data) {
        if (std::find_if(q.cbegin(), q.cend(), [&](int page) { return page == d.ref; }) == q.cend()) {
            if (q.size() == sum_up.frames) {
                // delete
                q.pop_back();
                // record
                ++sum_up.replaces;
            }

            ++sum_up.faults;
            q.insert(q.begin(), d.ref);
            d.fault = "F";
        }

        for (int page: q) d.frame += std::to_string(page); // draw queue
    }
}

void LRU(std::vector<Data> &data, SUM_UP &sum_up) {
    std::vector<int> q;
    for (Data &d: data) {
        auto it = std::find_if(q.cbegin(), q.cend(), [&](int page) { return page == d.ref; });
        if (it == q.cend()) {
            if (q.size() == sum_up.frames) {
                // delete
                q.pop_back();
                // record
                ++sum_up.replaces;
            }

            ++sum_up.faults;
            q.insert(q.begin(), d.ref);
            d.fault = "F";
        }
        else { // LRU
            q.erase(it);
            q.insert(q.begin(), d.ref);
        }

        for (int page: q) d.frame += std::to_string(page); // draw queue
    }
}

void LFU_FIFO(std::vector<Data> &data, SUM_UP &sum_up) {
    std::vector<int> q;
    std::unordered_map<int, int> counts;
    for (Data &d: data) {
        ++counts[d.ref];

        auto it = std::find_if(q.cbegin(), q.cend(), [&](int page) { return page == d.ref; });
        if (it == q.cend()) {
            if (q.size() == sum_up.frames) { // LFU
                // find the minimal count
                int min_ref = *(std::min_element(q.rbegin(), q.rend(), [&](int a, int b) {
                    return counts[a] < counts[b];
                }));
                // find reversely
                auto to_del = std::find_if(q.rbegin(), q.rend(), [&](int page) {
                    if (page == min_ref) return true;
                    else return false;
                }).base() - 1;
                // clear count
                counts[min_ref] = 0;
                // delete
                q.erase(to_del);
                // record
                ++sum_up.replaces;
            }

            ++sum_up.faults;
            q.insert(q.begin(), d.ref);
            d.fault = "F";
        }

        for (int page: q) d.frame += std::to_string(page); // draw queue
    }
}

void MFU_FIFO(std::vector<Data> &data, SUM_UP &sum_up) {
    std::vector<int> q;
    std::unordered_map<int, int> counts;
    for (Data &d: data) {
        ++counts[d.ref];

        auto it = std::find_if(q.cbegin(), q.cend(), [&](int page) { return page == d.ref; });
        if (it == q.cend()) {
            if (q.size() == sum_up.frames) { // MFU
                // find the maximal count
                int min_ref = *(std::max_element(q.rbegin(), q.rend(), [&](int a, int b) {
                    return counts[a] < counts[b];
                }));
                // find reversely
                auto to_del = std::find_if(q.rbegin(), q.rend(), [&](int page) {
                    if (page == min_ref) return true;
                    else return false;
                }).base() - 1;
                // clear count
                counts[min_ref] = 0;
                // delete
                q.erase(to_del);
                // record
                ++sum_up.replaces;
            }

            ++sum_up.faults;
            q.insert(q.begin(), d.ref);
            d.fault = "F";
        }

        for (int page: q) d.frame += std::to_string(page); // draw queue
    }
}

void LFU_LRU(std::vector<Data> &data, SUM_UP &sum_up) {
    std::vector<int> q;
    std::unordered_map<int, int> counts;
    for (Data &d: data) {
        ++counts[d.ref];

        auto it = std::find_if(q.cbegin(), q.cend(), [&](int page) { return page == d.ref; });
        if (it == q.cend()) {
            if (q.size() == sum_up.frames) { // LFU
                // find the minimal count
                int min_ref = *(std::min_element(q.rbegin(), q.rend(), [&](int a, int b) {
                    return counts[a] < counts[b];
                }));
                // find reversely
                auto to_del = std::find_if(q.rbegin(), q.rend(), [&](int page) {
                    if (page == min_ref) return true;
                    else return false;
                }).base() - 1;
                // clear count
                counts[min_ref] = 0;
                // delete
                q.erase(to_del);
                // record
                ++sum_up.replaces;
            }

            ++sum_up.faults;
            q.insert(q.begin(), d.ref);
            d.fault = "F";
        }
        else { // LRU
            q.erase(it);
            q.insert(q.begin(), d.ref);
        }

        for (int page: q) d.frame += std::to_string(page); // draw queue
    }
}

// write ------------------------------------------
void writeFile(std::ofstream &ofs, std::string header, std::vector<Data> data, SUM_UP sum_up) {
    ofs << header << "\n";
    for (Data d: data) {
        ofs << d.ref << "\t" << d.frame;
        if (d.fault != "") ofs << "\t" << d.fault;
        ofs << "\n";
    }
    ofs << "Page Fault = " << sum_up.faults << "  Page Replaces = " << sum_up.replaces << "  Page Frames = " << sum_up.frames << "\n";
}

// main -------------------------------------------
int main() {
    // init
    init:
    std::string filename = "", line = "";
    std::cout << "Please enter File Name (eg. input1 ¡B input1.txt): ";
    std::cin >> filename;
    size_t pos = filename.find('.');
    if (pos != std::string::npos) filename = filename.substr(0, pos);
    std::ifstream ifs(filename + ".txt");
    if (! ifs.good()) {
        std::cout << "File " << filename << ".txt doesn\'t exist!" << std::endl;
        goto init;
    }
    
    // read file
    int method, pageFrame;
    std::getline(ifs, line);
    std::istringstream iss(line);
    iss >> method >> pageFrame;
    std::getline(ifs, line);
    std::vector<Data> data;
    for (char ch: line) data.push_back(Data(ch - '0'));
    ifs.close();

    // prepare headers
    std::vector<std::string> headers = {
        "--------------FIFO-----------------------",
        "--------------LRU-----------------------",
        "--------------Least Frequently Used Page Replacement-----------------------",
        "--------------Most Frequently Used Page Replacement -----------------------",
        "--------------Least Frequently Used LRU Page Replacement-----------------------"
    };

    // prepare functions
    std::vector<pageReplacementFunction> functions = {
        FIFO,
        LRU,
        LFU_FIFO,
        MFU_FIFO,
        LFU_LRU
    };

    // prepare output file stream
    if (std::ifstream("out_" + filename + ".txt")) std::remove(("out_" + filename + ".txt").c_str());
    std::ofstream ofs("out_" + filename + ".txt", std::ios::app);

    // exec
    switch (method) {
        case 6: { // All
            for (int h = 0; h < 5; ++h) {
                // exec
                SUM_UP sum_up(pageFrame);
                functions[h](data, sum_up);
                
                // write
                writeFile(ofs, headers[h], data, sum_up);
                
                // clear
                for (Data &d: data) {
                    d.frame = "";
                    d.fault = "";
                }

                // new line
                if (h < 4) ofs << "\n";
            }

            break;
        }
        default: {
            SUM_UP sum_up(pageFrame);
            functions[method-1](data, sum_up);
            writeFile(ofs, headers[method-1], data, sum_up);
            break;
        }
    }

    ofs.close();
    return 0;
}