#include <future>
#include <curl/curl.h>
#include <string>
#include "../nbtp.h"

// 用来在子传输和主传输之间传输的结构体，如果传输完成，主传输将会获得这个结构体的内容。
struct communicate_message
{
    communicate_message(promise<string *> *report_destination, nbtp_chunk *server_response){
        this->report_destination = report_destination;
        this->server_response = server_response;
    }
    promise<string *> *report_destination;
    nbtp_chunk *server_response;
};

// 这里有一个什么问题：main函数需要先创建一个任务对象，这个任务对象将会影响下面这个函数的行为。首次调用创建任务，再次调用添加任务。这个函数应该对上传任务是透明的。
// promise<string &> start_network_upload(nbtp_chunk *chunk);// 这个函数有必要回收chunk。
