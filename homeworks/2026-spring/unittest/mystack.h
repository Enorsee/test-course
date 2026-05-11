#ifndef __MYSTACK_H__
#define __MYSTACK_H__

#include <cstddef>

namespace problem {

    class MyStack{
    public:
        // ********* Methods that you are required to test: *********

        /************************************************
        * Description: 
        *   A constructor that initialises an integral 
        * stack of size 'size'.
        * 
        * Design Requirements: 
        *   The author hates number 44, so the program 
        * should be unconditionally terminated if size 
        * be given a 44. 
        ************************************************/
        MyStack(size_t size);


        /************************************************
        * Description: 
        *   This method pops two top-most operands on the 
        * stack out to do division, then pushes the result 
        * back.
        *   E.g., if the stack is  
        *       ||...|| val3 || val2 || val1 ||<--- (Top), 
        * then it becomes 
        *       ||...|| val3 || val1 / val2 ||<--- (Top) 
        * after div()'s called.
        * 
        * Returns: 
        *   false if anything wrong happened;
        *   true if successful. 
        * 
        * Design Requirements: 
        *   Calling this method SHOULDN'T crash the whole 
        * program.
        ************************************************/
        bool div();


        /************************************************
        * Description: 
        *   This method checks if the topmost integer on 
        * the stack is a prime number, and stores the 
        * reuslt into somewhere REFERENCED(Not pointed to) 
        * by 'result'.
        * 
        * Returns: 
        *   false if anything wrong happened;
        *   true if successful.
        * 
        * Design Requirements: 
        *   Calling this method SHOULDN'T crash the whole 
        * program.
        ***********************************************/
        bool isPrime(bool &result);


        /***********************************************
        * Description: 
        *   This method replaces the top-most integer on 
        * the stack with that pointed to by 'src'.
        * 
        * Returns: 
        *   false if anything wrong happened;
        *   true if successful.
        * 
        * Design Requirements: 
        *   Calling this method SHOULDN'T crash the whole 
        * program.
        ***********************************************/
        bool replace(int *src);


        /***********************************************
        * Description: 
        *   This method prints the stack from bottom up 
        * to top on the standard output device.
        *  
        * Design Requirements: 
        *   Calling this method SHOULDN'T crash the whole 
        * program.
        ***********************************************/
        void print();


        // ******** Methods that you can use but NOT required to test: *******

        // Returns the value of the top-most integer. 
        // Returns -1 if there's no element on the stack.
        int top();

        // Pushes an integer on to the stack, whose value is 'val'.
        // Returns true if successful.
        bool push(int val);

        // Pops an integer out of the stack.
        // Returns true if successful.
        bool pop();

    private:
        int *stack = NULL;
        int topIndex = -1;
        size_t stackSize = 0;
    };

} // namespace problem

#endif // __MYSTACK_H__
