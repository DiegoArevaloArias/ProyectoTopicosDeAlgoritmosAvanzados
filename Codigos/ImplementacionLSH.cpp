#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <limits>
#include <sstream>
#include <unordered_set>

using namespace std;

struct Nodo
{
    vector<long long> data;
    Nodo* next = NULL;

    Nodo(const vector<long long>& punto, Nodo* siguiente){
        data = punto;
        next = siguiente;
    }
};

class TablaHash{
public:
    int dimension; // dimension de los puntos R^d
    long long cantidad; // cantidad de puntos 
    vector<vector<Nodo*>> tabla; // tabla[L][bucketIndex]
    
    // Parámetros Local Sensitive Hahsing
    int k; // número de funciones combinadas por tabla
    int L; // número de tablas
    double w; // anchura del bin window para LSH Euclidiana????

    vector<vector<vector<double>>> A;
    vector<vector<double>> B;

    std::mt19937_64 rng;

    TablaHash(int d, long long N){
        dimension = d;
        cantidad = max(2LL, N / 20); // buckets por tabla
        if(cantidad > 2000000){
            cantidad = 2000000; 
        } 

        // Calcular k 
        double logd = log2(max(2, d));
        double logN = log10(max(10LL, N));
        k = max(3, int(ceil(logd + logN - 2)));

        // Calcular L 
        L = max(5, int(ceil(sqrt(k) * 4)));
        if(L > 50) L = 50;

        w = 4.0; // se puede ajustar automáticamente por dataset

        // Inicializar RNG (semilla por reloj)
        std::random_device rd;
        rng.seed(rd());

        // Crear A y B
        std::normal_distribution<double> gaussian(0.0, 1.0); // para vectores a ~ N(0,1)
        std::uniform_real_distribution<double> uniform_b(0.0, w); // b en [0, w)

        A.resize(L);
        B.resize(L);
        for(int t = 0; t < L; ++t){
            A[t].resize(k, vector<double>(dimension));
            B[t].resize(k);
            for(int j = 0; j < k; ++j){
                for(int dim = 0; dim < dimension; ++dim){
                    A[t][j][dim] = gaussian(rng);
                }
                B[t][j] = uniform_b(rng);
            }
        }
    }



    double distanciaEntrePuntos(const vector<long long>& p1, const vector<long long>& p2){
        double sum = 0.0;
        for(int i = 0; i < dimension; ++i){

            double diff=double(p1[i])-double(p2[i]);
            sum+=diff*diff;

        }
        return std::sqrt(sum);
    }




    // FuncionHash: calcula el bucket index para 'punto' en la tabla 'tablaIdx'
    long long FuncionHash(const vector<long long>& punto, int tablaIdx = 0){
        // Para cada una de las k funciones: h_j(p) = floor((a_j . p + b_j) / w)
        // Combinamos los k valores (por ejemplo, mediante hashing)
        // Finalmente tomamos modulo cantidad para obtener bucket index.
        // Nota: cantidad debe ser > 0
        if(cantidad <= 0) return 0;

        // combinar en 64-bit
        uint64_t comb = 1469598103934665603ULL; // FNV offset basis
        const uint64_t FNV_PRIME = 1099511628211ULL;

        for(int j = 0; j < k; ++j){
            double dot = 0.0;
            for(int dim = 0; dim < dimension; ++dim){
                dot += A[tablaIdx][j][dim] * double(punto[dim]);
            }
            double val = (dot + B[tablaIdx][j]) / w;
            long long hj = static_cast<long long>(std::floor(val));

            // mezclar hj en comb (mezcla simple tipo FNV sobre bytes)
            uint64_t x = static_cast<uint64_t>(hj);
            for(int b = 0; b < 8; ++b){
                uint8_t byte = (x >> (b*8)) & 0xFF;
                comb ^= byte;
                comb *= FNV_PRIME;
            }
        }

        long long idx = static_cast<long long>(comb % static_cast<uint64_t>(cantidad));
        if(idx < 0) idx += cantidad;
        return idx;
    }

    // Insertar un punto en todas las L tablas (en su bucket correspondiente)
    void insertarPunto(const vector<long long>& punto){
        for(int t = 0; t < L; ++t){
            long long idx = FuncionHash(punto, t);
            Nodo* actual = tabla[t][idx];

            if(actual == nullptr){
                tabla[t][idx] = new Nodo(punto, nullptr);
                continue;
            }
            // recorrer hasta el final
            Nodo* prev = nullptr;
            while(actual != nullptr){
                if(actual->data == punto){
                    // ya existe no insertar duplicado
                    actual = nullptr;
                    prev = nullptr;
                    break;
                }

                prev = actual;
                actual = actual->next;
            }

            if(prev != nullptr){//insertamos al final el punto
                prev->next = new Nodo(punto, nullptr);
            }
        }
    }

    // Buscar vecino más cercano aproximado usando LSH:
    // - consultamos los buckets en cada tabla correspondientes al hash del query
    // - comparamos todos los puntos en esos buckets (evitando duplicados entre tablas)
    vector<long long> buscarPuntoMasCercano(const vector<long long>& punto){
        double minDist = std::numeric_limits<double>::infinity();
        vector<long long> puntoCercano;
        unordered_set<const vector<long long>*> yaVistos;

        for(int t = 0; t < L; ++t){//Buscamos en cada una de las L tablas con hashings diferentes

            long long idx = FuncionHash(punto, t);//Buscamos el inicio del bucket

            Nodo* actual = tabla[t][idx];

            while(actual != nullptr){//Comprobamos todas las didstancias en 
                
                const vector<long long>& vec = actual->data;


                if(yaVistos.find(&vec)==yaVistos.end()){//calculamos la distancia y puntos para los que no han sido visitados

                    yaVistos.insert(&vec);

                    double dist = distanciaEntrePuntos(punto, actual->data);
                    if(dist < minDist){
                        minDist = dist;
                        puntoCercano = actual->data;
                    }
                }
                actual = actual->next;
            }
        }

        return puntoCercano; 
    }

    // Destructor falta
    
};

int main(){
    
}
