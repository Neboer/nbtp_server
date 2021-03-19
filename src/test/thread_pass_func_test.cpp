#include <thread>
#include <iostream>
#include <functional>

using namespace std;
int what(int *a) {
  return (*a + 1);
}

void test(function<int(int*)> input) {
  cout << "start" << endl;
  int p = 1;
  int a = input(&p);
  cout << a;
}

int main() {
  thread* th = new thread(test, what);
  th->join();
}
