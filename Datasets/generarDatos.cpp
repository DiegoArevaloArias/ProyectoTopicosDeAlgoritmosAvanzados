#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <filesystem> 

using namespace std;
namespace fs = std::filesystem;

// Genera un dataset aleatorio y lo guarda en un archivo binario en la ruta exacta
void generarDatosBinario(const string& rutaCompleta, int d, long long N){
    ofstream fout(rutaCompleta, ios::binary);
    if(!fout){
        cerr << "No se puede abrir " << rutaCompleta << " para escritura\n";
        return;
    }

    mt19937_64 rng(42);
    uniform_int_distribution<long long> dist(0,1000);
    vector<long long> punto(d);

    for(long long i=0;i<N;i++){
        for(int j=0;j<d;j++) punto[j] = dist(rng);
        fout.write(reinterpret_cast<char*>(punto.data()), sizeof(long long)*d);
        if((i+1)%100000==0) cout << "  " << (i+1) << "/" << N << " puntos generados...\r" << flush;
    }

    fout.close();
    cout << "\nArchivo generado: " << rutaCompleta << " (" << N << " puntos, d=" << d << ")\n";
}

// Leer dataset binario
vector<vector<long long>> leerDatosBinario(const string& rutaCompleta, int d, long long N) {
    vector<vector<long long>> dataset(N, vector<long long>(d));

    ifstream fin(rutaCompleta, ios::binary);
    if(!fin){
        cerr << "Error al abrir el archivo para lectura: " << rutaCompleta << "\n";
        return {};
    }

    for(long long i = 0; i < N; ++i){
        fin.read(reinterpret_cast<char*>(dataset[i].data()), sizeof(long long) * d);
    }

    fin.close();
    return dataset;
}

int main() {
    int d[] = {2,10,100};// 2, 10, 100
    long long N[] = {1000, 10000, 1000000};/// 1000, 10 000, 1 000 000 factible, 100 000 000??
    string rutas[]={"small","medium","big"};
    string rutaArchivo = ".bin";
    for (int i = 0; i < 3; i++)
    {
        int dimension=d[i];
        for (int j = 0; j < 3; j++)
        {
            long long tam=N[j];
            string rutaArchivo = "d" + to_string(dimension) + rutas[j] + ".bin";

            generarDatosBinario(rutaArchivo, dimension, tam);

        }
        

    }
     

    return 0;
}