#include "image_process/encoder.h"
#include "libs/thread_safe_queue.hpp"
#include <cstdio>
#include <functional>
#include <future>
#include <string>
#include <thread>
#include <vector>

using namespace std;

void generator_manager(function<nbtp_chunk *()> generator,
                       ThreadsafeQueue<nbtp_chunk *> *generate_queue) {
  // when the generator is over, the process will insert a nullptr into
  // generator
  nbtp_chunk *last_generate_result = generator();
  while (last_generate_result) {
    generate_queue->push(last_generate_result);
  }
  generate_queue->push(nullptr);
  // when thread is quitting, push a nullptr into it to inform the process is
  // over.
}

void transformer_manager(ThreadsafeQueue<nbtp_chunk *> *waiting_upload_queue,
                         function<int(nbtp_chunk *)> add_task,
                         function<indexed_key*()> wait_one_result,
			 function<void(indexed_key*)> result_reporter) {
  
}

// 程序不提供可以自由定义convert方法的功能，因为确实没有这种必要，buffer1到buffer2的过程。
void converter_manager(ThreadsafeQueue<indexed_nbtp_chunk *> &generate_queue,
                       ThreadsafeQueue<indexed_nbtp_chunk *> &waiting_upload_queue) {
  indexed_nbtp_chunk *data_wait_for_encode = generate_queue.pop();
  while (data_wait_for_encode) {
    nbtp_chunk *png_data = encode_to_png(&data_wait_for_encode->data);
    delete &data_wait_for_encode->data;
    indexed_nbtp_chunk* indexed_encoded_data = new indexed_nbtp_chunk{}; 
    waiting_upload_queue.push(png_data);
  }
}

void upload(function<indexed_nbtp_chunk *()> generator,
            function<string &(indexed_nbtp_chunk *)> transformer,
            function<void(string &)> key_sender, parallel_settings parallel) {
  // generator和transformer都是异步函数，他们的实现完全异步。
  ThreadsafeQueue<indexed_nbtp_chunk *>
      generate_queue; //读取文件的队列，是swap1，压力马斯内！
  ThreadsafeQueue<indexed_nbtp_chunk *>
      waiting_upload_queue; //上传文件的等待队列，是swap2。
  bool generator_thread_is_quit = false, transform_thread_is_quit = false,
       convert_thread_is_quit = false;
  thread *generator_thread =
      new thread(generator_manager, generator, generate_queue,
                 parallel.read_count, generator_thread_is_quit);
  thread *converter_thread = new thread(
      converter_manager, generate_queue, waiting_upload_queue,
      generator_thread_is_quit, convert_thread_is_quit,
      parallel
          .read_count); // 最大的buffer2的空间应该和buffer1的空间相等，不用再进行设置啦。
  thread *transformer_thread =
      new thread(transformer_manager, waiting_upload_queue, key_sender,
                 convert_thread_is_quit, transform_thread_is_quit,
                 parallel.transfer_count);
  generator_thread->join();
  converter_thread->join();
  transformer_thread->join();
}
