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
    function<CURL *(unique_ptr<nbtp_chunk>)> convert_to_uploadable_object) {

  this->multi_handle = curl_multi_init();
  this->convert_to_uploadable_object = convert_to_uploadable_object;
}

size_t full_upload_task::server_response_collector (void *contents, size_t size, size_t nmemb,
                            void *target_result_block_write_location) {
    size_t real_size = size * nmemb;
    nbtp_chunk *target_chunk = (nbtp_chunk *)target_result_block_write_location;
    target_chunk->insert(target_chunk->end(), (unsigned char *)contents,
                         (unsigned char *)contents + real_size);
    return real_size;
}

int full_upload_task::add_task(unique_ptr<nbtp_chunk> data_wait_for_upload) {
  CURL* curl_obj = convert_to_uploadable_object(move(data_wait_for_upload));
  curl_multi_add_handle(multi_handle, curl_obj);
  int running_handles_count = 0;
  curl_multi_perform(multi_handle, &running_handles_count);
  return running_handles_count;
}
