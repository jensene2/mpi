// Create include guard.
#ifndef __LATINSQUARE_C_INCLUDED__
#define __LATINSQUARE_C_INCLUDED__

// Include dependencies.
#include <vector>
#include <cmath>

using namespace std;
class LatinSquare {
    // Use a one dimensional array to hold the body to make it easier to send.
    vector<int> body;

    // Store the size rather than computing square roots whenever it's needed.
    int size;
public:
    LatinSquare(int size) {
        this->size = size;

        for (int x = 0; x < size; x++) {
            for (int y = 0; y < size; y++) {
                this->body.push_back(-1);
            }
        }
    }

    LatinSquare(std::vector<int> body) {
        this->body = body;

        this->size = std::sqrt(body.size());
    }

    vector<int> getColumn(int x) {
        vector<int> column;

        for (int y = 0; y < this->size; y++) {
            column.push_back(this->get(x, y));
        }

        return column;
    }

    vector<int> getRow(int y) {
        vector<int> row;

        for (int x = 0; x < this->size; x++) {
            row.push_back(this->get(x, y));
        }

        return row;
    }

    int getFirstUnfinishedRowCoordinate() {
        for (int y = 0; y < this->size; y++) {
            for (int i : this->getRow(y)) {
                if (i == -1) {
                    return y;
                }
            }
        }
    }

    int get(int x, int y) {
        // x * size + y
        return this->body[(x * this->size) + y];
    }

    void set(int x, int y, int value) {
        this->body[(x * this->size) + y] = value;
    }

    bool isFinished() {
        for (int i = 0; i < this->body.size(); i++) {
            if (this->body[(i)] == -1) {
                return false;
            }
        }

        return true;
    }

    bool isEmpty() {
        for (int i = 0; i < this->body.size(); i++) {
            if (this->body[i] != -1) {
                return false;
            }
        }

        return true;
    }

    bool isValid() {
        // --------------------------------------------------------------------------------------
        //
        // TO DO: Write the code to check if a square is valid.
        // For now, sure. It's valid.
        return true;
        //
        // --------------------------------------------------------------------------------------
    }

    vector<int> getBody() {
        return this->body;
    }

    int getSize() {
        return this->size;
    }
};

#endif
