#include <cstddef>
#include <vector>
#include <string>
#include <memory>
using namespace std;

typedef vector<unsigned char> nbtp_chunk;

typedef unsigned long nbtp_index;

struct indexed_nbtp_chunk {
  nbtp_chunk data;
  nbtp_index index;
};

struct parallel_settings {
  int read_count;
  int transfer_count;
};

// when an upload is successful, upload process return this result.
struct indexed_key {
  nbtp_index index;
  string url;
};
