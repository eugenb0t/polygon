#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <functional>
#include <math.h>
#include <iomanip> // Для std::setprecision
#include <queue>
#include <algorithm>
#include <limits>
#include <cmath>
#include <chrono>
#include <unordered_set>
#include <stack>


// Хэш-функция для пар
struct hash_pair {
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1,T2>& pair) const {
        auto hash1 = std::hash<T1>{}(pair.first);
        auto hash2 = std::hash<T2>{}(pair.second);
        return hash1 ^ hash2; // Комбинирование хэшей
    }
};

struct Node {
    double lon, lat;
    std::vector<std::pair<Node*, double>> nodes; // Связи с другими узлами
    
    Node(double longitude, double latitude) : lon(longitude), lat(latitude) {}

    // Перегрузка оператора сравнения на равенство
    bool operator==(const Node& other) const {
        return (lon == other.lon) && (lat == other.lat);
    }
};

class Graph {
public:

    // Функция добавления ребра в граф
    void addEdge(double lon1, double lat1, double lon2, double lat2, double weight) {
        Node* node1 = getNode(lon1, lat1); // Получаем первую ноду по координатам
        Node* node2 = getNode(lon2, lat2); // Получаем вторую ноду по координатам
        node1->nodes.push_back({node2, weight}); // Добавляем новую связь node1 с node2
    }
    
    // Функция получения (добавления) ноды в граф
    Node* getNode(double lon, double lat) {
        std::pair<double, double> key = {lon, lat};
        if (nodeMap.find(key) == nodeMap.end()) { // Если не нашли ноду по координатам
            nodeMap[key] = new Node(lon, lat); // Добавляем её
        }
        return nodeMap[key]; // Возвращаем
    }

    Node* find_closest_node(double lat, double lon) {
        double min_distance = 99999;
        Node* node_founded = nullptr;

        for (auto pair : nodeMap) {
            Node* node = pair.second;
            double distance = std::sqrt(std::pow(node->lat - lat, 2) + std::pow(node->lon - lon, 2));
            if (distance < min_distance) {
                node_founded = node;
                min_distance = distance;
            }
        }

        return node_founded;
    }

    void printGraph(int n = 10) const {
        int counter = 0;
        for (const auto& pair : nodeMap) {
            const Node* node = pair.second;
            std::cout << std::fixed << std::setprecision(7);
            std::cout << "Node (" << node->lat << ", " << node->lon << ") connections:\n";
            for (const auto& connection : node->nodes) {
                std::cout << "  -> (" << connection.first->lat << ", " << connection.first->lon << ") with weight " << connection.second << "\n";
            }
            counter ++;
            if (counter >= n)
                return;
        }
    }

    ~Graph() {
        for (auto& pair : nodeMap) {
            delete pair.second; // Освобождаем память
        }
    }

private:
    std::unordered_map<std::pair<double, double>, Node*, hash_pair> nodeMap;
};

bool readGraph(const std::string& filename, Graph& graph) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error while opening file!" << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string start, end;

        // Читаем начальную точку
        std::getline(ss, start, ':');
        double lat1, lon1;
        sscanf(start.c_str(), "%lf,%lf", &lat1, &lon1);

        // Читаем остальные точки
        while (std::getline(ss, end, ';')) {
            std::stringstream ss_end(end);
            double lat2, lon2, weight;

            sscanf(end.c_str(), "%lf,%lf,%lf", &lat2, &lon2, &weight);

            // Добавляем рёбра в граф
            graph.addEdge(lon1, lat1, lon2, lat2, weight);
        }
    }

    file.close();
    return true;
}

std::pair<std::vector<Node*>, double> bfs(Graph& graph, Node* start, Node* target) {
    std::unordered_set<Node*> visited; // Хранит посещённые узлы
    std::queue<std::tuple<Node*, std::vector<Node*>, double>> queue; // Очередь для BFS
    std::vector<Node*> bestPath;
    double bestWeight = 0.0; // Хранит лучший вес

    queue.push({start, {start}, 0.0}); // Добавляем начальный узел в очередь с нулевым весом

    while (!queue.empty()) {
        auto [current, currentPath, currentWeight] = queue.front();
        queue.pop();

        // Если достигли целевой узел
        if (current == target) {
            bestPath = currentPath; // Сохраняем текущий путь
            bestWeight = currentWeight; // Сохраняем текущий вес
            return {bestPath, bestWeight}; // Возвращаем текущий путь и вес
        }

        // Добавляем текущий узел в посещённые
        if (visited.find(current) == visited.end()) {
            visited.insert(current);

            // Идем по соседям
            for (const auto& connection : current->nodes) {
                Node* neighbor = connection.first;
                double weight = connection.second;

                if (visited.find(neighbor) == visited.end()) {
                    std::vector<Node*> newPath = currentPath; // Создаём новый путь
                    newPath.push_back(neighbor); // Добавляем соседний узел в путь

                    // новый вес пути до соседа
                    double newWeight = currentWeight + weight; 
                    queue.push({neighbor, newPath, newWeight}); // Добавляем соседний узел в очередь
                }
            }
        }
    }

    return {{}, 0.0}; // Если не нашли путь, возвращаем пустой вектор и вес 0
}

std::pair<std::vector<Node*>, double> dfs(Graph& graph, Node* start, Node* target) {
    std::unordered_set<Node*> visited; // Хранит посещённые узлы
    std::stack<std::tuple<Node*, std::vector<Node*>, double>> stack; // Стек для DFS
    std::vector<Node*> bestPath;
    double bestWeight = 0.0; // Хранит общий вес найденного пути

    stack.push({start, {start}, 0.0}); // Добавляем начальный узел в стек с весом 0

    while (!stack.empty()) {
        auto [current, currentPath, currentWeight] = stack.top(); // Берем верхний элемент
        stack.pop();

        // Если достигли целевой узел
        if (current == target) {
            bestPath = currentPath; // Сохраняем текущий путь
            bestWeight = currentWeight; // Сохраняем текущий вес
            return {bestPath, bestWeight}; // Возвращаем путь и вес
        }

        // Добавляем текущий узел в посещённые
        if (visited.find(current) == visited.end()) {
            visited.insert(current);

            // Идем по соседям
            for (const auto& connection : current->nodes) {
                Node* neighbor = connection.first;
                double weight = connection.second;

                if (visited.find(neighbor) == visited.end()) {
                    std::vector<Node*> newPath = currentPath; // Создаём новый путь
                    newPath.push_back(neighbor); // Добавляем соседний узел в путь

                    double newWeight = currentWeight + weight; // Обновляем вес
                    stack.push({neighbor, newPath, newWeight}); // Добавляем соседний узел в стек
                }
            }
        }
    }

    return {{}, 0.0}; // Если не нашли путь, возвращаем пустой вектор и вес 0
}

std::pair<std::vector<Node*>, double> dijkstra(Graph& graph, Node* start, Node* target) {
    std::unordered_set<Node*> visited; // Хранит посещённые узлы
    std::priority_queue<std::tuple<double, Node*, std::vector<Node*>>, 
                        std::vector<std::tuple<double, Node*, std::vector<Node*>>>, 
                        std::greater<std::tuple<double, Node*, std::vector<Node*>>>> queue; // Очередь для минимального расстояния

    // Начальное расстояние до начального узла равно 0
    queue.push({0.0, start, {start}}); 

    while (!queue.empty()) {
        auto [currentDistance, current, currentPath] = queue.top(); // Получаем узел с минимальным расстоянием
        queue.pop();

        // Если достигли целевой узел
        if (current == target) {
            return {currentPath, currentDistance}; // Возвращаем текущий путь и вес, если достигли цели
        }

        // Добавляем текущий узел в посещённые
        if (visited.find(current) == visited.end()) {
            visited.insert(current);

            // Идем по соседям
            for (const auto& connection : current->nodes) {
                Node* neighbor = connection.first;
                double weight = connection.second;

                // Общий вес пути до соседа
                double newDistance = currentDistance + weight;

                // Если сосед еще не посещён, добавляем его в очередь
                if (visited.find(neighbor) == visited.end()) {
                    std::vector<Node*> newPath = currentPath; // Создаём новый путь
                    newPath.push_back(neighbor); // Добавляем соседний узел в путь
                    queue.push({newDistance, neighbor, newPath}); // Добавляем соседа в очередь с новым расстоянием
                }
            }
        }
    }

    return {{}, 0.0}; // Если не нашли путь, возвращаем пустой вектор и вес 0
}

// Функция для вычисления эвристики (евклидово расстояние)
double heuristic(Node* a, Node* b) {
    return std::sqrt(std::pow(a->lat - b->lat, 2) + std::pow(a->lon - b->lon, 2));
}

std::pair<std::vector<Node*>, double> aStar(Graph& graph, Node* start, Node* target) {
    std::unordered_set<Node*> visited; // Хранит посещённые узлы
    std::priority_queue<std::tuple<double, Node*, std::vector<Node*>>, 
                        std::vector<std::tuple<double, Node*, std::vector<Node*>>>, 
                        std::greater<std::tuple<double, Node*, std::vector<Node*>>>> queue; // Очередь для A*

    // Начальное расстояние до начального узла равно 0
    queue.push({0.0 + heuristic(start, target), start, {start}}); 

    while (!queue.empty()) {
        auto [currentF, current, currentPath] = queue.top(); // Получаем узел с минимальным f(n)
        queue.pop();

        // Если достигли целевой узел
        if (current == target) {
            double totalWeight = 0.0; // Подсчёт суммарного веса пути
            for (size_t i = 0; i < currentPath.size() - 1; ++i) {
                Node* nodeA = currentPath[i];
                Node* nodeB = currentPath[i + 1];

                // Находим вес ребра между nodeA и nodeB
                for (const auto& connection : nodeA->nodes) {
                    if (connection.first == nodeB) { // Сравниваем указатели
                        totalWeight += connection.second; // Добавляем вес
                        break; // Выход из цикла, когда нашли нужную пару
                    }
                }
            }
            return {currentPath, totalWeight}; // Возвращаем текущий путь и его вес
        }

        // Добавляем текущий узел в посещённые
        if (visited.find(current) == visited.end()) {
            visited.insert(current);

            // Идем по соседям
            for (const auto& connection : current->nodes) {
                Node* neighbor = connection.first;
                double weight = connection.second;

                // Общий вес пути до соседа
                double newGScore = currentPath.size() > 1 ? currentPath.size() - 1 : 0; // Обновляем g(n)
                newGScore += weight; // Добавляем вес текущего ребра
                
                // Если сосед еще не посещён, добавляем его в очередь
                if (visited.find(neighbor) == visited.end()) {
                    std::vector<Node*> newPath = currentPath; // Создаём новый путь
                    newPath.push_back(neighbor); // Добавляем соседний узел в путь

                    double fScore = newGScore + heuristic(neighbor, target); // Общая оценка f(n)
                    queue.push({fScore, neighbor, newPath}); // Добавляем соседа в очередь с новым расстоянием
                }
            }
        }
    }

    return {{}, 0.0}; // Если не нашли путь, возвращаем пустой вектор и вес 0
}


int main() {
    Graph graph;

    // Чтение графа из файла
    if (!readGraph("./spb_graph.txt", graph)) {
        return 1;
    }

    // Вывод графа
    // std::cout << "Graph read" << std::endl;
    // graph.printGraph();
    
    double home_lat = 30.314299;
    double home_lon = 59.928361;

    double itmo_lat = 30.307942;
    double itmo_lon = 59.957454;

    Node* home_closest_node = graph.find_closest_node(home_lat, home_lon);
    Node* itmo_closest_node = graph.find_closest_node(itmo_lat, itmo_lon);

    auto start = std::chrono::high_resolution_clock::now(); // Фиксируем время старта
    auto [shortest_path_bfs, bfs_summary_weight] = bfs(graph, home_closest_node, itmo_closest_node);
    auto end = std::chrono::high_resolution_clock::now(); // Фиксируем время окончания

    std::chrono::duration<double> duration = end - start;
    std::cout << "BFS time: " << duration.count() << std::endl;

    start = std::chrono::high_resolution_clock::now(); // Фиксируем время старта
    auto [shortest_path_dfs, dfs_summary_weight] = dfs(graph, home_closest_node, itmo_closest_node);
    end = std::chrono::high_resolution_clock::now(); // Фиксируем время окончания

    duration = end - start;
    std::cout << "DFS time: " << duration.count() << std::endl;

    start = std::chrono::high_resolution_clock::now(); // Фиксируем время старта
    auto [shortest_path_dijkstra, dijkstra_summary_weight] = dijkstra(graph, home_closest_node, itmo_closest_node);
    end = std::chrono::high_resolution_clock::now(); // Фиксируем время окончания

    duration = end - start;
    std::cout << "Dijkstra time: " << duration.count() << std::endl;

    start = std::chrono::high_resolution_clock::now(); // Фиксируем время старта
    auto [shortest_path_astar, astar_summary_weight] = aStar(graph, home_closest_node, itmo_closest_node);
    end = std::chrono::high_resolution_clock::now(); // Фиксируем время окончания

    duration = end - start;
    std::cout << "A* time: " << duration.count() << std::endl << std::endl;

    std::cout << "bfs total weight: " << bfs_summary_weight << std::endl;
    std::cout << "dfs total weight: " << dfs_summary_weight << std::endl;
    std::cout << "dijkstra total weight: " << dijkstra_summary_weight << std::endl;
    std::cout << "a* total weight: " << astar_summary_weight << std::endl;

    // std::cout << "BFS path: " << std::endl;
    // for (const auto& node : shortest_path_bfs) {
    //     std::cout << "(" << node->lat << ", " << node->lon << ") ";
    // }

    // std::cout << std::endl;

    //  std::cout << "DFS path: " << std::endl;
    // for (const auto& node : shortest_path_dfs) {
    //     std::cout << "(" << node->lat << ", " << node->lon << ") ";
    // }

    // std::cout << std::endl;

    //  std::cout << "Dijkstra path: " << std::endl;
    // for (const auto& node : shortest_path_dijkstra) {
    //     std::cout << "(" << node->lat << ", " << node->lon << ") ";
    // }

    // std::cout << std::endl;

    // std::cout << "A* path: " << std::endl;
    // for (const auto& node : shortest_path_astar) {
    //     std::cout << "(" << node->lat << ", " << node->lon << ") ";
    // }

    std::cout << std::endl;

    return 0;
}
