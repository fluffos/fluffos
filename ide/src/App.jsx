import React, { useState, useRef } from 'react';
import Editor from '@monaco-editor/react';
import FileTree from './FileTree.jsx';

function App() {
  const [selectedFile, setSelectedFile] = useState(null);
  const [fileContent, setFileContent] = useState('// Select a file to view its content');
  const editorRef = useRef(null);

  const handleFileSelect = (fileName) => {
    fetch(`/file?path=${fileName}`)
      .then(res => res.text())
      .then(data => {
        setFileContent(data);
        setSelectedFile(fileName);
      });
  };

  const handleSave = () => {
    if (selectedFile && editorRef.current) {
      const content = editorRef.current.getValue();
      fetch('/file', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ path: selectedFile, content }),
      })
      .then(res => {
        if (res.ok) {
          alert('File saved successfully!');
        } else {
          alert('Error saving file.');
        }
      });
    }
  };

  function handleEditorDidMount(editor, monaco) {
    editorRef.current = editor;
  }

  return (
    <div style={{ display: 'flex', height: '100vh', flexDirection: 'column' }}>
      <div style={{ flex: 1, display: 'flex', flexDirection: 'row' }}>
        <div style={{ width: '20%', overflowY: 'auto', borderRight: '1px solid #ccc' }}>
          <FileTree onFileSelect={handleFileSelect} />
        </div>
        <div style={{ width: '80%' }}>
          <Editor
            height="100%"
            defaultLanguage="javascript"
            value={fileContent}
            onMount={handleEditorDidMount}
          />
        </div>
      </div>
      <div style={{ padding: '10px', borderTop: '1px solid #ccc' }}>
        <button onClick={handleSave} disabled={!selectedFile}>Save</button>
      </div>
    </div>
  );
}

export default App;
