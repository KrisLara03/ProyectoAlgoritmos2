#include <iostream>
#include <vector>
#include <fstream>
#include "json.hpp"
#include "csv.h"
#include <limits> 
using json = nlohmann::json;

//---------------------------------------------------

// Nodo del Árbol de Decisiones
struct Nodo {
    std::string pregunta;
    Nodo* izquierda;
    Nodo* derecha;
    std::vector<int> identificadores; // Solo en nodos hoja
};

//---------------------------------------------------

// Estructura para la información de atracciones
struct Atraccion {
    int dentificador;
    std::string nombre;
    int tiempoEspera;
};

//---------------------------------------------------

// Estructura para el Grafo
struct Grafo {
    std::vector<std::vector<int>> matrizAdyacencia;
};

//---------------------------------------------------

// Funciones para construir y manipular el Grafo
void construirGrafo(Grafo& grafo, const std::string& archivoCSV) {
    std::ifstream archivo(archivoCSV);
    std::string linea;
    while (std::getline(archivo, linea)) {
        std::vector<int> fila;
        std::stringstream ss(linea);
        std::string valor;
        while (std::getline(ss, valor, ',')) {
            fila.push_back(std::stoi(valor));
        }
        grafo.matrizAdyacencia.push_back(fila);
    }
}

//---------------------------------------------------

// Funciones para construir y manipular el Árbol de Decisiones
// Aquí cambio

Nodo* construirArbol(const json& j) {
    Nodo* nodo = new Nodo();

    if (j.contains("pregunta")) {
        nodo->pregunta = j["pregunta"];
    } else {
        std::cerr << "Error: Clave 'pregunta' no encontrada en el objeto JSON." << std::endl;
        delete nodo;
        return nullptr;
    }

    if (j.contains("izquierda")) {
        nodo->izquierda = construirArbol(j["izquierda"]);
    } else {
        nodo->izquierda = nullptr;
    }

    if (j.contains("derecha")) {
        nodo->derecha = construirArbol(j["derecha"]);
    } else {
        nodo->derecha = nullptr;
    }

    if (j.contains("identificadores")) {
        nodo->identificadores = j["identificadores"].get<std::vector<int>>();
    } else {
        nodo->identificadores = {};
    }

    return nodo;
}

//Aqui también

Nodo* leerArbolDecisiones(const std::string& archivoJSON) {
    std::ifstream archivo(archivoJSON);
    if (!archivo.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << archivoJSON << std::endl;
        return nullptr;
    }

    if (archivo.peek() == std::ifstream::traits_type::eof()) {
        std::cerr << "Error: El archivo " << archivoJSON << " esta vacio." << std::endl;
        return nullptr;
    }

    try {
        json j;
        archivo >> j;
        if (j.is_null() || j.empty()) {
            std::cerr << "Error: El archivo " << archivoJSON << " contiene JSON invalido o vacio." << std::endl;
            return nullptr;
        }
        return construirArbol(j);
    } catch (const json::parse_error& e) {
        std::cerr << "Error de parseo en el archivo " << archivoJSON << ": " << e.what() << std::endl;
        return nullptr;
    }
}

std::vector<Atraccion> leerAtracciones(const std::string& archivoJSON) {
    std::vector<Atraccion> atracciones;
    std::ifstream archivo(archivoJSON);
    if (!archivo.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << archivoJSON << std::endl;
        return atracciones;
    }

    if (archivo.peek() == std::ifstream::traits_type::eof()) {
        std::cerr << "Error: El archivo " << archivoJSON << " está vacío." << std::endl;
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
            atraccion.id = entrada["id"];
            atraccion.tiempoEspera = entrada["tiempoEspera"];
            atraccion.nombre = entrada["nombre"];
            atracciones.push_back(atraccion);
        }
    } catch (const json::parse_error& e) {
        std::cerr << "Error de parseo en el archivo " << archivoJSON << ": " << e.what() << std::endl;
    }

    return atracciones;
}

//---------------------------------------------------



void editarTiempoEspera(std::vector<Atraccion>& atracciones) {
    std::cout << "Ingrese el ID de la atracción a editar: ";
    int id;
    std::cin >> id;
    
    std::cout << "Ingrese el nuevo tiempo de espera: ";
    int nuevoTiempo;
    std::cin >> nuevoTiempo;
    
    for (auto& atraccion : atracciones) {
        if (atraccion.id == id) {
            atraccion.tiempoEspera = nuevoTiempo;
            std::cout << "Tiempo de espera actualizado.\n";
            return;
        }
    }
    std::cout << "ID de atracción no encontrado.\n";
}

//---------------------------------------------------

// Algoritmo de Dijkstra para encontrar la ruta más corta
std::vector<int> dijkstra(const Grafo& grafo, int inicio, const std::vector<int>& atracciones) {
    int n = grafo.matrizAdyacencia.size();
    std::vector<int> distancia(n, std::numeric_limits<int>::max());
    std::vector<bool> visitado(n, false);

    distancia[inicio] = 0;

    for (int i = 0; i < n - 1; ++i) {
        int min_distancia = std::numeric_limits<int>::max();
        int min_indice = -1;

        for (int j = 0; j < n; ++j) {
            if (!visitado[j] && distancia[j] <= min_distancia) {
                min_distancia = distancia[j];
                min_indice = j;
            }
        }

        visitado[min_indice] = true;

        for (int j = 0; j < n; ++j) {
            if (!visitado[j] && grafo.matrizAdyacencia[min_indice][j] && distancia[min_indice] != std::numeric_limits<int>::max() && distancia[min_indice] + grafo.matrizAdyacencia[min_indice][j] < distancia[j]) {
                distancia[j] = distancia[min_indice] + grafo.matrizAdyacencia[min_indice][j];
            }
        }
    }

    return distancia;
}

//---------------------------------------------------

// Función principal del menú de la aplicación
void mostrarMenu() {
    std::cout << "1. Usar el arbol de decisiones\n";
    std::cout << "2. Seleccion manual de atracciones\n";
    std::cout << "3. Editar tiempo de espera\n";
    std::cout << "4. Salir\n";
    std::cout << "Seleccione una opcion: ";
}

//---------------------------------------------------

// Implementación de las funciones

// Usar el árbol de decisiones -> Para la funcionalidad del arbol de desiciones

void usarArbolDecisiones(Nodo* nodo, const std::vector<Atraccion>& atracciones) {
    if (!nodo->izquierda && !nodo->derecha) {
        // Nodo hoja, mostrar atracciones
        std::cout << "Atracciones sugeridas:\n";
        for (int id : nodo->identificadores) {
            for (const auto& atraccion : atracciones) {
                if (atraccion.id == id) {
                    std::cout << "ID: " << atraccion.identificador << ", Nombre: " << atraccion.nombre << ", Tiempo de espera: " << atraccion.tiempoEspera << " minutos\n";
                }
            }
        }
        return;
    }

//---------------------------------------------------

    // Hacer pregunta
    std::cout << nodo->pregunta << " (1. Sí / 2. No): ";
    int respuesta;
    std::cin >> respuesta;
    if (respuesta == 1) {
        usarArbolDecisiones(nodo->izquierda, atracciones);
    } else if (respuesta == 2) {
        usarArbolDecisiones(nodo->derecha, atracciones);
    } else {
        std::cout << "Respuesta no válida. Intente de nuevo.\n";
        usarArbolDecisiones(nodo, atracciones);
    }
}

//---------------------------------------------------

// Seleccion manual de atracciones

void seleccionManualDeAtracciones(const Grafo& grafo, const std::vector<Atraccion>& atracciones) {
    std::cout << "Ingrese el ID de la atracción de inicio: ";
    int inicio;
    std::cin >> inicio;

    std::cout << "Ingrese los IDs de las atracciones a visitar (separados por espacios, termine con -1): ";
    std::vector<int> seleccionadas;
    int id;
    while (std::cin >> id && id != -1) {
        seleccionadas.push_back(id);
    }

    std::vector<int> distancias = dijkstra(grafo, inicio, seleccionadas);
    std::cout << "Distancias desde la atracción de inicio:\n";
    for (int i = 0; i < seleccionadas.size(); ++i) {
        std::cout << "ID: " << seleccionadas[i] << ", Distancia: " << distancias[seleccionadas[i]] << "\n";
    }
}

//---------------------------------------------------


int main() {
    Grafo grafo;
    construirGrafo(grafo, "grafo.csv");
    Nodo* arbolDecisiones = leerArbolDecisiones("decisiones.json");
    std::vector<Atraccion> atracciones = leerAtracciones("atracciones.csv");

    bool salir = false;
    while (!salir) {
        mostrarMenu();
        int opcion;
        std::cin >> opcion;
        switch (opcion) {
            case 1:
                usarArbolDecisiones(arbolDecisiones, atracciones);
                break;
            case 2:
                seleccionManualDeAtracciones(grafo, atracciones);
                break;
            case 3:
                editarTiempoEspera(atracciones);
                break;
            case 4:
                salir = true;
                break;
            default:
                std::cout << "Opción no válida. Intente de nuevo.\n";
                break;
        }
    }

    return 0;
}
