import React, { useState, useEffect } from 'react';

const Node = ({ file, currentPath, onFileSelect }) => {
  const [isOpen, setIsOpen] = useState(false);
  const fullPath = currentPath === '.' ? file.name : `${currentPath}/${file.name}`;

  if (file.isDirectory) {
    return (
      <div>
        <div onClick={() => setIsOpen(!isOpen)} style={{ cursor: 'pointer' }}>
          {isOpen ? '▼' : '►'} {file.name}
        </div>
        {isOpen && <Directory path={fullPath} onFileSelect={onFileSelect} />}
      </div>
    );
  }

  return (
    <div onClick={() => onFileSelect(fullPath)} style={{ cursor: 'pointer', marginLeft: '15px' }}>
      {file.name}
    </div>
  );
};

const Directory = ({ path, onFileSelect, isRoot = false }) => {
  const [files, setFiles] = useState([]);
  const [error, setError] = useState(null);

  useEffect(() => {
    fetch(`/files?path=${encodeURIComponent(path)}`)
      .then(res => {
        if (!res.ok) {
          throw new Error(`HTTP error! status: ${res.status}`);
        }
        return res.json();
      })
      .then(data => {
        // Sort so directories are first
        data.sort((a, b) => {
          if (a.isDirectory && !b.isDirectory) return -1;
          if (!a.isDirectory && b.isDirectory) return 1;
          return a.name.localeCompare(b.name);
        });
        setFiles(data);
      })
      .catch(err => {
        console.error("Error fetching files for path:", path, err);
        setError(err.message);
      });
  }, [path]);

  if (error) {
    return <div style={{ color: 'red', marginLeft: '20px' }}>Error: {error}</div>;
  }

  const style = isRoot ? {} : { marginLeft: '20px', borderLeft: '1px solid #ccc', paddingLeft: '10px' };

  return (
    <div style={style}>
      {files.map(file => (
        <Node key={file.name} file={file} currentPath={path} onFileSelect={onFileSelect} />
      ))}
    </div>
  );
};

const FileTree = ({ onFileSelect }) => {
  return <Directory path='.' onFileSelect={onFileSelect} isRoot={true} />;
};

export default FileTree;
