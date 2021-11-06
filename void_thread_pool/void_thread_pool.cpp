/*
	
	void_thread_pool.cpp - An ultra-simple thread pool implementation for running void() functions in multiple worker threads.
	
	Copyright © 2021 Dr Seb N.F. Sikora.
	
	void_thread_pool.cpp is distributed under the terms of the MIT license.
	
	Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
	files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
	modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software
	is furnished to do so, subject to the following conditions:
	
	The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
	
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
	LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
	IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
	
	seb.nf.sikora@protonmail.com
	
	* A synthesis of the brilliantly helpful code snippets by users 'phd-ap-ece' and 'pio' on the following stackoverflow post -
	  https://stackoverflow.com/questions/15752659/thread-pooling-in-c11
	* Useful supplementary information found here - https://stackoverflow.com/questions/10673585/start-thread-with-member-function
	* Explanation of the syntax [this]() { ... } for the predicate second argument to condition_variable.wait() found here -
	  https://stackoverflow.com/questions/39565218/c-condition-variable-wait-for-predicate-in-my-class-stdthread-unresolved-o
	
*/

#include <vector>						// std::vector.
#include <iostream>						// std::cout, std::endl.
#include <thread>						// std::thread.
#include <mutex>						// std::mutex, std::unique_lock.
#include <condition_variable>			// std::condition_variable.
#include <queue>						// std::queue.
#include <functional>					// std::function.

#include "void_thread_pool.hpp"

VoidThreadPool::VoidThreadPool(bool display_messages, int number_of_workers) {
	m_display_messages = display_messages;
	int num_threads;
	if (number_of_workers == 0) {
		num_threads = std::thread::hardware_concurrency();
	} else {
		num_threads = number_of_workers;
	}
	if (m_display_messages) {
		std::cout << "Void ThreadPool starting " << num_threads << " worker threads..." << std::endl;
	}
	for (int i = 0; i < num_threads; i ++) {
		// Pass worker member function by reference to avoid the thread making it's own copy.
		m_threads.push_back(std::thread(&VoidThreadPool::WorkerFunction, this, i));
	}
}

VoidThreadPool::~VoidThreadPool() {
	if (!m_pool_stopped) {
		Finish();
	}
}

void VoidThreadPool::WorkerFunction(int worker_id) {
	if (m_display_messages) {
		// Pre-composing our whole message stops messages from one thread being interrupted by messages from another thread.
		std::string message = "Worker function " + std::to_string(worker_id) + " starting.\n";
		std::cout << message;
	}
	std::function<void()> job;
	while (true) {
		{
			std::unique_lock<std::mutex> lock(m_queue_lock);
			// See below for explanation of the syntax [this]() { ... } for the predicate second argument to condition_variable.wait().
			// https://stackoverflow.com/questions/39565218/c-condition-variable-wait-for-predicate-in-my-class-stdthread-unresolved-o
			m_queue_condition.wait(lock, [this]() { return !m_job_queue.empty() || m_finish; });
			
			if (m_finish && m_job_queue.empty()) {
				if (m_display_messages) {
					std::string message = "Worker function " + std::to_string(worker_id) + " stoppped.\n";
					std::cout << message;
				}
				return;
			} else {
				job = m_job_queue.front();
				m_job_queue.pop();
			}
		}
		job();
		{
			// Once the current job has finished, decrement the pending job counter.
			// If there are no pending jobs and we are waiting, alert the waiting function via condition variable.
			std::unique_lock<std::mutex> lock(m_counter_lock);
			m_jobs_pending --;
			if ((m_jobs_pending == 0) && (m_waiting_for_completion)) {
				m_counter_condition.notify_one();
			}
		}
	}
}

void VoidThreadPool::AddJob(std::function<void()> new_job) {
	// Increment the pending job counter and push the new job onto the queue.
	{
		std::unique_lock<std::mutex> lock(m_counter_lock);
		m_jobs_pending ++;
	}
	{
		std::unique_lock<std::mutex> lock(m_queue_lock);
		m_job_queue.push(new_job);
	}
	// Notify one worker function that there is now a pending job.
	m_queue_condition.notify_one();
}

void VoidThreadPool::WaitForAllJobs(void) {
	while (true) {		// Wrap the wait on m_job_counter_condition in a while loop in case the condition wakes up spuriously.
		{
			std::unique_lock<std::mutex> lock(m_counter_lock);
			if (m_jobs_pending > 0) {
				if (m_display_messages) {
					std::string message = "*** Waiting for all jobs to complete...\n";
					std::cout << message;
				}
				m_waiting_for_completion = true;
				m_counter_condition.wait(lock, [this]() { return m_jobs_pending == 0; });
				if (m_jobs_pending == 0) {
					m_waiting_for_completion = false;
					if (m_display_messages) {
						std::cout << "...all jobs completed. ***" << std::endl;
					}
					break;
				}
			} else {
				if (m_display_messages) {
					std::string message = "No uncompleted jobs.\n";
					std::cout << message;
				}
				break;
			}
		}
	}
}

void VoidThreadPool::Finish(void) {
	if (m_display_messages) {
		std::string shutdown_message = "Finish() called...\n"; 
		std::cout << shutdown_message;
	}
	// Set the stop flag and notify all processes.
	{
		std::unique_lock<std::mutex> lock(m_queue_lock);
		m_finish = true;
	}
	m_queue_condition.notify_all();
	// Wait and join all threads.
	for (std::thread& thread : m_threads) {
		thread.join();
	}
	m_threads.clear();
	m_pool_stopped = true;
	if (m_display_messages) {
		std::cout << "...all worker threads are stopped." << std::endl;
	}
}
