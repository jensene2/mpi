// Create include guard.
#ifndef __WORKER_C_INCLUDED__
#define __WORKER_C_INCLUDED__

// Include dependencies.

using namespace std;

class Worker {
	MPI_Comm *row;
	int rowRank;
	int size;
public:
	Worker(int size, MPI_Comm *row) {
		this->row = row;

		MPI_Comm_rank(*row, &this->rowRank);
		this->size = size;
	}

	void shutdown() {
		// Shutdown the communicators.
		MPI_Comm_free(this->row);
	}

	void run() {
		// Receive a square from the coordinator.
		LatinSquare square = this->receive();

		// Loop until a halter square is recieved.
		while (!square.isHalter()) {
			// Get the coordinate of the first unfinished row.
			int rowCoordinate = square.getFirstUnfinishedRowCoordinate();

			// Create a row to permute over.
			vector<int> row;
			for (int i = 0; i < size; i++) {
				row.push_back(i);
			}

			// std::next_permutation turns out to be perfect for generating
			//   and iterating over permutations. Who could have guessed?
			do {
				// Overwrite the existing square with the new row.
				for (int i = 0; i < size; i++) {
					square.set(rowCoordinate, i, row[i]);
				}

				// If it's valid, send it to a receiver.
				if (square.isValid()) {
					this->send(square);
				}
			} while (next_permutation(row.begin(), row.end()));

			// Create an empty square to let the coordinator that this worker
			//   has finished. The receiver will pass the message along.
			square = LatinSquare(size);
			this->send(square);

			// Receive a new square from the coordinator.
			square = this->receive();
		}

		// If this node has a rowRank of 1, notify the receiver in the row.
		if (this->rowRank == 1) {
			this->send(square);
		}
	}

	LatinSquare receive() {
		// Create a  vector to fill the data into.
		vector<int> vector;

		// Resize the vector to fit all the data.
		vector.resize(this->size * this->size);

		// Receive data from the coordinator.
		MPI_Recv(vector.data(), vector.size(), MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		// Construct a latin square from the vector.
		return LatinSquare(vector);
	}

	void send(LatinSquare square) {
		// Get the internal vector and send it the row's receiver.
		MPI_Send(square.getBody().data(), square.getBody().size(), MPI_INT, 0, 2, *this->row);
	}
};

#endif
