#include "net_upload.h"
#include <cstddef>
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/multi.h>
#include <future>
#include <mutex>

using namespace std;

full_upload_task::full_upload_task(
    string *target_url,
    function<void(CURL *, unique_ptr<nbtp_chunk>)> convert_to_uploadable_object) {

  this->multi_handle = curl_multi_init();
  this->convert_to_uploadable_object = convert_to_uploadable_object;
}

size_t full_upload_task::server_response_collector(void *contents, size_t size, size_t nmemb, void *target_result_block_write_location) {
  size_t real_size = size * nmemb;
  nbtp_chunk *target_chunk = (nbtp_chunk *)target_result_block_write_location;
  target_chunk->insert(target_chunk->end(), (unsigned char *)contents,
                       (unsigned char *)contents + real_size);
  return real_size;
}

int full_upload_task::add_task(unique_ptr<nbtp_chunk> data_wait_for_upload) {
  CURL *task_handle = curl_easy_init();
  convert_to_uploadable_object(task_handle, move(data_wait_for_upload)); // 将需要上传的信息数据等绑定上
  curl_easy_setopt(task_handle, CURLOPT_WRITEFUNCTION, server_response_collector);
  curl_easy_setopt(task_handle, CURLOPT_WRITEDATA, )
      curl_easy_setopt(curl_obj, CURLOPT_PRIVATE, )
          curl_multi_add_handle(multi_handle, curl_obj);
  int running_handles_count = 0;
  curl_multi_perform(multi_handle, &running_handles_count);
  return running_handles_count;
}

indexed_key_str full_upload_task::wait_for_an_result() {
  indexed_key_str result;
  CURLMsg *current_curl_message;
  int numfds, msgs;
  if (last_message) {
    current_curl_message = last_message;
  } else {
    curl_multi_wait(multi_handle, nullptr, 0, 0, &numfds);
    current_curl_message = curl_multi_info_read(multi_handle, &msgs);
  }
  CURL *current_handle = current_curl_message->easy_handle;
  if (current_curl_message->data.result == CURLE_OK) {
    message current_message;
    curl_easy_getinfo(current_handle, CURLINFO_PRIVATE, (void *)&current_message);
    return move(current_message.result);
  } else {
    message current_message;
    curl_easy_getinfo(current_handle, CURLINFO_PRIVATE, (void *)&current_message);
    if (current_message.failed_count > max_failed_count) {
      throw max_retry_time_exceed{};
    }
  }
}
