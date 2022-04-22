# Measurement Assignment Questions
You may add your answers to the assignment questions in the space provided
below.  This is a markdown file (.md), so you may use markdown formatting. If
you do not know about markdown (or know but don't care to format anything) then
you may just treat this as a plain text file.


# Part A
Answer these questions before you start coding.  You may discuss the **Part A
questions *only*** with your classmates, but you must write your own answers
in your own words.


## Question 1
Research the `gettimeofday()` function.  (`man gettimeofday`)  What does this
function do?  How would you use it to measure the amount of time required
("latency" or "delay") to perform an action (say, calling the `foo()` function)?

### Answer:
> int gettimeofday(struct timeval *tv, struct timezone *tz);

> The functions gettimeofday() and settimeofday() can get and set the time as
well as a timezone. 
To measure the amount of time required to perform an action, call
`gettimeofday()` with `end.tv_sec - begin.tv_sec` to get whole seconds and 
`end.tv_usec - begin.tv_usec` to get fractions of a second.


## Question 2
Now research the `getrusage()` function (`man getrusage`), paying particular
attention to the `ru_utime` and `ru_stime` fields in the result.  (Assume it's
being called with the `RUSAGE_SELF` argument.)  How are the user time
(`ru_utime`), system time (`ru_stime`), and the time returned by
`gettimeofday()` all different from each other?

### Answer:
> int getrusage(int who, struct rusage *usage);

> `getrusage()` returns resource usage measures for who, the calling proces,
which can be one of the following:
- RUSAGE_SELF (the calling process)
- RUSAGE_CHILDREN (all children of the calling process)
- RUSAGE_THREAD (the calling thread)

In the rusage struct that gets populated using the `getrusage()` call, there
are the fields:
- `struct timeval ru_utime` which is the total amount of time spent executing in
user mode, expressed in a timeval structure (seconds plus microseconds).
- `struct timeval ru_stime` which is the total amount of time spent executing in
kernel mode, expressed in a timeval structure (seconds plus microseconds).

`ru_utime` gets the user mode time, `ru_stime` gets the kernal mode time, and
`gettimeofday()` gets the current time. 

## Question 3
Suppose you want to measure the time it take to do one fast thing; something
that takes less than a minute to do (e.g., fill one glass with water).  However,
the only tool you have for taking measurements is a digital clock (regular
clock, not a stopwatch) that does not show seconds, only shows hours and
minutes.  How would you get an accurate measure of how long it takes to do the
thing once?

### Answer:
To get an accurate measure of how long something takes, perform the same action
several times until it is within range of your measurement device, measure the
amount time the repeated action takes, and then divide the total measured time by the amount of repetitions.


## Question 4
Suppose you want to find the average amount of time required to run function
`foo()`.  What is the difference between the following two approaches?  Which
one is better, and why?  (You may assume `foo()` is very fast.)

```c
latency = 0
loop N times
    measure start time
    call foo()
    measure end time
    latency += end time - start time
print latency/N
```

```c
measure start time
loop N times
    call foo()
measure end time
print (end time - start time) / N
```

### Answer:
Since the first approach starts and stops the time very often between a "very
fast" function, the latency can _run over_ and affect unnecessary overhead to
the overall latency time, even though appearing. The second approach is better,
since the time it takes to start and end is removed. However, this may change
with the relative time lengths of the loop and function `foo()`

The second approach therefore removes the latency of calling start and end,
which may be larger than the function call producing an incorrect measurement.


## Question 5
Consider the following code.  What work is this code doing, besides calling
`foo()`?

```c
int i;
for (i = 0; i < N; ++i) {
  foo();
}
```

### Answer:
The `int i` is being initialized to zero. At each loop iteration, i is
incremented and a conditional branch is taken/not taken.


## Question 6
If `foo()` is very fast, and the time to perform `for (i = 0; i < N; ++i)` may
be significant (relative to the time needed to call `foo()`), how could you make
your final measurement value include *only* the average time required to call
`foo()`?

### Answer:
Time an empty loop of size N, and subtract that time out at the end.


# Part B
Now that you've run all your experiments, answer the questions in "Part B".
You should **complete these questions on your own**, without discussing the
answers with anyone (unless you have questions for the instructor or TAs, of
course). Each question should only require approximately *a couple sentences*
to answer properly, so don't write an entire that isn't needed.


## Question 7
What was your general strategy for getting good measurements for all the
items?  (i.e., things you did in common for all of them, not the one-off
adjustments you had to figure out just for specific ones)

### Answer:
I always took many measurements and divided out the amount of measurements to
get a consistently good average measurement.

## Question 8
What measurement result did you get for each of the six measurements?  Based on
these results, which functions do you think are system calls (syscalls) and
which do you think are not?

### Answer:

* allocate one page of memory with `mmap()`
  * user time:       2.568900e-08
  * system time:     3.001990e-07

* lock a mutex with `pthread_mutex_lock()`
  * user time:       8.925999e-09
  * system time:     -4.820000e-10

* writing 4096 Bytes directly (bypassing the disk page cache) to /tmp
  * wall-clock time: 9.321817e-04

* reading 4096 Bytes directly (bypassing the disk page cache) from /tmp
  * wall-clock time: 2.859169e-04

* writing 4096 Bytes to the disk page cache
  * wall-clock time: 2.752250e-06

* reading 4096 Bytes from the disk page cache
  * wall-clock time: 2.834000e-07

* Syscalls:     getrusage, timeval, mmap(), read/write
* Not syscalls: mutex 



## Question 9
What is the memory page size?  (i.e., that you used with `mmap`)

### Answer:
4096 Bytes


## Question 10
How did you deal with the problem of not being able to lock a mutex
more than once without unlocking it first?

### Answer:
I mad an array to keep track of all of the mutex.


## Question 11
Was performance affected by whether a file access operation is a read or write?
If so, how?

### Answer:
Writing takes longer on the homework VMs.


## Question 12
What affect did the disk page cache have on file access performance?  Did it
affect reads and write differently?

### Answer:
Using the disk page cache was a lot faster, reading was faster than writing still.



provide comp111 measurement README.md Makefile timing.c