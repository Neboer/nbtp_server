#pragma once
#include <memory>
#include <string>
#include <vector>

using namespace std;

typedef vector<unsigned char> nbtp_chunk;

typedef unsigned long nbtp_index;

struct indexed_nbtp_chunk {
public:
  indexed_nbtp_chunk() {
    index = 0; // 创建一个空的对象，表示一些特殊的含义。
  }
  indexed_nbtp_chunk(nbtp_index index, unique_ptr<nbtp_chunk> data) {
    this->index = index;
    this->data = move(data);
  }
  nbtp_index index;
  unique_ptr<nbtp_chunk> data;
};

struct indexed_key_str {
  indexed_key_str() { this->index = 0; }
  indexed_key_str(nbtp_index index, unique_ptr<string> key) {
    this->index = index;
    this->key = move(key);
  }
  nbtp_index index;
  unique_ptr<string> key;
};

struct parallel_settings {
  int read_count;
  int transfer_count;
};
