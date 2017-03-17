#include "mpi.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "coordinator.c"
#include "receiver.c"
#include "worker.c"

using namespace std;

int main(int argc, char *argv[]) {
	// Creates a char array, aka a string.
	char hostname[MPI_MAX_PROCESSOR_NAME];

	// MPI Setup.
	int resultLength, worldRank, worldSize;
	MPI_Init(&argc, &argv);
	MPI_Get_processor_name(hostname, &resultLength);
	MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

	// printf ("Number of tasks = %d; My rank = %d; Running on %s;\n", worldSize, worldRank, hostname);

	int numReceivers = worldSize / 10;
	if (numReceivers == 0) {
		numReceivers = 1;
	}
	// numReceivers = 2;

	vector<int> receiverRanks, workerRanks;
	workerRanks.push_back(0);
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

		// Check if the receivers world exists.
		if (receivers == MPI_COMM_NULL) {
			printf("Coordinator - The receiver world is null.\n");
			return 1;
		}

		// Check if the workers world exists.
		if (workers == MPI_COMM_NULL) {
			printf("Coordinator - The worker world is null.\n");
			return 1;
		}

		// int receiverRank, workerRank;
		// MPI_Comm_size(receivers, &receiverRank);
		// MPI_Comm_size(workers, &workerRank);
		// printf ("Coordinator; Receiver rank = %d; Worker rank = %d;\n", receiverRank, workerRank);

		Coordinator coordinator = Coordinator(size, &receivers, &workers);
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

		// Check if the row world exists.
		if (row == MPI_COMM_NULL) {
			printf("The row world is null.\n");
			return 1;
		}

		if (worldRank <= numReceivers) {
			// Check if the receiver world exists.
			if (receivers == MPI_COMM_NULL) {
				printf("Receiver - The receiver world is null.\n");
				return 1;
			}

			// int receiverRank, rowRank;
			// MPI_Comm_size(receivers, &receiverRank);
			// MPI_Comm_size(row, &rowRank);
			// printf ("Receiver; Rank = %d; Receiver rank = %d; Row rank = %d;\n", worldRank, receiverRank, rowRank);

			Receiver receiver = Receiver(size, &receivers, &row);
			receiver.run();
			receiver.shutdown();
		} else {
			// Check if the workers world exists.
			if (workers == MPI_COMM_NULL) {
				printf("Worker - The worker world is null.\n");
				return 1;
			}

			// int rowRank, workerRank;
			// MPI_Comm_size(row, &rowRank);
			// MPI_Comm_size(workers, &workerRank);
			// printf ("Worker; Rank = %d; Workers rank = %d; Row rank = %d;\n", worldRank, workerRank, rowRank);

			Worker worker = Worker(size, &workers, &row);
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
