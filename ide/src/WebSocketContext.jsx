import React, { createContext, useContext, useState, useEffect, useRef, useCallback } from 'react';

const WebSocketContext = createContext(null);

let requestIdCounter = 0;

export const WebSocketProvider = ({ children }) => {
  const [isConnected, setIsConnected] = useState(false);
  const [isLoading, setIsLoading] = useState(true);
  const wsRef = useRef(null);
  const pendingRequests = useRef(new Map()); // Map to store {requestId: {resolve, reject}}

  const sendWebSocketMessage = useCallback((message) => {
    return new Promise((resolve, reject) => {
      if (wsRef.current && wsRef.current.readyState === WebSocket.OPEN) {
        const requestId = requestIdCounter++;
        const requestPayload = { ...message, requestId };
        wsRef.current.send(JSON.stringify(requestPayload));
        pendingRequests.current.set(requestId, { resolve, reject });
      } else {
        reject(new Error('WebSocket is not connected.'));
      }
    });
  }, []);

  useEffect(() => {
    const connectWebSocket = () => {
      setIsLoading(true);
      const wsUrl = `ws://${window.location.hostname}:${window.location.port}/debug`;
      console.log('Connecting to WebSocket:', wsUrl);
      const ws = new WebSocket(wsUrl, ['debug']);

      ws.onopen = () => {
        console.log('WebSocket connected!');
        setIsConnected(true);
        setIsLoading(false);
        wsRef.current = ws;
      };

      ws.onmessage = (event) => {
        try {
          const response = JSON.parse(event.data);
          const { requestId, ...data } = response;

          if (requestId !== undefined && pendingRequests.current.has(requestId)) {
            const { resolve, reject } = pendingRequests.current.get(requestId);
            pendingRequests.current.delete(requestId);

            if (data.error) {
              reject(new Error(data.error));
            } else {
              resolve(data);
            }
          } else {
            console.warn('Received unsolicited WebSocket message:', response);
          }
        } catch (e) {
          console.error('Failed to parse WebSocket message:', e, event.data);
        }
      };

      ws.onclose = () => {
        console.log('WebSocket disconnected. Attempting to reconnect...');
        setIsConnected(false);
        setIsLoading(true);
        wsRef.current = null;
        // Clear any pending requests with an error
        pendingRequests.current.forEach(({ reject }) => {
          reject(new Error('WebSocket disconnected during request.'));
        });
        pendingRequests.current.clear();
        setTimeout(connectWebSocket, 3000); // Attempt to reconnect after 3 seconds
      };

      ws.onerror = (error) => {
        console.error('WebSocket error:', error);
        ws.close(); // Close to trigger reconnect logic
      };
    };

    connectWebSocket();

    return () => {
      if (wsRef.current) {
        wsRef.current.close();
      }
    };
  }, []);

  const value = { isConnected, isLoading, sendWebSocketMessage };

  return (
    <WebSocketContext.Provider value={value}>
      {children}
    </WebSocketContext.Provider>
  );
};

export const useWebSocket = () => {
  const context = useContext(WebSocketContext);
  if (context === undefined) {
    throw new Error('useWebSocket must be used within a WebSocketProvider');
  }
  return context;
};
