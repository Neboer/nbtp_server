#include "net_upload.h"
#include <cstddef>
#include <curl/curl.h>
#include <curl/easy.h>
#include <future>
// #include <curl/curl.h>

class full_upload_task
{
  CURLM *multi_handle;
  string *target_url;
  // notice here! if curl_mime has been created, the nbtp_chunk will be delete.
  // the wrap_func is used for multiple picture bed upload
  function<curl_mime *(nbtp_chunk *, CURL *)> wrap_func;

  full_upload_task(string target_url, function<curl_mime *(nbtp_chunk *, CURL *)> wrap_func)
  {
    multi_handle = curl_multi_init();
    this->wrap_func = wrap_func;
  }

private:
  static size_t server_response_collector(void *contents, size_t size, size_t nmemb, void *target_result_block_write_location)
  {
    size_t real_size = size * nmemb;
    nbtp_chunk *target_chunk = (nbtp_chunk *)target_result_block_write_location;
    target_chunk->insert(target_chunk->end(), (unsigned char *)contents, (unsigned char *)contents + real_size);
    return real_size;
  }

public:
  promise<string *> *add_task(nbtp_chunk *data_wait_for_upload)
  {
    CURL *current_easy_handle = curl_easy_init();
    promise<string *> *promise_containing_result = new promise<string *>();
    nbtp_chunk *server_response = new nbtp_chunk();
    auto task_info = new communicate_message(promise_containing_result, server_response);
    curl_easy_setopt(current_easy_handle, CURLOPT_MIMEPOST, wrap_func(data_wait_for_upload, current_easy_handle));
    curl_easy_setopt(current_easy_handle, CURLOPT_URL, target_url->c_str());
    curl_easy_setopt(current_easy_handle, CURLOPT_WRITEFUNCTION, server_response_collector);
    curl_easy_setopt(current_easy_handle, CURLOPT_WRITEDATA, server_response);
    curl_easy_setopt(current_easy_handle, CURLOPT_PRIVATE, task_info);
    curl_multi_add_handle(multi_handle, current_easy_handle);
    return promise_containing_result;
  }

  

};