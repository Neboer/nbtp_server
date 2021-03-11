#include <cstdio>
#include "image_process/encoder.h"
#include "libs/thread_safe_queue.hpp"
#include <future>
#include <string>
#include <thread>
#include <vector>

using namespace std;
void generator_manager(function<nbtp_chunk *()> generator, ThreadsafeQueue<nbtp_chunk *> &generate_queue, const int max_queue_size, bool &generate_is_quitted)
{
    nbtp_chunk *previous_generated_val = generator();
    while ((*previous_generated_val).size())
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

// buffer2到key的过程，waiting_upload_queue是等待上传的、已经转换完成的png块的队列，在这里维护了一个upload_task_list，存储着需要等待的上传任务列表。
void transformer_manager(function<promise<string &>(nbtp_chunk *)> transformer, ThreadsafeQueue<nbtp_chunk *> &waiting_upload_queue, function<void(string &)> key_sender, bool &converter_is_quitted, bool &uploader_is_quitted, int concurrent_upload_proc_count)
{
    // 和上面那位不同，这里所有的操作都是异步的。一旦有一个新的key从消息队列里拿了下来，那么就通过sender直接发送给接收端。
    vector<promise<string &>> upload_task_list;
    while (!converter_is_quitted || waiting_upload_queue.size()) // 如果转换进程已经退出（不会有新的待发送块添加到缓冲池中），而且缓冲池已经空了，那么是时候退出了。否则继续执行。
    {
        for (size_t i = 0; i < upload_task_list.size(); i++)
        {
            // 依次判断给定进程是否执行完毕。
            auto current_future_obj = upload_task_list[i].get_future();
            if (current_future_obj.wait_for(0ms) == future_status::ready) // 如果有一个上传进程已经准备就绪，立即派发结果。
            {
                string &result_key = current_future_obj.get();
                key_sender(result_key);
                upload_task_list.erase(upload_task_list.begin() + i); // 删去队列中的已完成对象。
            }
        }
        if (upload_task_list.size() < concurrent_upload_proc_count && waiting_upload_queue.size() > 0) // 如果同时上传的并行任务数不够，同时外部队列里有新的数据等待上传……
        {
            // 启动新的上传进程！同时将这个进程推入到等待上传的块的列表中。
            upload_task_list.push_back(transformer(waiting_upload_queue.pop()));
        }
    }
    uploader_is_quitted = true; // 一般来讲，如果uploader退出了，程序也就即将结束了。
}

// 程序不提供可以自由定义convert方法的功能，因为确实没有这种必要，buffer1到buffer2的过程。
void converter_manager(ThreadsafeQueue<nbtp_chunk *> &generate_queue, ThreadsafeQueue<nbtp_chunk *> &waiting_upload_queue, bool &generator_is_quitted, bool &converter_is_quitted, int max_waiting_upload_blocks_count)
{
    while (!generator_is_quitted || generate_queue.size())
    {
        if (generate_queue.size() && waiting_upload_queue.size() < max_waiting_upload_blocks_count)
        {
            // 存在输入，且输出缓冲有空，赶紧加工成图片！
            nbtp_chunk *data_wait_for_encode = generate_queue.pop(); // 先拿出一块数据
            nbtp_chunk *png_data = encode_to_png(data_wait_for_encode);
            delete data_wait_for_encode;
            waiting_upload_queue.push(png_data);
        }
    }
    converter_is_quitted = true;
}

void upload(function<nbtp_chunk *()> generator, function<string &(nbtp_chunk *)> transformer, function<void(string &)> key_sender, parallel_settings parallel)
{
    // generator和transformer都是异步函数，他们的实现完全异步。
    ThreadsafeQueue<nbtp_chunk *> generate_queue;       //读取文件的队列，是swap1，压力马斯内！
    ThreadsafeQueue<nbtp_chunk *> waiting_upload_queue; //上传文件的等待队列，是swap2。
    bool generator_thread_is_quit = false, transform_thread_is_quit = false, convert_thread_is_quit = false;
    thread *generator_thread = new thread(generator_manager, generator, generate_queue, parallel.read_count, generator_thread_is_quit);
    thread *converter_thread = new thread(converter_manager, generate_queue, waiting_upload_queue, generator_thread_is_quit, convert_thread_is_quit, parallel.read_count); // 最大的buffer2的空间应该和buffer1的空间相等，不用再进行设置啦。
    thread *transformer_thread = new thread(transformer_manager, waiting_upload_queue, key_sender, convert_thread_is_quit, transform_thread_is_quit, parallel.transfer_count);
    generator_thread->join();
    converter_thread->join();
    transformer_thread->join();
}
