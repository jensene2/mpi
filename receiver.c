// Create include guard.
#ifndef __RECEIVER_C_INCLUDED__
#define __RECEIVER_C_INCLUDED__

// Include dependencies.

using namespace std;

class Receiver {
	MPI_Comm *receivers, *row;
	int receiverSize, rowSize;
	int receiverRank;
	int size;
public:
	Receiver(int size, MPI_Comm *receivers, MPI_Comm *row) {
		MPI_Comm_size(*receivers, &this->receiverSize);
		MPI_Comm_size(*row, &this->rowSize);

		this->receivers = receivers;
		this->row = row;

		MPI_Comm_rank(*receivers, &this->receiverRank);
		this->size = size;
	}

	void shutdown() {
		// Shutdown the communicators.
		MPI_Comm_free(this->receivers);
		MPI_Comm_free(this->row);
	}

	void run() {
		LatinSquare square = this->receive();

		Queue queue = Queue(this->receiverRank);

		while (!square.isHalter()) {
			if (square.isFinished()) {
				this->send(square);
			} else if (square.isEmpty()) {
				this->send(square);
			} else {
				queue.push(square); // HERE <-------------------------------------------------------------------
				// Somehow empty squares have the chance of getting here.
				// This should not be possible. It seems to come at the end.
				//   Maybe call date somehow and append that to the "Empty ..."
				//   message?
				// Possibly also do a check again to see if it is empty here.
				//   It means the receivers are slower, but whatever.
			}

			square = this->receive();
		}
	}

	LatinSquare receive() {
		// Create a  vector to fill the data into.
		vector<int> vector;

		// Resize the vector to fit all the data.
		vector.resize(this->size * this->size);

		// Receive data from any of the worker nodes in the row.
		MPI_Recv(vector.data(), vector.size(), MPI_INT, MPI_ANY_SOURCE, 2, *this->row, MPI_STATUS_IGNORE);

		// Construct a latin square from the vector.
		return LatinSquare(vector);
	}

	void send(LatinSquare square) {
		// Get the internal vector and send it to the coordinator.
		MPI_Send(square.getBody().data(), square.getBody().size(), MPI_INT, 0, 0, *this->receivers);
	}
};

#endif
