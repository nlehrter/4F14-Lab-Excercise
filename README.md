A lab exercise for a Course on Computer architecture and multithreading.

The aim of this exercise is to create a queue of 255 random strings. The queue is then iterated through in three threads
simaultaneously, with one printing the queue, one deleting a random item, and one reversing the queue. This process is
repeated until the queue is empty

A number of solutions to the coursework task were investigated, exploring different components of the lecture
series. The first complete solution involved a global lock on a vector of tuples for every thread operation, this is 
the baseline solution which is provided at the bottom commented out. Next a solution was tested 
which involved creating 3 vectors, one for each thread, and upating the master vector, the one in the thread 
that reads and prints the queue, at the end of a complete reversing or deleting operation. The delete operation would
update both the reversing vector and the printing one, and be updated by the reversing thread. This solution is provided
immediately below.
