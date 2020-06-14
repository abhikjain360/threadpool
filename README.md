# Threadpool in C++

I needed a simple threadpool in C++ for some of my projects, so I made one.
The threadpool class has ```add()``` function,
which takes arguments similiar to ```std::thread```,
and adds the function to the queue.
Automatically assigns maximum possible threads according to hardware,
or less if lower number is specified in ``threadpool(u_int num_threads)`` constructor.

I didn't do any benchmarks, but if used properly has shown to outperform
normal code and also ones which simply assign threads instead of pooling.
