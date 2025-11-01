import { useWebSocket } from './WebSocketContext';

// This hook provides the WebSocket send function
// It should be called within a component wrapped by WebSocketProvider
export const useApi = () => {
  const { sendWebSocketMessage } = useWebSocket();

  const compileCode = async (filePath) => {
    const response = await sendWebSocketMessage({
      command: 'compile',
      path: filePath,
    });
    return response;
  };

  const fetchFiles = async (dirPath = '.') => {
    const response = await sendWebSocketMessage({
      command: 'files',
      path: dirPath,
    });
    return response;
  };

  const fetchFileContent = async (filePath) => {
    const response = await sendWebSocketMessage({
      command: 'file_get',
      path: filePath,
    });
    return response.content; // Extract the content field
  };

  const saveFile = async (filePath, content) => {
    const response = await sendWebSocketMessage({
      command: 'file_post',
      path: filePath,
      content: content,
    });
    return response.message; // Extract the message field
  };

  return { compileCode, fetchFiles, fetchFileContent, saveFile };
};
