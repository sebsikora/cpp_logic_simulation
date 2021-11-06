#include "void_thread_pool.hpp"		// VoidThreadPool.

#include <functional>			// std::bind.
#include <vector>				// std::vector.
#include <iostream>				// std::cout, std::endl.
#include <sstream>				// std::stringstream.
#include <thread>				// std::thread, std::this_thread::sleep_for.
#include <chrono>				// std::chrono::seconds.
#include <cstdlib>				// rand().

// Dummy class that has a member function that returns void that we wish to run in it's own thread.
class Foo {
	private:
		int m_id = 0;
	public:
		Foo(int id) { m_id = id; }
		~Foo() { }
		void Bar(void) {
			std::string message = "Foo number " + std::to_string(m_id) + " checking-in.\n";
			std::cout << message;
			int bar_delay_seconds = (rand() % 5) + 2;
			std::this_thread::sleep_for(std::chrono::seconds(bar_delay_seconds));
			message = "Foo number " + std::to_string(m_id) + " checking-out.\n";
			std::cout << message;
		}
};

void FooBar1(void);
void FooBar1(void) {
	std::string message = "FooBar1 checking-in.\n";
	std::cout << message;
	int bar_delay_seconds = (rand() % 5) + 2;
	std::this_thread::sleep_for(std::chrono::seconds(bar_delay_seconds));
	message = "FooBar1 checking-out.\n";
	std::cout << message;
}

void FooBar2(int id = 0);
void FooBar2(int id) {
	std::string message = "FooBar2 number " + std::to_string(id) + " checking-in.\n";
	std::cout << message;
	int bar_delay_seconds = (rand() % 5) + 2;
	std::this_thread::sleep_for(std::chrono::seconds(bar_delay_seconds));
	message = "FooBar2 number " + std::to_string(id) + " checking-out.\n";
	std::cout << message;
}

int main() {
	
	// Create a threadpool with a number of threads equal to the maximum supported on the run time architecture.
	VoidThreadPool thread_pool(true);
	
	// Create a vector of Foos. Foos have a member function that returns void that we want to call.
	int foo_count = 10;
	std::vector<Foo*> foos = {};
	for (int i = 0; i < foo_count; i ++) {
		foos.push_back(new Foo(i));
	}

	// Example of adding a void function.
	// Without arguments.
	thread_pool.AddJob(&FooBar1);
	// If we want to pass arguments with the function we pass them together using std::bind.
	for (int i = 1; i < 5; i ++) {
		thread_pool.AddJob(std::bind(&FooBar2, i));
	}
	
	// We add the void member function calls to our thread pool's job queue.
	// As we want to add a class member function for a particular class instance as the void function, we need to
	// wrap it (and a reference to the specific class instance) in a call to std::bind.
	// Both arguments to std::bind need to be by reference if we don't want the thread to make a copy of the object,
	// but it is then up to us to make sure the object stays in scope until the thread has finished executing.
	for (int i = 0; i < foo_count; i ++) {
		thread_pool.AddJob(std::bind(&Foo::Bar, foos[i]));
	}
	
	// Call this to wait here until all the jobs in the queue have finished.
	thread_pool.WaitForAllJobs();
	
	// Call this to stop all threads. 
	thread_pool.Finish();
	
	// Clean up.
	for (int i = 0; i < foo_count; i ++) {
		delete foos[i];
	}
	
	return 0;
}
