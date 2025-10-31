import React, { useState, useEffect, useCallback, useMemo } from 'react';
import { Tree, Input } from 'antd';
import {
  FolderOutlined,
  FolderOpenOutlined,
  FileOutlined,
  FileTextOutlined,
  CodeOutlined,
  FileImageOutlined,
  FilePdfOutlined,
  FileMarkdownOutlined,
  FileZipOutlined,
  SearchOutlined,
} from '@ant-design/icons';
import { useApi } from '../api';

const { Search } = Input;

const pathUtils = {
  join: (...segments) => segments.join('/').replace(/\/\/+/g, '/').replace(/^\.\//g, ''),
  normalize: (p) => p.replace(/\/\/+/g, '/').replace(/^\.\//g, ''),
};

const getFileIcon = (fileName, isDirectory) => {
  if (isDirectory) {
    return <FolderOutlined style={{ color: '#faad14' }} />;
  }

  const extension = fileName.split('.').pop().toLowerCase();
  const iconStyle = { fontSize: '14px' };

  const iconMap = {
    // Code files
    'js': <CodeOutlined style={{ ...iconStyle, color: '#f7df1e' }} />,
    'jsx': <CodeOutlined style={{ ...iconStyle, color: '#61dafb' }} />,
    'ts': <CodeOutlined style={{ ...iconStyle, color: '#3178c6' }} />,
    'tsx': <CodeOutlined style={{ ...iconStyle, color: '#3178c6' }} />,
    'c': <CodeOutlined style={{ ...iconStyle, color: '#555555' }} />,
    'h': <CodeOutlined style={{ ...iconStyle, color: '#555555' }} />,
    'cpp': <CodeOutlined style={{ ...iconStyle, color: '#00599c' }} />,
    'hpp': <CodeOutlined style={{ ...iconStyle, color: '#00599c' }} />,
    'lpc': <CodeOutlined style={{ ...iconStyle, color: '#a074c4' }} />,
    'py': <CodeOutlined style={{ ...iconStyle, color: '#3776ab' }} />,
    'java': <CodeOutlined style={{ ...iconStyle, color: '#007396' }} />,
    'go': <CodeOutlined style={{ ...iconStyle, color: '#00add8' }} />,
    'rs': <CodeOutlined style={{ ...iconStyle, color: '#ce422b' }} />,

    // Markup/Data
    'html': <FileTextOutlined style={{ ...iconStyle, color: '#e34c26' }} />,
    'css': <FileTextOutlined style={{ ...iconStyle, color: '#264de4' }} />,
    'json': <FileTextOutlined style={{ ...iconStyle, color: '#5a5a5a' }} />,
    'xml': <FileTextOutlined style={{ ...iconStyle, color: '#e37933' }} />,
    'yml': <FileTextOutlined style={{ ...iconStyle, color: '#cb171e' }} />,
    'yaml': <FileTextOutlined style={{ ...iconStyle, color: '#cb171e' }} />,
    'toml': <FileTextOutlined style={{ ...iconStyle, color: '#9c4121' }} />,

    // Documentation
    'md': <FileMarkdownOutlined style={{ ...iconStyle, color: '#083fa1' }} />,
    'txt': <FileTextOutlined style={{ ...iconStyle, color: '#ffffff' }} />,
    'pdf': <FilePdfOutlined style={{ ...iconStyle, color: '#f40f02' }} />,

    // Images
    'png': <FileImageOutlined style={{ ...iconStyle, color: '#4caf50' }} />,
    'jpg': <FileImageOutlined style={{ ...iconStyle, color: '#4caf50' }} />,
    'jpeg': <FileImageOutlined style={{ ...iconStyle, color: '#4caf50' }} />,
    'gif': <FileImageOutlined style={{ ...iconStyle, color: '#4caf50' }} />,
    'svg': <FileImageOutlined style={{ ...iconStyle, color: '#ffb13b' }} />,

    // Archives
    'zip': <FileZipOutlined style={{ ...iconStyle, color: '#d4ac0d' }} />,
    'tar': <FileZipOutlined style={{ ...iconStyle, color: '#d4ac0d' }} />,
    'gz': <FileZipOutlined style={{ ...iconStyle, color: '#d4ac0d' }} />,
  };

  return iconMap[extension] || <FileOutlined style={{ ...iconStyle, color: '#bfbfbf' }} />;
};

function FileTree({ onFileSelect, initialFiles }) {
  const { fetchFiles } = useApi();
  const [treeData, setTreeData] = useState([]);
  const [loadedKeys, setLoadedKeys] = useState(new Set());
  const [expandedKeys, setExpandedKeys] = useState([]);
  const [searchValue, setSearchValue] = useState('');
  const [autoExpandParent, setAutoExpandParent] = useState(true);

  useEffect(() => {
    if (initialFiles && initialFiles.files && treeData.length === 0) {
      const formattedData = formatFilesToTreeData(initialFiles.files, '.');
      setTreeData(formattedData);
      setLoadedKeys(new Set(['.']));
    }
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [initialFiles]);

  const formatFilesToTreeData = (files, parentPath) => {
    return files
      .sort((a, b) => {
        // Directories first, then alphabetically
        if (a.isDirectory !== b.isDirectory) {
          return a.isDirectory ? -1 : 1;
        }
        return a.name.localeCompare(b.name);
      })
      .map(file => {
        const fullPath = pathUtils.normalize(pathUtils.join(parentPath, file.name));
        const fileName = file.name;

        return {
          title: fileName,
          key: fullPath,
          icon: getFileIcon(fileName, file.isDirectory),
          isLeaf: !file.isDirectory,
          children: file.isDirectory ? [] : undefined,
          isDirectory: file.isDirectory,
        };
      });
  };

  // Recursively search tree and return all matching keys
  const getAllMatchingKeys = useCallback((nodes, searchVal) => {
    const keys = [];
    const search = searchVal.toLowerCase();

    const traverse = (nodeList) => {
      nodeList.forEach(node => {
        if (node.title.toLowerCase().includes(search)) {
          keys.push(node.key);
        }
        if (node.children && node.children.length > 0) {
          traverse(node.children);
        }
      });
    };

    traverse(nodes);
    return keys;
  }, []);

  // Get all parent keys for a given key
  const getParentKeys = useCallback((key, tree) => {
    const parents = [];
    const path = key.split('/');

    for (let i = 1; i < path.length; i++) {
      const parentPath = path.slice(0, i).join('/') || '.';
      parents.push(parentPath);
    }

    return parents;
  }, []);

  // Handle search
  const handleSearch = useCallback((value) => {
    setSearchValue(value);

    if (!value) {
      setExpandedKeys([]);
      setAutoExpandParent(false);
      return;
    }

    const matchingKeys = getAllMatchingKeys(treeData, value);
    const parentKeys = new Set();

    matchingKeys.forEach(key => {
      const parents = getParentKeys(key, treeData);
      parents.forEach(p => parentKeys.add(p));
    });

    setExpandedKeys([...parentKeys]);
    setAutoExpandParent(true);
  }, [treeData, getAllMatchingKeys, getParentKeys]);

  // Highlight search matches in titles
  const getHighlightedTitle = useCallback((title) => {
    if (!searchValue) {
      return title;
    }

    const index = title.toLowerCase().indexOf(searchValue.toLowerCase());
    if (index === -1) {
      return title;
    }

    const beforeStr = title.substring(0, index);
    const matchStr = title.substring(index, index + searchValue.length);
    const afterStr = title.substring(index + searchValue.length);

    return (
      <span>
        {beforeStr}
        <span style={{ color: '#4ec9b0', fontWeight: 'bold', backgroundColor: 'rgba(78, 201, 176, 0.2)' }}>
          {matchStr}
        </span>
        {afterStr}
      </span>
    );
  }, [searchValue]);

  // Update tree data with highlighted titles
  const highlightedTreeData = useMemo(() => {
    if (!searchValue) {
      return treeData;
    }

    const updateTitles = (nodes) => {
      return nodes.map(node => ({
        ...node,
        title: getHighlightedTitle(node.title),
        children: node.children ? updateTitles(node.children) : undefined,
      }));
    };

    return updateTitles(treeData);
  }, [treeData, searchValue, getHighlightedTitle]);

  const onLoadData = useCallback(async (node) => {
    const { key } = node;

    if (loadedKeys.has(key)) {
      return;
    }

    try {
      const data = await fetchFiles(key);
      const newChildren = formatFilesToTreeData(data.files, key);

      setTreeData(prevTreeData => {
        const updateTreeData = (list) => {
          return list.map(item => {
            if (item.key === key) {
              return {
                ...item,
                children: newChildren,
              };
            }
            if (item.children) {
              return {
                ...item,
                children: updateTreeData(item.children),
              };
            }
            return item;
          });
        };
        return updateTreeData(prevTreeData);
      });

      setLoadedKeys(prev => new Set([...prev, key]));
    } catch (error) {
      console.error('Failed to load directory:', error);
    }
  }, [fetchFiles, loadedKeys]);

  const onSelect = useCallback((selectedKeys, info) => {
    const { node } = info;

    // If it's a directory, toggle its expanded state
    if (node.isDirectory) {
      const isExpanded = expandedKeys.includes(node.key);
      if (isExpanded) {
        setExpandedKeys(expandedKeys.filter(k => k !== node.key));
      } else {
        setExpandedKeys([...expandedKeys, node.key]);
      }
    } else if (selectedKeys.length > 0 && node.isLeaf) {
      // If it's a file, open it
      onFileSelect(selectedKeys[0]);
    }
  }, [onFileSelect, expandedKeys]);

  const onExpand = useCallback((expandedKeysValue) => {
    setExpandedKeys(expandedKeysValue);
    setAutoExpandParent(false);
  }, []);

  return (
    <div style={{
      height: '100%',
      display: 'flex',
      flexDirection: 'column',
      backgroundColor: '#1e1e1e',
    }}>
      <div style={{
        padding: '8px',
        borderBottom: '1px solid #333',
      }}>
        <div style={{
          color: '#cccccc',
          fontSize: '12px',
          fontWeight: 'bold',
          textTransform: 'uppercase',
          marginBottom: '8px',
        }}>
          Explorer
        </div>
        <Search
          placeholder="Search files..."
          allowClear
          value={searchValue}
          onChange={(e) => handleSearch(e.target.value)}
          prefix={<SearchOutlined style={{ color: '#858585' }} />}
          style={{
            backgroundColor: '#3c3c3c',
          }}
          size="small"
        />
      </div>
      <div style={{
        flexGrow: 1,
        overflow: 'auto',
        padding: '8px',
      }}>
        <Tree
          showIcon
          loadData={onLoadData}
          treeData={highlightedTreeData}
          onSelect={onSelect}
          onExpand={onExpand}
          expandedKeys={expandedKeys}
          autoExpandParent={autoExpandParent}
          style={{
            backgroundColor: 'transparent',
            color: '#cccccc',
          }}
        />
      </div>
    </div>
  );
}

export default FileTree;