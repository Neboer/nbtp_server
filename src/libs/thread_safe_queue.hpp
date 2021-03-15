#include <deque>
#include <mutex>
#include <queue>

// this is a very fast read-write block FIFO queue. When the queue is empty, the read process will block until a push into the queue, the same as write when the queue is full.
using namespace std;
template <typename T> class ThreadsafeQueue {
  queue<T> queue_;
  mutex atom_mutex, // 确保操作原子性的mutex.
    write_mutex, read_mutex;
  size_t max_size;
  // Moved out of public interface to prevent races between this
  // and pop().
  bool empty() const { return queue_.empty(); }

public:
  ThreadsafeQueue(size_t max_size) {
    const lock_guard<mutex> lock(atom_mutex);
    write_mutex.unlock();
    read_mutex.unlock();
    this->max_size = max_size;
  }
  ThreadsafeQueue() = delete;
  // ThreadsafeQueue(const ThreadsafeQueue<T> &) = delete;
  // Threadsafequeue &Operator=(Const Threadsafequeue<T> &) = Delete;

  // ThreadsafeQueue(ThreadsafeQueue<T> &&other)
  // {
  //   lock_guard<mutex> lock(atom_mutex);
  //   queue_ = move(other.queue_);
  // }

  virtual ~ThreadsafeQueue() {}

  unsigned long size() {
    lock_guard<mutex> lock(atom_mutex);
    return queue_.size();
  }

  T pop() {
    // 获得原子性保证之后，判断是否要打开写入锁。
    if (queue_.size() == max_size) {
      write_mutex.unlock();
    }
    read_mutex.lock();
    lock_guard<mutex> lock(atom_mutex);
    T tmp = queue_.front();
    queue_.pop();
    if (!queue_.empty()) {
      read_mutex.unlock();
    }
    return tmp;
  }

  void push(const T &item) {
    if (queue_.empty()){
      read_mutex.unlock();
    }
    write_mutex.lock();
    lock_guard<mutex> lock(atom_mutex);
    queue_.push(item);
    if (queue_.size() < max_size) {
      write_mutex.unlock(); // 如果没满，释放已加的锁。。
    }
    // 如果满了，就一直锁着吧。
  }
};
