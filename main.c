#include "mpi.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

// #include "latinsquare.c"
// #include "queue.c"

#include "coordinator.c"
#include "receiver.c"
#include "worker.c"

using namespace std;

// LatinSquare receiveLatinSquare(int size, int source, int tag);
// void sendLatinSquare(LatinSquare square, int source, int tag);
// void coordinator(int size, int worldSize);
// void worker(int size);
// void clean();
// void setup();

int main(int argc, char *argv[]) {
	// Creates a char array, aka a string.
	char hostname[MPI_MAX_PROCESSOR_NAME];

	// MPI Setup.
	int resultLength, worldRank, worldSize;
	MPI_Init(&argc, &argv);
	MPI_Get_processor_name(hostname, &resultLength);
	MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

	int numReceivers = worldSize / 10;
	if (numReceivers == 0) {
		numReceivers = 1;
	}

	vector<int> receiverRanks, workerRanks;
	for (int i = 0; i < worldSize; i++) {
		if (i <= numReceivers) {
			receiverRanks.push_back(i);
		} else {
			workerRanks.push_back(i);
		}
	}

	MPI_Group worldGroup;
	MPI_Comm_group(MPI_COMM_WORLD, &worldGroup);

	MPI_Group receiverGroup, workerGroup;
	MPI_Group_incl(worldGroup, receiverRanks.size(), &receiverRanks[0], &receiverGroup);
	MPI_Group_incl(worldGroup, workerRanks.size(), &workerRanks[0], &workerGroup);

	MPI_Comm receivers, workers, row;
	MPI_Comm_create_group(MPI_COMM_WORLD, receiverGroup, 0, &receivers);
	MPI_Comm_create_group(MPI_COMM_WORLD, workerGroup, 0, &workers);

	// Set up the row communicators.
	if (worldRank == 0) {
		// The coordinator receives its own color so it isn't involved with
		//   any other node.
		MPI_Comm_split(MPI_COMM_WORLD, numReceivers, 0, &row);
	} else {
		if (worldRank <= numReceivers) {
			MPI_Comm_split(MPI_COMM_WORLD, worldRank % numReceivers, 0, &row);
		} else {
			MPI_Comm_split(MPI_COMM_WORLD, worldRank % numReceivers, worldRank, &row);
		}
	}

	int size;
	if (worldRank == 0) {
		if (argc < 2) {
			cout << "A size argument is required." << endl;
			return 1;
		}
		istringstream ss(argv[1]);
		if (!(ss >> size)) {
			size = 0;
		}

		MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
		if (size < 1) {
			cout << "The size must be an integer greater than 0." << endl;
			return 1;
		}

		Coordinator coordinator = Coordinator(size, &receivers);
		coordinator.setup();
		coordinator.run();
		coordinator.shutdown();
		coordinator.clean();
	} else {
		if (argc < 2) {
			return 1;
		}

		MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
		if (size < 1) {
			return 1;
		}

		if (worldRank <= numReceivers) {
			Receiver receiver = Receiver(size, &receivers, &row);
			receiver.run();
			receiver.shutdown();
		} else {
			Worker worker = Worker(size, &row);
			worker.run();
			worker.shutdown();
		}
	}

	// MPI Cleanup.
	MPI_Group_free(&receiverGroup);
	MPI_Group_free(&workerGroup);

	if (receivers != MPI_COMM_NULL) {
		MPI_Comm_free(&receivers);
	}
	if (workers != MPI_COMM_NULL) {
		MPI_Comm_free(&workers);
	}
	if (row != MPI_COMM_NULL) {
		MPI_Comm_free(&row);
	}

	MPI_Finalize();
}
