<template>
    <div id="map" ref="mapContainer" style="height: 100vh; width: 90vw;"></div>
  </template>
  
  <script>
  import L from 'leaflet';
  
  export default {
    name: 'Map',
    mounted() {
      this.initMap();
    },
    data() {
      return {
        markers: [], // 存储 marker 引用的数组
        pathCoordinates: [],
        currentPolyline: null,
        have_path: false
        // ... 其他数据属性
      };
    },
    methods: {
      initMap() {
        this.map = L.map('map', {
          maxBounds: L.latLngBounds(
              L.latLng(31.2447000, 121.3981000), // 西南角坐标
              L.latLng(31.3522000, 121.6003000)  // 东北角坐标
          ),
          center: [31.30, 121.45], // 地图中心坐标
          zoom: 13,                         // 初始缩放级别
          minZoom: 12,                       // 最小缩放级别
          maxZoom: 18                       // 最大缩放级别
        });
  
        L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
          attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
        }).addTo(this.map);
        // 你可以在这里添加更多的地图图层、标记、多边形等
      },
      addMarker(lat, lng) {
        if (this.map) {
          this.markers.push(L.marker([lat, lng]).addTo(this.map)
            .bindPopup(`Latitude: ${lat}, Longitude: ${lng}`)
            .openPopup());
        }
      },
      removeMarker(indexOrMarker) {
        let markerToRemove;
        
        if (typeof indexOrMarker === 'number') {
          markerToRemove = this.markers[indexOrMarker];
          this.markers.splice(indexOrMarker, 1);
        } else if (indexOrMarker instanceof L.Marker) {
          markerToRemove = indexOrMarker;
        }

        if (markerToRemove && this.map.hasLayer(markerToRemove)) {
          this.map.removeLayer(markerToRemove);
          console.log('Marker removed successfully.');
        } else {
          console.log('No marker found to remove.');
        }
      },
      async showPath(path) {
        try {
          const coordinates = await Promise.all(path.map(nodeId => this.getNodeCoordinates(nodeId)));
          this.pathCoordinates = coordinates;

          if (this.pathCoordinates.length > 1) {
            // 如果已经存在 polyline，则先移除它
            if (this.currentPolyline && this.map.hasLayer(this.currentPolyline)) {
              this.map.removeLayer(this.currentPolyline);
            }

            // 创建新的 polyline 并添加到地图
            this.currentPolyline = L.polyline(this.pathCoordinates, { color: 'green' }).addTo(this.map);

            // 调整地图视图以适应新路径
            this.map.fitBounds(this.currentPolyline.getBounds());
            
            // 设置 have_path 标志为 true
            this.have_path = true;
          } else {
            // 如果没有足够的点来绘制路径，确保清除任何现有的 polyline
            if (this.currentPolyline && this.map.hasLayer(this.currentPolyline)) {
              this.map.removeLayer(this.currentPolyline);
              this.currentPolyline = null;
            }
            this.have_path = false;
          }
        } catch (error) {
          console.error("Error fetching node coordinates:", error);
        }
      },
      removePath() {
        if (this.currentPolyline && this.map.hasLayer(this.currentPolyline)) {
          this.map.removeLayer(this.currentPolyline);
          this.currentPolyline = null;
          this.have_path = false; // 重置路径标志
        }
      },
      async getNodeCoordinates(nodeId) {
        const response = await fetch(`https://api.openstreetmap.org/api/0.6/node/${nodeId}`);
        if (!response.ok) {
          throw new Error(`Failed to fetch node ${nodeId}: ${response.statusText}`);
        }
        const xmlText = await response.text();
        const parser = new DOMParser();
        const xmlDoc = parser.parseFromString(xmlText, "text/xml");
        const lat = parseFloat(xmlDoc.getElementsByTagName("node")[0].getAttribute("lat"));
        const lon = parseFloat(xmlDoc.getElementsByTagName("node")[0].getAttribute("lon"));
        return [lat, lon];
        }
      },
      beforeDestroy() {
        if (this.map) {
          this.map.remove();
        }
      }
  };
  </script>
  
  <style scoped>
  #map {
    height: 100vh;
    width: 100vw;
  }
  </style>