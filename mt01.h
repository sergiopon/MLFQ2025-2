#ifndef MT01_H
#define MT01_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdio>

using namespace std;

// Enum para los diferentes algoritmos de planificacion
enum SchedulingAlgorithm {
    ROUND_ROBIN,
    SJF,
    STCF
};

// Clase para representar un proceso
class Process {
private:
    string label;           // Etiqueta del proceso
    int burstTime;         // Tiempo de CPU requerido
    int arrivalTime;       // Tiempo de llegada
    int queueLevel;        // Nivel de cola (1, 2, 3)
    int priority;          // Prioridad (5 más alta, 1 más baja)
    int waitingTime;       // Tiempo de espera
    int completionTime;    // Tiempo de finalizacion
    int responseTime;      // Tiempo de respuesta
    int turnaroundTime;    // Tiempo de retorno
    int remainingTime;     // Tiempo restante para completar
    bool hasStarted;       // Si el proceso ya empezo a ejecutarse

public:
    // Constructor
    Process(string lbl, int bt, int at, int q, int pr);
    
    // Getters
    string getLabel() const;
    int getBurstTime() const;
    int getArrivalTime() const;
    int getQueueLevel() const;
    int getPriority() const;
    int getWaitingTime() const;
    int getCompletionTime() const;
    int getResponseTime() const;
    int getTurnaroundTime() const;
    int getRemainingTime() const;
    bool getHasStarted() const;
    
    // Setters
    void setWaitingTime(int wt);
    void setCompletionTime(int ct);
    void setResponseTime(int rt);
    void setTurnaroundTime(int tat);
    void setRemainingTime(int rt);
    void setHasStarted(bool started);
    
    // Metodos utilitarios
    void calculateMetrics();
    string toString() const;
};


// Clase para manejar una cola con algoritmo especifico
class ProcessQueue {
public:
    vector<Process> processes;
    
private:
    SchedulingAlgorithm algorithm;
    int quantum;           // Para Round Robin
    int currentIndex;      // Para Round Robin
    
public:
    // Constructor
    ProcessQueue(SchedulingAlgorithm alg, int q = 0);
    
    // Métodos principales
    void addProcess(Process p);
    Process getNextProcess(int currentTime);
    bool isEmpty() const;
    int size() const;
    void removeProcess(Process p);
    
    // Métodos específicos para algoritmos
    Process roundRobin(int currentTime);
    Process shortestJobFirst(int currentTime);
    Process shortestTimeToCompletion(int currentTime);
    
    // Getters
    int getQuantum() const;
    SchedulingAlgorithm getAlgorithm() const;
};

// Clase principal del scheduler MLFQ
class MLFQ_Scheduler {
private:
    vector<ProcessQueue> queues;
    vector<Process> allProcesses;
    vector<Process> finishedProcesses;
    int currentTime;
    int scheme; // 1, 2, o 3 para los diferentes esquemas
    
public:
    // Constructor
    MLFQ_Scheduler(int schemeNumber);
    
    // Destructor
    ~MLFQ_Scheduler();
    
    // Metodos principales
    void addProcess(Process p);
    void simulate();
    void executeProcess(Process& p, int timeSlice);
    void checkArrivals();
    
    // Metodos de entrada/salida
    bool loadProcessesFromFile(const string& filename);
    void saveResultsToFile(const string& filename);
    
    // Metodos de calculo
    void calculateAverages();
    void printResults();
    
    // Metodo auxiliar
    void updateProcessInQueue(int queueIndex, const Process& updatedProcess);
    
    // Getters
    int getCurrentTime() const;
    vector<Process> getFinishedProcesses() const;
};

#endif