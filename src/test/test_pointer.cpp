#include <memory>
#include <iostream>

using namespace std;
struct test {
  unique_ptr<string> data;
  int size;
  test(unique_ptr<string> data, int size) {
    data = move(data);
    size = size;
  }
};

int main() {
  string* a = new string("123456");
  test *b = new test(unique_ptr<string>(a), 10);
  cout << *(b->data);
}