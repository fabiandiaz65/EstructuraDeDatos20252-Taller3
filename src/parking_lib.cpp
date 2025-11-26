#include "parking_lib.h"

#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>

struct InternalEvent {
    std::string plate;
    int spot;
    int occupied;
    long timestamp;
};

static std::vector<InternalEvent> g_history;
static const char* EVENT_FILE = "eventos_parqueadero.log";
static int g_num_spots = 0;   // <= número de celdas
static std::vector<int> g_occupancy;  // 1 = ocupado, 0 = libre

extern "C" {

PARKING_API void init_parking(int num_spots) {
    g_history.clear();
    g_num_spots = num_spots;
    g_occupancy.assign(num_spots, 0);

    // Reinicia el archivo
    std::ofstream f(EVENT_FILE, std::ios::trunc);
    f.close();
}

PARKING_API void add_event(const char* plate, int spot, int occupied, long timestamp) {
    InternalEvent ev{plate, spot, occupied, timestamp};
    g_history.push_back(ev);

    // Actualizar ocupación actual
    if (spot >= 0 && spot < g_num_spots) {
        g_occupancy[spot] = (occupied == 1 ? 1 : 0);
    }

    // Guardar en archivo
    std::ofstream f(EVENT_FILE, std::ios::app);
    f << plate << "," << spot << "," << occupied << "," << timestamp << "\n";
    f.close();
}

static void load_history_from_file() {
    g_history.clear();

    std::ifstream f(EVENT_FILE);
    if (!f.is_open()) return;

    std::string line;
    while (std::getline(f, line)) {
        std::stringstream ss(line);
        std::string plate;
        int spot, occupied;
        long timestamp;
        char sep;

        std::getline(ss, plate, ',');
        ss >> spot >> sep >> occupied >> sep >> timestamp;

        g_history.push_back({plate, spot, occupied, timestamp});
    }
    f.close();
}

PARKING_API int get_history_length() {
    load_history_from_file();
    return g_history.size();
}

PARKING_API void get_history(ParkingEvent* buffer, int max_len) {
    load_history_from_file();

    int n = g_history.size();
    if (max_len < n) n = max_len;

    for (int i = 0; i < n; ++i) {
        auto& src = g_history[i];
        auto& dst = buffer[i];

        std::memset(dst.plate, 0, sizeof(dst.plate));
        std::strncpy(dst.plate, src.plate.c_str(), sizeof(dst.plate) - 1);

        dst.spot = src.spot;
        dst.occupied = src.occupied;
        dst.timestamp = src.timestamp;
    }
}

PARKING_API int get_num_spots() {
    return g_num_spots;
}

PARKING_API int get_current_occupied() {
    int count = 0;
    for (int x : g_occupancy) {
        if (x == 1) count++;
    }
    return count;
}

} // extern "C"
