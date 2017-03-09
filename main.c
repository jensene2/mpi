#include "mpi.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "latinsquare.c"
#include "queue.c"

using namespace std;

LatinSquare receiveLatinSquare(int size, int source, int tag);
void sendLatinSquare(LatinSquare square, int source, int tag);
void coordinator(int size, int worldSize);
void worker(int size);
void clean();
void setup();

int main(int argc, char *argv[]) {
    // Creates a char array, aka a string.
    char hostname[MPI_MAX_PROCESSOR_NAME];

    // MPI Setup.
    int resultLength, worldRank, worldSize;
    MPI_Init(&argc, &argv);
    MPI_Get_processor_name(hostname, &resultLength);
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    // printf ("Number of tasks= %d My rank= %d Running on %s\n", worldSize, worldRank, hostname);

    // Designate by rank. 0 is the coordinator, the rest are workers.
    int size;
    if (worldRank == 0) {
        // Get the size.
        printf("Enter the size: ");
        scanf("%d", &size);

        // Share the size with the workers.
        MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);

		// Setup.
		// If any of this fails, this should be changed to gracefully crash.
		setup();

		// Run coordinator.
        coordinator(size, worldSize);

		// Clean up.
		clean();
    } else if (worldRank != 0) {
        // Receive the size and share with other workers.
        MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);

        worker(size);
    }

		// MPI Cleanup.
		MPI_Finalize();
}

void setup() {
	setupQueue();
}

void clean() {
	cleanQueue();
}

void coordinator(int size, int worldSize) {
	// Create an initial empty latin square.
    LatinSquare square = LatinSquare(size);

    // For now assume reduced latin squares. So force this one to be reduced.
    for (int i = 0; i < size; i++) {
        square.set(0, i, i);
        square.set(i, 0, i);
    }

    // Keeping a count would be a pain to manage because of potential overflow.
    //   Instead, use a Queue object that can push and retrieve latin squares
    //   and has a isEmpty() method as a stopping condition.
    Queue queue = Queue();
    queue.push(square);

    int count = 0;
    while (!queue.isEmpty()) {
        int workersDispatched = 0;

        // Send latin squares to all workers until either there are no free
        //   workers or the queue is empty. Duplicate the !queue.isEmpty()
        //   here because there are two exit conditions for this loop.
        for (int i = 1; !queue.isEmpty() && i < worldSize; i++) {
			// Get a square.
			// cout << "Getting a square." << endl;
            square = queue.get();

			// cout << "Received square from queue." << endl;
			// for (int x = 0; x < square.getSize(); x++) {
			// 	for (int y = 0; y < square.getSize(); y++) {
			// 		cout << square.get(x, y) << " ";
			// 	}
			// 	cout << endl;
			// }
			// cout << endl;

            // Make sure not to send empty squares;
            if (square.isEmpty()) {
				// cout << "Ooops, it's empty." << endl;
                continue;
            }

			// cout << "Sending square." << endl;
			// for (int x = 0; x < square.getSize(); x++) {
			// 	for (int y = 0; y < square.getSize(); y++) {
			// 		cout << square.get(x, y) << " ";
			// 	}
			// 	cout << endl;
			// }
			// cout << endl;

            sendLatinSquare(square, i, 0);
            workersDispatched++;
        }

        // Collect the results. A worker will denote that it is done by
        //   sending back an empty latin square (all -1s).
        while (workersDispatched != 0) {
            square = receiveLatinSquare(size, MPI_ANY_SOURCE, 0);

			// cout << "Received square from worker." << endl;
			// for (int x = 0; x < square.getSize(); x++) {
			// 	for (int y = 0; y < square.getSize(); y++) {
			// 		cout << square.get(x, y) << " ";
			// 	}
			// 	cout << endl;
			// }
			// cout << endl;

            if (square.isFinished()) {
                count++;

                // --------------------------------------------------------------------------------------
                //
                // TO DO: Figure out file writing for the output.
                // For now, do nothing.
                //
                // --------------------------------------------------------------------------------------

            } else {
				// If the square received is empty, then the worker finished.
                if (square.isEmpty()) {
                    workersDispatched--;
                } else {
					queue.push(square);
				}
            }

			// cout << "Queue isEmpty: " << queue.isEmpty() << endl;
        }
    }

    // The entire queue is finished, that should mean all have been processed.
    //   The coordinator needs to shut down the workers at this point. The
    //   workers have already started listening for their next work, sending
    //   an empty latin square can signal to stop.
    square = LatinSquare(size);
    for (int i = 1; i < worldSize; i++) {
        sendLatinSquare(square, i, 0);
    }

    // All of them should be stopping or stopped, so this is safe to
    //   return to clean up.

    cout << count << " reduced latin squares." << endl;
}

void worker(int size) {
    // Receive a square from the coordinator.
    LatinSquare square = receiveLatinSquare(size, 0, 0);

	// cout << "Received square from coordinator." << endl;
	// for (int x = 0; x < square.getSize(); x++) {
	// 	for (int y = 0; y < square.getSize(); y++) {
	// 		cout << square.get(x, y) << " ";
	// 	}
	// 	cout << endl;
	// }
	// cout << endl;

    // Start a loop that runs until an empty latin square is received.
    while (!square.isEmpty()) {
        // Find the first unfinished row.
        int rowCoordinate = square.getFirstUnfinishedRowCoordinate();

        // Since the validity is being checked, may as well generate all rows
        //   and check what works. It's easier. So create a row to permute over.
        vector<int> row;

        for (int i = 0; i < size; i++) {
            row.push_back(i);
        }

        // std::next_permutation, turns out, is perfect for permutations.
        //   Who could have guessed that?
        do {
            for (int i = 0; i < size; i++) {
                square.set(rowCoordinate, i, row[i]);
            }

			// cout << "Testing square." << endl;
			// for (int x = 0; x < square.getSize(); x++) {
			// 	for (int y = 0; y < square.getSize(); y++) {
			// 		cout << square.get(x, y) << " ";
			// 	}
			// 	cout << endl;
			// }
			// cout << endl;

            // Check if square is valid.
            if (square.isValid()) {
                sendLatinSquare(square, 0, 0);
            }
        } while (std::next_permutation(row.begin(), row.end()));

        // Get a new square to work on. Alert the coordinator by sending an
        //   empty latin square.
        square = LatinSquare(size);
        sendLatinSquare(square, 0, 0);

        // Receive the new square.
        square = receiveLatinSquare(size, 0, 0);
    }

    // An empty square was encountered. This should only happen if the
    //   coordinator has signaled a stop. This can return safely to clean up.
}

LatinSquare receiveLatinSquare(int size, int source, int tag) {
    // Create a vector to fill the data into.
    vector<int> vector;

    // Resize it so it'll fit everything.
    vector.resize(size * size);

    // MPI Receive call.
    MPI_Recv(vector.data(), vector.size(), MPI_INT, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Construct a new latin square from the vector.
    return LatinSquare(vector);
}

void sendLatinSquare(LatinSquare square, int dest, int tag) {
    // Get the internal vector and send it.
    MPI_Send(square.getBody().data(), square.getBody().size(), MPI_INT, dest, tag, MPI_COMM_WORLD);
}
