#include <mutex>
#include <queue>


using namespace std;
template <typename T> class ThreadsafeQueue {
  queue<T> queue_;
  mutable mutex atom_mutex; // 确保操作原子性的mutex.
  size_t max_size;
  unique_lock<mutex>
      write_full_lock; // 推入时先锁住，如果推完还没满就释放，如果推完满了就一直锁，推过来的数据将会阻塞，直到一个pop将锁打开。
  unique_lock<mutex>
      read_full_lock; // 弹出后发现已空，

  // Moved out of public interface to prevent races between this
  // and pop().
  bool empty() const { return queue_.empty(); }

public:
  ThreadsafeQueue(size_t max_size) {
    lock_guard<mutex> lock(atom_mutex);
    this->max_size = max_size;
  }
  ThreadsafeQueue() = delete;
  // ThreadsafeQueue(const ThreadsafeQueue<T> &) = delete;
  // ThreadsafeQueue &operator=(const ThreadsafeQueue<T> &) = delete;

  // ThreadsafeQueue(ThreadsafeQueue<T> &&other)
  // {
  //   lock_guard<mutex> lock(atom_mutex);
  //   queue_ = move(other.queue_);
  // }

  virtual ~ThreadsafeQueue() {}

  unsigned long size() const {
    lock_guard<mutex> lock(atom_mutex);
    return queue_.size();
  }

  T pop() {
    lock_guard<mutex> lock(atom_mutex);
    // 获得原子性保证之后，判断是否要打开写入锁。

    if (queue_.empty()) {
      return {};
    }
    T tmp = queue_.front();
    queue_.pop();
    return tmp;
  }

  void push(const T &item) {
    lock_guard<mutex> lock(atom_mutex);
    write_full_lock.lock();
    queue_.push(item);
    if (queue_.size() < max_size) {
      write_full_lock.unlock(); // 如果没满，释放已加的锁。。
    }
    // 如果满了，就一直锁着吧。
  }
};