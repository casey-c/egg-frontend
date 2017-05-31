#ifndef QUANTUMBOOL_H
#define QUANTUMBOOL_H


/*
 * Basic datastructure that extends a boolean. A QuantumBool, named after the
 * concept in quantum physics, may change its state after being observed.
 *
 * Basically, it's a container for a bool that allows for a one time flag to be
 * set. A return value is given for the original value and depending on the
 * function called, the QuantumBool will be updated.
 *
 * This function can be made thread-safe, making it safer to use in multi-
 * threaded environments where a context switch may allow multiple threads to
 * work off a single flag. NOTE: this is not yet implemented.
 *
 * ---------------------------
 *
 * Original code:
 *
 * bool flag;
 * if ( flag == true ) {
 *   flag = false
 *   doStuff();
 * }
 *
 * ---------------------------
 *
 * With QuantumBool:
 *
 * QuantumBool flag;
 * if ( flag.checkAndClear() )
 *   doStuff();
 *
 * ---------------------------
 *
 * Usage:
 *
 *   check(): simply returns the bool without changing it
 *   checkAndClear(): returns the bool then sets it to false
 *   checkAndSet(): returns the bool then sets it to true
 *   set(): makes it true
 *   clear(): makes it false
 */
class QuantumBool
{
public:
    QuantumBool(bool start = false) : b(start) {}

    bool check() { return b; }

    bool checkAndClear();
    bool checkAndSet();

    void set() { b = true; }
    void clear() { b = false; }

private:
    bool b;
};

#endif // QUANTUMBOOL_H
