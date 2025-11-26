#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <random>
#include <map>
#include <ctime>

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

// Genera una placa aleatoria, por ejemplo "ABC123"
std::string random_plate(std::mt19937& rng) {
    std::uniform_int_distribution<int> letters('A', 'Z');
    std::uniform_int_distribution<int> digits(0, 9);
    std::string plate;
    for (int i = 0; i < 3; ++i) plate.push_back(char(letters(rng)));
    for (int i = 0; i < 3; ++i) plate.push_back(char('0' + digits(rng)));
    return plate;
}

int main() {
    const char* SERVER_IP = "127.0.0.1"; // Máquina local
    const int   PORT = 5000;
    const int   NUM_SPOTS = 50;

#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    // 1. Crear socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Error creando socket\n";
        return 1;
    }

    // 2. Configurar dirección del servidor
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // 3. Conectarse al servidor
    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "No se pudo conectar al servidor\n";
        return 1;
    }

    std::cout << "Conectado al servidor\n";

    // RNG para aleatoriedad
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> spot_dist(0, NUM_SPOTS - 1);
    std::uniform_int_distribution<int> sleep_dist(2, 5);

    // Mapa placa -> spot para recordar qué puesto ocupa cada carro activo
    std::map<std::string, int> active_cars;

    while (true) {
        bool use_existing = !active_cars.empty() && (rng() % 2 == 0);

        std::string plate;
        int spot;
        int occupied;

        if (use_existing) {
            // Caso: sacamos un carro que ya estaba adentro
            auto it = active_cars.begin();
            std::advance(it, rng() % active_cars.size());
            plate = it->first;
            spot = it->second;
            occupied = 0;      // 0 = sale (libera)
            active_cars.erase(it);
        } else {
            // Caso: entra un carro nuevo
            plate = random_plate(rng);
            spot = spot_dist(rng);
            occupied = 1;      // 1 = entra (ocupa)
            active_cars[plate] = spot;
        }

        long timestamp = std::time(nullptr);

        // Mensaje con el formato acordado
        std::string msg = plate + " " +
                          std::to_string(spot) + " " +
                          std::to_string(occupied) + " " +
                          std::to_string(timestamp) + "\n";

        // Enviamos al servidor
        send(sock, msg.c_str(), (int)msg.size(), 0);

        std::cout << "Enviado: " << msg;

        // Esperamos de 2 a 5 segundos
        int sleep_s = sleep_dist(rng);
        std::this_thread::sleep_for(std::chrono::seconds(sleep_s));
    }

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

    return 0;
}