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

    void printGraph() const {
        for (const auto& pair : nodeMap) {
            const Node* node = pair.second;
            std::cout << "Node (" << node->lat << ", " << node->lon << ") connections:\n";
            for (const auto& connection : node->nodes) {
                std::cout << "  -> (" << connection.first->lat << ", " << connection.first->lon << ") with weight " << connection.second << "\n";
            }
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
            std::string coords;

            std::getline(ss_end, coords, ',');
            std::getline(ss_end, coords, ',');
            lat2 = std::stod(coords);
            std::getline(ss_end, coords);
            weight = std::stod(coords);

            // Добавляем рёбра в граф
            graph.addEdge(lon1, lat1, lon2, lat2, weight);
        }
    }

    file.close();
    return true;
}

int main() {
    Graph graph;

    // Чтение графа из файла
    if (!readGraph("./spb_graph.txt", graph)) {
        return 1;
    }

    // Вывод графа
    // graph.printGraph();
    double home_lat = 30.314299;
    double home_lon = 59.928361;

    double itmo_lat = 30.307942;
    double itmo_lon = 59.957454;

    Node* closest_node = graph.find_closest_node(home_lat, home_lon);
    // std::cout << "Node (" << closest_node->lat << ", " << closest_node->lon << ") connections:\n";
    //         for (const auto& connection : closest_node->nodes) {
    //             std::cout << "  -> (" << connection.first->lat << ", " << connection.first->lon << ") with weight " << connection.second << "\n";
    //         }

    return 0;
}
