// Create include guard.
#ifndef __QUEUE_C_INCLUDED__
#define __QUEUE_C_INCLUDED__

// Include dependencies.
#ifndef __LATINSQUARE_C_INCLUDED__
#include "latinsquare.c"
#endif


// --------------------------------------------------------------------------------------
//
// TO DO: This needs to queue things not in memory. But for testing, it'll do.
//
// --------------------------------------------------------------------------------------

using namespace std;
class Queue {
    // Temporary memory for the queue.
    vector<LatinSquare> q;

public:
    void push(LatinSquare square) {
        this->q.push_back(square);
    }

    LatinSquare get() {
        LatinSquare square = this->q.back();
        this->q.pop_back();

        return square;
    }

    bool isEmpty() {
        return this->q.empty();
    }
};

#endif
