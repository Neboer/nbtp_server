#include "nbtp.h"
#include <string>
#include <functional>
#include <fstream>
using namespace std;
// 一个基于文件制作生成器的工具。其实也可以基于socket来生成。
function<nbtp_chunk *()> file_generator_maker(string &filename, size_t block_size)
{
    // 先打开文件，然后准备读取。
    ifstream infile(filename, ifstream::binary|ifstream::in);
    return [&infile, block_size](void) {
        vector<unsigned char> *current_chunk = (vector<unsigned char> *)new vector(block_size, 0);
        infile.read((char *)&(current_chunk[0]), block_size);
        return current_chunk;
    };
}