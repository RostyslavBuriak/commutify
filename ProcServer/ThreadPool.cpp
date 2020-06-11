#include "ThreadPool.h"


ThreadPool::ThreadPool() :nthreads(1),stop(false) {

	for (unsigned int i = 0; i < nthreads; ++i) {

		threads.emplace_back( //add thread to threads container
			std::thread( //thread creation

				[this]() { //lambda for wating for tasks

					std::function<void()> task; //create it before mutex lock 

					{
						std::unique_lock<std::mutex> ul(mtx); //unique_lock is used for condition var

						cv.wait(ul, [this]() { //wait for tasks or stop of the programm

							return !tasks.empty() || stop;

							});

						if (stop)
							return;

						task = std::move(tasks.front()); //take task from tasks container

						tasks.pop(); //delete taken task from container

					}

					task(); //run task

				}));

	}
}


ThreadPool::ThreadPool(const size_t n):nthreads(n),stop(false) {

	for (unsigned int i = 0; i < nthreads; ++i) {

		threads.emplace_back( //add thread to threads container
			std::thread( //thread creation

				[this]() { //lambda for wating for tasks

					std::function<void()> task; //create it before mutex lock 

					{
						std::unique_lock<std::mutex> ul(mtx); //unique_lock is used for condition var

						cv.wait(ul, [this]() { //wait for tasks or stop of the programm

							return !tasks.empty() || stop;

							});

						if (stop)
							return;

						task = std::move(tasks.front()); //take task from tasks container

						tasks.pop(); //delete taken task from container

					}

					task(); //run task

				}));

	}

}



ThreadPool::~ThreadPool() {
	if(!stop)
		StopPool();
}



void ThreadPool::StopPool() {

	stop = true;

	cv.notify_all();

	for (auto& thread : threads) {
		thread.join();
	}

}


void ThreadPool::AddThread(const size_t n) {

	nthreads += n;

	for (unsigned int i = 0; i < n; ++i) {

		threads.emplace_back( //add thread to threads container
			std::thread( //thread creation

				[this]() { //lambda for wating for tasks

					std::function<void()> task; //create it before mutex lock 

					{
						std::unique_lock<std::mutex> ul(mtx); //unique_lock is used for condition var

						cv.wait(ul, [this]() { //wait for tasks or stop of the programm

							return !tasks.empty() || stop;

							});

						if (stop)
							return;

						task = std::move(tasks.front()); //take task from tasks container

						tasks.pop(); //delete taken task from container

					}

					task(); //run task

				}));

	}
}


size_t ThreadPool::ThreadsNumber() {
	return nthreads;
}