/*
VERSION 2: added more estimates

Small program that uses minhash functions to estimate the number of different elements in a stream.

The class MinHash mantains a vector P and coefficients A, B.
For every new element n in the stream, the process method computes the hash functions (A*n+B) mod P[i]
and keeps track of the minimum in the vector M.

The estimate for the number of different elements is the inverse of the min hash.

USE: minhash numDist numStream numHash

Author: Lorenzo Prelli (lorenzo.prelli@gmail.com)
Spring 2015 
*/

#include <iostream>
#include <cstdlib>     /* srand, rand */
#include <ctime>
#include <vector>
#include <string>

using namespace std;
typedef unsigned int T;


//median of a vector
int median(vector<int> vec)
{
        
	sort(vec.begin(), vec.end());
	auto mid = vec.size()/2;

	return vec[mid];
}

double medianDouble(vector<double> vec)
{
        
	sort(vec.begin(), vec.end());
	auto mid = vec.size()/2;

	return vec[mid];
}

class MinHash {
	//setting up hash functions
	vector<T> P;
	T A, B;

	//stores the min hashes
	vector<double> M;

	//stores the estimates
	vector<int> E;

	int numHash;

	T hash(int n, T mod) {
		return (A*n + B) % mod;
	}

public: 
	MinHash(int num) {
		//initializes the vector P and the coefficients 
		numHash = num;
		srand (time(NULL));
		for(int i =0; i< numHash; i++) {
			P.push_back(rand());
		}

		A = rand();
		B = rand();
	}

	void processFirst(int n) {
		for(auto el : P) {
			auto intVal = hash(n, el);
			M.push_back((double) intVal/el);
			E.push_back((int) el/intVal );
		}

	}

	void process(int n) {
		for(int i = 0; i < P.size(); i++) {
			auto intVal = hash(n, P[i]);
			double currHash = (double) intVal/P[i];
			if(currHash < M[i])
				M[i] = currHash;
		}

	}

	int estimateAveMinHash() {
		double sum = 0.0;
		for(auto el : M) {
			sum +=  el;
		}
		sum = (double) sum/ ((double)numHash);
		return 1.0/sum;
	}

	int estimateAveEst() {
		int sum = 0;
		for(auto num : E)
			sum+=num;
		return sum/numHash;
	}

	int estimateMedianEst() {
		return (int) median(E);
	}

	int estimateMedianMinhash() {
		return 1.0/medianDouble(M);
	}

};



int main(int argc, char const *argv[])
{
	
	if(argc != 4) //checks is the number of parameters is correct
		return 1;

	int numDist = stoi(argv[1]);
	int numStream = stoi(argv[2]); 
	int numHash= stoi(argv[3]);

	cout << "Initializing the data structure: number of hash functions = " << numHash << endl << endl;
	MinHash minnie(numHash);

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

	cout << "The first estimate is " << minnie.estimateAveMinHash() << endl;
	cout << "The second estimate is " << minnie.estimateMedianMinhash() << endl << endl;
	cout << "The true value is " << numDist << endl;



	return 0;
}