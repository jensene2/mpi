// Create include guard.
#ifndef __LATINSQUARE_C_INCLUDED__
#define __LATINSQUARE_C_INCLUDED__

// Include dependencies.
#include <algorithm>
#include <cmath>
#include <vector>

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

	static LatinSquare createHalter(int size) {
		LatinSquare square = LatinSquare(size);

		square.set(0, 0, -2);

		return square;
	}

	vector<int> getRow(int x) {
		vector<int> row;

		for (int y = 0; y < this->size; y++) {
			row.push_back(this->get(x, y));
		}

		return row;
	}

	vector<int> getColumn(int y) {
		vector<int> column;

		for (int x = 0; x < this->size; x++) {
			column.push_back(this->get(x, y));
		}

		return column;
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

	bool isHalter() {
		return (this->get(0, 0) == -2);
	}

	bool isValid() {
		vector<int> row;
		vector<int> column;

		for (int i = 0; i < this->getSize(); i++) {
			row = getRow(i);
			column = getColumn(i);

			sort(row.begin(), row.end());
			sort(column.begin(), column.end());

			for (int j = 0; j < row.size()-1; j++) {
				if (row[j] == row[j+1] && row[j] != -1) {
					return false;
				}

				if (column[j] == column[j+1] && column[j] != -1) {
					return false;
				}
			}
		}

		return true;
	}

	vector<int> getBody() {
		return this->body;
	}

	int getSize() {
		return this->size;
	}
};

#endif
