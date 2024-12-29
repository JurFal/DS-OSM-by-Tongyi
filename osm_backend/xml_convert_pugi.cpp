#include <iostream>
#include <unordered_map>
#include <vector>
#include <chrono>
#include "pugixml.hpp"
#include "graph.hpp"
#include "httplib.h"
#include "nlohmann/json.hpp"

using namespace pugi;
using json = nlohmann::json;

void handlePathFinding(const httplib::Request& req, httplib::Response& res) {
    
    //cout << "waiting" << endl;
  try {
    auto parsed_json = json::parse(req.body);
    double startLat = parsed_json["start"]["lat"];
    double startLng = parsed_json["start"]["lng"];
    double endLat = parsed_json["end"]["lat"];
    double endLng = parsed_json["end"]["lng"];
    auto mode = parsed_json["algorithm"];
    auto find_start = std::chrono::high_resolution_clock::now();

    // 这里调用你的寻路函数，传入经纬度作为参数
    // std::vector<GeoPoint> path = findPath(startLat, startLng, endLat, endLng);
    long long startNodeId = findNearestNode(startLat, startLng);
    long long endNodeId = findNearestNode(endLat, endLng);
    auto find_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> find_duration = find_end - find_start;
    //cout << "Find shortest path: " << find_duration.count() << " ms" << endl;
    
    // 查找最短路径
    cout << startNodeId << ": lat: " << nodes[startNodeId].lat << " lon: " << nodes[startNodeId].lon << endl;
    cout << endNodeId << ": lat: " << nodes[endNodeId].lat << " lon: " << nodes[endNodeId].lon << endl;
    vector<long long> shortestPath;
    if(mode == "dijkstra") shortestPath = graph.dijkstra(startNodeId, endNodeId);
    else if(mode == "a-star") shortestPath = graph.a_star(startNodeId, endNodeId);
    else shortestPath = graph.bidirectional_a_star(startNodeId, endNodeId);
    auto find_path_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> find_path_duration = find_path_end - find_end;
    //cout << "Find shortest path: " << find_duration.count() << " ms" << endl;
    if (!shortestPath.empty()) {
        cout << "Shortest path found: " << "\n";
        for (long long nodeId : shortestPath) {
            cout << on_way[nodeId].speedLimit << on_way[nodeId].name << " ";
            cout << nodeId << ": lat: " << nodes[nodeId].lat << " lon: " << nodes[nodeId].lon << endl;
        }
        cout << endl;
    } else {
        cout << "No path found." << endl;
    }
    // 构建响应体
    json response;
    // 将路径信息加入response
    response["path"] = shortestPath;
    response["time1"] = find_duration.count();
    response["time2"] = find_path_duration.count();

    res.set_content(response.dump(), "application/json");
  } catch (const std::exception& e) {
    res.status = 400;
    cout << e.what() << endl;
    res.set_content("Bad Request", "text/plain");
  }
}

void initialize(){
    auto load_start = std::chrono::high_resolution_clock::now();
    xml_document doc;
    if (!doc.load_file("map.osm")) {
        std::cerr << "Failed to load file" << std::endl;
        return;
    }
    xml_node osm = doc.document_element();

    for (xml_node node : osm.children("node")) {
        long long id = node.attribute("id").as_llong();
        double lat = node.attribute("lat").as_double();
        double lon = node.attribute("lon").as_double();
        false_nodes[id] = {id, lat, lon};
        //cout << id << ": lat: " << nodes[id].lat << " lon: " << nodes[id].lon << endl;
    }

    for (xml_node way : osm.children("way")) {
        std::unordered_map<std::string, double> speedLimits = {
            {"motorway", 120},
            {"trunk", 100},
            {"motorway_junction", 100},
            {"primary", 60},
            {"secondary", 40},
            {"tertiary", 30},
            {"unclassified", 20},
            {"residential", 20},
            {"service", 20}
        };
        bool is_way = false;
        long long id = way.attribute("id").as_llong();
        Way w{ id };
        for (xml_node nd : way.children("nd")) {
            long long id = nd.attribute("id").as_llong();
            double lat = nd.attribute("lat").as_double();
            double lon = nd.attribute("lon").as_double();
            nodes[id] = {id, lat, lon};
            w.node_ids.push_back(nd.attribute("ref").as_llong());
        }
        w.speedLimit = 30.0;
        w.name = "unknown";
        w.oneway = false;
        for (auto tagNode : way.children("tag")) {
            auto kAttr = tagNode.attribute("k");
            auto vAttr = tagNode.attribute("v");

            if (kAttr && vAttr) {
                std::string key = kAttr.value();
                std::string value = vAttr.value();
                if (key == "highway") {
                    is_way = true;
                    w.highwayType = value;
                } 
                else if (key == "name:en") {
                    w.name = value;
                }
                else if (key == "oneway") {
                    w.oneway = value == "yes" ? true : false;
                }
            }
            
        }
        if (!w.highwayType.empty()) {
            w.speedLimit = speedLimits.count(w.highwayType) ? speedLimits[w.highwayType] : 30.0;
        }
        if(is_way) {
            for (xml_node nd : way.children("nd")) {
                long long id = nd.attribute("ref").as_llong();
                nodes[id] = false_nodes[id];
                w.node_ids.push_back(id);
                // 将节点插入K-d树
                KDTree::Point point{nodes[id].lat, nodes[id].lon, id};
                kdtree.insert(point);
            }
            ways.push_back(w);
        }
    }

    // 解析OSM XML文件并填充nodes和ways...
    // （这部分代码与之前的解析部分相同）

// 假设你已经有了nodes和ways的数据
    for (const auto& way : ways) {
        for (size_t i = 0; i < way.node_ids.size() - 1; ++i) {
            long long from = way.node_ids[i];
            long long to = way.node_ids[i + 1];
            on_way[from] = way;
            on_way[to] = way;
            // 添加边到图中
            double weight = calculateDistance(nodes[from], nodes[to]) / way.speedLimit;
            graph.addEdge(from, to, weight);
            if(!way.oneway) graph.addEdge(to, from, weight);
        }
    }

    auto load_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> load_duration = load_end - load_start;
    cout << "Loading xml: " << load_duration.count() << " ms" << endl;
}

int main() {
    initialize();
    httplib::Server svr;
    svr.Post("/path-finding", handlePathFinding);
    svr.listen("localhost", 8080);
    return 0;
}