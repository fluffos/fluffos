import React, { useState, useEffect, useRef } from 'react';
import { Button, Tooltip, message as antMessage, Empty } from 'antd';
import { ThunderboltOutlined, SaveOutlined } from '@ant-design/icons';
import Editor from '@monaco-editor/react';
import { Allotment } from 'allotment';
import CompilationResults from './CompilationResults';
import { useApi } from '../api';

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

function CompilationView({ currentFilePath, onCompile }) {
  const { fetchFileContent, saveFile } = useApi();
  const [fileContent, setFileContent] = useState('');
  const [originalContent, setOriginalContent] = useState('');
  const [isDirty, setIsDirty] = useState(false);
  const [isSaving, setIsSaving] = useState(false);
  const [isCompiling, setIsCompiling] = useState(false);
  const [language, setLanguage] = useState('plaintext');
  const [compilationResult, setCompilationResult] = useState(null);
  const [compilationError, setCompilationError] = useState(null);
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

    // Add keyboard shortcut for compile (Ctrl+Shift+B / Cmd+Shift+B)
    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyMod.Shift | monaco.KeyCode.KeyB, () => {
      handleCompile();
    });
  };

  const handleSave = async (silent = false) => {
    if (!isDirty) {
      if (!silent) {
        antMessage.info('No changes to save');
      }
      return true;
    }

    setIsSaving(true);
    try {
      await saveFile(currentFilePath, fileContent);
      setOriginalContent(fileContent);
      setIsDirty(false);
      if (!silent) {
        antMessage.success('File saved successfully');
      }
      return true;
    } catch (err) {
      antMessage.error(`Failed to save: ${err.message}`);
      return false;
    } finally {
      setIsSaving(false);
    }
  };

  const handleCompile = async () => {
    // Auto-save before compiling (silent save)
    if (isDirty) {
      const saved = await handleSave(true);
      if (!saved) {
        antMessage.error('Cannot compile: file save failed');
        return;
      }
      antMessage.success('File saved and compiling...');
    }

    setIsCompiling(true);
    setCompilationResult(null);
    setCompilationError(null);

    try {
      const result = await onCompile(currentFilePath);
      setCompilationResult(result);
      setCompilationError(null);
      antMessage.success('Compilation successful');
    } catch (err) {
      setCompilationError(err.message);
      setCompilationResult(null);
      // Error message already shown by catch
    } finally {
      setIsCompiling(false);
    }
  };

  const fileName = currentFilePath ? currentFilePath.split('/').pop() : '';
  const isLpcFile = currentFilePath && (currentFilePath.endsWith('.c') || currentFilePath.endsWith('.lpc'));

  if (!currentFilePath) {
    return (
      <div style={{
        display: 'flex',
        justifyContent: 'center',
        alignItems: 'center',
        height: '100%',
        backgroundColor: '#1e1e1e',
      }}>
        <Empty
          image={Empty.PRESENTED_IMAGE_SIMPLE}
          description={
            <span style={{ color: '#858585' }}>
              Select a file to edit and compile
            </span>
          }
        />
      </div>
    );
  }

  if (!isLpcFile) {
    return (
      <div style={{
        display: 'flex',
        justifyContent: 'center',
        alignItems: 'center',
        height: '100%',
        backgroundColor: '#1e1e1e',
      }}>
        <Empty
          image={Empty.PRESENTED_IMAGE_SIMPLE}
          description={
            <span style={{ color: '#858585' }}>
              This file type cannot be compiled. Only LPC files (.c, .lpc) can be compiled.
            </span>
          }
        />
      </div>
    );
  }

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
              icon={<SaveOutlined />}
              onClick={handleSave}
              loading={isSaving}
              disabled={!isDirty}
              size="small"
            >
              Save
            </Button>
          </Tooltip>
          <Tooltip title="Compile (Ctrl+Shift+B)">
            <Button
              type="primary"
              icon={<ThunderboltOutlined />}
              onClick={handleCompile}
              loading={isCompiling}
              size="small"
            >
              Compile
            </Button>
          </Tooltip>
        </div>
      </div>

      {/* Split view: Editor + Compilation Results */}
      <div style={{ flexGrow: 1, overflow: 'hidden' }}>
        <Allotment vertical>
          <Allotment.Pane preferredSize="50%">
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
          </Allotment.Pane>
          <Allotment.Pane preferredSize="50%">
            <CompilationResults
              compilationResult={compilationResult}
              compilationError={compilationError}
              isCompiling={isCompiling}
              currentFile={currentFilePath}
            />
          </Allotment.Pane>
        </Allotment>
      </div>
    </div>
  );
}

export default CompilationView;
