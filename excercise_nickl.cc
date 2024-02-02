/***********************************************************************************************************************
*
*  A number of solutions to the coursework task were investigated, exploring different components of the lecture
*  series. The first complete solution involved a global lock on a vector of tuples for every thread operation, this is 
*  the baseline solution which is provided at the bottom commented out. Next a solution was tested 
*  which involved creating 3 vectors, one for each thread, and upating the master vector, the one in the thread 
*  that reads and prints the queue, at the end of a complete reversing or deleting operation. The delete operation would
*  update both the reversing vector and the printing one, and be updated by the reversing thread. This solution is provided
*  immediately below.
*
***********************************************************************************************************************/

#include <iostream>
#include <thread>
#include <tuple>
#include <vector>
#include <random>
#include <mutex>
#include <chrono>

class Queue_Structure {
    private:
    /*
    * The queue data structure is based on a vector of tuples, each tuple contains a string and an integer, locking structure
    * follows the convention Delete vector lock -> Reverse vector lock -> Master vector lock -> Print lock
    */
        std::vector<std::tuple<std::string, int>> container;
        std::vector<std::tuple<std::string, int>> container_reverse;
        std::vector<std::tuple<std::string, int>> container_delete;
    public:

        //A mutex is required to ensure two different parts of the code are not trying to print at the same time
        std::mutex print_lock; 

        // A global mutex is used to ensure that the vector is only altered by one thread at a time, avoiding broken invariant issues
        std::mutex global_list; 

        // A mutex to lock the queue which is used for the reversing thread 
        std::mutex reverse_lock; 

        // A mutex to lock the delete vector
        std::mutex delete_lock; 
        int prints;

        void fill_queue(void){
            /*
            * Fill the queue with tuples of random integers in the range 0-255 and a string of random characters of random length 
            * in the range 3-7
            */
            const char alph[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
            std::tuple <std::string, int> value;
            int randome;

            for(unsigned int j =0; j <80; j++){
                std::string rand_string;
                
                // Assign a random length for the string
                randome = 3 + std::rand()%5;
                    
                for(int run = 0; run < randome; run++){
                    //run through the randomly assigned length and create a randomly generated string
                    rand_string += alph[rand()%26];
                }
                //Generate a random integer

                randome = std::rand()%255;

                // Assign one tuple to the string, integer pair and push back to the vector
                value = std::make_tuple(rand_string, randome);
                container.push_back(value);
                }
            
            //Set the two auxilliary vectors equal to the original
            container_delete = container;
            container_reverse = container;
        }
        
        void add_queue(std::tuple <std::string, int> new_item){
            /*
            * Add a new member to the back of the queue
            */
            std::unique_lock<std::mutex> deletelock(delete_lock);
            std::unique_lock<std::mutex> reverselock(reverse_lock);
            std::unique_lock<std::mutex> dellock(global_list);
            container.push_back(new_item);
            container_reverse.push_back(new_item);
            container_delete.push_back(new_item);;
        }

        int queue_size(void){
            /*
            * Return the number of elements in the queue
            */
           std::unique_lock<std::mutex> dellock(global_list);
            return container.size();
        }

        std::tuple <std::string, int> queue_element(int element){
            std::unique_lock<std::mutex> dellock(global_list);
            return container[element];
        }

        void remove_item(void){
            /*
            * Remove the first item from the queue, i.e. the oldest item
            */
            std::unique_lock<std::mutex> deletelock(delete_lock);
            std::unique_lock<std::mutex> reverselock(reverse_lock);
            std::unique_lock<std::mutex> dellock(global_list);
            container.erase(container.begin());
            container_reverse.erase(container_reverse.begin());
            container_delete.erase(container_delete.begin());
        }

        std::tuple <std::string, int> queue_front(void){
            /*
            * Return the tuple at the front of the queue
            */ 
           std::unique_lock<std::mutex> dellock(global_list);
            return container[0];
        }
        void print_int(int printer){
            /*
            * Print an integer to the console
            */ 
            std::unique_lock<std::mutex> printlock(print_lock);
            std::cout<< "Integer: " << printer << "\n";
        }


        void reverse_queue(void){
            /*
            * Reverse the ordering of all elements in the queue, summing the values of all integers
            * read during the reversing process
            */

           // Create a holding tuple for the switching process
            std::tuple <std::string, int> holding;
            while(1){
                // lock the delete and reversing vectors so they can't be altered whilst switching occurs
                std::unique_lock<std::mutex> deletelock(delete_lock);
                std::unique_lock<std::mutex> reverselock(reverse_lock);
                int length = container_reverse.size();

                //Ensure the vector is not empty
                if(length != 0){

                    // Create an integer to hold the sum of all integers read during the reversing operation
                    int sum = 0;

                    // Reverse the queue by iterating through the first half of the vecotr and swapping elements 
                    // with the second half counterpart
                    for(int l = 0; l < length/2; l++){
                        holding = container_reverse[l];
                        sum = sum + std::get<1>(holding);
                        container_reverse[l] = container_reverse[length-l-1];
                        sum = sum + std::get<1>(container_reverse[length-l-1]);
                        container_reverse[length-l-1] = holding;
                    }
                    if(length == 1){
                        
                        // If the length is 1, no reversing occurs but a sum is still needed
                        sum = std::get<1>(container_reverse[0]);
                    }

                    // Lock the master queue and write back the reverse queue to it
                    std::unique_lock<std::mutex> dellock(global_list);
                    std::unique_lock<std::mutex> printlock(print_lock);
                    container = container_reverse;
                    container_delete = container_reverse;
                    std::cout<< "Sum of Integers: " << sum << "\n";
                    printlock.unlock();
                    dellock.unlock();
                    
                    
                    reverse_lock.unlock();
                    deletelock.unlock();
                }
                else{

                    // If the queue is empty exit the while loop and allow the thread to join
                    reverse_lock.unlock();
                    break;
                }
                // Sleep to avoid overloading the console
                std::this_thread::sleep_for(std::chrono::milliseconds(15));
            }
        }

        void random_delete(void){
            /*
            * Select tuples at random from the list and remove them, with a 200ms delay, as the selection
            * is random it is not an issue that the element vector which the delete value is selected from
            * is never reversed
            */
            while(1){
                std::unique_lock<std::mutex> deletelock(delete_lock);
                if(container_delete.size() != 0){
                    container_delete.erase(container_delete.begin() + std::rand() % container_delete.size());

                    // Lock both other vectors and write back the vector with the deleted value
                    // Keep consistent locking order, locking the reverse lock and then the global lock
                    std::unique_lock<std::mutex> reverselock(reverse_lock);
                    std::unique_lock<std::mutex> dellock(global_list);
                    
                    container = container_delete;
                    container_reverse = container_delete;
                    dellock.unlock();
                    reverselock.unlock();
                }
                else{
                    break;
            }
            deletelock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }

        void print_all(void){
            /*
            * Iterate through the list elements and print all of the Integer, String pairs
            */

           // Use an integer to track how many prints are executed during the entire thread run
            prints =0;
            while(1){
            std::unique_lock<std::mutex> dellock(global_list);

            // Implement a basic try catch sequence despite the safety in use of lock guards
            try{
                int length = container.size();
                if( length == 0){

                    // If the queue is empty break out of while loop and exit thread
                    dellock.unlock();
                    break;
                }
                else{
                    // Increment the number of prints to compare with other implementations
                    prints ++;
                    std::unique_lock<std::mutex> printlock(print_lock);
                    try{
                        for(int pair = 0; pair < length; pair ++){
                            int Integer = std::get<1>(container[pair]);
                            std::string String = std::get<0>(container[pair]);
                            std::cout << "Integer: " << Integer << ", string:" <<  String << "\n";
                        }
                        std::cout << "\n";
                        printlock.unlock();
                    }
                    catch(...){
                            printlock.unlock();
                        }
                }
            }
            catch(...){
                dellock.unlock();
            }
            dellock.unlock();

            // Sleep to avoid overloading the console
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
};

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    Queue_Structure queue;
    queue.fill_queue();

    // Begin the three threads required
    std::thread printing_thread(&Queue_Structure::print_all, &queue);
    std::thread deleting_thread(&Queue_Structure::random_delete, &queue);
    std::thread reversing_thread(&Queue_Structure::reverse_queue, &queue);

    // Wait for all threads to finish executing, when the queue is empty
    deleting_thread.join();
    printing_thread.join();
    reversing_thread.join();

    // For performance results, return the elapsed time and the total number of queue prints
    auto stop = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Number of prints: " << queue.prints <<" Time Elapsed: " << time.count() << std::endl;
}

/*******************************************************************************


          First solution to the problem is included below, it involves global 
          locking of the entire vector in order to undertake any operations on
          it. It is a complete solution to the question but no two threads 
		  can operate on the vector at the same time.


*******************************************************************************/
/*
#include <iostream>
#include <thread>
#include <tuple>
#include <vector>
#include <random>
#include <mutex>
#include <chrono>

class Queue_Structure {
    private:
    
    // The queue data structure is based on a vector of tuples, each tuple contains a string and an integer
    
        std::vector<std::tuple<std::string, int>> container;

    public:
        std::mutex print_lock;  //A mutex is required to ensure two different parts of the code are not trying to print at the same time
        std::mutex global_list; // A global mutex is used to ensure that the vector is only altered by one thread at a time, avoiding broken invariant issues
        int prints;

        void fill_queue(void){
            // Fill the queue with tuples of random integers in the range 0-255 and a string of random characters of random length 
            // in the range 3-7
            const char alph[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
            std::tuple <std::string, int> value;
            int randome;

            for(unsigned int j =0; j <80; j++){
                std::string rand_string;

                // Create a random integer between 3 and 7
                randome = 3 + std::rand()%5;
                    
                for(int run = 0; run < randome; run++){
                    // Create a random string of random length
                    rand_string += alph[rand()%26];
                }
                randome = std::rand()%255;

                // Make the string, integer pair into a tuple
                value = std::make_tuple(rand_string, randome);
                container.push_back(value);
                }
        }
        
        void add_queue(std::tuple <std::string, int> new_item){
            // Add a new member to the back of the queue
            std::unique_lock<std::mutex> globallock(global_list);
            container.push_back(new_item);
        }

        int queue_size(void){
            // Return the number of elements in the queue
            std::unique_lock<std::mutex> globallock(global_list);
            return container.size();
        }

        std::tuple <std::string, int> queue_element(int element){
            std::unique_lock<std::mutex> globallock(global_list);
            return container[element];
        }

        void remove_item(void){
            // Remove the first item from the queue, i.e. the oldest item
            std::lock_guard<std::mutex> globallock(global_list);
            container.erase(container.begin());
        }

        std::tuple <std::string, int> queue_front(void){
            // Return the tuple at the front of the queue
            return container[0];
        }

        void reverse_queue(void){
            // Reverse the ordering of all elements in the queue, summing the values of all integers
            // read during the reversing process
            
            std::tuple <std::string, int> holding;
            while(1){

                // Lock the queue before working with it
                std::unique_lock<std::mutex> globallock(global_list);
                int length = container.size();

                // Ensure that the queue is not empty, if it is, exit the loop
                if(length != 0){
                    int sum = 0;

                    // Implement a reversing algorithm, swapping each vector element with its opposing counterpart
                    for(int l = 0; l < length/2; l++){
                        holding = container[l];
                        sum = sum + std::get<1>(holding);
                        container[l] = container[length-l-1];
                        sum = sum + std::get<1>(container[length-l-1]);
                        container[length-l-1] = holding;
                    }
                    if(length == 1){
                        // If only one item is present, its integer value is the sum of integers
                        sum = std::get<1>(container[0]);
                    }
                    print_int(sum);
                    globallock.unlock();
                }
                else{
                    globallock.unlock();
                    break;
                }
                // Sleep to avoid overloading the console
                std::this_thread::sleep_for(std::chrono::milliseconds(15));
            }
        }

        void random_delete(void){
            // Select tuples at random from the list and remove them, with a 200ms delay
            while(1){
                std::unique_lock<std::mutex> globallock(global_list);
                if(container.size() != 0){
                    // Generate a random index to delete from the queue
                    container.erase(container.begin() + std::rand() % container.size());
                    globallock.unlock();
                }
                else{
                    globallock.unlock();
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }

        void print_int(int printer){
            // Print an integer to the console
            
            std::unique_lock<std::mutex> printlock(print_lock);
            std::cout<< "Sum of Integers: " << printer << "\n";
            printlock.unlock();
        }

        void print_all(void){
            // Iterate through the list elements and print all of the Integer, String pairs

            // Count the total number of prints, for reference once the thread has joined
            prints = 0;
            while(1){
            std::unique_lock<std::mutex> globallock(global_list);

            // Implement a basic try catch sequence despite the safety in use of lock guards
            try{
                int length = container.size();
                if( length == 0){
                    globallock.unlock();
                    break;
                }
                else{
                    prints ++;
                    std::unique_lock<std::mutex> printlock(print_lock);
                    try{
                        for(int pair = 0; pair < length; pair ++){
                            int Integer = std::get<1>(container[pair]);
                            std::string String = std::get<0>(container[pair]);
                            std::cout << "Integer: " << Integer << ", string:" <<  String << "\n";
                        }
                        std::cout << "\n";
                        printlock.unlock();
                    }
                    catch(...){
                            printlock.unlock();
                        }
                }
            }
            catch(...){
                globallock.unlock();
            }
            globallock.unlock();

            // Sleep to avoid overloading the console
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
};

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    Queue_Structure queue;
    queue.fill_queue();

    std::thread deleting_thread(&Queue_Structure::random_delete, &queue);
    std::thread printing_thread(&Queue_Structure::print_all, &queue);
    std::thread reversing_thread(&Queue_Structure::reverse_queue, &queue);

    deleting_thread.join();
    printing_thread.join();
    reversing_thread.join();
    auto stop = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    // Print the elapsed time and total number of queue prints executed to the console
    std::cout << "Number of prints: " << queue.prints <<" Time Elapsed: " << time.count() << std::endl;
}

*/
