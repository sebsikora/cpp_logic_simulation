# void_thread_pool.cpp

© 2021 Dr Sebastien Sikora.

[seb.nf.sikora@protonmail.com](mailto:seb.nf.sikora@protonmail.com)

Updated 06/11/2021.

What is it?
-------------------------
void_thread_pool.cpp is an ultra-simple thread pool implementation for running functions that return `void` in multiple worker threads.

It is a synthesis of the brilliantly helpful code snippets and examples by users [phd-ap-ece](https://stackoverflow.com/users/3818417/phd-ap-ece) and [pio](https://stackoverflow.com/users/2724420/pio) [here](https://stackoverflow.com/questions/15752659/thread-pooling-in-c11).
Useful supplementary information found [here](https://stackoverflow.com/questions/10673585/start-thread-with-member-function).
Explanation of the syntax `[this]() { ... }` for the predicate second argument to `std::condition_variable.wait()` found [here](https://stackoverflow.com/questions/39565218/c-condition-variable-wait-for-predicate-in-my-class-stdthread-unresolved-o).

How to use it?
-------------------------
Say we have a function, or a class with a member function that returns `void`, that we would like to call in it's own thread.
```cpp
void FooBar1() {
	std::string message = "FooBar1 call.\n";
	std::cout << message;
}

void FooBar2(int id) {
	std::string message = "My id is " + std::to_string(id) + "\n";
	std::cout << message;
}

class Foo {
	private:
		int m_id = 0;
	public:
		Foo(int id) { m_id = id; }
		~Foo() { }
		void Bar(void) { *** Stuff happens *** }
};
```
First we instantiate the thread pool.
```cpp
VoidThreadPool thread_pool(true);

// Optional first argument set to true prints pool messages to the console.

VoidThreadPool thread_pool(true, 8)

// If we don't specify the optional second number_of_workers argument it defaults to the
// thread count returned by std::thread::hardware_concurrency().
```
Now, to queue a job for solution, we add it to the thread pool job queue using the thread pool's `AddJob()` member function.
```cpp
thread_pool.AddJob(FooBar1);
thread_pool.AddJob(&FooBar1);

// We can pass the function by reference to avoid making a copy.

thread_pool.AddJob(std::bind(&FooBar2, i));

// If we need to pass arguments with the function we encapsulate both using std::bind.

Foo a_foo;

thread_pool.AddJob(std::bind(&Foo::Bar, a_foo));

// If we want to pass a class member function, we again use std::bind to encapsulate a
// pointer to a member function of the class, followed by an instance of that class.

thread_pool.AddJob(std::bind(&Foo::Bar, &a_foo));

// We can pass the class instance by reference, otherwise std::bind will make a copy of the
// object. However, we are then taking responsibility for making sure the referenced object
// still exists for the duration of the thread's execution.
```
Worker threads will begin to execute jobs as soon as they are added to the queue. If we need to wait for all jobs added to the queue to be completed prior to doing something else, we call the thread pool's `WaitForAllJobs()` member function. This will return when the job queue is empty and all running jobs are completed.


To stop the thread pool, we call it's `Finish()` method, which will stop the worker threads once all currently queued jobs are completed. `delete`ing a VoidThreadPool via a pointer to call it's destructor will first call `Finish()`.  


For a complete usage example [see](https://github.com/sebsikora/void_thread_pool/blob/main/demo.cpp) `demo.cpp`.


To compile the example (for example, using gcc), enter `g++ -pthread -o main -g void_thread_pool.cpp demo.cpp` on the command line. Note the `-pthread` compiler flag to add multi-threading support via the pthreads library.

License:
-------------------------
![Mit License Logo](./220px-MIT_logo.png)
<br/><br/>
void_thread_pool.cpp is distributed under the terms of the MIT license.
Learn about the MIT license [here](https://choosealicense.com/licenses/mit/)
