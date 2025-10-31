import React from 'react';
import ReactDOM from 'react-dom/client';
import App from './App.jsx';
import { ConfigProvider, theme } from 'antd';
import 'allotment/dist/style.css';
import './index.css';

ReactDOM.createRoot(document.getElementById('root')).render(
  <React.StrictMode>
    <ConfigProvider
      theme={{
        algorithm: theme.darkAlgorithm,
        token: {
          colorBgContainer: '#1e1e1e',
          colorBgElevated: '#2d2d30',
          colorBorder: '#3e3e42',
          colorText: '#cccccc',
          colorTextSecondary: '#858585',
          colorPrimary: '#0e639c',
          fontSize: 13,
        },
      }}
    >
      <App />
    </ConfigProvider>
  </React.StrictMode>,
);
