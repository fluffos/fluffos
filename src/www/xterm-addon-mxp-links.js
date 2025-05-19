/**
* MXP Links Addon for xterm.js
* 支持MXP标签的识别和显示
*/
!function(e, t) {
  "object" == typeof exports && "object" == typeof module ? module.exports = t() : "function" == typeof define && define.amd ? define([], t) : "object" == typeof exports ? exports.MXPLinksAddon = t() : e.MXPLinksAddon = t()
}(window, (function() {
  return function(e) {
    var t = {};
    function n(r) {
      if (t[r])
      return t[r].exports;
      var i = t[r] = {
        i: r,
        l: !1,
        exports: {}
      };
      return e[r].call(i.exports, i, i.exports, n),
      i.l = !0,
      i.exports
    }
    return n.m = e,
    n.c = t,
    n.d = function(e, t, r) {
      n.o(e, t) || Object.defineProperty(e, t, {
        enumerable: !0,
        get: r
      })
    },
    n.r = function(e) {
      "undefined" != typeof Symbol && Symbol.toStringTag && Object.defineProperty(e, Symbol.toStringTag, {
        value: "Module"
      }),
      Object.defineProperty(e, "__esModule", {
        value: !0
      })
    },
    n.t = function(e, t) {
      if (1 & t && (e = n(e)),
      8 & t)
      return e;
      if (4 & t && "object" == typeof e && e && e.__esModule)
      return e;
      var r = Object.create(null);
      if (n.r(r),
      Object.defineProperty(r, "default", {
        enumerable: !0,
        value: e
      }),
      2 & t && "string" != typeof e)
      for (var i in e)
      n.d(r, i, function(t) {
        return e[t]
      }
      .bind(null, i));
      return r
    },
    n.n = function(e) {
      var t = e && e.__esModule ? function() {
        return e.default
      }
      : function() {
        return e
      }
      ;
      return n.d(t, "a", t),
      t
    },
    n.o = function(e, t) {
      return Object.prototype.hasOwnProperty.call(e, t)
    },
    n.p = "",
    n(n.s = 0)
  }([function(e, t, n) {
    "use strict";
    Object.defineProperty(t, "__esModule", {
      value: !0
    }),
    t.MXPLinksAddon = void 0;

    // MXP标签正则表达式 - 更简单的版本

    const OSC8_REGEX = /\x1b\]8;id=link-\d+;(link-\d+)\x1b\\([^\x1b]+)\x1b\]8;;\x1b\\/g;
    let linkCounter = 0;
    const linkMap = new Map();
    let websocket;
    let link_flag = false;


    // 调试函数

    function debugLog(...args) {
      //console.log('[MXP Debug]', ...args);
    }
  

    class MXPLinksAddon {
      constructor(ws) {
        debugLog('MXPLinksAddon constructor called');
        this.websocket = ws;
        this._buffer = new Uint8Array(0);
        this._decoder = new TextDecoder('utf-8');
        this._encoder = new TextEncoder();
        this.link_flag=false;
      }

    yesno(hint, websocket) {
      // 创建确认框
      const dialog = document.createElement('div');
      dialog.innerHTML = `
      <p>${hint}</p>
      <button id="yesBtn">是</button>
      <button id="noBtn">否</button>
      `;

      // 确认框内联样式（紧凑、12px 字体、圆角）
      dialog.style.position = 'absolute';
      dialog.style.top = '50%';
      dialog.style.left = '50%';
      dialog.style.transform = 'translate(-50%, -50%)';
      dialog.style.background = '#fff';
      dialog.style.color = '#000';
      dialog.style.padding = '10px';
      dialog.style.border = '1px solid #000';
      dialog.style.borderRadius = '8px';
      dialog.style.boxShadow = '0 0 8px rgba(0,0,0,0.3)';
      dialog.style.zIndex = '1000';
      dialog.style.fontFamily = 'monospace';
      dialog.style.fontSize = '12px';
      dialog.style.lineHeight = '1.4';
      dialog.style.textAlign = 'center';

      // 提示文本样式
      const promptText = dialog.querySelector('p');
      promptText.style.margin = '0 0 6px 0';

      // 按钮样式
      const buttons = dialog.querySelectorAll('button');
      buttons.forEach(button => {
        button.style.padding = '4px 8px';
        button.style.margin = '0 4px';
        button.style.fontSize = '12px';
        button.style.border = '1px solid #ccc';
        button.style.borderRadius = '4px';
        button.style.background = '#f0f0f0';
        button.style.cursor = 'pointer';
      });

      // 添加到终端容器
      const terminalContainer = document.getElementById('terminal');
      terminalContainer.style.position = 'relative';
      terminalContainer.appendChild(dialog);

      // 聚焦“是”按钮
      const yesBtn = dialog.querySelector('#yesBtn');
      yesBtn.focus();

      // 点击“是”按钮
      window.yesInput = function(ws) {
        ws.send("yes\n")
        cleanup();
      };

      // 点击“否”按钮
      window.noInput = function(ws) {
        ws.send("no\n")
        cleanup();
      };

      // 清理函数
      function cleanup() {
        terminalContainer.removeChild(dialog);
        delete window.yesInput;
        delete window.noInput;
        document.removeEventListener('keydown', keyHandler);
      }

      // 绑定按钮事件
      yesBtn.addEventListener('click', () => yesInput(websocket));
      dialog.querySelector('#noBtn').addEventListener('click', () => noInput(websocket));

      // 按键处理：Enter 触发“是”，Escape 触发“否”
      function keyHandler(e) {
        if (e.key === 'Enter') {
          yesInput(websocket);
        } else if (e.key === 'Escape') {
          noInput(websocket);
        }
      }
      document.addEventListener('keydown', keyHandler);
    }

      // 处理单行数据
      _processLine(line) {
        try {
          // 匹配 <send> 标签，捕获所有属性和内容
          const MXP_TAG_REGEX = /\x1B\[1z<send\s+([^>]*)>\s*([^<]+)\s*<\/send>/gi;
          // 移除 style，保留 href、send、hint、id
          const ATTR_REGEX = /(href|send|hint|id)\s*=\s*'([^']*)'/gi;

          if (!line || typeof line !== 'string') {
            debugLog('Invalid line input:', line);
            return line;
          }

          if (!line.includes('\x1B[1z')) {
            debugLog('Processing line (no MXP):', line);
            return line; // 没有 MXP 标记，直接返回
          }

          return line.replace(MXP_TAG_REGEX, (match, attrs, displayText) => {
            try {
              debugLog('MXP match:', { match, attrs, displayText });

              // 提取 href, send, hint, id
              let href = '', send = '', hint = '', id = '';
              let attrMatch;
              ATTR_REGEX.lastIndex = 0; // 重置正则索引
              while ((attrMatch = ATTR_REGEX.exec(attrs)) !== null) {
                const [, key, value] = attrMatch;
                debugLog('Attribute match:', { key, value });
                if (key.toLowerCase() === 'href') href = value;
                if (key.toLowerCase() === 'send') send = value;
                if (key.toLowerCase() === 'hint') hint = value;
                if (key.toLowerCase() === 'id') id = value;
              }

              debugLog('Parsed attributes:', { href, send, hint, id, displayText });

              // 构建以 ^ 分隔的 URI（不包含 id）
              const uriParts = [
                href ? encodeURIComponent(href) : '?',
                hint ? encodeURIComponent(hint) : '',
                send ? encodeURIComponent(send) : ''
              ].filter(part => part);
              const uri = uriParts.join('^');

              // 构建 OSC 8 链接，包含 id 参数
              const result = `\x1b]8;${id};${uri}\x1b\\${displayText}\x1b]8;;\x1b\\`;
              debugLog('Generated output:', result);

              // 检查 send 是否为 'YRN'（大小写不敏感）
              if (send && send.toUpperCase() === 'YRN') {
                debugLog('YRN detected, calling yesno with hint:', hint);
                // 显式调用 yesno，确保 this 绑定
                this.yesno.call(this, hint || '请确认操作', this.websocket);
              }

              return result;
            } catch (error) {
              debugLog('Error in replace callback:', error);
              return displayText; // 出错时返回原始文本
            }
          });
        } catch (error) {
          debugLog('Error in _processLine:', error);
          return line;
        }
      }


      // 处理缓冲区数据
      _processBuffer() {
        const text = this._decoder.decode(this._buffer);
        const lines = text.split('\n');
        let processedText = '';
        let remainingBuffer = '';

        // 检查最后一行是否完整
        const lastLine = lines[lines.length - 1];
        const hasIncompleteLine = !lastLine.endsWith('\r') && !lastLine.endsWith('\n');

        // 处理除最后一行外的所有行
        for (let i = 0; i < lines.length - (hasIncompleteLine ? 1 : 0); i++) {
          processedText += this._processLine(lines[i]) + '\n';
        }

        // 处理不完整行

        if(hasIncompleteLine && !lines[lines.length - 1].includes('\x1B[1z')) {
          processedText += lines[lines.length - 1];
          this._buffer = new Uint8Array(0);
          return processedText;
        }

        if (hasIncompleteLine) {
          remainingBuffer = lastLine;
          this._buffer = this._encoder.encode(remainingBuffer);
        } else {
          this._buffer = new Uint8Array(0);
        }
        return processedText;
      }

      decodeUriToArray(uri) {
        try {
          // 解码 URI 并以 && 分隔
          const decoded = decodeURIComponent(uri);
          const [href='', hint = '', send = ''] = decoded.split('^');
          return [href, hint, send];
        } catch (e) {
          console.error('Invalid URI component:', e);
          return ['', '', '']; // 返回空值数组作为默认
        }
      }

      

      
      image_scr(image, hint) {
        // 创建图片元素
        const img = document.createElement('img');
        img.id = 'displayImage';
        img.src = image;
        img.alt = 'Image';

        // 图片样式（居中、圆角）
        img.style.position = 'absolute';
        img.style.top = '50%';
        img.style.left = '50%';
        img.style.transform = 'translate(-50%, -50%)';
        img.style.borderRadius = '4px';
        img.style.zIndex = '1000';
        img.style.cursor = 'pointer';

        // 添加到终端容器
        const terminalContainer = document.getElementById('terminal');
        terminalContainer.style.position = 'relative';
        terminalContainer.appendChild(img);

        // 自适应终端宽度
        function resizeImage() {
          const terminalWidth = terminalContainer.offsetWidth;
          const maxImageWidth = terminalWidth * 0.9; // 留 10% 边距

          // 等待图片加载以获取自然尺寸
          if (img.naturalWidth && img.naturalWidth > 0) {
            if (img.naturalWidth <= maxImageWidth) {
              // 图片小于或等于终端宽度，不缩放
              img.style.width = `${img.naturalWidth}px`;
              img.style.height = `${img.naturalHeight}px`;
            } else {
              // 图片大于终端宽度，按比例缩放
              img.style.maxWidth = `${maxImageWidth}px`;
              img.style.width = '100%';
              img.style.height = 'auto';
            }
          }
        }

        // 图片加载完成后调整大小
        img.onload = () => {
          resizeImage();
        };

        // 监听终端容器大小变化
        const resizeObserver = new ResizeObserver(resizeImage);
        resizeObserver.observe(terminalContainer);

        // 图片加载错误处理
        img.onerror = () => {
          term.writeln('图片加载失败');
          cleanup();
        };

        // 清理函数
        function cleanup() {
          resizeObserver.disconnect();
          terminalContainer.removeChild(img);
          document.removeEventListener('keydown', keyHandler);
          term.writeln('图片已关闭');
        }

        // 点击图片发送 hint 并关闭
        img.addEventListener('click', () => {
          term.writeln(hint); // 点击时发送 hint
          cleanup();
        });

        // 按 Enter 或 Escape 关闭
        function keyHandler(e) {
          if (e.key === 'Enter' || e.key === 'Escape') {
            cleanup();
          }
        }
        document.addEventListener('keydown', keyHandler);
      }
     
    generateMenu(tags, position) {
      const commands = tags[0].split('|');
      const menuItems = tags[1].split('|');
      const ws = window.globalWebSocket;

      // 确保指令和项目名数量匹配
      debugLog("menu", tags[0], tags[1]);
      if (commands.length !== menuItems.length) {
        console.error('命令和菜单项数量不匹配');
        return null;
      }

      // 创建菜单 DOM
      const menu = document.createElement('div');
      menu.style.position = 'absolute';
      menu.style.background = '#ffffff';
      menu.style.border = '1px solid #e0e0e0';
      menu.style.borderRadius = '4px'; // 添加小圆角
      menu.style.boxShadow = '0 2px 8px rgba(0,0,0,0.15)'; // 优化阴影效果
      menu.style.zIndex = '999999';
      menu.style.display = 'block';
      menu.style.fontFamily = 'system-ui, -apple-system, sans-serif'; // 使用更现代的字体
      menu.style.fontSize = '12px'; // 略微增大字体
      menu.style.left = `${position.x}px`;

      // 判断鼠标位置并设置菜单方向
      const isMouseInTopThird = position.y < window.innerHeight / 3;
      menu.style.top = isMouseInTopThird
        ? `${position.y}px`
        : `${position.y - menu.offsetHeight}px`;

      // 生成菜单项
      menuItems.forEach((item, index) => {
        const menuItem = document.createElement('div');
        menuItem.textContent = item;
        menuItem.style.padding = '5px 10px'; // 增加内边距
        menuItem.style.cursor = 'pointer';
        menuItem.style.whiteSpace = 'nowrap';
        menuItem.style.transition = 'background 0.2s ease'; // 添加平滑过渡效果
        menuItem.addEventListener('click', () => {   
          const command = commands[index];
          // 检查前五个字符是否为 '#DIAL '
          if (command.startsWith('#DIAL ')) {
            // 提取第6个字符开始的子字符串
            const dialArgs = command.substring(6);
            debugLog('Calling diag with:', { dialArgs, menuItem: item });
            this.diag(item,dialArgs);
          } else {
            this.websocket.send(command + '\n');
          }
          menu.remove();
        });
        menuItem.addEventListener('mouseover', () => {
          menuItem.style.background = '#e5e5e5'; // 优化悬停背景色
        });
        menuItem.addEventListener('mouseout', () => {
          menuItem.style.background = 'transparent';
        });
        menu.appendChild(menuItem);
      });

      document.body.appendChild(menu);

      // 菜单向上生成时调整位置
      if (!isMouseInTopThird) {
        menu.style.top = `${position.y - menu.offsetHeight}px`;
      }

      return menu;
}

      diag(hint, cmd) {
        // 创建对话框
        const dialog = document.createElement('div');
        dialog.innerHTML = `
        <p>${hint}</p>
        <input type="text" id="userInput">
        <button id="submitBtn">确定</button>
        <button id="cancelBtn">取消</button>
        `;

        // 对话框内联样式（紧凑、12px 字体、圆角）
        dialog.style.position = 'absolute';
        dialog.style.top = '50%';
        dialog.style.left = '50%';
        dialog.style.transform = 'translate(-50%, -50%)';
        dialog.style.background = '#fff';
        dialog.style.color = '#000';
        dialog.style.padding = '10px';
        dialog.style.border = '1px solid #000';
        dialog.style.borderRadius = '8px';
        dialog.style.boxShadow = '0 0 8px rgba(0,0,0,0.3)';
        dialog.style.zIndex = '1000';
        dialog.style.fontFamily = 'monospace';
        dialog.style.fontSize = '12px';
        dialog.style.lineHeight = '1.4';

        // 提示文本样式
        const promptText = dialog.querySelector('p');
        promptText.style.margin = '0 0 6px 0';

        // 输入框样式
        const input = dialog.querySelector('#userInput');
        input.style.margin = '0 0 6px 0';
        input.style.padding = '4px';
        input.style.width = '150px';
        input.style.fontSize = '12px';
        input.style.border = '1px solid #ccc';
        input.style.borderRadius = '4px';

        // 按钮样式
        const buttons = dialog.querySelectorAll('button');
        buttons.forEach(button => {
          button.style.padding = '4px 8px';
          button.style.margin = '0 4px';
          button.style.fontSize = '12px';
          button.style.border = '1px solid #ccc';
          button.style.borderRadius = '4px';
          button.style.background = '#f0f0f0';
          button.style.cursor = 'pointer';
        });

        // 添加到终端容器
        const terminalContainer = document.getElementById('terminal');
        terminalContainer.style.position = 'relative';
        terminalContainer.appendChild(dialog);

        // 聚焦输入框
        input.focus();

        // 提交输入
        window.submitInput = function(ws) {
          const userInput = input.value;
          if (userInput != '') {          
            const output = `${cmd+' '}${userInput+'\n'}`;
            debugLog('send',output);
            ws.send(output);
          }
          cleanup();
        };

        // 取消输入
        window.cancelInput = function() {
          term.writeln('输入已取消');
          cleanup();
        };

        // 清理函数
        function cleanup() {
          terminalContainer.removeChild(dialog);
          delete window.submitInput;
          delete window.cancelInput;
        }

        // 程序化绑定按钮事件
        dialog.querySelector('#submitBtn').addEventListener('click', () => submitInput(this.websocket));
        dialog.querySelector('#cancelBtn').addEventListener('click', () => cancelInput());

        // 按键处理：Enter 提交，Escape 取消
        input.addEventListener('keydown', (e) => {
          if (e.key === 'Enter') {
            submitInput(context); // 传递 WebSocket 句柄
          } else if (e.key === 'Escape') {
            cancelInput();
          }
        });
      }

      
      // 激活插件
      activate(terminal) {
        debugLog('MXPLinksAddon activated');
        this._terminal = terminal;
        this.terminalElement = terminal.element;
        this.terminalElement.addEventListener('contextmenu', (event) => {
          debugLog("sssss",this.link_flag);
          if(this.link_flag) {
            debugLog("ban default_menu");
            event.preventDefault();
            event.stopPropagation();
          }
        });

        const handler = {
          allowNonHttpProtocols: true, // 允许非 http/https 协议
          activate: (event, text, range) => {
            const tag = this.decodeUriToArray(text);
            if(tag[2]==='DIAL' && event.button === 0){      
                this.diag(tag[1],tag[0]);
            }
            else if(tag[2]==='IMG' && event.button === 0){      
                this.image_scr(tag[1],tag[0]);
            }
            else if(tag[2]==='YRN' && event.button === 0){      
                this.yesno(tag[1]);
            }
            else if (event.button === 0) {
              const cmd = tag[0].split('|');
              debugLog('left click activated:', cmd);
              this.websocket.send(cmd[0] + '\n');

              // 左键点击时关闭菜单
              if (this.currentMenu) {
                this.currentMenu.remove();
                document.removeEventListener('click', this.hideMenu);
                this.currentMenu = null;
                this.currentLinkElement = null;
              }
            } else if (event.button === 2) {
              this.link_flag=true;
              event.preventDefault(); // 阻止浏览器默认右键菜单

              const tags = this.decodeUriToArray(text);
              debugLog('tags:', text,tags);
              const menu = this.generateMenu(tags, {
                x: event.clientX,
                y: event.clientY
              });

              if (!menu) return;

              const linkElement = event.target; // 触发右键事件的链接元素

              // 移除现有菜单
              if (this.currentMenu) {
                this.currentMenu.remove();
                document.removeEventListener('click', this.hideMenu);
              }

              // 存储当前菜单和链接元素
              this.currentMenu = menu;
              this.currentLinkElement = linkElement;

              // 定义关闭菜单的函数
              this.hideMenu = (e) => {
                const isOutsideMenu = !menu.contains(e.target);
                const isOutsideLink = !linkElement.contains(e.target);

                // 点击菜单或链接外的区域时关闭菜单
                if (isOutsideMenu && isOutsideLink) {
                  menu.remove();
                  document.removeEventListener('click', this.hideMenu);
                  this.currentMenu = null;
                  this.currentLinkElement = null;
                }
              };

              // 添加点击事件监听以关闭菜单
              document.addEventListener('click', this.hideMenu);
            }
          },
          contextmenu: () => {}, // 禁用 contextmenu 事件
          hover: () => {this.link_flag=true;}, // 禁用悬停事件
          leave: () => {this.link_flag=false;}  // 禁用离开事件
        };

        terminal.options.linkHandler = handler;
        // 拦截 write 方法
        const originalWrite = this._terminal.write.bind(this._terminal);
        this._terminal.write = (data) => {
          try {
            if (data instanceof Uint8Array) {
              // 合并新数据到缓冲区
              const newBuffer = new Uint8Array(this._buffer.length + data.length);
              newBuffer.set(this._buffer);
              newBuffer.set(data, this._buffer.length);
              this._buffer = newBuffer;

              // 检查是否有完整行
              const text = this._decoder.decode(this._buffer);
              const lines = text.split('\n');
              if (lines.length > 1) {
                const processedText = this._processBuffer();
                if (processedText) {
                  return originalWrite(processedText);
                }
              }
              return;
            }
            // 直接处理字符串数据
            const processed = this._processLine(data);
            return originalWrite(processed);
          } catch (error) {
            return originalWrite(data); // 出错时回退
          }
        };
      }
    }
    t.MXPLinksAddon = MXPLinksAddon;
  }])
}));