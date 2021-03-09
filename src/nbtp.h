#include <vector>
using namespace std;

constexpr auto MSS = 100;
constexpr auto STAT_COMPLETE = 0;
constexpr auto STAT_PROGRESS = 1;
constexpr auto STAT_FAILED = 2;


typedef vector<unsigned char> nbtp_chunk;

struct parallel_settings
{
    int read_count;
    int transfer_count;
};
