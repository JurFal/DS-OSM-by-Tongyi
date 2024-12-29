import { fileURLToPath, URL } from 'node:url'

import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import vueDevTools from 'vite-plugin-vue-devtools'

// https://vite.dev/config/
export default defineConfig({
  plugins: [
    vue(),
    vueDevTools(),
  ],
  resolve: {
    alias: {
      '@': fileURLToPath(new URL('./src', import.meta.url))
    },
  },
  server: {
    proxy: {
      '/api': {
        target: 'http://localhost:8080/', // 你要访问的 API 的地址
        changeOrigin: true,
        //secure: false, // 如果是 HTTPS 请求，设置为 false 可以避免一些证书错误
        rewrite: (path) => path.replace(/^\/api/, '')  // 你可以选择重写路径
      }
    }
  }
})