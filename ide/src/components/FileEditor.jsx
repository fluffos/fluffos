import React, { useState, useEffect, useRef } from 'react';
import { useApi } from '../api';
import Editor from '@monaco-editor/react';
import { Button, Tooltip, message as antMessage } from 'antd';
import {
  SaveOutlined,
} from '@ant-design/icons';

const getLanguageFromExtension = (filePath) => {
  if (!filePath) return 'plaintext';
  const extension = filePath.split('.').pop().toLowerCase();
  const languageMap = {
    'js': 'javascript',
    'jsx': 'javascript',
    'ts': 'typescript',
    'tsx': 'typescript',
    'json': 'json',
    'css': 'css',
    'scss': 'scss',
    'less': 'less',
    'html': 'html',
    'xml': 'xml',
    'py': 'python',
    'c': 'c',
    'h': 'c',
    'cpp': 'cpp',
    'hpp': 'cpp',
    'cc': 'cpp',
    'cxx': 'cpp',
    'lpc': 'c',
    'md': 'markdown',
    'sh': 'shell',
    'bash': 'shell',
    'yaml': 'yaml',
    'yml': 'yaml',
    'toml': 'toml',
    'rs': 'rust',
    'go': 'go',
    'java': 'java',
    'sql': 'sql',
  };
  return languageMap[extension] || 'plaintext';
};

function FileEditor({ currentFilePath }) {
  const { fetchFileContent, saveFile } = useApi();
  const [fileContent, setFileContent] = useState('');
  const [originalContent, setOriginalContent] = useState('');
  const [isDirty, setIsDirty] = useState(false);
  const [isSaving, setIsSaving] = useState(false);
  const [language, setLanguage] = useState('plaintext');
  const editorRef = useRef(null);

  useEffect(() => {
    if (currentFilePath) {
      setLanguage(getLanguageFromExtension(currentFilePath));
      fetchFileContent(currentFilePath)
        .then(content => {
          setFileContent(content);
          setOriginalContent(content);
          setIsDirty(false);
        })
        .catch(err => {
          antMessage.error(`Failed to load file: ${err.message}`);
          setFileContent('');
          setOriginalContent('');
        });
    }
  }, [currentFilePath, fetchFileContent]);

  const handleEditorChange = (value) => {
    setFileContent(value || '');
    setIsDirty(value !== originalContent);
  };

  const handleEditorDidMount = (editor, monaco) => {
    editorRef.current = editor;

    // Add keyboard shortcut for save (Ctrl+S / Cmd+S)
    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyS, () => {
      handleSave();
    });
  };

  const handleSave = async () => {
    if (!isDirty) {
      antMessage.info('No changes to save');
      return;
    }

    setIsSaving(true);
    try {
      await saveFile(currentFilePath, fileContent);
      setOriginalContent(fileContent);
      setIsDirty(false);
      antMessage.success('File saved successfully');
    } catch (err) {
      antMessage.error(`Failed to save: ${err.message}`);
    } finally {
      setIsSaving(false);
    }
  };

  const fileName = currentFilePath ? currentFilePath.split('/').pop() : '';

  return (
    <div style={{
      height: '100%',
      display: 'flex',
      flexDirection: 'column',
      backgroundColor: '#1e1e1e',
    }}>
      {/* Top bar with file name and actions */}
      <div style={{
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'space-between',
        padding: '8px 16px',
        backgroundColor: '#2d2d30',
        borderBottom: '1px solid #3e3e42',
      }}>
        <div style={{
          color: isDirty ? '#4ec9b0' : '#cccccc',
          fontSize: '13px',
          fontWeight: 500,
        }}>
          {isDirty && '● '}{fileName}
        </div>
        <div style={{ display: 'flex', gap: '8px' }}>
          <Tooltip title="Save (Ctrl+S)">
            <Button
              type="primary"
              icon={<SaveOutlined />}
              onClick={handleSave}
              loading={isSaving}
              disabled={!isDirty}
              size="small"
            >
              Save
            </Button>
          </Tooltip>
        </div>
      </div>

      {/* Editor */}
      <div style={{ flexGrow: 1, overflow: 'hidden' }}>
        <Editor
          height="100%"
          language={language}
          value={fileContent}
          theme="vs-dark"
          onChange={handleEditorChange}
          onMount={handleEditorDidMount}
          options={{
            minimap: { enabled: true },
            fontSize: 14,
            fontFamily: 'Consolas, Monaco, "Courier New", monospace',
            lineHeight: 20,
            scrollBeyondLastLine: false,
            automaticLayout: true,
            tabSize: 2,
            insertSpaces: true,
            renderWhitespace: 'selection',
            bracketPairColorization: { enabled: true },
            guides: {
              bracketPairs: true,
              indentation: true,
            },
            suggest: {
              showKeywords: true,
              showSnippets: true,
            },
            quickSuggestions: {
              other: true,
              comments: false,
              strings: false,
            },
          }}
        />
      </div>
    </div>
  );
}

export default FileEditor;
