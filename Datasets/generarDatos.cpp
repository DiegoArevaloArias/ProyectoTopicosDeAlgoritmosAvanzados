#include <iostream>
#include <fstream>
#include <vector>
#include <random>
using namespace std;

/*
    Generador de dataset adecuado para pruebas de Vecino Más Cercano y LSH.
    Características:
      - K clusters gaussianos en R^d
      - cada cluster tiene su propio centro
      - se genera un % de puntos con “vecinos cercanos garantizados”
      - los datos se guardan en binario como en tu código original
*/

void generarDatasetConEstructura(
        const string& ruta,
        int d, 
        long long N,
        int K = 5,                 // número de clusters
        double clusterStd = 80.0,  // dispersión del cluster
        double smallNoiseStd = 5.0,// ruido para crear vecinos cercanos
        double fractionTwin = 0.01 // 1% de puntos con gemelos
    )
{
    ofstream out(ruta, ios::binary);
    if(!out){
        cerr << "No se puede abrir archivo: " << ruta << "\n";
        return;
    }

    mt19937_64 rng(42);

    // distribuciones
    normal_distribution<double> noise(0.0, clusterStd);
    normal_distribution<double> smallNoise(0.0, smallNoiseStd);
    uniform_real_distribution<double> centerDist(0.0, 1000.0);

    // Generar centros de los clusters
    vector<vector<double>> centros(K, vector<double>(d));
    for(int k = 0; k < K; k++){
        for(int j = 0; j < d; j++){
            centros[k][j] = centerDist(rng);
        }
    }

    long long numTwins = (long long)(N * fractionTwin);

    for(long long i = 0; i < N; i++){
        
        vector<long long> punto(d);

        // Seleccionar cluster aleatorio
        int c = rng() % K;

        for(int j = 0; j < d; j++){
            double val = centros[c][j] + noise(rng);
            punto[j] = (long long) val;
        }

        // Guardar punto base
        out.write(reinterpret_cast<char*>(punto.data()), sizeof(long long)*d);

        // Crear un gemelo, solo para una parte del dataset
        if(i < numTwins){
            vector<long long> twin(d);
            for(int j = 0; j < d; j++){
                double val = punto[j] + smallNoise(rng);
                twin[j] = (long long) val;
            }
            // también lo guardamos inmediatamente
            out.write(reinterpret_cast<char*>(twin.data()), sizeof(long long)*d);
            i++; // consumimos una posición
        }
    }

    out.close();
    cout << "Archivo generado: " << ruta 
         << " (N=" << N << ", d=" << d << ", K=" << K << " clusters)"
         << endl;
}

int main() {

    // Ejemplo de uso
    vector<int> dims = {2, 10, 100};
    vector<long long> sizes = {1000, 10000, 1000000};

    for(int d : dims){
        for(long long N : sizes){
            string name = "dataset_d" + to_string(d) 
                        + "_N" + to_string(N) + ".bin";

            generarDatasetConEstructura(name, d, N);
        }
    }

    return 0;
}