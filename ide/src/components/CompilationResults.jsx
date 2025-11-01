import React from 'react';
import { Tabs, Empty, Spin } from 'antd';
import {
  CheckCircleOutlined,
  CloseCircleOutlined,
  FileTextOutlined,
  InfoCircleOutlined,
  LoadingOutlined,
} from '@ant-design/icons';

function CompilationResults({ compilationResult, compilationError, isCompiling, currentFile }) {
  // Show loading state
  if (isCompiling) {
    return (
      <div style={{
        display: 'flex',
        flexDirection: 'column',
        justifyContent: 'center',
        alignItems: 'center',
        height: '100%',
        backgroundColor: '#1e1e1e',
        gap: '16px',
      }}>
        <Spin indicator={<LoadingOutlined style={{ fontSize: 48, color: '#4ec9b0' }} spin />} />
        <span style={{ color: '#cccccc', fontSize: '14px' }}>
          Compiling {currentFile}...
        </span>
      </div>
    );
  }

  // Check if current file is an LPC file
  const isLpcFile = currentFile && (currentFile.endsWith('.c') || currentFile.endsWith('.lpc'));

  if (!compilationResult && !compilationError) {
    if (currentFile && !isLpcFile) {
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
              No compilation results yet. Select an LPC file to see compilation results here.
            </span>
          }
        />
      </div>
    );
  }

  if (compilationError) {
    return (
      <div style={{
        height: '100%',
        backgroundColor: '#1e1e1e',
        display: 'flex',
        flexDirection: 'column',
      }}>
        <div style={{
          display: 'flex',
          alignItems: 'center',
          gap: '12px',
          padding: '16px',
          backgroundColor: '#2d2d30',
          borderBottom: '1px solid #3e3e42',
        }}>
          <CloseCircleOutlined style={{ color: '#f48771', fontSize: '20px' }} />
          <div>
            <div style={{ color: '#f48771', fontSize: '14px', fontWeight: 'bold' }}>
              Compilation Failed
            </div>
            <div style={{ color: '#858585', fontSize: '12px', marginTop: '4px' }}>
              An error occurred during compilation
            </div>
          </div>
        </div>
        <div style={{
          flexGrow: 1,
          overflow: 'auto',
          padding: '16px',
        }}>
          <pre style={{
            margin: 0,
            padding: '12px',
            backgroundColor: '#5a1d1d',
            border: '1px solid #8b2e2e',
            borderRadius: '4px',
            color: '#f48771',
            fontSize: '13px',
            fontFamily: 'Consolas, Monaco, "Courier New", monospace',
            whiteSpace: 'pre-wrap',
            wordWrap: 'break-word',
          }}>
            {compilationError}
          </pre>
        </div>
      </div>
    );
  }

  const tabItems = [
    {
      key: 'overview',
      label: (
        <span>
          <InfoCircleOutlined /> Overview
        </span>
      ),
      children: (
        <div style={{ padding: '16px' }}>
          <div style={{
            display: 'grid',
            gridTemplateColumns: 'auto 1fr',
            gap: '12px 24px',
            fontSize: '13px',
          }}>
            <div style={{ color: '#858585' }}>File Path:</div>
            <div style={{ color: '#4ec9b0', fontFamily: 'monospace' }}>
              {compilationResult.path}
            </div>

            <div style={{ color: '#858585' }}>Inherits:</div>
            <div style={{ color: '#cccccc' }}>
              {compilationResult.inherits} {compilationResult.inherits === 1 ? 'object' : 'objects'}
            </div>

            <div style={{ color: '#858585' }}>Program Size:</div>
            <div style={{ color: '#cccccc' }}>
              {compilationResult.program_size.toLocaleString()} bytes
            </div>
          </div>
        </div>
      ),
    },
    {
      key: 'disassembly',
      label: (
        <span>
          <FileTextOutlined /> Disassembly
        </span>
      ),
      children: (
        <pre style={{
          margin: 0,
          padding: '16px',
          fontSize: '12px',
          fontFamily: 'Consolas, Monaco, "Courier New", monospace',
          whiteSpace: 'pre-wrap',
          wordWrap: 'break-word',
          color: '#cccccc',
          lineHeight: '1.6',
        }}>
          {compilationResult.disassembly}
        </pre>
      ),
    },
  ];

  return (
    <div style={{
      height: '100%',
      backgroundColor: '#1e1e1e',
    }}>
      <Tabs
        defaultActiveKey="overview"
        items={tabItems}
        size="small"
        style={{ height: '100%' }}
        tabBarStyle={{
          margin: 0,
          paddingLeft: '16px',
          backgroundColor: '#2d2d30',
          borderBottom: '1px solid #3e3e42',
        }}
      />
    </div>
  );
}

export default CompilationResults;
