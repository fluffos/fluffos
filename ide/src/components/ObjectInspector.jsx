import React from 'react';
import { Descriptions, Tag, Collapse, Empty } from 'antd';
import { FunctionOutlined } from '@ant-design/icons';

const { Panel } = Collapse;

const ObjectInspector = ({ objectData }) => {
    if (!objectData) {
        return (
            <div style={{ padding: '20px', textAlign: 'center' }}>
                <Empty description="Select an object to inspect" />
            </div>
        );
    }

    const { obname, load_time, flags, ref_count, variables, functions, inherits } = objectData;

    const formatTimestamp = (ts) => {
        return new Date(ts * 1000).toLocaleString();
    };

    return (
        <div style={{ height: '100%', overflow: 'auto', padding: '16px' }}>
            <h3 style={{ marginBottom: '16px', color: '#ffffff' }}>{obname}</h3>

            <Descriptions bordered size="small" column={1} style={{ marginBottom: '16px' }}>
                <Descriptions.Item label="Load Time">{formatTimestamp(load_time)}</Descriptions.Item>
                <Descriptions.Item label="Flags">0x{flags.toString(16)}</Descriptions.Item>
                <Descriptions.Item label="Ref Count">{ref_count}</Descriptions.Item>
            </Descriptions>

            <Collapse defaultActiveKey={['variables', 'functions']} ghost>
                <Panel header={<span><FunctionOutlined /> Variables ({variables?.length || 0})</span>} key="variables">
                    {variables && variables.length > 0 ? (
                        <table style={{ width: '100%', fontSize: '13px' }}>
                            <thead>
                                <tr style={{ borderBottom: '1px solid #3e3e42' }}>
                                    <th style={{ padding: '4px 8px', textAlign: 'left' }}>Name</th>
                                    <th style={{ padding: '4px 8px', textAlign: 'left' }}>Value</th>
                                    <th style={{ padding: '4px 8px', textAlign: 'left' }}>Type</th>
                                </tr>
                            </thead>
                            <tbody>
                                {variables.map((v, idx) => (
                                    <tr key={idx} style={{ borderBottom: '1px solid #303030' }}>
                                        <td style={{ padding: '4px 8px', fontFamily: 'monospace' }}>{v.name}</td>
                                        <td style={{ padding: '4px 8px', fontFamily: 'monospace', maxWidth: '200px', overflow: 'hidden', textOverflow: 'ellipsis' }}>
                                            {v.value}
                                        </td>
                                        <td style={{ padding: '4px 8px' }}>
                                            <Tag color="blue" style={{ fontSize: '11px' }}>{v.type}</Tag>
                                        </td>
                                    </tr>
                                ))}
                            </tbody>
                        </table>
                    ) : (
                        <Empty description="No variables" image={Empty.PRESENTED_IMAGE_SIMPLE} />
                    )}
                </Panel>

                <Panel header={<span><FunctionOutlined /> Functions ({functions?.length || 0})</span>} key="functions">
                    {functions && functions.length > 0 ? (
                        <div style={{ display: 'flex', flexWrap: 'wrap', gap: '4px' }}>
                            {functions.map((fn, idx) => (
                                <Tag key={idx} style={{ margin: 0, fontFamily: 'monospace', fontSize: '12px' }}>
                                    {fn}
                                </Tag>
                            ))}
                        </div>
                    ) : (
                        <Empty description="No functions" image={Empty.PRESENTED_IMAGE_SIMPLE} />
                    )}
                </Panel>

                <Panel header={<span>📦 Inherits ({inherits?.length || 0})</span>} key="inherits">
                    {inherits && inherits.length > 0 ? (
                        <div style={{ display: 'flex', flexDirection: 'column', gap: '4px' }}>
                            {inherits.map((inh, idx) => (
                                <div key={idx} style={{ fontFamily: 'monospace', fontSize: '13px', padding: '4px 0' }}>
                                    {inh}
                                </div>
                            ))}
                        </div>
                    ) : (
                        <Empty description="No inheritance" image={Empty.PRESENTED_IMAGE_SIMPLE} />
                    )}
                </Panel>
            </Collapse>
        </div>
    );
};

export default ObjectInspector;
