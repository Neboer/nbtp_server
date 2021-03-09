#include <cstdio>
#include "image_process/encoder.h"
#include "libs/thread_safe_queue.hpp"
#include <future>
#include <string>
#include <thread>
#include <vector>

using namespace std;
void generator_manager(function<nbtp_chunk&()> generator, ThreadsafeQueue<nbtp_chunk&> &generate_queue, const int max_queue_size, bool &generate_is_quitted)
{
    generate_is_quitted = false;
    nbtp_chunk& previous_generated_val = generator();
    while (previous_generated_val.size())
    {
        if (generate_queue.size() < max_queue_size)
        {
            generate_queue.push(previous_generated_val);
            previous_generated_val = generator();
        }
    }
    generate_is_quitted = true;
    return;
    // 如果generator表示自己耗尽，那么此线程退出。
}

void transformer_manager(function<string &(nbtp_chunk&)> transformer, vector<promise<string &>> &waiting_upload_queue, function<void(string &)> key_sender, bool &converter_is_quitted, bool &uploader_is_quitted)
{
    uploader_is_quitted = false;
    // 和上面那位不同，这里所有的操作都是异步的。一旦有一个新的key从消息队列里拿了下来，那么就通过sender直接发送给接收端。
    while (!converter_is_quitted || waiting_upload_queue.size()) // 如果转换进程已经退出（不会有新的待发送块添加到缓冲池中），而且缓冲池已经空了，那么是时候退出了。否则继续执行。
    {
        for (size_t i = 0; i < waiting_upload_queue.size(); i++)
        {
            // 依次判断给定进程是否执行完毕。
            auto current_future_obj = waiting_upload_queue[i].get_future();
            if (current_future_obj.wait_for(0ms) == future_status::ready) // 如果有一个上传进程已经准备就绪，那就派发结果，放心，会有人及时用新的进程将它填满的。
            {
                string &result_key = current_future_obj.get();
                key_sender(result_key);
                waiting_upload_queue.erase(waiting_upload_queue.begin() + i); // 删去队列中的已完成对象，等待新线程作用
            }
        }
    }
    uploader_is_quitted = true;// 一般来讲，如果uploader退出了，程序也就即将结束了。
}

// 程序不提供可以自由定义convert方法的功能，因为确实没有这种必要。
void converter_manager(ThreadsafeQueue<nbtp_chunk&>& generate_queue ,vector<promise<string &>> &waiting_upload_queue, bool &generator_is_quitted, bool &converter_is_quitted, int max_upload_tasks_count){
    converter_is_quitted = false;
    while (!generator_is_quitted || generate_queue.size())
    {
        if (generate_queue.size() && waiting_upload_queue.size() < max_upload_tasks_count)
        {
            // 存在输入，且输出缓冲有空，赶紧加工成图片！
            nbtp_chunk& data_wait_for_encode = generate_queue.pop(); // 先拿出一块数据
            nbtp_chunk& png_data = encode_to_png(data_wait_for_encode);
            

        }
        
    }
    converter_is_quitted = true;
}

void upload(function<nbtp_chunk&()> generator, function<string &(nbtp_chunk&)> transformer, function<void(string &)> key_sender, parallel_settings parallel)
{
    // generator和transformer都是异步函数，他们的实现完全异步。
    ThreadsafeQueue<nbtp_chunk&> generate_queue;              //读取文件的队列，是swap1，压力马斯内！
    ThreadsafeQueue<promise<string &>> waiting_upload_queue; //上传文件的等待队列，是swap2。
}
