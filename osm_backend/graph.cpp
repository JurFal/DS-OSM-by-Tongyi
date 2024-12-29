
#include "graph.hpp"

using namespace std;

// 计算两个地理坐标之间的距离（简化版）
double calculateDistance(const Node& from, const Node& to) {
    // 使用Haversine公式计算地球表面上两点之间的距离
    // 这里仅提供了一个简化的例子，实际应用中应该更精确地实现
    double R = 6371e3; // 地球半径，单位：米
    double phi1 = from.lat * M_PI / 180;
    double phi2 = to.lat * M_PI / 180;
    double delta_phi = (to.lat - from.lat) * M_PI / 180;
    double delta_lambda = (to.lon - from.lon) * M_PI / 180;

    double a = sin(delta_phi/2) * sin(delta_phi/2) +
               cos(phi1) * cos(phi2) *
               sin(delta_lambda/2) * sin(delta_lambda/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));

    return R * c; // 返回距离，单位：米
}

double calculateManhattanDistance(const Node& from, const Node& to) {
    // 地球半径，单位：米
    double R = 6371e3;

    // 将经纬度从度数转换为弧度
    double lat1 = from.lat * M_PI / 180;
    double lon1 = from.lon * M_PI / 180;
    double lat2 = to.lat * M_PI / 180;
    double lon2 = to.lon * M_PI / 180;

    // 计算纬度和经度的差异（以弧度为单位）
    double delta_lat = fabs(lat2 - lat1);
    double delta_lon = fabs(lon2 - lon1);

    // 使用平均纬度来估算每个经度度数对应的地面距离
    double avg_lat = (lat1 + lat2) / 2;

    // 计算南北方向和东西方向的距离
    double north_south_distance = delta_lat * R;
    double east_west_distance = delta_lon * R * 0.8544588301328;

    // 曼哈顿距离为两个方向距离之和
    return min(north_south_distance, east_west_distance); // 返回距离，单位：米
}

double calculateDistanceWithLatAndLon(double lat1, double lon1, double lat2, double lon2) {
    // 使用Haversine公式计算地球表面上两点之间的距离
    // 这里仅提供了一个简化的例子，实际应用中应该更精确地实现
    double R = 6371e3; // 地球半径，单位：米
    double phi1 = lat1 * M_PI / 180;
    double phi2 = lat2 * M_PI / 180;
    double delta_phi = (lat2 - lat1) * M_PI / 180;
    double delta_lambda = (lon2 - lon1) * M_PI / 180;

    double a = sin(delta_phi/2) * sin(delta_phi/2) +
               cos(phi1) * cos(phi2) *
               sin(delta_lambda/2) * sin(delta_lambda/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));

    return R * c; // 返回距离，单位：米
}


long long findNearestNode(double targetLat, double targetLon) {
    
    return kdtree.findNearestNode(targetLat, targetLon);
}

void Graph::addEdge(VertexId from, VertexId to, double weight) {
        adjacency_list[from].emplace_back(to, weight);
        adjacency_list[to].emplace_back(from, weight); // 如果是无向图
}

    // Dijkstra算法用于查找最短路径
vector<long long> Graph::dijkstra(VertexId start, VertexId end) {
    unordered_map<VertexId, double> distances;
    unordered_map<VertexId, VertexId> previous;
    priority_queue<pair<double, VertexId>, vector<pair<double, VertexId>>, greater<>> pq;

    for (auto& pair : adjacency_list) {
        distances[pair.first] = numeric_limits<double>::max();
    }
    distances[start] = 0;
    pq.push({0, start});

    while (!pq.empty()) {
        auto [current_dist, current_node] = pq.top();
        pq.pop();
        //cout << current_dist << ":" << current_node << endl;
        if (current_node == end) break;

        if (current_dist > distances[current_node]) continue;

        for (const auto& edge : adjacency_list[current_node]) {
            //cout << edge.target << endl;
            double distance_through_current = current_dist + edge.weight;
            if (distance_through_current < distances[edge.target]) {
                distances[edge.target] = distance_through_current;
                previous[edge.target] = current_node;
                pq.push({distance_through_current, edge.target});
            }
        }
    }

        // 构建最短路径
    if (distances[end] == numeric_limits<double>::max()) return {}; // 没有路径

    vector<VertexId> path;
    for (VertexId at = end; at != start; at = previous[at]) {
        path.push_back(at);
    }
    path.push_back(start);
    reverse(path.begin(), path.end());

    return path;
}

vector<long long> Graph::a_star(VertexId start, VertexId end) {
    unordered_map<VertexId, double> g_costs; // 从起点到当前节点的实际成本
    unordered_map<VertexId, double> f_costs; // 实际成本加估计成本
    unordered_map<VertexId, VertexId> previous;
    priority_queue<pair<double, VertexId>, vector<pair<double, VertexId>>, greater<>> pq;

    auto heuristic = [](VertexId a, VertexId b) -> double {
        // 这里需要实现一个启发式函数，例如曼哈顿距离或欧几里得距离。
        // 具体实现取决于顶点表示的位置信息。
        // 返回从a到b的估计成本。
        return calculateManhattanDistance(nodes[a], nodes[b]); // 默认返回0作为占位符。
    };

    for (auto& pair : adjacency_list) {
        g_costs[pair.first] = numeric_limits<double>::max();
        f_costs[pair.first] = numeric_limits<double>::max();
    }
    g_costs[start] = 0;
    f_costs[start] = heuristic(start, end);
    pq.push({f_costs[start], start});

    while (!pq.empty()) {
        auto [current_f_cost, current_node] = pq.top();
        pq.pop();

        if (current_node == end) break;

        if (current_f_cost > f_costs[current_node]) continue;

        for (const auto& edge : adjacency_list[current_node]) {
            double tentative_g_cost = g_costs[current_node] + edge.weight;
            if (tentative_g_cost < g_costs[edge.target]) {
                // 找到了更短的路径到edge.target
                previous[edge.target] = current_node;
                g_costs[edge.target] = tentative_g_cost;
                f_costs[edge.target] = g_costs[edge.target] + heuristic(edge.target, end);
                pq.push({f_costs[edge.target], edge.target});
            }
        }
    }

    // 构建最短路径
    if (f_costs[end] == numeric_limits<double>::max()) return {}; // 没有路径

    vector<VertexId> path;
    for (VertexId at = end; at != start; at = previous[at]) {
        path.push_back(at);
    }
    path.push_back(start);
    reverse(path.begin(), path.end());

    return path;
}

std::vector<long long> Graph::bidirectional_a_star(VertexId start, VertexId end) {
    using SearchState = std::pair<double, VertexId>;
    auto heuristic = [](VertexId a, VertexId b) -> double {
        // 这里需要实现一个启发式函数，例如曼哈顿距离或欧几里得距离。
        // 具体实现取决于顶点表示的位置信息。
        // 返回从a到b的估计成本。
        return calculateManhattanDistance(nodes[a], nodes[b]); // 默认返回0作为占位符。
    };
    // 正向搜索结构
    std::unordered_map<VertexId, double> forward_g_costs;
    std::unordered_map<VertexId, double> forward_f_costs;
    std::unordered_map<VertexId, VertexId> forward_previous;
    std::priority_queue<SearchState, std::vector<SearchState>, std::greater<>> forward_pq;

    // 反向搜索结构
    std::unordered_map<VertexId, double> backward_g_costs;
    std::unordered_map<VertexId, double> backward_f_costs;
    std::unordered_map<VertexId, VertexId> backward_previous;
    std::priority_queue<SearchState, std::vector<SearchState>, std::greater<>> backward_pq;

    // 初始化正向搜索
    forward_g_costs[start] = 0.0;
    forward_f_costs[start] = heuristic(start, end);
    forward_pq.push({forward_f_costs[start], start});

    // 初始化反向搜索
    backward_g_costs[end] = 0.0;
    backward_f_costs[end] = heuristic(end, start);
    backward_pq.push({backward_f_costs[end], end});

    while (!forward_pq.empty() && !backward_pq.empty()) {
        if (search_meets(forward_previous, backward_previous, forward_f_costs, backward_f_costs)) break;

        // 执行一次正向搜索步骤
        auto [current_f_cost, current_node] = forward_pq.top();
        forward_pq.pop();

        for (const auto& edge : adjacency_list[current_node]) {
            double tentative_g_cost = forward_g_costs[current_node] + edge.weight;
            if (tentative_g_cost < forward_g_costs[edge.target]) {
                forward_previous[edge.target] = current_node;
                forward_g_costs[edge.target] = tentative_g_cost;
                forward_f_costs[edge.target] = tentative_g_cost + heuristic(edge.target, end);
                forward_pq.push({forward_f_costs[edge.target], edge.target});
            }
        }

        // 执行一次反向搜索步骤
        auto [back_current_f_cost, back_current_node] = backward_pq.top();
        backward_pq.pop();

        for (const auto& edge : adjacency_list[back_current_node]) {
            double tentative_g_cost = backward_g_costs[back_current_node] + edge.weight;
            if (tentative_g_cost < backward_g_costs[edge.target]) {
                backward_previous[edge.target] = back_current_node;
                backward_g_costs[edge.target] = tentative_g_cost;
                backward_f_costs[edge.target] = tentative_g_cost + heuristic(edge.target, start);
                backward_pq.push({backward_f_costs[edge.target], edge.target});
            }
        }
    }

    // 构建最短路径
    return reconstruct_path(forward_previous, backward_previous, start, end);
}

bool Graph::search_meets(std::unordered_map<VertexId, VertexId>& forward_prev,
                         std::unordered_map<VertexId, VertexId>& backward_prev,
                         std::unordered_map<VertexId, double>& forward_f_costs,
                         std::unordered_map<VertexId, double>& backward_f_costs) {
    for (auto& [node, _] : forward_prev) {
        if (backward_prev.find(node) != backward_prev.end()) {
            return true;
        }
    }
    return false;
}

std::vector<long long> Graph::reconstruct_path(
    std::unordered_map<VertexId, VertexId>& forward_previous,
    std::unordered_map<VertexId, VertexId>& backward_previous,
    VertexId start, VertexId end) {

    // 查找交汇点
    VertexId meet_point;
    for (auto& [node, _] : forward_previous) {
        if (backward_previous.find(node) != backward_previous.end()) {
            meet_point = node;
            break;
        }
    }

    if (forward_previous.find(meet_point) == forward_previous.end() ||
        backward_previous.find(meet_point) == backward_previous.end()) {
        return {}; // 没有找到路径
    }

    // 构建正向路径
    std::vector<VertexId> forward_path;
    for (VertexId at = meet_point; at != start; at = forward_previous[at]) {
        forward_path.push_back(at);
    }
    forward_path.push_back(start);
    std::reverse(forward_path.begin(), forward_path.end());

    // 构建反向路径
    std::vector<VertexId> backward_path;
    for (VertexId at = meet_point; at != end; at = backward_previous[at]) {
        backward_path.push_back(at);
    }
    backward_path.push_back(end);

    // 合并路径
    forward_path.insert(forward_path.end(), backward_path.begin(), backward_path.end());
    return forward_path;
}