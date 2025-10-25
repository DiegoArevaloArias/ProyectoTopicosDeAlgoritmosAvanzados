#  Nearest Neighbor Search (NNS) — KD-Tree, LSH & HNSW

Este repositorio implementa y compara tres enfoques para el problema del **Nearest Neighbor Search (NNS)**: dado un espacio métrico y un conjunto de puntos, queremos encontrar el punto más cercano a una consulta.

Se evaluará su rendimiento en diferentes dimensiones para estudiar el impacto de la **curse of dimensionality**.

---

##  Algoritmos incluidos

###  KD-Tree (Búsqueda exacta)
- **Idea:** Divide recursivamente el espacio mediante particiones ortogonales.
- **Técnica:** Divide y vencerás.
- **Ventajas:** Muy eficiente en espacios **de baja dimensión**.
- **Complejidad:**  
  - Promedio: **O(log n)**  
  - Peor caso: **O(n)**
- **Invariante:** Cada nodo mantiene puntos dentro del rango definido por su hiperplano en la dimensión de partición.
- **Referencia:** Bentley (1975)

---

###  Locality Sensitive Hashing (LSH) — Aproximado probabilístico
- **Idea:** Hashing sensible a la localidad: puntos cercanos tienen alta probabilidad de colisión.
- **Técnica:** Algoritmos aleatorizados.
- **Ventajas:** Tiempo de búsqueda **sublineal** en alta dimensión.
- **Complejidad esperada:** **O(n^ρ)**, con 0 < ρ < 1
- **Invariante:** Las funciones hash preservan la vecindad esperada.
- **Referencia:** Indyk & Motwani (1998), Andoni & Indyk (2008)

---

###  HNSW (Hierarchical Navigable Small World Graphs)  
— Aproximado moderno basado en grafos
- **Idea:** Grafo jerárquico tipo *small world* para búsqueda eficiente por navegación local.
- **Técnica:** Grafos multicapa + routing greedy.
- **Ventajas:** Excelente equilibrio entre **precisión, memoria y tiempo**.
- **Complejidad:** Promedio **O(log n)** dependiendo de conectividad.
- **Invariante:** Cada nivel forma un grafo *small world* navegable.
- **Referencia:** Malkov & Yashunin (2018)

---

##  Experimentos

Las tres implementaciones serán probadas en datasets sintéticos o de referencia con dimensiones:

| Dimensión | Propósito |
|----------:|-----------|
| 2D        | Visualización y validación |
| 10D       | Región práctica |
| 100D      | Alta dimensión (curse of dimensionality) |

Se medirán:
- Tiempo de búsqueda
- Precisión (recall para métodos aproximados)
- Uso de memoria

---

##  Estructura del repositorio

---
##  Referencias

> Artículos base

- **KD-Tree**
  - Bentley, J. L. (1975). *Multidimensional binary search trees used for associative searching.*
- **LSH**
  - Indyk, P., & Motwani, R. (1998). *Approximate nearest neighbors: Towards removing the curse of dimensionality.*
  - Andoni, A. & Indyk, P. (2008). *Near-optimal hashing algorithms for high-dimensional ANN.*
- **HNSW**
  - Malkov, Y. A., & Yashunin, D. A. (2018). *Efficient and robust approximate nearest neighbor search using HNSW.*

> Paginas web

---
