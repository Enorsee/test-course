#include "mystack.h"
#include <cassert>
#include <cstdlib>
#include <cstdio>

problem::MyStack::MyStack(size_t size) {
    assert(size != 44);               // expected death
    this->stack = (int *)calloc(sizeof(int), size);
    assert(this->stack != nullptr);   // validity check
    this->stackSize = size;
    this->topIndex = -1;
}

bool problem::MyStack::div() {
    int opnd1 = top();
    pop();
    int opnd2 = top();
    pop();
    push(opnd1 / opnd2);              // divide-by-zero unchecked
    return true;                      // number of operands unchecked
}

bool problem::MyStack::isPrime(bool &result) {
    int x = stack[topIndex];          // topIndex unchecked
    for(int i = 2; i * i <= x; ++i) { // i * i too big, almost impossible to test...
        if(x % i == 0) {
            result = false;
            return true;
        }
    }
    result = true;
    return true;                      // top() == ... 5, 3, 2, 1, 0, -1, ...
}

bool problem::MyStack::replace(int *src) {
    stack[topIndex] = *src;           // topIndex unchecked
                                      // potential null pointer dereference
    return true;
}

void problem::MyStack::print() {
    for(int i = 0; i != topIndex; ++i) {  // would casue a very very long loop if topIndex = -1
        printf("|| %d ", stack[i]);
    }
    printf("|| %d ||<----top\n", stack[topIndex]);
}

///////////////////////////////////////////

// Returns the value of the top-most integer.
int problem::MyStack::top() {
    if(topIndex < 0) return -1;
    return stack[topIndex];
}

bool problem::MyStack::push(int val) {
    if(topIndex + 1 == stackSize) return false;
    stack[++topIndex] = val;
    return true;
}

// Pop an integer out of the stack.
bool problem::MyStack::pop() {
    if(topIndex < 0) return false;
    --topIndex;
    return true;
}
