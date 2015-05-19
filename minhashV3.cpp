/*
VERSION 3: uses parallelism to speed up the hashing

Small program that uses minhash functions to estimate the number of different elements in a stream.

The class MinHash mantains a vector P and coefficients A, B.
For every new element n in the stream, the process method computes the hash functions (A*n+B) mod P[i]
and keeps track of the minimum in the vector M.

The estimate for the number of different elements is the inverse of the min hash.

USE: ./minhash numDist numStream numHash numThreads

Author: Lorenzo Prelli (lorenzo.prelli@gmail.com)
Spring 2015 

Portions of the code (the ones handling multilple threads) are taken shamelessly from the CSE 374 Wi15 website
*/

#include <iostream>
#include <cstdlib>     /* srand, rand */
#include <ctime>
#include <vector>
#include <string>
#include <thread>

using namespace std;
typedef unsigned int T;

// represents the integer range [start, start+len)
struct range {
  int start;
  int len;
};

// Calculate a 1-dimensional block distribution of n elements into b blocks 
// n number of elements
// b number of blocks
// id the block number asked for
// Assumes b <= n
range block_1d(int n, int b, int id) {
  int base_per_proc = n / b;
  int remainder = n % b;

  range result;

  // every block gets at least base_per_proc elements 

  // give +1 to each block where id < remainder
  if (id < remainder) {
    result.start = (base_per_proc+1)*id;
    result.len = base_per_proc + 1;
  } else {
    result.start = (base_per_proc+1)*remainder + base_per_proc*(id-remainder);
    result.len = base_per_proc;
  }

  return result;
}

//median of a vector of double
double medianDouble(vector<double> vec)
{
        
	sort(vec.begin(), vec.end());
	auto mid = vec.size()/2;

	return vec[mid];
}


//class that handles min hashing
class MinHash {
	//setting up hash functions
	vector<T> P;
	T A, B;

	//stores the min hashes
	vector<double> M;
	//number of hash functions to use
	int numHash;
	//number of threads to use
	int numThreads;

	//hash function with parameter mod
	T hash(int n, T mod) {
		return (A*n + B) % mod;
	}

	//processes a region in the specified range
	void processRegion(int input, range r) {
		for(int i = r.start; i< r.len + r.start; i++) {
			auto intVal = hash(input, P[i]);
			double currHash = (double) intVal/P[i];
			if(currHash < M[i])
				M[i] = currHash;
		}
	}

public: 
	//constructor. num = number of hash functions to use, numT = number of threads to use
	MinHash(int num, int numT) {
		//initializes the vector P and the coefficients 
		numHash = num;
		numThreads = numT;
		srand (time(NULL));
		for(int i =0; i< numHash; i++) {
			P.push_back(rand());
		}

		A = rand();
		B = rand();


	}

	void processFirst(int n) {
		for(auto el : P) {
			auto intVal = hash(n,el);
			M.push_back((double) intVal/el);
		}

	}

	//divide the vector P in chunks and create threads
	void process(int input) {

	thread** ts = new thread*[numThreads];
  	for (int i = 0; i<numThreads; i++) {
   		// assign a range of the arrays to thread i
    	range r = block_1d(numHash, numThreads, i);
    	// create and start thread i
    	ts[i] = new thread(  bind(  &MinHash::processRegion, this, input, r));
  	}

  	// wait for all threads to complete
 	for (int i = 0; i<numThreads; i++) {
    	// wait for thread i to complete
    	ts[i]->join();

    	// done with thread i, so delete it
    	delete ts[i];
  	}

	delete[] ts;

	}

	int estimateAveMinHash() {
		double sum = 0.0;
		for(auto el : M) {
			sum +=  el;
		}
		sum = (double) sum/ ((double)numHash);
		return 1.0/sum;
	}

	int estimateMedianMinhash() {
		return 1.0/medianDouble(M);
	}

};



int main(int argc, char const *argv[])
{
	
	if(argc != 5) //checks is the number of parameters is correct
		return 1;

	int numDist = stoi(argv[1]);
	int numStream = stoi(argv[2]); 
	int numHash= stoi(argv[3]);
	int numThreads = stoi(argv[4]);

	cout << "Initializing the data structure: number of hash functions = " << numHash << endl << endl;
	MinHash minnie(numHash, numThreads);

	cout << "Beginning of a simulation: ";
	cout << "Elements in the stream = " << numStream << endl << endl;

	//process all the distinct element first
	minnie.processFirst(1);
	for(int i = 2; i <= numDist; i++)
		minnie.process(i);

	//process some other elements in the range [0, numDist]
	srand (time(NULL));
	for(int i = numDist+1; i <= numStream; i++) {
		minnie.process(rand() % (numDist));
	}

	cout << "I'm done!" << endl << endl;

	cout << "The first estimate (average) is " << minnie.estimateAveMinHash() << endl;
	cout << "The second estimate (median) is " << minnie.estimateMedianMinhash() << endl << endl;
	cout << "The true value is " << numDist << endl;

	return 0;
}