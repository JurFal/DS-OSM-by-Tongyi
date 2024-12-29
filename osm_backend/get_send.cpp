#include "httplib.h"
#include "nlohmann/json.hpp"
#include "get_send.hpp"

using json = nlohmann::json;

void handlePathFinding(const httplib::Request& req, httplib::Response& res) {
  try {
    auto parsed_json = json::parse(req.body);
    double startLat = parsed_json["start"]["lat"];
    double startLng = parsed_json["start"]["lng"];
    double endLat = parsed_json["end"]["lat"];
    double endLng = parsed_json["end"]["lng"];

    // 这里调用你的寻路函数，传入经纬度作为参数
    // std::vector<GeoPoint> path = findPath(startLat, startLng, endLat, endLng);

    // 构建响应体
    json response;
    // 将路径信息加入response
    // response["path"] = path;

    res.set_content(response.dump(), "application/json");
  } catch (const std::exception& e) {
    res.status = 400;
    res.set_content("Bad Request", "text/plain");
  }
}
