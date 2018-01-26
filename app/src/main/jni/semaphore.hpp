#include <stdio.h>
#include <mutex>
#include <thread>
#include <condition_variable>

class semaphore 
{
public:
	semaphore(int value = 1) : count_{ value }, wakeups_{ 0 } {}

	void wait()
	{
		std::unique_lock<std::mutex> lock{ mutex_ };
		if (--count_ < 0) { // count is not enough ?
			condition_.wait(lock, [&]()->bool{ return wakeups_ > 0; }); // suspend and wait ...
			--wakeups_;  // ok, me wakeup !
		}
	}
	void signal()
	{
		std::lock_guard<std::mutex> lock{ mutex_ };
		if (++count_ <= 0) { // have some thread suspended ?
			++wakeups_;
			condition_.notify_one(); // notify one !
		}
	}

private:
	int count_;
	int wakeups_;
	std::mutex mutex_;
	std::condition_variable condition_;
};