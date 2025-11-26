// server.cpp
#include <iostream>
#include <sstream>
#include <string>

#include "parking_lib.h"

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "Ws2_32.lib")
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #define INVALID_SOCKET -1
  #define SOCKET_ERROR -1
  typedef int SOCKET;
#endif

int main() {
    const int PORT = 5000;

    // Inicializamos el parqueadero con 50 posiciones
    init_parking(50);

#ifdef _WIN32
    // Inicialización de la librería de sockets en Windows
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    // 1. Crear socket de escucha
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) {
        std::cerr << "Error creando socket\n";
        return 1;
    }

    // 2. Asociarlo a un puerto
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Error en bind\n";
        return 1;
    }

    // 3. Poner el socket en modo escucha
    listen(listen_sock, 1);
    std::cout << "Servidor escuchando en puerto " << PORT << "...\n";

    // 4. Aceptar un cliente
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    SOCKET client_sock = accept(listen_sock, (sockaddr*)&client_addr, &client_len);
    if (client_sock == INVALID_SOCKET) {
        std::cerr << "Error en accept\n";
        return 1;
    }

    std::cout << "Cliente conectado.\n";

    char buffer[512];
    std::string partial;

    // 5. Bucle de recepción de mensajes
    while (true) {
        int bytes = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            std::cout << "Cliente desconectado.\n";
            break;
        }

        buffer[bytes] = '\0';
        partial += buffer;

        // Procesar línea por línea
        size_t pos;
        while ((pos = partial.find('\n')) != std::string::npos) {
            std::string line = partial.substr(0, pos);
            partial.erase(0, pos + 1);

            if (line.empty()) continue;

            std::istringstream iss(line);
            std::string plate;
            int spot;
            int occupied;
            long timestamp;

            // Parseamos: PLACA CELDA ACCION TIMESTAMP
            if (!(iss >> plate >> spot >> occupied >> timestamp)) {
                std::cerr << "Linea mal formada: " << line << "\n";
                continue;
            }

            // Guardamos el evento en la librería
            add_event(plate.c_str(), spot, occupied, timestamp);

            std::cout << "Evento recibido: " << plate
                      << " spot=" << spot
                      << " occ=" << occupied
                      << " t=" << timestamp
                      << " | ocupados=" << get_current_occupied() << "\n";
        }
    }

#ifdef _WIN32
    closesocket(client_sock);
    closesocket(listen_sock);
    WSACleanup();
#else
    close(client_sock);
    close(listen_sock);
#endif

    return 0;
}