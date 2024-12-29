const fs = require('fs');
const path = require('path');
const osmtogeojson = require('osmtogeojson');

// 指定 OSM XML 文件的路径
const osmFilePath = path.join(__dirname, 'map');

// 读取 OSM XML 文件
fs.readFile(osmFilePath, 'utf8', (err, data) => {
  if (err) {
    console.error('Error reading OSM file:', err);
    return;
  }

  // 使用 osmtogeojson 将 OSM XML 数据转换为 GeoJSON
  const geoJson = osmtogeojson(data);

  // 将 GeoJSON 数据写入文件或进行其他处理
  const geoJsonFilePath = path.join(__dirname, 'map-convert.geojson');
  fs.writeFile(geoJsonFilePath, JSON.stringify(geoJson, null, 2), 'utf8', (err) => {
    if (err) {
      console.error('Error writing GeoJSON file:', err);
      return;
    }
    console.log('GeoJSON file written successfully:', geoJsonFilePath);
  });
});