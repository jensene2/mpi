// Create include guard.
#ifndef __COORDINATOR_C_INCLUDED__
#define __COORDINATOR_C_INCLUDED__

// Include dependencies.
#ifndef __LATINSQUARE_C_INCLUDED__
#include "latinsquare.c"
#endif

#ifndef __QUEUE_C_INCLUDED__
#include "queue.c"
#endif

using namespace std;

class Coordinator {
	MPI_Comm *receivers, *workers;
	int receiverSize, workerSize;
	int size;
	int count;

	fstream outputFile;
public:
	Coordinator(int size, MPI_Comm *receivers, MPI_Comm *workers) {
		MPI_Comm_size(*receivers, &this->receiverSize);
		MPI_Comm_size(*workers, &this->workerSize);

		this->receivers = receivers;
		this->workers = workers;

		this->size = size;
		this->count = 0;
	}

	void setup() {
		setupQueue();
	}

	void shutdown() {
		// Create a halter square.
		LatinSquare square = LatinSquare::createHalter(this->size);

		// Send the halter square to the workers. If the worker is rank #1
		//   in its row, it will pass it on to the row's receiver.
		for (int i = 1; i < this->workerSize; i++) {
			this->send(square, i);
		}

		// Shutdown the communicators.
		MPI_Comm_free(this->receivers);
	}

	void clean() {
		cleanQueue();
	}

	void run() {
		// Create an initial empty latin square.
		LatinSquare square = LatinSquare(this->size);

		// Assume that it is a reduced latin square.
		for (int i = 0; i < size; i++) {
			square.set(0, i, i);
			square.set(i, 0, i);
		}

		// Create a queue of some store unfinished latin squares.
		Queue queue = Queue(0);
		queue.push(square);

		while (!queue.isEmpty()) {
			int workersDispatched = 0;

			// Dispatch workers until the queue is either empty or every
			//   worker has been given a job.
			for (int i = 1; !queue.isEmpty() && i < this->workerSize; i++) {
				// Receive a new square from the queue.
				square = queue.get();

				// If an empty one got into the queue, just skip it. This
				//   isn't something that should happen, but be safe.
				if (square.isEmpty()) {
					printf("Empty square discovered in queue.\n");
					printf("Printing square to verify.\n");
					for (int x = 0; x < square.getSize(); x++) {
						for (int y = 0; y < square.getSize(); y++) {
							cout << square.get(x, y) << " ";
						}
						cout << endl;
					}
					cout << endl;
					continue;
				}

				// Dispatch a worker to work on finishing the square.
				this->send(square, i);
				workersDispatched++;
			}

			// printf("%d workers dispatched.\n", workersDispatched);

			// While there are workers still working, receive squares from
			//   the receivers.
			while (workersDispatched != 0) {
				square = this->receive();

				// An empty square means a worker finished.
				if (square.isEmpty()) {
					// printf("Empty received.\n");
					workersDispatched--;
				} else {
					this->count++;

					// --------------------------------------------
					//
					// Write the output.
					//
					// --------------------------------------------
				}
			}
		}

		cout << this->count << " reduced latin squares." << endl;
	}

	LatinSquare receive() {
		// Create a  vector to fill the data into.
		vector<int> vector;

		// Resize the vector to fit all the data.
		vector.resize(this->size * this->size);

		// Receive data from any of the receiver nodes.
		MPI_Recv(vector.data(), vector.size(), MPI_INT, MPI_ANY_SOURCE, 0, *this->receivers, MPI_STATUS_IGNORE);

		// Construct a latin square from the vector.
		return LatinSquare(vector);
	}

	void send(LatinSquare square, int dest) {
		// Get the internal vector and send it to any worker.
		MPI_Send(square.getBody().data(), square.getBody().size(), MPI_INT, dest, 1, *this->workers);
	}
};

#endif
