#pragma once
#include "../nbtp.h"
#include <cstddef>
#include <curl/curl.h>
#include <curl/multi.h>
#include <exception>
#include <future>
#include <memory>
#include <string>

// 用来在子传输和主传输之间传输的结构体，如果传输完成，主传输将会获得这个结构体的内容。
// 主上传任务和子上传任务之间传递的结构体就是indexed_nbtp_chunk。这个结构体在创建任务的时候被初始化，然后在从线程中取下来的时候返回。
class full_upload_task {
  CURLM *multi_handle;
  shared_ptr<string> target_url;                                               // 这个string并没有被多线程访问，要限制这个访问仅限于此线程中。
  function<void(CURL *, unique_ptr<nbtp_chunk>)> convert_to_uploadable_object; // 传入一个普通的curl_easy_handle和一个二进制数据块，此方法应该将数据块作为文件内容附加在easy_handle中。
  int max_failed_count;                                                        // max try time after one failed upload.
  full_upload_task(string *target_url, function<void(CURL *, unique_ptr<nbtp_chunk>)> convert_to_uploadable_object);

private:
  static size_t server_response_collector(void *contents, size_t size, size_t nmemb, void *target_result_block_write_location);
  CURLMsg *last_message = nullptr;

public:
  int add_task(unique_ptr<nbtp_chunk> data_wait_for_upload); // 添加一个上传任务，同时返回现在正在传输的上传任务的数量。
  indexed_key_str wait_for_an_result();                      // 阻塞的等待一个上传任务结束，内部使用curl poll来实现。
};

// data transferred between curl single handle and multi interface.
struct message {
  int failed_count = 0; // counter of failed transfer
  indexed_key_str result;
};

struct max_retry_time_exceed : exception {};
