#include "../libs/thread_safe_queue.hpp"
#include <iostream>
#include <thread>
#include <vector>
using namespace std;
void keep_push(ThreadsafeQueue<int>* list) {
  int cur = 1;
  while (true) {
    cout << cur;
    list->push(cur++);
    cout << ".";
  }
}

void dotly_read(ThreadsafeQueue<int> *list, vector<int> *result_report) {
  while (true) {
    int a = list->pop();
    result_report->push_back(a);
    this_thread::sleep_for(100ms);
  }
}

void report_result(vector<int> &result_list) {
  cout << "start reporting... \n";
  char head = '.';
  while (true) {
    if (head == '.') {
      head = ' ';
    }
    cout << head << " ";
    for(int i : result_list){
      cout << i << ", ";
    }
    cout << "\r";
    this_thread::sleep_for(100ms);
  }
}

void report_result() {
  while(true){
    
  }
}

int main() {
  ThreadsafeQueue<int> num_list(10);
  vector<int> result_list;
  thread push_thread(keep_push, &num_list);
  thread read_thread(dotly_read, &num_list, &result_list);
  report_result();
}
