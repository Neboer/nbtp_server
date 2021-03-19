#include "image_process/encoder.h"
#include "libs/thread_safe_queue.hpp"
#include "nbtp.h"
#include <cstdio>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

using namespace std;
void generator_manager(function<indexed_nbtp_chunk()> generator,
                       ThreadsafeQueue<indexed_nbtp_chunk> *generate_queue) {
  indexed_nbtp_chunk last_generated_result;
  do {
    last_generated_result = generator();
    generate_queue->push(last_generated_result);
  } while (last_generated_result.index);
}

// buffer2到key的过程，waiting_upload_queue是等待上传的、已经转换完成的png块的队列，在这里维护了一个upload_task_list，存储着需要等待的上传任务列表。
void transformer_manager(
    function<int(indexed_nbtp_chunk)> start_upload_bk, // 非阻塞的启动上传
    function<indexed_key_str()>
        wait_for_an_result, // 阻塞等待之前安排的所有上传任务
    ThreadsafeQueue<indexed_nbtp_chunk> *waiting_upload_queue,
    function<void(indexed_key_str)> key_sender,
    int concurrent_upload_proc_count) {

  indexed_nbtp_chunk current_upload_task = waiting_upload_queue->pop();
  int current_upload_count = 1;
  while (current_upload_count >
         0) { // 只要还有正在上传的任务，主循环就不能停下。
    while (current_upload_count < concurrent_upload_proc_count &&
           current_upload_task.index) {
      current_upload_count = start_upload_bk(move(
          current_upload_task)); // 在下面的阻塞等待中，一旦有一个上传完成，立刻准备拿出新的开始上传，同时也可以等之前的转换一段时间。
      current_upload_task = waiting_upload_queue->pop();
    }
    indexed_key_str key =
        wait_for_an_result(); // 等待一个上传结果，如果有则立即返回
    key_sender(move(key));
  }
}

// 程序不提供可以自由定义convert方法的功能，因为确实没有这种必要，buffer1到buffer2的过程。
void converter_manager(
    ThreadsafeQueue<indexed_nbtp_chunk> *generate_queue,
    ThreadsafeQueue<indexed_nbtp_chunk> *waiting_upload_queue) {

  indexed_nbtp_chunk current_ori_data = generate_queue->pop();
  while (current_ori_data.index) {
    unique_ptr<nbtp_chunk> current_converted_chunk =
        encode_to_png(move(current_ori_data.data));
    indexed_nbtp_chunk current_converted_data = indexed_nbtp_chunk(
        current_ori_data.index, move(current_converted_chunk));
    waiting_upload_queue->push(current_converted_data);
  }
  waiting_upload_queue->push(indexed_nbtp_chunk());// 一旦到队列末尾，放入一个空的符号标记队列结束。
}
