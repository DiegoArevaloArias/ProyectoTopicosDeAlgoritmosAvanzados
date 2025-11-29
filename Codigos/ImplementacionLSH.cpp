#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <limits>
#include <unordered_set>
#include <cstdint>

using namespace std;

struct Nodo {
    int puntoId;
    Nodo* next;
    Nodo(int id, Nodo* n = nullptr) : puntoId(id), next(n) {}
};

class TablaHash {
private:
    const vector<vector<long long>>& dataset; // referencia al dataset maestro
    int dimension;
    long long cantidadBuckets;               // buckets por tabla
    int k;
    int L;
    double w;

    // A[t][j][dim], B[t][j]
    vector<vector<vector<double>>> A;
    vector<vector<double>> B;
    // coeficientes R para mezcla universal por tabla
    vector<vector<uint64_t>> R;

    vector<vector<Nodo*>> tabla; // tabla[t][bucket] -> lista de Nodo*

    std::mt19937_64 rng;

    static constexpr uint64_t P = 4294967311ULL; // primo grande (menor que 2^64)

public:
    TablaHash(const vector<vector<long long>>& datos,
              int numTablas = -1,
              int numFunciones = -1,
              long long buckets_hint = -1,
              double w_hint = -1.0)
        : dataset(datos), dimension(datos.empty() ? 0 : (int)datos[0].size())
    {
        long long N = (long long)datos.size();

        // buckets: preferir una heurística razonable si no dan buckets_hint
        if (buckets_hint > 0) cantidadBuckets = buckets_hint;
        else cantidadBuckets = max(1LL, N / 20); 
        if (cantidadBuckets > 1000000) cantidadBuckets = 1000000;

        // k, L por heurística si no se dan
        k = (numFunciones > 0) ? numFunciones : max(3, (int)ceil(log2(max(1, dimension)) + 2));
        L = (numTablas > 0) ? numTablas : min(50, 5 + (int)(sqrt(max(1, dimension)) * 1.5));

        // w por heurística o override
        if (w_hint > 0.0) w = w_hint;
        else w = 50.0 + 0.5 * sqrt(max(1, dimension)) * 20.0;

        // reservar tabla
        tabla.resize(L, vector<Nodo*>(cantidadBuckets, nullptr));

        // rng
        std::random_device rd;
        rng.seed(rd());

        // inicializar A y B
        std::normal_distribution<double> gaussian(0.0, 1.0);
        std::uniform_real_distribution<double> uniform_b(0.0, w);

        A.assign(L, vector<vector<double>>(k, vector<double>(dimension)));
        B.assign(L, vector<double>(k));
        R.assign(L, vector<uint64_t>(k));

        // coeficientes R aleatorios en [1, P-1]
        std::uniform_int_distribution<uint64_t> unifR(1, P - 1);

        for (int t = 0; t < L; ++t) {
            for (int j = 0; j < k; ++j) {
                for (int dim = 0; dim < dimension; ++dim) {
                    A[t][j][dim] = gaussian(rng);
                }
                B[t][j] = uniform_b(rng);
                R[t][j] = unifR(rng);
            }
        }
    }

    ~TablaHash() {
        // liberar nodos
        for (int t = 0; t < L; ++t) {
            for (long long b = 0; b < cantidadBuckets; ++b) {
                Nodo* ptr = tabla[t][b];
                while (ptr) {
                    Nodo* nxt = ptr->next;
                    delete ptr;
                    ptr = nxt;
                }
            }
        }
    }

    // distancia al cuadrado (evita sqrt para comparaciones)
    double distanciaCuadrada(const vector<long long>& p1, const vector<long long>& p2) const {
        double s = 0.0;
        for (int i = 0; i < dimension; ++i) {
            double d = double(p1[i]) - double(p2[i]);
            s += d * d;
        }
        return s;
    }

    // calcular el bucket para un punto (tablaIdx)
    long long FuncionHashId(const vector<long long>& punto, int tablaIdx) const {
        if (cantidadBuckets <= 0) return 0;

        __uint128_t g = 0;

        for (int j = 0; j < k; ++j) {
            double dot = 0.0;
            for (int dim = 0; dim < dimension; ++dim) {
                dot += A[tablaIdx][j][dim] * double(punto[dim]);
            }
            long long hj = (long long)floor((dot + B[tablaIdx][j]) / w);

            // normalizar hj a entero positivo modulo P
            uint64_t hj_mod;
            if (hj >= 0) hj_mod = (uint64_t)hj % P;
            else {
                // convertir negativo a [0, P-1]
                long long tmp = hj % (long long)P;
                if (tmp < 0) tmp += (long long)P;
                hj_mod = (uint64_t)tmp;
            }

            // g = (g + hj_mod * R[tablaIdx][j]) % P
            __uint128_t mul = ( __uint128_t ) hj_mod * ( __uint128_t ) R[tablaIdx][j];
            g = (g + (mul % P)) % P;
        }

        uint64_t g64 = (uint64_t)g;
        return (long long)(g64 % (uint64_t)cantidadBuckets);
    }

    // insertar por índices (se asume dataset ya poblado externamente)
    void insertarIndice(int puntoId) {
        if (puntoId < 0 || puntoId >= (int)dataset.size()) return;
        const auto& punto = dataset[puntoId];

        for (int t = 0; t < L; ++t) {
            long long idx = FuncionHashId(punto, t);
            // inserción en cabeza (O(1))
            Nodo* nuevo = new Nodo(puntoId, tabla[t][idx]);
            tabla[t][idx] = nuevo;
        }
    }

    // insertar todos (helper)
    void insertarTodos() {
        for (int i = 0; i < (int)dataset.size(); ++i) insertarIndice(i);
    }

    // buscar vecino más cercano aproximado (devuelve índice en dataset o -1)
    int buscarVecinoMasCercano(const vector<long long>& query, int maxCandidatos = -1) const {
        double minDistSq = std::numeric_limits<double>::infinity();
        int bestId = -1;
        unordered_set<int> vistos;
        int revisados = 0;
        if (maxCandidatos <= 0) maxCandidatos = 10 * L; // heurística

        for (int t = 0; t < L; ++t) {
            long long idx = FuncionHashId(query, t);
            Nodo* ptr = tabla[t][idx];
            while (ptr != nullptr) {
                int id = ptr->puntoId;
                if (!vistos.count(id)) {
                    vistos.insert(id);
                    double d2 = distanciaCuadrada(query, dataset[id]);
                    if (d2 < minDistSq) {
                        minDistSq = d2;
                        bestId = id;
                    }
                    if (++revisados >= maxCandidatos) return bestId;
                }
                ptr = ptr->next;
            }
        }
        return bestId;
    }

    // getters para parámetros si se necesitan
    int getL() const { return L; }
    int getK() const { return k; }
    long long getBuckets() const { return cantidadBuckets; }
    double getW() const { return w; }
};
