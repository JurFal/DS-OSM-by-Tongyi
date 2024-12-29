#include <iostream>
#include <string>
#include <memory>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <queue>
#include <limits>
#include "pugixml.hpp"

#define M_PI		3.14159265358979323846

using namespace std;

struct Node {
    long long id;
    double lat, lon;
};

struct Edge {
    long long target;   // 目标节点ID
    double weight; // 边的权重（例如距离）

    Edge(long long t, double w) : target(t), weight(w) {}
};

struct Way {
    long long id;
    bool oneway;
    double speedLimit;
    std::string highwayType;
    std::string name;
    std::vector<long long> node_ids; // 节点ID列表

};

class Graph {
public:
    using VertexId = long long;
    using EdgeList = vector<Edge>;
    using AdjacencyList = unordered_map<VertexId, EdgeList>;

    void addEdge(VertexId from, VertexId to, double weight);

    // Dijkstra算法用于查找最短路径
    vector<VertexId> dijkstra(VertexId start, VertexId end);
    vector<VertexId> a_star(VertexId start, VertexId end);
    std::vector<VertexId> bidirectional_a_star(VertexId start, VertexId end);
    std::vector<VertexId> reconstruct_path(
    std::unordered_map<VertexId, VertexId>& forward_previous,
    std::unordered_map<VertexId, VertexId>& backward_previous,
    VertexId start, VertexId end);
private:
    AdjacencyList adjacency_list;
    bool search_meets(std::unordered_map<VertexId, VertexId>& forward_prev,
                      std::unordered_map<VertexId, VertexId>& backward_prev,
                      std::unordered_map<VertexId, double>& forward_f_costs,
                      std::unordered_map<VertexId, double>& backward_f_costs);
};



// 计算两个地理坐标之间的距离（简化版）
double calculateDistance(const Node& from, const Node& to);
double calculateDistanceWithLatAndLon(double lat1, double lat2, double lon1, double lon2);
long long findNearestNode(double targetLat, double targetLon);

class KDTree {
public:
    struct Point {
        double lat, lon;
        long long id;

        // 比较函数用于排序，这里只提供了一个示例，根据实际情况修改
        bool operator<(const Point& other) const {
            return std::tie(lat, lon, id) < std::tie(other.lat, other.lon, other.id);
        }
    };

    KDTree() : root_(nullptr), depth_(0) {}

    void insert(const Point& point) {
        root_ = insertRec(std::move(root_), point, 0);
    }
    long long findNearestNode(double targetLat, double targetLon) const {
        Point bestPoint;
        double minDistSquared = std::numeric_limits<double>::max();
        nearestNeighborSearch(root_.get(), targetLat, targetLon, 0, minDistSquared, bestPoint);

        return bestPoint.id;
    }

private:
    struct Node {
        Point point;
        std::unique_ptr<Node> left, right;
    };

    std::unique_ptr<Node> root_;
    size_t depth_;

    // 返回类型为 std::unique_ptr<Node>, 参数为右值引用
    std::unique_ptr<Node> insertRec(std::unique_ptr<Node> node, const Point& point, size_t depth) {
        if (!node) {
            // 使用 new 创建 Node 对象，并用 std::unique_ptr 包装它
            return std::make_unique<Node>(Node{point, nullptr, nullptr});
        }

        size_t cd = depth % 2; // 假设我们交替使用纬度和经度来分割空间
        if (cd == 0 ? point.lat < node->point.lat : point.lon < node->point.lon) {
            node->left = insertRec(std::move(node->left), point, depth + 1);
        } else {
            node->right = insertRec(std::move(node->right), point, depth + 1);
        }

        return node;
    }

    // 递归查找最近邻点
    void nearestNeighborSearch(Node* node, double targetLat, double targetLon, size_t depth,
                               double& minDistSquared, Point& bestPoint) const {
        if (!node) return;

        // 更新当前最佳点
        double distSquared = calculateDistanceWithLatAndLon(targetLat, targetLon, node->point.lat, node->point.lon);
        if (distSquared < minDistSquared) {
            minDistSquared = distSquared;
            bestPoint = node->point;
        }

        // 决定先访问哪个子树
        size_t cd = depth % 2;
        Node* nextBranch = nullptr;
        Node* oppositeBranch = nullptr;

        if (cd == 0 ? targetLat < node->point.lat : targetLon < node->point.lon) {
            nextBranch = node->left.get();
            oppositeBranch = node->right.get();
        } else {
            nextBranch = node->right.get();
            oppositeBranch = node->left.get();
        }

        // 先递归进入更可能包含最近点的子树
        nearestNeighborSearch(nextBranch, targetLat, targetLon, depth + 1, minDistSquared, bestPoint);

        // 检查是否需要检查另一个子树
        double distToSplitPlane = (cd == 0 ? targetLat - node->point.lat : targetLon - node->point.lon);
        if (distToSplitPlane * distToSplitPlane < minDistSquared) {
            nearestNeighborSearch(oppositeBranch, targetLat, targetLon, depth + 1, minDistSquared, bestPoint);
        }
    }
    // 辅助函数如计算距离等...
};

inline std::unordered_map<long long, Node> nodes;
inline std::unordered_map<long long, Node> false_nodes;
inline std::vector<Way> ways;
inline Graph graph;
inline std::unordered_map<long long, Way> on_way;
inline KDTree kdtree;
