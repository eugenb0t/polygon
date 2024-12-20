#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <functional> // Не забудьте добавить этот заголовочный файл!
#include <math.h>
#include <iomanip> // Для std::setprecision
#include <queue>
#include <algorithm>
#include <limits>
#include <cmath>

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
};

class Graph {
public:
    void addEdge(double lon1, double lat1, double lon2, double lat2, double weight) {
        Node* node1 = getNode(lon1, lat1);
        Node* node2 = getNode(lon2, lat2);
        node1->nodes.push_back({node2, weight});
    }
    
    Node* getNode(double lon, double lat) {
        std::pair<double, double> key = {lon, lat};
        if (nodeMap.find(key) == nodeMap.end()) {
            nodeMap[key] = new Node(lon, lat);
        }
        return nodeMap[key];
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
        // std::cout << start << std::endl;

        // Читаем остальные точки
        while (std::getline(ss, end, ';')) {
            // std::cout << end << std::endl;
            std::stringstream ss_end(end);
            double lat2, lon2, weight;
            std::string coords;

            // std::getline(ss_end, coords, ',');
            // std::getline(ss_end, coords, ',');
            // lat2 = std::stod(coords);
            // std::getline(ss_end, coords);
            // weight = std::stod(coords);
            sscanf(end.c_str(), "%lf,%lf,%lf", &lat2, &lon2, &weight);

            // Добавляем рёбра в граф
            graph.addEdge(lon1, lat1, lon2, lat2, weight);
        }
    }

    file.close();
    return true;
}

// Функция для поиска кратчайшего пути
double dfs(Node* current, Node* target, double currentWeight, double& minWeight,
           std::vector<Node*>& path, std::vector<Node*>& bestPath, 
           std::unordered_map<double, bool>& visited) {
    if (current == target) {
        if (currentWeight < minWeight) {
            minWeight = currentWeight;
            bestPath = path; // Обновляем лучший путь
        }
        return currentWeight;
    }

    visited[current->lat] = true; // Обозначаем текущий узел как посещённый
    for (const auto& connection : current->nodes) {
        Node* neighbor = connection.first;
        double weight = connection.second;

        if (!visited[neighbor->lat]) { // Если соседний узел не посещён
            path.push_back(neighbor); // Добавляем в текущий путь
            dfs(neighbor, target, currentWeight + weight, minWeight, path, bestPath, visited); // Рекурсивно ищем
            path.pop_back(); // Убираем узел из текущего пути
        }
    }
    visited[current->lat] = false; // Возвращаем состояние посещённости
    return minWeight;
}

// Функция для инициации поиска кратчайшего пути
std::vector<Node*> findShortestPathDFS(Graph& graph, Node* start_node, Node* target_node) {
    double minWeight = std::numeric_limits<double>::max();
    std::vector<Node*> path, bestPath;

    std::unordered_map<double, bool> visited; // Используем map для отметки посещённости (по lat)
    path.push_back(start_node);
    dfs(start_node, target_node, 0.0, minWeight, path, bestPath, visited);
    
    return bestPath; // Возвращаем лучший найденный путь
}

std::vector<Node*> findShortestPathBFS(Graph& graph, Node* start_node, Node* target_node) {
    std::unordered_map<Node*, Node*> parentMap; // Для отслеживания родительских узлов
    std::unordered_map<Node*, double> weightMap; // Для отслеживания весов
    std::queue<Node*> queue;
    std::unordered_set<Node*> visited;

    queue.push(start_node);
    visited.insert(start_node);
    weightMap[start_node] = 0.0; // Исходный узел имеет 0 вес

    while (!queue.empty()) {
        Node* current = queue.front();
        queue.pop();

        // Если достигли целевого узла, выстраиваем путь
        if (current == target_node) {
            std::vector<Node*> path;
            for (Node* node = target_node; node != nullptr; node = parentMap[node]) {
                path.push_back(node);
            }
            std::reverse(path.begin(), path.end()); // Обратим порядок, чтобы получить правильный путь
            return path;
        }

        // Проходим по всем соседям узла
        for (const auto& connection : current->nodes) {
            Node* neighbor = connection.first;
            double weight = connection.second;

            // Проверяем, был ли сосед уже посещён
            if (visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                queue.push(neighbor);
                parentMap[neighbor] = current; // Запоминаем родителя
                weightMap[neighbor] = weightMap[current] + weight; // Считаем общий вес
            }
        }
    }

    return {}; // Если путь не найден, возвращаем пустой вектор
}

// Структура для сравнения в очереди
struct NodeComparator {
    bool operator()(const std::pair<Node*, double>& left, const std::pair<Node*, double>& right) {
        return left.second > right.second; // Минимум по весу
    }
};

std::vector<Node*> findShortestPathDijkstra(Graph& graph, Node* start_node, Node* target_node) {
    std::unordered_map<Node*, Node*> parentMap; // Для отслеживания родительских узлов
    std::unordered_map<Node*, double> distanceMap; // Для отслеживания минимального расстояния
    std::priority_queue<std::pair<Node*, double>, std::vector<std::pair<Node*, double>>, NodeComparator> queue;
    std::unordered_set<Node*> visited;

    // Инициализация
    for (Node* node : graph.allNodes) {
        distanceMap[node] = std::numeric_limits<double>::max(); // Устанавливаем начальные расстояния как бесконечность
    }
    distanceMap[start_node] = 0.0; // Расстояние до стартового узла равно 0
    queue.push({start_node, 0.0}); // Добавляем стартовый узел в очередь

    while (!queue.empty()) {
        auto [current, currentDistance] = queue.top(); // Извлекаем узел с минимальным для него весом
        queue.pop();

        if (visited.find(current) != visited.end()) {
            continue; // Пропускаем, если уже посещён
        }
        visited.insert(current); // Помечаем текущий узел как посещённый

        // Если достигли целевого узла, выстраиваем путь
        if (current == target_node) {
            std::vector<Node*> path;
            for (Node* node = target_node; node != nullptr; node = parentMap[node]) {
                path.push_back(node);
            }
            std::reverse(path.begin(), path.end()); // Обратим порядок, чтобы получить правильный путь
            return path;
        }

        // Проходим по всем соседям узла
        for (const auto& connection : current->nodes) {
            Node* neighbor = connection.first;
            double weight = connection.second;

            double newDistance = currentDistance + weight; // Считаем новый вес до соседа

            // Если новый найденный вес меньше, чем известный, обновляем
            if (newDistance < distanceMap[neighbor]) {
                distanceMap[neighbor] = newDistance; // Обновляем расстояние
                parentMap[neighbor] = current; // Запоминаем родителя
                queue.push({neighbor, newDistance}); // Добавляем соседа в очередь
            }
        }
    }

    return {}; // Если путь не найден, возвращаем пустой вектор
}

// Евклидово расстояние как эвристическая функция
double heuristic(Node* a, Node* b) {
    return std::sqrt(std::pow(a->lat - b->lat, 2) + std::pow(a->lon - b->lon, 2));
}

std::vector<Node*> findShortestPathAStar(Graph& graph, Node* start_node, Node* target_node) {
    std::unordered_map<Node*, Node*> parentMap; // Для отслеживания родительских узлов
    std::unordered_map<Node*, double> gScore; // Минимальная стоимость от старта до узла
    std::unordered_map<Node*, double> fScore; // Суммарная стоимость от старта до цели
    std::priority_queue<std::pair<Node*, double>, std::vector<std::pair<Node*, double>>, NodeComparator> queue;
    std::unordered_set<Node*> visited;

    // Инициализация
    for (Node* node : graph.allNodes) {
        gScore[node] = std::numeric_limits<double>::max(); // Устанавливаем начальные стоимости как бесконечность
        fScore[node] = std::numeric_limits<double>::max(); // Устанавливаем начальные стоимости как бесконечность
    }
    gScore[start_node] = 0.0; // Стоимость до стартового узла равна 0
    fScore[start_node] = heuristic(start_node, target_node); // Эвристика для стартового узла
    queue.push({start_node, fScore[start_node]}); // Добавляем стартовый узел в очередь

    while (!queue.empty()) {
        auto [current, fCurrent] = queue.top(); // Узел с наименьшим f(n)
        queue.pop();

        // Если достигли целевого узла, выстраиваем путь
        if (current == target_node) {
            std::vector<Node*> path;
            for (Node* node = target_node; node != nullptr; node = parentMap[node]) {
                path.push_back(node);
            }
            std::reverse(path.begin(), path.end()); // Обратим порядок, чтобы получить правильный путь
            return path;
        }

        visited.insert(current); // Помечаем текущий узел как посещённый

        // Проходим по всем соседям узла
        for (const auto& connection : current->nodes) {
            Node* neighbor = connection.first;
            double weight = connection.second;

            double tentative_gScore = gScore[current] + weight; // Считаем новый g(n)

            // Проверяем, если это лучший путь
            if (tentative_gScore < gScore[neighbor]) {
                parentMap[neighbor] = current; // Запоминаем родителя
                gScore[neighbor] = tentative_gScore; // Обновляем g(n)
                fScore[neighbor] = gScore[neighbor] + heuristic(neighbor, target_node); // Обновляем f(n)

                // Если сосед еще не в очереди, добавим его
                if (visited.find(neighbor) == visited.end()) {
                    queue.push({neighbor, fScore[neighbor]});
                }
            }
        }
    }

    return {}; // Если путь не найден, возвращаем пустой вектор
}

int main() {
    Graph graph;

    // Чтение графа из файла
    if (!readGraph("./spb_graph.txt", graph)) {
        return 1;
    }

    // Вывод графа
    //graph.printGraph();
    double home_lat = 30.314299;
    double home_lon = 59.928361;

    double itmo_lat = 30.307942;
    double itmo_lon = 59.957454;

    Node* home_closest_node = graph.find_closest_node(home_lat, home_lon);
    Node* itmo_closest_node = graph.find_closest_node(itmo_lat, itmo_lon);
    // std::cout << "Node (" << closest_node->lat << ", " << closest_node->lon << ") connections:\n";
    //         for (const auto& connection : closest_node->nodes) {
    //             std::cout << "  -> (" << connection.first->lat << ", " << connection.first->lon << ") with weight " << connection.second << "\n";
    //         }
    std::vector<Node*> shortest_path_dfs = findShortestPathDFS(graph, home_closest_node, itmo_closest_node);
    std::vector<Node*> shortest_path_bfs = findShortestPathBFS(graph, home_closest_node, itmo_closest_node);
    std::vector<Node*> shortest_path_dijkstra = findShortestPathDijkstra(graph, home_closest_node, itmo_closest_node);
    std::vector<Node*> shortest_path_astar = findShortestPathAStar(graph, home_closest_node, itmo_closest_node);
    

    for (const auto& node : shortest_path_dfs) {
        std::cout << "(" << node->lat << ", " << node->lon << ") ";
    }
    std::cout << std::endl;

    return 0;
}
