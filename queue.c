// Create include guard.
#ifndef __QUEUE_C_INCLUDED__
#define __QUEUE_C_INCLUDED__

// Include dependencies.
#ifndef __LATINSQUARE_C_INCLUDED__
#include "latinsquare.c"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <dirent.h>
#include <stdio.h>

#include <sys/time.h>

void setupQueue();
void cleanQueue();

char directory[] = "./queue/";

using namespace std;
class Queue {
	int id;
public:
	Queue(int id) {
		this->id = id;
	}

	void push(LatinSquare square) {
		struct timeval tv;
		gettimeofday(&tv, NULL); // TO DO: Check the error on this.
		unsigned long long microtime = 1000000 * tv.tv_sec + tv.tv_usec;

		vector<int> body = square.getBody();
		char str[30];
		sprintf(str, "%s%d-%llu", directory, this->id, microtime);

		ofstream tmpfile (str, ofstream::out);
		for (int i = 0; i < body.size()-1; i++) {
			tmpfile << body[i] << " ";
		}

		tmpfile << body[body.size() - 1] << endl;

		tmpfile.flush();
		tmpfile.close();
	}

	LatinSquare get() {
		DIR *d;
		struct dirent *dir;
		vector<int> v;

		d = opendir(directory);
		if (d) {
			while ((dir = readdir(d)) != NULL) {
				if (strcmp(dir->d_name, ".") == 0) {
					continue;
				}

				if (strcmp(dir->d_name, "..") == 0) {
					continue;
				}

				v.clear();

				// Read the file into a latin square.
				char filename[30];
				sprintf(filename, "%s%s", directory, dir->d_name);
				ifstream inputFile(filename);

				if (inputFile) {
					int value;

					while (inputFile >> value) {
						v.push_back(value);
					}
				}

				inputFile.close();
				remove(filename);
				break;
			}

			closedir(d);

			return LatinSquare(v);
		}

		return LatinSquare(1);
	}

	bool isEmpty() {
		DIR *d;
		struct dirent *dir;

		d = opendir(directory);
		if (d) {
			int count = 0;

			while ((dir = readdir(d)) != NULL) {
				count++;

				// At minimum, the directory will contain '.' and '..'. It
				//   is not empty if it contains more than that.
				if (count > 2) {
					closedir(d);
					return false;
				}
			}
			closedir(d);
		} else {
			cout << "Failed to open directory! PANIC" << endl;
			cout << "Error opening directory: " << strerror(errno) << endl;
		}

		return true;
	}
};

void setupQueue() {
	// Create Queue directory.
	int result = mkdir(directory, 0700);
	if (result != 0) {
		cout << "Error creating directory: " << strerror(errno) << endl;
	}
}

void cleanQueue() {
	// Remove Queue directory.
	int result = rmdir("./queue");
	if (result != 0) {
		cout << "Error removing directory: " << strerror(errno) << endl;
	}
}

#endif
