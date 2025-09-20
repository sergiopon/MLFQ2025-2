# MLFQ (Multilevel Feedback Queue) – Informe breve

Este proyecto implementa un simulador MLFQ en C++ usando orientación a objetos. El planificador mantiene varias colas de distinta prioridad y, en cada “tick” de tiempo, siempre atiende la cola de mayor prioridad que tenga procesos listos. Las colas pueden usar distintas políticas: Round Robin (RR), Shortest Job First (SJF) o Shortest Time to Completion First (STCF).

## Estructura del código

- `Process`
	- Datos: etiqueta, BT (burst time), AT (arrival), Q (nivel de cola), Pr (prioridad), métricas (WT, CT, RT, TAT), `remainingTime`, `hasStarted`.
	- Responsabilidad: conservar el estado y calcular métricas al terminar.
		- TAT = CT − AT; WT = TAT − BT; RT se fija cuando entra por primera vez a CPU.

- `ProcessQueue`
	- Datos: `vector<Process>`, algoritmo (`ROUND_ROBIN`, `SJF`, `STCF`), `quantum` e índice para RR.
	- Responsabilidad: insertar procesos y entregar el siguiente según política.
		- SJF/STCF: orden por menor tiempo restante (STCF reevalúa en cada tick).
		- RR: rotación circular en la misma cola usando `currentIndex` y `quantum`.

- `MLFQ_Scheduler`
	- Datos: conjunto de colas, lista de procesos por llegar, lista de terminados, `currentTime`, `scheme`.
	- Responsabilidad: simular por ticks, gestionar llegadas, preempción, selección y ejecución, y calcular métricas finales.

## Lógica de simulación (por tick)

1) Llegadas: se mueven procesos con AT ≤ tiempo actual a su cola `Q`.
2) Prioridad entre colas: se elige la primera cola (de mayor prioridad) que no esté vacía.
3) Selección dentro de la cola:
	 - RR: siguiente por rotación.
	 - SJF: menor tiempo restante (no expropiativo).
	 - STCF: menor tiempo restante con reevaluación cada tick (expropiativo).
4) Ejecución: se corre exactamente 1 unidad de tiempo, se reduce `remainingTime` y avanza `currentTime`.
5) Post-ejecución:
	 - Si `remainingTime == 0`: se fija CT y se calculan TAT y WT; el proceso pasa a terminados.
	 - En RR, si agota `quantum`: rota dentro de la misma cola (no se demueve de cola).
	 - Si no terminó ni agotó `quantum`: se actualiza su estado y continúa el siguiente tick.

Preempción: si llega trabajo a una cola de mayor prioridad, se interrumpe lo que corre y se reevalúa. En STCF también hay preempción interna de la cola por menor tiempo restante.

## Esquemas disponibles

Al iniciar, el programa te pide elegir uno:

1) RR(1), RR(3), RR(4), SJF
2) RR(2), RR(3), RR(4), STCF
3) RR(3), RR(5), RR(6), RR(20)

La CPU siempre atiende primero la cola 1; cuando se vacía, pasa a la siguiente.

## Formato de entrada y salida

- Entrada (`.txt`): una línea por proceso en el formato `Etiqueta;BT;AT;Q;Pr`. Se ignoran líneas que comienzan con `#`.
- Salida (`.out`):
	- Cabecera `# etiqueta; BT; AT; Q; Pr; WT; CT; RT; TAT`.
	- Una línea por proceso con sus métricas y una línea final con promedios `WT=...; CT=...; RT=...; TAT=...;`.
	- La consola imprime un timeline por tick: `Tiempo t: Ejecutando proceso X ...`.

## Compilar y ejecutar (Windows/PowerShell)

Compilar con g++:

```powershell
g++ -o mt01 mt01.cpp
```

Ejecutar (interactivo):

```powershell
./mt01.exe
```

Sigue los prompts para: esquema (1/2/3), archivo de entrada y archivo de salida.

## Notas

- El campo `Pr` se reporta en la salida; por defecto no se usa para desempates dentro de la misma cola. Si se requiere, puede añadirse como criterio secundario (p. ej., en SJF/STCF cuando hay empates por `remainingTime`).
- La simulación opera en ticks de 1 unidad de tiempo, lo cual facilita explicar y verificar el orden de ejecución y las métricas en el informe o el video.

