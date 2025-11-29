#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>

using namespace std;

struct Nodo {
    vector<long long> data;
    Nodo* left;
    Nodo* right;

    Nodo(const vector<long long>& punto){
        data=punto; 
        left=nullptr;
        right=nullptr;

    }
    
};

class KdTree {
public:
    Nodo* root;
    int dimension;

    KdTree(int d, vector<vector<long long>>& pts){
        dimension=d;
        root=build(pts, 0, (int)pts.size(), 0);
        
    }

    Nodo* build(vector<vector<long long>>& pts, int l, int r, int depth) {
        if (l >= r) return nullptr;
        int axis = depth % dimension;
        int mid = l + (r - l) / 2;
        nth_element(pts.begin() + l, pts.begin() + mid, pts.begin() + r,
                    [axis](const vector<long long>& a, const vector<long long>& b) {
                        return a[axis] < b[axis];
                    });
        Nodo* node = new Nodo(pts[mid]);
        node->left  = build(pts, l,     mid, depth + 1);
        node->right = build(pts, mid+1, r,   depth + 1);
        return node;
    }

    Nodo* insertar(Nodo* nodo, const vector<long long>& p, int depth) {
        if (!nodo){//si estamos al final insertamos
            return new Nodo(p);
        }

        int axis = depth % dimension;

        if (p[axis] < nodo->data[axis]){//si es menor que el dato en la dimension actual insertamo a izquierda
            nodo->left = insertar(nodo->left, p, depth + 1);
        }else{//si es mayor al valor en dimension actual insertamos a derecha
            nodo->right = insertar(nodo->right, p, depth + 1);
        }

        return nodo;
    }

    // Distancia Euclídea
    double distancia(const vector<long long>& p1, const vector<long long>& p2) {
        double sum = 0.0;
        for (int i = 0; i < dimension; ++i) {
            double diff = (double)p1[i] - (double)p2[i];
            sum += diff * diff;
        }
        return sum;
    }

    // Búsqueda del vecino más cercano recursiva (branch & bound podado)
    void nearest(Nodo* nodo, const vector<long long>& target,int depth, Nodo*& mejorNodo, double& mejorDist) {
        if (!nodo) {
            return;
        }

        double d = distancia(target, nodo->data);
        if (d<mejorDist) {
            mejorDist=d;
            mejorNodo=nodo;
        }

        int axis = depth % dimension;
        bool irIzq= target[axis] < nodo->data[axis];

        Nodo* prim = irIzq ? nodo->left : nodo->right;
        Nodo* sec = irIzq ? nodo->right : nodo->left;

        nearest(prim, target, depth + 1, mejorNodo, mejorDist);

        // Ver si vale la pena explorar el otro subárbol
        double distPlano=abs(target[axis] - nodo->data[axis]);
        double distPlanoSq = distPlano * distPlano; // Distancia al cuadrado al plano de corte
        if (distPlanoSq<mejorDist) {
            nearest(sec, target, depth + 1, mejorNodo, mejorDist);
        }
    }

    
    void insertarPunto(const vector<long long>& p) {
        root = insertar(root, p, 0);
    }

    vector<long long> puntoMasCercano(const vector<long long>& punto) {
        Nodo* mejorNodo = nullptr;
        double mejorDist = numeric_limits<double>::infinity();

        nearest(root, punto, 0, mejorNodo, mejorDist);

        if (mejorNodo)
        {
            return mejorNodo->data; 
        }else{
            return vector<long long>();
        }
        
    }

    //Destructor limpiar memoria



};
