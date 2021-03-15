#include <future>
#include <curl/curl.h>
#include <string>
#include "../nbtp.h"

/ 用来在子传输和主传输之间传输的结构体，如果传输完成，主传输将会获得这个结构体的内容。
struct communicate_message
{
    communicate_message(promise<string *> *report_destination, nbtp_chunk *server_response){
        this->report_destination = report_destination;
        this->server_response = server_response;
    }
    promise<string *> *report_destination;
    nbtp_chunk *server_response;
};

