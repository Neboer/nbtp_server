#include "../libs/thread_safe_queue.hpp"
#include <cstdlib>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
using namespace std;
void keep_push(ThreadsafeQueue<int>* list) {
  int cur = 1;
  while (true) {
    list->push(cur);
    cerr << "->" << cur << endl;
    cur++;
    this_thread::sleep_for(chrono::milliseconds(rand()%1000)); 
  }
}

void dotly_read(ThreadsafeQueue<int> *list, vector<int> *result_report) {
  while (true) {
    int a = list->pop();
    result_report->push_back(a);
    cerr << "<-" << a << endl;
    this_thread::sleep_for(chrono::milliseconds(rand()%1000));
  }
}

void report_result(vector<int> &result_list) {
  cout << "start reporting... \n";
  char head = '.';
  while (true) {
    head = (head == '.' ? ' ' : '.');
    cerr << head << " ";
    for(int i : result_list){
      cerr << i << ", ";
    }
    cerr << "\r";
    this_thread::sleep_for(100ms);
  }
}

void report_result() {
  while(true){
    
  }
}

int main() {
  ThreadsafeQueue<int> num_list(5);
  vector<int> result_list;
  thread push_thread(keep_push, &num_list);
  thread read_thread(dotly_read, &num_list, &result_list);
  report_result();
}
