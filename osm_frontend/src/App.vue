<template>
  <header>
    <div class="wrapper">
      <!-- 添加搜索框和自动完成结果列表 -->
      <div class="search-container">
        <input v-model="searchQuery" @input="debouncedSearch" placeholder="Enter address or location name" />
        <ul v-if="searchResults.length > 0" class="autocomplete-results">
          <li v-for="(result, index) in searchResults" :key="index" @click="selectLocation(result)">
            {{ result.display_name }}
          </li>
        </ul>
      </div>
      
      <!-- 显示已选择点的信息 -->
      <div v-if="selectedPoints.length > 0" class="selected-points">
        <h3>Selected Points:</h3>
        <ul>
          <li v-for="(point, index) in selectedPoints" :key="index">
            Point {{ index + 1 }}: {{ point.display_name }}, Lat {{ point.lat }}, Lng {{ point.lng }}
            <button @click="removePoint(index)">Remove</button>
          </li>
        </ul>
        <!-- 当选择了两个点时显示“寻路”按钮 -->
        <div v-if="selectedPoints.length === 2" class="algorithm-selection">
        <label for="algorithm">Choose Algorithm:</label>
        <select id="algorithm" v-model="selectedAlgorithm">
          <option value="dijkstra">Dijkstra</option>
          <option value="a-star">A-star</option>
          <option value="bidirectional-a-star">Bidirectional A-star</option>
        </select>
        <button @click="sendPathFindingRequest">Find Path</button>
      </div>
      </div>

      <Map ref="mapComponent" />
    </div>
  </header>
</template>

<script>
import Map from './components/MapTest.vue';
import axios from 'axios';
import _ from 'lodash';

export default {
  name: 'App',
  components: {
    Map
  },
  data() {
    return {
      searchQuery: '', // 用户输入的查询字符串
      selectedPoints: [], // 已选择的点
      isSelectingStart: true, // 当前是否在选择起点
      searchResults: [] // 自动完成的搜索结果
    }
  },
  methods: {
    async geocodeAddress(address) {
      try {
        const response = await axios.get('https://nominatim.openstreetmap.org/search', {
          params: {
            q: address,
            format: 'json',
            limit: 5, // 返回多个结果
            accept_language: 'zh-CN',
            viewbox: '121.3981000,31.2447000,121.6003000,31.3522000', // 设置地理边界框，请替换为实际坐标值
            bounded: 1 // 确保结果严格位于viewbox所定义的区域内
          }
        });
        if (response.data.length > 0) {
          return response.data.map(item => ({
            lat: parseFloat(item.lat),
            lng: parseFloat(item.lon),
            display_name: item.display_name,
          }));
        } else {
          throw new Error("No results found");
        }
      } catch (error) {
        console.error("Error in geocoding:", error);
        throw error;
      }
    },
    debouncedSearch: _.debounce(async function () {
      if (this.searchQuery.length >= 3) { // 当输入达到一定长度时开始搜索
        this.searchResults = await this.geocodeAddress(this.searchQuery);
      } else {
        this.searchResults = [];
      }
    }, 300), // 设置防抖时间，避免频繁请求
    selectLocation(location) {
      this.selectedPoints.push({
        lat: location.lat,
        lng: location.lng,
        display_name: location.display_name, // 添加显示名称以供展示
      });

      // 清空搜索框并隐藏结果列表
      this.searchQuery = '';
      this.searchResults = [];

      // 在地图上标记新选中的点
      this.$refs.mapComponent.addMarker(location.lat, location.lng);

      if (!this.isSelectingStart) {
        this.isSelectingStart = true; // 重置状态为选择起点
      }
    },
    async sendPathFindingRequest() {
      if (this.selectedPoints.length !== 2) {
        alert("Please select exactly two points before finding a path.");
        return;
      }

      try {
        const config = {
          headers: {
            'Content-Type': 'application/json'
          }
        };
        const response = await axios.post('/api/path-finding', 
          JSON.stringify({
            start: this.selectedPoints[0],
            end: this.selectedPoints[1],
            algorithm: this.selectedAlgorithm
          }), config);

        // 假设后端返回了一条路径（一系列坐标点）
        const path = response.data.path;
        console.log('node found in ' + response.data.time1 + 'ms');
        console.log('path found in ' + response.data.time2 + 'ms');
        // 更新地图显示路径
        if(path) {
          this.$refs.mapComponent.removePath();
          this.$refs.mapComponent.showPath(path);
        }
        // 可选：清空已选点以便重新选择
        // this.selectedPoints = [];
      } catch (error) {
        alert(error);
        console.error("Error sending path finding request:", error);
        //alert("Failed to find a path. Please try again later.");
      }
    },
    removePoint(index) {
      // 移除指定索引的选定点，并从地图上删除对应的标记
      this.$refs.mapComponent.removeMarker(index); // 假设Map组件有这个方法
      this.selectedPoints.splice(index, 1);

      // 如果移除了最后一个点，则允许再次选择终点
      if (this.selectedPoints.length < 2) {
        this.isSelectingStart = false;
      }
    }
  }
};
</script>
<style scoped>
/* ... 保留原有样式 ... */

.search-container {
  position: relative;
}

.autocomplete-results {
  position: absolute;
  width: 100%;
  background-color: white;
  border: 1px solid #ddd;
  list-style-type: none;
  padding: 0;
  margin-top: 5px;
  max-height: 200px;
  overflow-y: auto;
  z-index: 1000;
}

.autocomplete-results li {
  padding: 8px 12px;
  cursor: pointer;
}

.autocomplete-results li:hover {
  background-color: #f0f0f0;
}

/* ... 保留原有样式 ... */

.selected-points {
  margin-top: 1rem;
}

.selected-points ul {
  list-style-type: none;
  padding: 0;
}

.selected-points li {
  padding: 0.5rem;
  border-bottom: 1px solid #ddd;
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.selected-points button {
  background-color: #ff4d4d;
  color: white;
  border: none;
  padding: 0.25rem 0.5rem;
  cursor: pointer;
}

.selected-points button:hover {
  background-color: #e53935;
}

.selected-points button {
  margin-top: 1rem;
  padding: 0.5rem 1rem;
  background-color: #4CAF50;
  color: white;
  border: none;
  cursor: pointer;
}

.selected-points button:hover {
  background-color: #45a049;
}
</style>