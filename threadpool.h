#pragma once

#include <sys/types.h>
#include <thread>
#include <functional>
#include <condition_variable>
#include <future>
#include <vector>
#include <queue>

const u_int MAX_THREADS = std::thread::hardware_concurrency() - 1;


class threadpool {
  public:
	using Task = std::function<void()>;

	explicit threadpool(u_int num_threads) {
		if (num_threads > MAX_THREADS)
			num_threads = MAX_THREADS;

		threads.reserve(num_threads);

		for (u_int i = 0; i < num_threads; ++i) {
			threads.emplace_back(std::thread(&threadpool::thread_manager, this));
			// Might need to uncomment this if bug occurs
			//threads.back().detach();
		}
	}

	~threadpool() noexcept {
		{
			std::unique_lock<std::mutex> lock(mu);
			stopvar = true;
		}
		cond.notify_all();

		for (auto &t : threads)
			t.join();
	}

	template<typename F, typename... Args>
	auto add(F&& func, Args&&... args);

  private:
	void thread_manager();


	std::vector<std::thread> threads;
	std::queue<Task> tasks;
	std::mutex mu;
	std::condition_variable cond;
	bool stopvar = false;
};

inline void threadpool::thread_manager() {
	while (true) {
		Task task;
		{
			std::unique_lock<std::mutex> lock(mu);
			cond.wait(lock, [=] { return stopvar || !tasks.empty(); });
			if (stopvar && tasks.empty()) break;

			task = std::move(tasks.front());
			tasks.pop();
		}
		task();
	}
}

template<typename F, typename... Args>
auto threadpool::add(F&& func, Args&&... args) {
	typedef decltype(func(args...)) return_type;

	auto task = std::make_shared<std::packaged_task<return_type()>>(
        	std::bind(std::forward<F>(func), std::forward<Args>(args)...)
        );

	std::future<return_type> res = task->get_future();

	{
		std::unique_lock<std::mutex> lock(mu);
		if (stopvar)
			throw std::runtime_error("adding to a stopped threadpool");
		tasks.emplace([task]() { (*task)(); });
	}
	cond.notify_one();

	return res;
}
