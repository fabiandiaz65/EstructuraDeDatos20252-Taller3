import ctypes
from ctypes import c_char, c_int, c_long
import matplotlib.pyplot as plt

parking_lib = ctypes.CDLL("./libparking.so")

class ParkingEvent(ctypes.Structure):
    _fields_ = [
        ("plate", c_char * 16),
        ("spot", c_int),
        ("occupied", c_int),
        ("timestamp", c_long),
    ]


# 3. Configuramos los tipos de las funciones que vamos a usar
parking_lib.get_history_length.restype = c_int
parking_lib.get_num_spots.restype = c_int

parking_lib.get_history.argtypes = (ctypes.POINTER(ParkingEvent), c_int)
parking_lib.get_history.restype = None


def main():
    # Pedimos cuántos eventos hay
    n = parking_lib.get_history_length()
    print("Eventos en historial:", n)

    if n == 0:
        print("No hay datos todavía. Ejecuta primero servidor y cliente.")
        return

    # Creamos un arreglo de ParkingEvent para recibir los datos
    events_array = (ParkingEvent * n)()
    parking_lib.get_history(events_array, n)

    # Pasamos a lista de Python
    history = list(events_array)

    # Ordenamos por timestamp (por si acaso)
    history.sort(key=lambda ev: ev.timestamp)

    times = []
    occupied_values = []
    occupied = 0

    # Recorremos el historial y vamos calculando cuántos puestos están ocupados
    for ev in history:
        if ev.occupied == 1:
            occupied += 1
        else:
            occupied -= 1
            if occupied < 0:
                occupied = 0  # seguridad

        times.append(ev.timestamp)
        occupied_values.append(occupied)

    # 4. Generamos el gráfico de línea
    plt.plot(times, occupied_values)
    plt.xlabel("Tiempo (timestamp)")
    plt.ylabel("Puestos ocupados")
    plt.title("Ocupación del parqueadero en el tiempo")
    plt.grid(True)
    plt.show()


if __name__ == "__main__":
    main()