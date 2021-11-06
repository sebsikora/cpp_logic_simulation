/*
	
	void_thread_pool.hpp
	
	Copyright © 2021 Dr Seb N.F. Sikora.
	
	void_thread_pool.hpp is distributed under the terms of the MIT license.
	
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

#ifndef VOID_TPOOL_H
#define VOID_TPOOL_H

#include <vector>						// std::vector.
#include <thread>						// std::thread.
#include <mutex>						// std::mutex, std::unique_lock.
#include <condition_variable>			// std::condition_variable.
#include <queue>						// std::queue.
#include <functional>					// std::function.

class VoidThreadPool {
	private:
		bool m_display_messages = false;
		std::vector<std::thread> m_threads = {};
		bool m_pool_stopped = false;
		// Synchronisation of job queue and stop flag.
		std::condition_variable m_queue_condition;
		std::mutex m_queue_lock;
		std::queue<std::function<void()>> m_job_queue = {};
		bool m_finish = false;
		// Synchronisation of pending job counter and waiting flag.
		std::condition_variable m_counter_condition;
		std::mutex m_counter_lock;
		int m_jobs_pending = 0;
		bool m_waiting_for_completion = false;
		
	public:
		VoidThreadPool(bool display_messages = false, int number_of_workers = 0);
		~VoidThreadPool();
		void WorkerFunction(int worker_id);
		void AddJob(std::function<void()> new_job);
		void WaitForAllJobs(void);
		void Finish(void);
};

#endif
