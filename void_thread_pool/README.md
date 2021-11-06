# void_thread_pool.cpp

© 2021 Dr Sebastien Sikora.

[seb.nf.sikora@protonmail.com](mailto:seb.nf.sikora@protonmail.com)

Updated 05/11/2021.

What is it?
-------------------------
void_thread_pool.cpp is an ultra-simple thread pool implementation for running functions that return `void` in multiple worker threads.

It is a synthesis of the brilliantly helpful code snippets and examples by users [phd-ap-ece](https://stackoverflow.com/users/3818417/phd-ap-ece) and [pio](https://stackoverflow.com/users/2724420/pio) [here](https://stackoverflow.com/questions/15752659/thread-pooling-in-c11).

Useful supplementary information found [here](https://stackoverflow.com/questions/10673585/start-thread-with-member-function).
Explanation of the syntax `[this]() { ... }` for the predicate second argument to `std::condition_variable.wait()` found [here](https://stackoverflow.com/questions/39565218/c-condition-variable-wait-for-predicate-in-my-class-stdthread-unresolved-o).

License:
-------------------------
![Mit License Logo](/220px-MIT_logo.png)
<br/><br/>
void_thread_pool.cpp is distributed under the terms of the MIT license.
Learn about the MIT license [here](https://choosealicense.com/licenses/mit/)
