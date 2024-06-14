
#include <iostream>
#include <vector>
#include <fstream>
#include "json.hpp"
#include "csv.h"
#include <limits>
#include <sstream> 
#include <queue>
#include <unordered_map>

using json = nlohmann::json;

// Nodo del Árbol de Decisiones
struct Nodo {
    std::string pregunta;
    Nodo* izquierda;
    Nodo* derecha;
    std::vector<int> identificadores; // Solo en nodos hoja
};

// Estructura para la información de atracciones
struct Atraccion {
    int identificador;
    std::string nombre;
    int tiempo_espera;
};

// Estructura para el Grafo
struct Grafo {
    std::vector<std::vector<int>> matrizAdyacencia;
};

// Función para construir el Grafo 
void construirGrafo(Grafo& grafo, const std::string& archivoCSV) {
    std::ifstream archivo(archivoCSV);
    if (!archivo.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << archivoCSV << std::endl;
        return;
    }

    if (archivo.peek() == std::ifstream::traits_type::eof()) {
        std::cerr << "Error: El archivo " << archivoCSV << " está vacío." << std::endl;
        return;
    }

    std::string linea;
    int fila_numero = 0;
    while (std::getline(archivo, linea)) {
        std::vector<int> fila;
        std::stringstream ss(linea);
        std::string valor;
        int columna_numero = 0;
        while (std::getline(ss, valor, ',')) {
            try {
                int num = std::stoi(valor);
                fila.push_back(num);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Error: Valor inválido en el archivo " << archivoCSV << " en la fila "
                          << fila_numero + 1 << ", columna " << columna_numero + 1 << ". No es un entero." << std::endl;
                return;
            } catch (const std::out_of_range& e) {
                std::cerr << "Error: Valor fuera de rango en el archivo " << archivoCSV << " en la fila "
                          << fila_numero + 1 << ", columna " << columna_numero + 1 << "." << std::endl;
                return;
            }
            ++columna_numero;
        }
        grafo.matrizAdyacencia.push_back(fila);
        ++fila_numero;
    }
// Verificar matriz
    std::size_t num_columnas = grafo.matrizAdyacencia[0].size();
    for (const auto& fila : grafo.matrizAdyacencia) {
        if (fila.size() != num_columnas) {
            std::cerr << "Error: Inconsistencia en el número de columnas en el archivo " << archivoCSV << "." << std::endl;
            return;
        }
    }

    //std::cout << "Grafo construido con éxito desde el archivo " << archivoCSV << "." << std::endl;
}

//-----------------------------------------------------------

// Función para construir el Árbol de Decisiones 
Nodo* construirArbol(const json& j) {
    Nodo* nodo = new Nodo();

    // Verificar pregunta

    if (j.contains("pregunta")) {
        nodo->pregunta = j["pregunta"];
    } else {
        nodo->pregunta = ""; 
    }

// Verificar izquierda
    if (j.contains("izquierda") && j["izquierda"].is_object()) {
        nodo->izquierda = construirArbol(j["izquierda"]);
    } else {
        nodo->izquierda = nullptr;
    }
// Verificar derecho 
    if (j.contains("derecha") && j["derecha"].is_object()) {
        nodo->derecha = construirArbol(j["derecha"]);
    } else {
        nodo->derecha = nullptr;
    }
// Verificar identificadores 
    if (j.contains("identificadores") && j["identificadores"].is_array()) {
        nodo->identificadores = j["identificadores"].get<std::vector<int>>();
    } else {
        nodo->identificadores = {}; 
    }

    return nodo;
}

//-----------------------------------------------------------

// Función para leer el Árbol de Decisiones 
Nodo* leerArbolDecisiones(const std::string& archivoJSON) {
    // Intentamos abrir el archivo
    std::ifstream archivo(archivoJSON);
    if (!archivo.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << archivoJSON << std::endl;
        return nullptr;
    }
// Verificamos si el archivo está vacío
    if (archivo.peek() == std::ifstream::traits_type::eof()) {
        std::cerr << "Error: El archivo " << archivoJSON << " está vacío." << std::endl;
        return nullptr;
    }
// Intentamos leer y parsear el contenido del archivo
    try {
        json j;
        archivo >> j;
// Verificamos si el JSON está vacío o es nulo
        if (j.is_null() || j.empty()) {
            std::cerr << "Error: El archivo " << archivoJSON << " contiene JSON inválido o vacío." << std::endl;
            return nullptr;
        }
// Construimos el árbol a partir del JSON
        return construirArbol(j);

    } catch (const json::parse_error& e) {
        // Capturamos errores de parseo del JSON y mostramos un mensaje de error detallado
        std::cerr << "Error de parseo en el archivo " << archivoJSON << ": " << e.what() << std::endl;
        return nullptr;
    } catch (const std::exception& e) {
        // Capturamos cualquier otra excepción y mostramos un mensaje de error
        std::cerr << "Error desconocido al leer el archivo " << archivoJSON << ": " << e.what() << std::endl;
        return nullptr;
    }
}
// Función auxiliar para liberar la memoria del árbol de decisiones
void liberarArbol(Nodo* nodo) {
    if (nodo) {
        liberarArbol(nodo->izquierda);
        liberarArbol(nodo->derecha);
        delete nodo;
    }
}

//-----------------------------------------------------

// Función para leer Atracciones

std::vector<Atraccion> leerAtracciones(const std::string& archivoJSON) {
    std::vector<Atraccion> atracciones;
    std::ifstream archivo(archivoJSON);
    if (!archivo.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << archivoJSON << std::endl;
        return atracciones;
    }

    try {
        json j;
        archivo >> j;
        if (j.is_null() || j.empty()) {
            std::cerr << "Error: El archivo " << archivoJSON << " contiene JSON inválido o vacío." << std::endl;
            return atracciones;
        }

        for (const auto& entrada : j) {
            Atraccion atraccion;
            if (!entrada.contains("identificador") || !entrada.contains("nombre") || !entrada.contains("tiempo_espera")) {
                std::cerr << "Error: Falta una clave requerida en una entrada de atracción en el archivo " << archivoJSON << std::endl;
                continue;
            }
            atraccion.identificador = entrada["identificador"];
            atraccion.tiempo_espera = entrada["tiempo_espera"];
            atraccion.nombre = entrada["nombre"];
            atracciones.push_back(atraccion);
        }
    } catch (const json::parse_error& e) {
        std::cerr << "Error de parseo en el archivo " << archivoJSON << ": " << e.what() << std::endl;
    }

    return atracciones;
}

//--------------------------------------------------------

// Función editarTiempoEspera
void editarTiempoEspera(std::vector<Atraccion>& atracciones) {
    std::cout << "Ingrese el identificador de la atraccion a editar: ";
    int identificador;
    std::cin >> identificador;
    
    std::cout << "Ingrese el nuevo tiempo de espera: ";
    int nuevoTiempo;
    std::cin >> nuevoTiempo;
    
    for (auto& atraccion : atracciones) {
        if (atraccion.identificador == identificador) {
            atraccion.tiempo_espera = nuevoTiempo;
            std::cout << "Tiempo de espera actualizado.\n";
            return;
        }
    }
    std::cout << "Identificador de atraccion no encontrado.\n";
}

//--------------------------------------------------------

// Función guardarTiempoEspera
void guardarTiempoEspera(const std::string& archivoJSON, const std::vector<Atraccion>& atracciones) {
    std::ofstream archivo(archivoJSON);
    if (!archivo.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << archivoJSON << " para escribir." << std::endl;
        return;
    }
    
    json j;
    for (const auto& atraccion : atracciones) {
        j.push_back({
            {"identificador", atraccion.identificador},
            {"nombre", atraccion.nombre},
            {"tiempo_espera", atraccion.tiempo_espera}
        });
    }
    archivo << j.dump(4);
}
//-----------------------------------------------------------

// Función para realizar el algoritmo de Dijkstra 
std::pair<std::vector<int>, std::vector<int>> dijkstra(const Grafo& grafo, int inicio, const std::vector<int>& seleccionadas, const std::vector<Atraccion>& atracciones) {
    int n = grafo.matrizAdyacencia.size();
    std::vector<int> distancia(n, std::numeric_limits<int>::max());
    std::vector<int> previo(n, -1);
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> pq;

    distancia[inicio] = 0;
    pq.push({0, inicio});

    while (!pq.empty()) {
        int peso_actual = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        if (peso_actual > distancia[u]) continue;

        for (int v = 0; v < n; ++v) {
            if (grafo.matrizAdyacencia[u][v] > 0) {
                // Sumamos el tiempo de espera de la atracción actual al peso de la ruta
                int peso_ruta = distancia[u] + grafo.matrizAdyacencia[u][v] + atracciones[v].tiempo_espera;
                if (peso_ruta < distancia[v]) {
                    distancia[v] = peso_ruta;
                    previo[v] = u;
                    pq.push({peso_ruta, v});
                }
            }
        }
    }

    // Reconstruir el camino más corto en términos de nodos visitados
    std::vector<int> ruta_optima;
    int destino;
    for (int atraccion : seleccionadas) {
        destino = atraccion - 1; 
        while (destino != -1) {
            ruta_optima.push_back(destino + 1); 
            destino = previo[destino];
        }
        std::reverse(ruta_optima.begin(), ruta_optima.end());
    }

    return {distancia, ruta_optima};
}


//-------------------------------------------------------------

// Función principal del menú de la aplicación

void mostrarMenu() {
    std::cout << "\n-----------------------------------------\n";
    std::cout << "Bienvenido a C++ Kingdom\n";
    std::cout << "\n-----------------------------------------\n";
    std::cout << "1. Usar el arbol de decisiones\n";
    std::cout << "2. Seleccion manual de atracciones\n";
    std::cout << "3. Editar tiempo de espera\n";
    std::cout << "4. Salir\n";
    std::cout << "\n-----------------------------------------\n";
    std::cout << "Seleccione una opcion: ";
}

//--------------------------------------------------------

// Función para imprimir la ruta más eficiente

void imprimirRuta(const std::vector<int>& ruta, const std::vector<Atraccion>& atracciones) {
    std::cout << " \n";
    std::cout << "La ruta mas eficiente para realizar la visita es:\n";
    for (int i = 0; i < ruta.size(); ++i) {
        int id = ruta[i];
        if (id >= 1 && id <= atracciones.size()) {
            std::cout << "- Atraccion " << atracciones[id - 1].identificador << ": " << atracciones[id - 1].nombre << "\n";
        }
    }
}

//--------------------------------------------------------

// Usar el árbol de decisiones 

void usarArbolDecisiones(Nodo* nodo, const std::vector<Atraccion>& atracciones, const Grafo& grafo) {
    if (!nodo->izquierda && !nodo->derecha) {
        std::cout << "\nAtracciones sugeridas:\n";
        for (int identificador : nodo->identificadores) {
            for (const auto& atraccion : atracciones) {
                if (atraccion.identificador == identificador) {
                    std::cout << "Identificador: " << atraccion.identificador << ", Nombre: " << atraccion.nombre << ", Tiempo de espera: " << atraccion.tiempo_espera << " minutos\n";
                }
            }
        }

        std::vector<int> seleccionadas;
        for (int identificador : nodo->identificadores) {
            seleccionadas.push_back(identificador);
        }

        std::cout << "\nCalculando la ruta mas eficiente...\n";

        // Encontrar el índice en el vector de atracciones para el inicio (podría ser el primero de los identificadores sugeridos)
        int inicio_indice = -1;
        for (int i = 0; i < atracciones.size(); ++i) {
            if (atracciones[i].identificador == nodo->identificadores[0]) {
                inicio_indice = i;
                break;
            }
        }

        if (inicio_indice == -1) {
            std::cerr << "Error: Identificador de atraccion de inicio no encontrado.\n";
            return;
        }

        auto resultados_dijkstra = dijkstra(grafo, inicio_indice, seleccionadas, atracciones);
        std::vector<int> distancias = resultados_dijkstra.first;
        std::vector<int> predecesores = resultados_dijkstra.second;

        // Imprimir las distancias mínimas a cada atracción seleccionada
        std::cout << "\nDistancias desde la atraccion de inicio (" << atracciones[inicio_indice].nombre << "):\n";
        for (int id : seleccionadas) {
            std::cout << "Identificador: " << id << ", Distancia: " << distancias[id - 1] << " metros\n";
        }

        // Imprimir la ruta más eficiente
        imprimirRuta(predecesores, atracciones);

        return;
    }

    // Hacer pregunta
    std::cout << nodo->pregunta << " (1. Si / 2. No): ";
    int respuesta;
    std::cin >> respuesta;
    if (respuesta == 1) {
        usarArbolDecisiones(nodo->izquierda, atracciones, grafo);
    } else if (respuesta == 2) {
        usarArbolDecisiones(nodo->derecha, atracciones, grafo);
    } else {
        std::cout << "Respuesta no valida. Intente de nuevo.\n";
        usarArbolDecisiones(nodo, atracciones, grafo);
    }
}


//---------------------------------------------------------------

// Función para seleccionar manualmente las atracciones 

void seleccionManualDeAtracciones(const Grafo& grafo, const std::vector<Atraccion>& atracciones) {
    std::cout << "Lista de atracciones disponibles:\n";
    for (const auto& atraccion : atracciones) {
        std::cout << "Identificador: " << atraccion.identificador << ", Nombre: " << atraccion.nombre << ", Tiempo de espera: " << atraccion.tiempo_espera << " minutos\n";
    }

    std::cout << "Ingrese el identificador de la atraccion de inicio: ";
    int inicio_id;
    std::cin >> inicio_id;

    // Encontrar el índice en el vector de atracciones
    int inicio_indice = -1;
    for (int i = 0; i < atracciones.size(); ++i) {
        if (atracciones[i].identificador == inicio_id) {
            inicio_indice = i;
            break;
        }
    }

    if (inicio_indice == -1) {
        std::cerr << "Error: Identificador de atraccion de inicio no encontrado.\n";
        return;
    }

    std::cout << "Ingrese los identificadores de las atracciones a visitar (separados por espacios) o 'todos' para visitar todas: ";
    std::vector<int> seleccionadas;
    std::string entrada;
    std::cin.ignore();
    std::getline(std::cin, entrada);

    if (entrada == "todos") {
        for (const auto& atraccion : atracciones) {
            seleccionadas.push_back(atraccion.identificador);
        }
    } else {
        std::istringstream iss(entrada);
        int identificador;
        while (iss >> identificador) {
            seleccionadas.push_back(identificador);
        }
    }

    auto resultados_dijkstra = dijkstra(grafo, inicio_indice, seleccionadas, atracciones);
    std::vector<int> distancias = resultados_dijkstra.first;
    std::vector<int> predecesores = resultados_dijkstra.second;

    // Imprimir las distancias mínimas a cada atracción seleccionada
    std::cout << "  \n";
    std::cout << "Distancias desde la atraccion de inicio (" << atracciones[inicio_indice].nombre << "):\n";
    for (int i = 0; i < seleccionadas.size(); ++i) {
        int id = seleccionadas[i];
        std::cout << "Identificador: " << id << ", Distancia: " << distancias[id - 1] << " metros\n"; 
    }

    // Imprimir la ruta más eficiente
    imprimirRuta(predecesores, atracciones);
}


//--------------------------------------------------------
int main() {
    Grafo grafo;
    construirGrafo(grafo, "grafo.csv");
    Nodo* arbolDecisiones = leerArbolDecisiones("decisiones.json");
    std::vector<Atraccion> atracciones = leerAtracciones("atracciones.json");

    bool salir = false;
    while (!salir) {
        mostrarMenu();
        int opcion;
        std::cin >> opcion;
        switch (opcion) {
            case 1:
                usarArbolDecisiones(arbolDecisiones, atracciones, grafo);
                break;
            case 2:
                seleccionManualDeAtracciones(grafo, atracciones);
                break;
            case 3:
                editarTiempoEspera(atracciones);
                guardarTiempoEspera("atracciones.json", atracciones);
                break;
            case 4:
                salir = true;
                break;
            default:
                std::cout << "Opcion no valida. Intente de nuevo.\n";
                break;
        }
    }

    liberarArbol(arbolDecisiones);
    return 0;
}

