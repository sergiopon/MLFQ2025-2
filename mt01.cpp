#include "mt01.h"
#include <cstdlib>
#include <limits>

// Helpers
static inline Process makeEmptyProcess() { return Process("", 0, 0, 0, 0); }

// Ordenamiento simple por tiempo restante (burbuja)
void sortByRemainingTime(vector<Process>& processes) {
    for (int i = 0; i < (int)processes.size() - 1; i++) {
        for (int j = 0; j < (int)processes.size() - 1 - i; j++) {
            if (processes[j].getRemainingTime() > processes[j + 1].getRemainingTime()) {
                Process temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }
}

// ---- Process ----

Process::Process(string lbl, int bt, int at, int q, int pr) 
    : label(lbl), burstTime(bt), arrivalTime(at), queueLevel(q), priority(pr),
      waitingTime(0), completionTime(0), responseTime(0), turnaroundTime(0),
      remainingTime(bt), hasStarted(false) {}

// Getters
string Process::getLabel() const { return label; }
int Process::getBurstTime() const { return burstTime; }
int Process::getArrivalTime() const { return arrivalTime; }
int Process::getQueueLevel() const { return queueLevel; }
int Process::getPriority() const { return priority; }
int Process::getWaitingTime() const { return waitingTime; }
int Process::getCompletionTime() const { return completionTime; }
int Process::getResponseTime() const { return responseTime; }
int Process::getTurnaroundTime() const { return turnaroundTime; }
int Process::getRemainingTime() const { return remainingTime; }
bool Process::getHasStarted() const { return hasStarted; }

// Sets
void Process::setWaitingTime(int wt) { waitingTime = wt; }
void Process::setCompletionTime(int ct) { completionTime = ct; }
void Process::setResponseTime(int rt) { responseTime = rt; }
void Process::setTurnaroundTime(int tat) { turnaroundTime = tat; }
void Process::setRemainingTime(int rt) { remainingTime = rt; }
void Process::setHasStarted(bool started) { hasStarted = started; }

void Process::calculateMetrics() {
    turnaroundTime = completionTime - arrivalTime;
    waitingTime = turnaroundTime - burstTime;
}

string Process::toString() const {
    stringstream ss;
    ss << label << ";" << burstTime << ";" << arrivalTime << ";" << queueLevel 
       << ";" << priority << ";" << waitingTime << ";" << completionTime 
       << ";" << responseTime << ";" << turnaroundTime;
    return ss.str();
}

// ---- ProcessQueue ----

ProcessQueue::ProcessQueue(SchedulingAlgorithm alg, int q) 
    : algorithm(alg), quantum(q), currentIndex(0) {}

void ProcessQueue::addProcess(Process p) {
    processes.push_back(p);
    
    // Ordenar segun el algoritmo
    if (algorithm == SJF || algorithm == STCF) {
        sortByRemainingTime(processes);
    }
}

Process ProcessQueue::getNextProcess(int currentTime) {
    if (processes.empty()) return makeEmptyProcess();
    
    switch (algorithm) {
        case ROUND_ROBIN:
            return roundRobin(currentTime);
        case SJF:
            return shortestJobFirst(currentTime);
        case STCF:
            return shortestTimeToCompletion(currentTime);
        default:
            return makeEmptyProcess();
    }
}

Process ProcessQueue::roundRobin(int currentTime) {
    if (processes.empty()) return makeEmptyProcess();
    
    Process selected = processes[currentIndex];
    currentIndex = (currentIndex + 1) % (int)processes.size();
    return selected;
}

Process ProcessQueue::shortestJobFirst(int currentTime) {
    if (processes.empty()) return makeEmptyProcess();
    
    // Ya esta ordenado por tiempo restante
    return processes[0];
}

Process ProcessQueue::shortestTimeToCompletion(int currentTime) {
    if (processes.empty()) return makeEmptyProcess();
    
    // Reordenar por tiempo restante (STCF es preventivo)
    sortByRemainingTime(processes);
    
    return processes[0];
}

bool ProcessQueue::isEmpty() const {
    return processes.empty();
}

int ProcessQueue::size() const {
    return processes.size();
}

void ProcessQueue::removeProcess(Process p) {
    for (int i = 0; i < (int)processes.size(); i++) {
        if (processes[i].getLabel() == p.getLabel()) {
            processes.erase(processes.begin() + i);
            if (currentIndex >= (int)processes.size() && !processes.empty()) currentIndex = 0;
            break;
        }
    }
}

int ProcessQueue::getQuantum() const { return quantum; }
SchedulingAlgorithm ProcessQueue::getAlgorithm() const { return algorithm; }

// ---- MLFQ_Scheduler ----

MLFQ_Scheduler::MLFQ_Scheduler(int schemeNumber) 
    : currentTime(0), scheme(schemeNumber) {
    
    // Inicializar las 4 colas segun el esquema seleccionado
    switch (scheme) {
        case 1: // RR(1), RR(3), RR(4), SJF
            queues.push_back(ProcessQueue(ROUND_ROBIN, 1));
            queues.push_back(ProcessQueue(ROUND_ROBIN, 3));
            queues.push_back(ProcessQueue(ROUND_ROBIN, 4));
            queues.push_back(ProcessQueue(SJF, 0));
            break;
        case 2: // RR(2), RR(3), RR(4), STCF
            queues.push_back(ProcessQueue(ROUND_ROBIN, 2));
            queues.push_back(ProcessQueue(ROUND_ROBIN, 3));
            queues.push_back(ProcessQueue(ROUND_ROBIN, 4));
            queues.push_back(ProcessQueue(STCF, 0));
            break;
        case 3: // RR(3), RR(5), RR(6), RR(20)
            queues.push_back(ProcessQueue(ROUND_ROBIN, 3));
            queues.push_back(ProcessQueue(ROUND_ROBIN, 5));
            queues.push_back(ProcessQueue(ROUND_ROBIN, 6));
            queues.push_back(ProcessQueue(ROUND_ROBIN, 20));
            break;
        default:
            // Esquema por defecto
            queues.push_back(ProcessQueue(ROUND_ROBIN, 1));
            queues.push_back(ProcessQueue(ROUND_ROBIN, 3));
            queues.push_back(ProcessQueue(ROUND_ROBIN, 4));
            queues.push_back(ProcessQueue(SJF, 0));
            break;
    }
}

MLFQ_Scheduler::~MLFQ_Scheduler() {
    // No hay punteros que liberar
}

void MLFQ_Scheduler::addProcess(Process p) {
    allProcesses.push_back(p);
}

void MLFQ_Scheduler::simulate() {
    cout << "Iniciando simulacion MLFQ - Esquema " << scheme << endl;

    // Estado del proceso en CPU
    bool hasRunning = false;
    Process running = Process("", 0, 0, 0, 0);
    int runningQueue = -1;
    int rrQuantumUsed = 0; // unidades usadas del quantum en RR

    auto anyWorkLeft = [&]() -> bool {
        if (!allProcesses.empty()) return true;
        for (int i = 0; i < (int)queues.size(); i++) if (!queues[i].isEmpty()) return true;
        return false;
    };

    while (anyWorkLeft()) {
        // Llegadas en este tiempo
        checkArrivals();

        // Preempcion por llegada a cola de mayor prioridad
        if (hasRunning) {
            for (int i = 0; i < runningQueue; i++) {
                if (!queues[i].isEmpty()) {
                    // Devolver el proceso en ejecucion a su cola (sin democion)
                    updateProcessInQueue(runningQueue, running);
                    hasRunning = false;
                    rrQuantumUsed = 0;
                    break;
                }
            }
        }

        // Para STCF, re-evaluar la eleccion dentro de su cola en cada tick
        if (hasRunning && queues[runningQueue].getAlgorithm() == STCF) {
            // Seleccionar el de menor remaining en esa cola
            Process candidate = queues[runningQueue].getNextProcess(currentTime);
            if (candidate.getLabel() != running.getLabel()) {
                // Cambiar por el mas corto
                updateProcessInQueue(runningQueue, running);
                running = candidate;
                rrQuantumUsed = 0;
                // Si primera vez en CPU, setear RT
                if (!running.getHasStarted()) {
                    running.setResponseTime(currentTime - running.getArrivalTime());
                    running.setHasStarted(true);
                    updateProcessInQueue(runningQueue, running);
                }
            }
        }

        // Elegir un nuevo proceso si no hay uno corriendo
        if (!hasRunning) {
            runningQueue = -1;
            for (int i = 0; i < (int)queues.size(); i++) {
                if (!queues[i].isEmpty()) {
                    running = queues[i].getNextProcess(currentTime);
                    runningQueue = i;
                    hasRunning = true;
                    rrQuantumUsed = 0;
                    if (!running.getHasStarted()) {
                        running.setResponseTime(currentTime - running.getArrivalTime());
                        running.setHasStarted(true);
                        updateProcessInQueue(runningQueue, running);
                    }
                    break;
                }
            }
        }

        // Si aun no hay listo, avanzar el reloj
        if (!hasRunning) { currentTime++; continue; }

        // Ejecutar exactamente 1 unidad de tiempo y luego re-evaluar
        executeProcess(running, 1);
        if (queues[runningQueue].getAlgorithm() == ROUND_ROBIN) rrQuantumUsed++;

        // Termino?
        if (running.getRemainingTime() == 0) {
            running.setCompletionTime(currentTime);
            running.calculateMetrics();
            finishedProcesses.push_back(running);
            queues[runningQueue].removeProcess(running);
            hasRunning = false;
            rrQuantumUsed = 0;
            continue;
        }

        // Si es RR y agoto quantum, rotar dentro de la misma cola (sin democión)
        if (queues[runningQueue].getAlgorithm() == ROUND_ROBIN &&
            rrQuantumUsed == queues[runningQueue].getQuantum()) {
            queues[runningQueue].removeProcess(running);
            int nextQ = runningQueue; // mantener en su misma cola
            queues[nextQ].addProcess(running);
            hasRunning = false;
            rrQuantumUsed = 0;
            continue;
        }

        // Caso contrario, actualizar el proceso en su misma cola y seguir al siguiente tick
        updateProcessInQueue(runningQueue, running);
    }

    cout << "Simulacion completada en tiempo: " << currentTime << endl;
}

void MLFQ_Scheduler::executeProcess(Process& p, int timeSlice) {
    cout << "Tiempo " << currentTime << ": Ejecutando proceso "
         << p.getLabel() << " por " << timeSlice << " unidades" << endl;
    
    currentTime += timeSlice;
    p.setRemainingTime(p.getRemainingTime() - timeSlice);
}

void MLFQ_Scheduler::checkArrivals() {
    for (int i = 0; i < (int)allProcesses.size();) {
        if (allProcesses[i].getArrivalTime() <= currentTime) {
            int queueLevel = allProcesses[i].getQueueLevel() - 1; // 0-based
            if (queueLevel >= 0 && queueLevel < (int)queues.size())
                queues[queueLevel].addProcess(allProcesses[i]);
            else
                queues[0].addProcess(allProcesses[i]);
            allProcesses.erase(allProcesses.begin() + i);
        } else {
            i++;
        }
    }
}

// Actualiza un proceso ya presente en la cola (por etiqueta)
void MLFQ_Scheduler::updateProcessInQueue(int queueIndex, const Process& updatedProcess) {
    for (int i = 0; i < queues[queueIndex].size(); i++) {
        // Necesitamos acceder directamente al vector de procesos
        // Por simplicidad, removemos y agregamos el proceso actualizado
        if (queues[queueIndex].processes[i].getLabel() == updatedProcess.getLabel()) {
            queues[queueIndex].processes[i] = updatedProcess;
            break;
        }
    }
}

bool MLFQ_Scheduler::loadProcessesFromFile(const string& filename) {
    FILE* file = fopen(filename.c_str(), "r");
    if (file == NULL) {
        cout << "Error: No se pudo abrir el archivo " << filename << endl;
        return false;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Convertir char array a string
        string lineStr(line);
        
        // Remover salto de linea
        if (!lineStr.empty() && lineStr[lineStr.length() - 1] == '\n') {
            lineStr = lineStr.substr(0, lineStr.length() - 1);
        }
        
        // Ignorar lineas de comentario
        if (lineStr.empty() || lineStr[0] == '#') continue;
        
        // Parsear linea: etiqueta;BT;AT;Q;Priority
        stringstream ss(lineStr);
        string label, btStr, atStr, qStr, prStr;
        
        if (getline(ss, label, ';') && 
            getline(ss, btStr, ';') && 
            getline(ss, atStr, ';') && 
            getline(ss, qStr, ';') && 
            getline(ss, prStr, ';')) {
            
            int bt = atoi(btStr.c_str());
            int at = atoi(atStr.c_str());
            int q = atoi(qStr.c_str());
            int pr = atoi(prStr.c_str());
            
            Process process(label, bt, at, q, pr);
            addProcess(process);
            
            cout << "Proceso cargado: " << label << " BT=" << bt 
                 << " AT=" << at << " Q=" << q << " Pr=" << pr << endl;
        }
    }
    
    fclose(file);
    return true;
}

void MLFQ_Scheduler::saveResultsToFile(const string& filename) {
    FILE* file = fopen(filename.c_str(), "w");
    if (file == NULL) {
        cout << "Error: No se pudo crear el archivo " << filename << endl;
        return;
    }
    
    fprintf(file, "# etiqueta; BT; AT; Q; Pr; WT; CT; RT; TAT\n");
    
    double totalWT = 0, totalCT = 0, totalRT = 0, totalTAT = 0;
    
    for (int i = 0; i < (int)finishedProcesses.size(); i++) {
        Process p = finishedProcesses[i];
        fprintf(file, "%s;%d;%d;%d;%d;%d;%d;%d;%d\n",
                p.getLabel().c_str(),
                p.getBurstTime(),
                p.getArrivalTime(),
                p.getQueueLevel(),
                p.getPriority(),
                p.getWaitingTime(),
                p.getCompletionTime(),
                p.getResponseTime(),
                p.getTurnaroundTime());
        
        totalWT += p.getWaitingTime();
        totalCT += p.getCompletionTime();
        totalRT += p.getResponseTime();
        totalTAT += p.getTurnaroundTime();
    }
    
    int numProcesses = (int)finishedProcesses.size();
    if (numProcesses > 0) {
        fprintf(file, "WT=%.1f; CT=%.1f; RT=%.1f; TAT=%.1f;\n",
                (totalWT / numProcesses),
                (totalCT / numProcesses),
                (totalRT / numProcesses),
                (totalTAT / numProcesses));
    }
    
    fclose(file);
    cout << "Resultados guardados en: " << filename << endl;
}

void MLFQ_Scheduler::calculateAverages() {
    if (finishedProcesses.empty()) return;
    
    double totalWT = 0, totalCT = 0, totalRT = 0, totalTAT = 0;
    
    for (int i = 0; i < (int)finishedProcesses.size(); i++) {
        Process p = finishedProcesses[i];
        totalWT += p.getWaitingTime();
        totalCT += p.getCompletionTime();
        totalRT += p.getResponseTime();
        totalTAT += p.getTurnaroundTime();
    }
    
    int numProcesses = (int)finishedProcesses.size();
    
    cout << "\n=== METRICAS PROMEDIO ===" << endl;
    printf("Tiempo de Espera Promedio (WT): %.2f\n", (totalWT / numProcesses));
    printf("Tiempo de Finalizacion Promedio (CT): %.2f\n", (totalCT / numProcesses));
    printf("Tiempo de Respuesta Promedio (RT): %.2f\n", (totalRT / numProcesses));
    printf("Tiempo de Retorno Promedio (TAT): %.2f\n", (totalTAT / numProcesses));
}

void MLFQ_Scheduler::printResults() {
    cout << "\n=== RESULTADOS DE LA SIMULACION ===" << endl;
    cout << "Proceso\tBT\tAT\tQ\tPr\tWT\tCT\tRT\tTAT" << endl;
    cout << "-------\t--\t--\t-\t--\t--\t--\t--\t---" << endl;
    
    for (int i = 0; i < (int)finishedProcesses.size(); i++) {
        Process p = finishedProcesses[i];
        cout << p.getLabel() << "\t" << p.getBurstTime() << "\t" 
             << p.getArrivalTime() << "\t" << p.getQueueLevel() << "\t"
             << p.getPriority() << "\t" << p.getWaitingTime() << "\t"
             << p.getCompletionTime() << "\t" << p.getResponseTime() << "\t"
             << p.getTurnaroundTime() << endl;
    }
    
    calculateAverages();
}

int MLFQ_Scheduler::getCurrentTime() const { return currentTime; }
vector<Process> MLFQ_Scheduler::getFinishedProcesses() const { return finishedProcesses; }

// ---- MAIN ----

int main() {
    cout << "=== SIMULADOR MLFQ (Multilevel Feedback Queue) ===" << endl;
    // Permitir al usuario seleccionar el esquema de colas
    int scheme = 2;
    cout << "Seleccione el esquema (1, 2 o 3):" << endl;
    cout << "  1) RR(1), RR(3), RR(4), SJF" << endl;
    cout << "  2) RR(2), RR(3), RR(4), STCF" << endl;
    cout << "  3) RR(3), RR(5), RR(6), RR(20)" << endl;
    cout << "Ingrese opcion [1-3] (Enter para 2 por defecto): ";
    string schemeInput;
    getline(cin, schemeInput);
    if (schemeInput.empty()) {
        scheme = 2;
    } else {
        // Si previamente se usó >> para leer, puede quedar un salto pendiente; intentar parsear robustamente
        stringstream ss(schemeInput);
        int opt = 2;
        if (ss >> opt) {
            if (opt >= 1 && opt <= 3) scheme = opt; else scheme = 2;
        }
    }
    switch (scheme) {
        case 1:
            cout << "Usando esquema 1: RR(1), RR(3), RR(4), SJF" << endl;
            break;
        case 2:
            cout << "Usando esquema 2: RR(2), RR(3), RR(4), STCF" << endl;
            break;
        case 3:
            cout << "Usando esquema 3: RR(3), RR(5), RR(6), RR(20)" << endl;
            break;
        default:
            cout << "Opcion invalida, usando esquema 2 por defecto: RR(2), RR(3), RR(4), STCF" << endl;
            scheme = 2;
            break;
    }
    
    string inputFile, outputFile;
    cout << "Ingrese el nombre del archivo de entrada: ";
    cin >> inputFile;
    cout << "Ingrese el nombre del archivo de salida: ";
    cin >> outputFile;
    // Limpiar el buffer por si quedan saltos de linea para futuros getline
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    
    // Crear el scheduler
    MLFQ_Scheduler scheduler(scheme);
    
    // Cargar procesos desde archivo
    if (!scheduler.loadProcessesFromFile(inputFile)) {
        cout << "Error cargando el archivo de entrada." << endl;
        return 1;
    }
    
    // Ejecutar simulación
    scheduler.simulate();
    
    // Mostrar resultados
    scheduler.printResults();
    
    // Guardar resultados
    scheduler.saveResultsToFile(outputFile);
    
    return 0;
}