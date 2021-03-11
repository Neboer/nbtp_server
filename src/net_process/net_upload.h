#include <future>
#include <string>
#include "../nbtp.h"

// 这里有一个什么问题：main函数需要先创建一个任务对象，这个任务对象将会影响下面这个函数的行为。首次调用创建任务，再次调用添加任务。这个函数应该对上传任务是透明的。
promise<string &> start_network_upload(nbtp_chunk *chunk);// 这个函数有必要回收chunk。