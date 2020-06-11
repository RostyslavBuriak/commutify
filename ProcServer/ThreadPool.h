#pragma once
//Example of thread pool

#include<thread>
#include<condition_variable>
#include<mutex>
#include<future>
#include <queue>
#include <vector>
#include<functional>
#include <atomic>

class ThreadPool
{
public:

	ThreadPool(); //on default creates 1 thread
	ThreadPool(const size_t); //creates number of threads
	ThreadPool(const ThreadPool&) = delete; //no copy constructor
	ThreadPool(ThreadPool&&) = delete; //no move constructor

	~ThreadPool();

	ThreadPool& operator=(const ThreadPool&) = delete; //no copy operator
	ThreadPool& operator=(ThreadPool&&) = delete; //no move operator

	void AddThread(const size_t n); //Increase number of threads in the pool

	void StopPool(); //Stop poooling

	size_t ThreadsNumber();//return the nuber of the working threads


	template<typename T, typename... Args>
	auto AddTask(T&& obj, Args&&... args)
		-> std::future<decltype(std::declval<T>()(std::declval<Args>()...))> { //return future which contains function return value

		using rtype = decltype(std::declval<T>()(std::declval<Args>()...)); // deduce function return type

		auto ptask = std::make_shared<std::packaged_task<rtype()>>(std::bind(std::forward<T>(obj), std::forward<Args>(args)...)); //shared pointer is needed because packaged_task has deleted copy-constructor 
																																		  //so we cant pass it to labmda by value

		std::future<rtype> future = ptask->get_future(); //get future from packaged task

		{
			std::lock_guard<std::mutex> lg(mtx); //needed to safely add task to tasks container

			tasks.emplace([ptask]() { (*ptask)(); }); //add task to task container.

		}

		cv.notify_one(); //notify waiting thread

		return future; //return future
	}


private:
	size_t nthreads; //number of threads

	std::atomic<bool> stop; //variable to check if we have to stop threading

	std::mutex mtx;

	std::condition_variable cv;

	std::queue< std::function<void()> > tasks; //container for tasks

	std::vector<std::thread> threads; //container for threads

};