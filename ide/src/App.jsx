import React, { useState, useEffect } from 'react';
import FileTree from './components/FileTree';
import FileEditor from './components/FileEditor';
import CompilationView from './components/CompilationView';
import Terminal from './components/Terminal';
import { Allotment } from 'allotment';
import { Tabs } from 'antd';
import { FileTextOutlined, ThunderboltOutlined } from '@ant-design/icons';
import 'allotment/dist/style.css';
import { WebSocketProvider, useWebSocket } from './WebSocketContext';
import { useApi } from './api';

function AppContent() {
  const { isConnected, isLoading } = useWebSocket();
  const { fetchFiles, compileCode } = useApi();
  const [selectedFile, setSelectedFile] = useState(null);
  const [initialFiles, setInitialFiles] = useState([]);
  const [initialFilesError, setInitialFilesError] = useState(null);
  const [activeTab, setActiveTab] = useState('editor');

  useEffect(() => {
    if (isConnected && initialFiles.length === 0) {
      fetchFiles('.') // Request files from the root directory once
        .then(data => {
          setInitialFiles(data);
          setInitialFilesError(null);
        })
        .catch(err => {
          setInitialFilesError(err.message);
          console.error("Failed to fetch initial files:", err);
        });
    }
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [isConnected]);

  const handleCompile = async (filePath) => {
    const result = await compileCode(filePath);
    return result;
  };

  if (isLoading) {
    return (
      <div style={{
        display: 'flex',
        flexDirection: 'column',
        justifyContent: 'center',
        alignItems: 'center',
        height: '100vh',
        backgroundColor: '#1e1e1e',
        color: '#cccccc'
      }}>
        <h1>Connecting to IDE server...</h1>
        <div className="spinner"></div>
      </div>
    );
  }

  if (!isConnected) {
    return (
      <div style={{
        display: 'flex',
        justifyContent: 'center',
        alignItems: 'center',
        height: '100vh',
        backgroundColor: '#1e1e1e',
        color: '#f48771'
      }}>
        <h1>Failed to connect to IDE server. Retrying...</h1>
      </div>
    );
  }

  const tabItems = [
    {
      key: 'editor',
      label: (
        <span>
          <FileTextOutlined /> Editor
        </span>
      ),
      children: selectedFile ? (
        <FileEditor currentFilePath={selectedFile} />
      ) : (
        <div style={{
          display: 'flex',
          justifyContent: 'center',
          alignItems: 'center',
          height: '100%',
          color: '#858585',
          fontSize: '16px',
          backgroundColor: '#1e1e1e'
        }}>
          Select a file to edit
        </div>
      ),
    },
    {
      key: 'compilation',
      label: (
        <span>
          <ThunderboltOutlined /> Compilation
        </span>
      ),
      children: (
        <CompilationView
          currentFilePath={selectedFile}
          onCompile={handleCompile}
        />
      ),
    },
  ];

  return (
    <div style={{ height: '100vh', backgroundColor: '#1e1e1e' }}>
      <Allotment vertical>
        {/* Top section: File tree + Editor/Compilation tabs */}
        <Allotment.Pane>
          <Allotment>
            <Allotment.Pane preferredSize={280} minSize={200}>
              {initialFilesError && (
                <div style={{
                  color: '#f48771',
                  padding: '8px',
                  backgroundColor: '#5a1d1d',
                  margin: '8px',
                  borderRadius: '4px'
                }}>
                  Error: {initialFilesError}
                </div>
              )}
              <FileTree onFileSelect={setSelectedFile} initialFiles={initialFiles} />
            </Allotment.Pane>
            <Allotment.Pane>
              <Tabs
                activeKey={activeTab}
                onChange={setActiveTab}
                items={tabItems}
                style={{
                  height: '100%',
                  backgroundColor: '#1e1e1e',
                }}
                tabBarStyle={{
                  margin: 0,
                  paddingLeft: '16px',
                  backgroundColor: '#2d2d30',
                  borderBottom: '1px solid #3e3e42',
                }}
              />
            </Allotment.Pane>
          </Allotment>
        </Allotment.Pane>
        {/* Bottom section: Terminal */}
        <Allotment.Pane preferredSize={250} minSize={150}>
          <Terminal />
        </Allotment.Pane>
      </Allotment>
    </div>
  );
}

function App() {
  return (
    <WebSocketProvider>
      <AppContent />
    </WebSocketProvider>
  );
}

export default App;
