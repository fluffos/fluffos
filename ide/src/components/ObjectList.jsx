import React, { useEffect, useState } from 'react';
import { List, Tag, Spin } from 'antd';
import { FileOutlined, CopyOutlined } from '@ant-design/icons';
import { useWebSocket } from '../WebSocketContext';

const ObjectList = ({ onObjectSelect }) => {
    const [objects, setObjects] = useState([]);
    const [loading, setLoading] = useState(true);
    const { sendWebSocketMessage } = useWebSocket();

    useEffect(() => {
        const fetchObjects = async () => {
            try {
                setLoading(true);
                const response = await sendWebSocketMessage({ command: 'objects' });
                if (response.objects) {
                    setObjects(response.objects);
                }
            } catch (err) {
                console.error('Failed to fetch objects:', err);
            } finally {
                setLoading(false);
            }
        };

        fetchObjects();

        // Refresh objects every 5 seconds
        const interval = setInterval(fetchObjects, 5000);
        return () => clearInterval(interval);
    }, [sendWebSocketMessage]);

    const isClone = (obname) => obname.includes('#');

    const getObjectBasename = (obname) => {
        const parts = obname.split('/');
        return parts[parts.length - 1];
    };

    const handleObjectClick = async (obname) => {
        if (onObjectSelect) {
            try {
                const response = await sendWebSocketMessage({
                    command: 'object_inspect',
                    obname
                });
                onObjectSelect(response);
            } catch (err) {
                console.error('Failed to inspect object:', err);
            }
        }
    };

    if (loading) {
        return (
            <div style={{ padding: '20px', textAlign: 'center' }}>
                <Spin tip="Loading objects..." />
            </div>
        );
    }

    return (
        <div style={{ height: '100%', overflow: 'auto' }}>
            <List
                size="small"
                dataSource={objects}
                renderItem={(obj) => (
                    <List.Item
                        key={obj.obname}
                        style={{
                            padding: '4px 12px',
                            cursor: 'pointer',
                            fontSize: '13px',
                        }}
                        onClick={() => handleObjectClick(obj.obname)}
                    >
                        <div style={{ display: 'flex', alignItems: 'center', gap: '8px', flex: 1 }}>
                            {isClone(obj.obname) ? <CopyOutlined /> : <FileOutlined />}
                            <span style={{ flex: 1, overflow: 'hidden', textOverflow: 'ellipsis' }}>
                                {getObjectBasename(obj.obname)}
                            </span>
                            <Tag color={isClone(obj.obname) ? 'blue' : 'green'} style={{ fontSize: '11px' }}>
                                refs: {obj.ref_count}
                            </Tag>
                        </div>
                    </List.Item>
                )}
            />
            <div style={{ padding: '8px 12px', fontSize: '11px', color: '#888', borderTop: '1px solid #303030' }}>
                {objects.length} objects loaded
            </div>
        </div>
    );
};

export default ObjectList;
