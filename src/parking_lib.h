#pragma once

#ifdef _WIN32
  // En Windows, usamos __declspec para exportar/importar funciones de la DLL
  #ifdef PARKINGLIB_EXPORTS
    #define PARKING_API __declspec(dllexport)
  #else
    #define PARKING_API __declspec(dllimport)
  #endif
#else
  // En Linux/Mac no se necesita nada especial
  #define PARKING_API
#endif

// extern "C" para evitar name mangling y poder llamar fácil desde Python (ctypes)
extern "C" {

// Estructura que representa un evento de parqueo
struct ParkingEvent {
    char plate[16];   // Placa del carro, por ejemplo "ABC123"
    int  spot;        // Número de celda de parqueo
    int  occupied;    // 1 = entra (ocupa), 0 = sale (libera)
    long timestamp;   // Momento del evento (segundos desde epoch o similar)
};

// Inicializa el parqueadero con N puestos
PARKING_API void init_parking(int num_spots);

// Agrega un evento al historial y actualiza la ocupación
PARKING_API void add_event(const char* plate, int spot, int occupied, long timestamp);

// Devuelve cuántos puestos tiene el parqueadero
PARKING_API int get_num_spots();

// Devuelve cuántos puestos están ocupados actualmente
PARKING_API int get_current_occupied();

// Devuelve cuántos eventos hay en el historial
PARKING_API int get_history_length();

// Copia hasta max_len eventos al buffer proporcionado
PARKING_API void get_history(ParkingEvent* buffer, int max_len);

} // extern "C"