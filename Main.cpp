//g++ .\Main.cpp -o main.exe

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
    int identificador;
    std::string nombre;
    int tiempo_espera;
};

//---------------------------------------------------

// Estructura para el Grafo
struct Grafo {
    std::vector<std::vector<int>> matrizAdyacencia;
};

//---------------------------------------------------

// Función para construir el Grafo desde un archivo CSV
void construirGrafo(Grafo& grafo, const std::string& archivoCSV) {
    // Intentar abrir el archivo CSV
    std::ifstream archivo(archivoCSV);
    if (!archivo.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << archivoCSV << std::endl;
        return;
    }

    // Verificar si el archivo está vacío
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
                // Intentar convertir el valor a entero
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

    // Verificar si todas las filas tienen la misma longitud
    std::size_t num_columnas = grafo.matrizAdyacencia[0].size();
    for (const auto& fila : grafo.matrizAdyacencia) {
        if (fila.size() != num_columnas) {
            std::cerr << "Error: Inconsistencia en el número de columnas en el archivo " << archivoCSV << "." << std::endl;
            return;
        }
    }

    std::cout << "Grafo construido con exito desde el archivo " << archivoCSV << "." << std::endl;
}

//---------------------------------------------------

// Función auxiliar para construir el Árbol de Decisiones a partir de un objeto JSON
Nodo* construirArbol(const json& j) {
    // Imprimir el contenido del nodo actual para depuración
    //std::cout << "Procesando nodo: " << j.dump(4) << std::endl;

    Nodo* nodo = new Nodo();

    // Verificar y asignar la pregunta si está presente
    if (j.contains("pregunta")) {
        nodo->pregunta = j["pregunta"];
    } else {
        nodo->pregunta = ""; // Asignar una cadena vacía si no hay pregunta
    }

    // Verificar y asignar el nodo izquierdo si está presente
    if (j.contains("izquierda") && j["izquierda"].is_object()) {
        nodo->izquierda = construirArbol(j["izquierda"]);
    } else {
        nodo->izquierda = nullptr;
    }

    // Verificar y asignar el nodo derecho si está presente
    if (j.contains("derecha") && j["derecha"].is_object()) {
        nodo->derecha = construirArbol(j["derecha"]);
    } else {
        nodo->derecha = nullptr;
    }

    // Verificar y asignar los identificadores si está presente
    if (j.contains("identificadores") && j["identificadores"].is_array()) {
        nodo->identificadores = j["identificadores"].get<std::vector<int>>();
    } else {
        nodo->identificadores = {}; // Asignar un vector vacío si no hay identificadores
    }

    return nodo;
}
// Función para leer y construir el Árbol de Decisiones a partir de un archivo JSON
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
                continue; // Omitir esta entrada y pasar a la siguiente
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

//---------------------------------------------------



void editarTiempoEspera(std::vector<Atraccion>& atracciones) {
    std::cout << "Ingrese el identificador de la atracción a editar: ";
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
    std::cout << "identificador de atraccion no encontrado.\n";
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
        for (int identificador : nodo->identificadores) {
            for (const auto& atraccion : atracciones) {
                if (atraccion.identificador == identificador) {
                    std::cout << "identificador: " << atraccion.identificador << ", Nombre: " << atraccion.nombre << ", Tiempo de espera: " << atraccion.tiempo_espera << " minutos\n";
                }
            }
        }
        return;
    }

//---------------------------------------------------

    // Hacer pregunta
    std::cout << nodo->pregunta << " (1. Si / 2. No): ";
    int respuesta;
    std::cin >> respuesta;
    if (respuesta == 1) {
        usarArbolDecisiones(nodo->izquierda, atracciones);
    } else if (respuesta == 2) {
        usarArbolDecisiones(nodo->derecha, atracciones);
    } else {
        std::cout << "Respuesta no valida. Intente de nuevo.\n";
        usarArbolDecisiones(nodo, atracciones);
    }
}

//---------------------------------------------------

// Seleccion manual de atracciones

void seleccionManualDeAtracciones(const Grafo& grafo, const std::vector<Atraccion>& atracciones) {
    std::cout << "Ingrese el identificador de la atraccion de inicio: ";
    int inicio;
    std::cin >> inicio;

    std::cout << "Ingrese los identificador de las atracciones a visitar (separados por espacios, termine con -1): ";
    std::vector<int> seleccionadas;
    int identificador;
    while (std::cin >> identificador && identificador != -1) {
        seleccionadas.push_back(identificador);
    }

    std::vector<int> distancias = dijkstra(grafo, inicio, seleccionadas);
    std::cout << "Distancias desde la atraccion de inicio:\n";
    for (int i = 0; i < seleccionadas.size(); ++i) {
        std::cout << "identificador: " << seleccionadas[i] << ", Distancia: " << distancias[seleccionadas[i]] << "\n";
    }
}

//---------------------------------------------------


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
                std::cout << "Opcion no valida. Intente de nuevo.\n";
                break;
        }
    }

    return 0;

}