//clang++ -O3 -o d -Xpreprocessor -fopenmp -lomp -lzmq pp.cc
#include "include/rapidjson/document.h"
#include "include/rapidjson/writer.h"
#include "include/rapidjson/stringbuffer.h"
#include  "zmq.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <random>
#include <omp.h>

using namespace std;


using Point = vector<double>;
using DataFrame = vector<Point>;

inline double square(double value){
	return value * value;
}

inline double squared_12_distance(const Point& first,const Point& second){
	double d = 0.0;
	for(size_t dim = 0; dim < first.size();dim++){
		d += square(first[dim]-second[dim]);
	}
	return d;
}

inline double distanciakmeans(DataFrame means,DataFrame data,vector<size_t> v){
	double distance = 0;
	for(int i=0;i<v.size();i++){
		distance =+ squared_12_distance(data[i],means[v[i]]);
	}
	return distance;
}


double k_means( const DataFrame& data, size_t k, size_t number_of_iterations, double ep,const int empty, const DataFrame Imeans){
	size_t dimensions = data[0].size();
	static random_device seed;
	static mt19937 random_number_generator(seed());
	uniform_int_distribution<size_t> indices(0,data.size()-1);/// change		  
	// pick centroids as random points from the dataset
	DataFrame means(k);// 
	double distanciaepsilon;
	int contador = 0;
	size_t epsilon = numeric_limits<size_t>::max();
	double distancemeans;
//------------------asignacion de primeros cluster--------------------------	
	if(empty == 0){
	for (Point& cluster : means){ // cluster -> means
		size_t i = indices(random_number_generator);
		cluster = data[i];//cluster inicial		
		}}
	if(empty == 1)
		means = Imeans;
	vector<size_t> assignments(data.size());
	
	size_t iteration = 0;
//--------------------------ciclo de kmeans-------------------------------
	while((iteration < number_of_iterations)&&(contador != k)){
	
		// find assignements ---- 
		#pragma omp parallel for //num_threads(8)
		for (size_t point = 0; point < data.size() ; ++point){
			double best_distance = numeric_limits<double>::max();// variable mejor distacia, inicializada con la maxima
			size_t best_cluster = 0; // variable mejor cluster, inicializada con 0
			for (size_t cluster = 0; cluster < k; cluster++){
				const double distance = squared_12_distance(data[point], means[cluster]);
				if(distance < best_distance){
				    best_distance = distance;
				    best_cluster = cluster;
					}
				}
			assignments[point] = best_cluster;
			}


		DataFrame new_means(k,vector<double>(dimensions,0.0));// means nuevo
		DataFrame new_meansaux(k,vector<double>(dimensions,0.0));//means actual
		vector<size_t> counts(k, 0);
	//----------------------- asigna cluster a los puntos----------------
		for (size_t point = 0; point < data.size(); ++point){
		    const size_t cluster = assignments[point];
		    for(size_t d = 0; d < dimensions; d++){
		    	new_means[cluster][d] += data[point][d];
		    	}			
			counts[cluster] += 1;
			}
	//------------------- divide sumas por saltos para obtener centroides
		contador = 0;
		for (size_t cluster = 0; cluster < k; ++cluster){
			const size_t count = max<size_t>(1, counts[cluster]);
			for(size_t d = 0; d < dimensions;d++){
				new_meansaux[cluster][d] = means[cluster][d];
				means[cluster][d] = new_means[cluster][d] / count;
				}
			distanciaepsilon = squared_12_distance(new_meansaux[cluster],means[cluster]);
			if(distanciaepsilon < ep){
				contador++;
				}	
			}
		++iteration;
		//-------------final de centroides nuevos---------------
	//-------------retorno si los centroides no cambian o el cambio es menor a epsilon
		
	//--------------fin retorno---------------------

	}
	cout<<"iteracion#:"<<iteration<<endl;
	distancemeans = distanciakmeans(means,data,assignments);

//----------------termina iteaciones--------------------------
	return distancemeans;
}


DataFrame readData(string File,int nVariables ){
	DataFrame data;
	ifstream input(File);
	string line;
	while(getline(input,line)){
		istringstream iss(line);
		double v;
		Point p;
		for(int i = 0;i < nVariables; i++){
			iss >> v;
			p.push_back(v);
			}
		data.push_back(p);
		}
		cout << data.size() << endl;
		return data;
}





int main(){
	// main
	string dataset;
	int numeroVariables;
	int numeroCluster;
	int numeroIT;
	double epsilon;

	dataset= "arrhythmia.dat";
	numeroVariables = 279;
	numeroCluster = 10;
	numeroIT = 1000000;
	epsilon = 0.1;

	DataFrame data = readData(dataset,numeroVariables);
	DataFrame means;
	double c;
	vector<size_t> a;
	
		for(int i=0;i<10;i++){
			c = k_means(data,numeroCluster,numeroIT,epsilon,0,means);
			cout<<c<<endl;
		}

	
	return 0;

}