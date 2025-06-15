/**
* Determine the mobile operating system.
*
* @returns {Boolean}
*/
function isMobile() {
  const userAgent = navigator.userAgent || navigator.vendor || window.opera;
  // Windows Phone must come first because its UA also contains "Android"
  if (/windows phone/i.test(userAgent)) {
    return true;
  }

  if (/android/i.test(userAgent)) {
    return true;
  }

  // iOS detection from: http://stackoverflow.com/a/9039885/177710
  if (/iPad|iPhone|iPod/.test(userAgent) && !window.MSStream) {
    return true;
  }

  return false;
}


const WebFontSize = 14;
const MobileFontSize = 24;
const term = new Terminal({
  allowProposedApi:true,
  convertEol: true,
  disableStdin: true,
  letterSpacing: 0,
  customGlyphs: false,
  fontFamily: 'Noto Sans Mono',
  fontSize: isMobile() ?  MobileFontSize: WebFontSize,
  rightClickSelectsWord: true,
  linkHandler: {
    allowNonHttpProtocols: true, // 允许非 http/https 协议
    activate: (event, text, range) => {
      console.log(`Clicked: ${text}`);
      // 自定义点击处理，例如：window.open(text, '_blank');
    },
    hover: (event, text, range) => {
      console.log(`Hover: ${text}`);
      // 显示 tooltip
    },
    leave: (event, text, range) => {
      // 隐藏 tooltip
    }
  }
});



const unicode14Addon = new Unicode14Addon(true);
term.loadAddon(unicode14Addon);
term.unicode.activeVersion = '14';

const fitAddon  = new FitAddon.FitAddon();
term.loadAddon(fitAddon);

const weblinksAddon = new WebLinksAddon.WebLinksAddon();
term.loadAddon(weblinksAddon);

// Handle MXP link clicks

let history = [];
let history_index = -1;
let current_command = "";


function get_appropriate_ws_url(extra_url) {
  let pcol;
  let u = document.URL;

  /*
  * We open the websocket encrypted if this page came on an
  * https:// url itself, otherwise unencrypted
  */

  if (u.substring(0, 5) === "https") {
    pcol = "wss://";
    u = u.substr(8);
  } else {
    pcol = "ws://";
    if (u.substring(0, 4) === "http")
    u = u.substr(7);
  }

  u = u.split("/");

  /* + "/xxx" bit is for IE10 workaround */

  return pcol + u[0] + "/" + extra_url;
}

function new_ws(urlpath, protocol) {
  if (typeof MozWebSocket != "undefined")
  return new MozWebSocket(urlpath, protocol);

  return new WebSocket(urlpath, protocol);
}

function onload() {
  const el_container = document.getElementById('container');
  const el_terminal = document.getElementById('terminal');
  const el_input = document.getElementById('command');
  const containerEl = document.getElementById('container');
  
  const terminalEl = document.getElementById('terminal');
  let reflag = true;

  function updateTerminal(ws) {
    const viewportHeight = window.innerHeight;
    this.reflag = true;
    console.log("ut",this.reflag);
    // 清除可能的重置样式
    containerEl.style.height = '';
    terminalEl.style.height = '';
    // 设置高度
    containerEl.style.height = `${viewportHeight}px`;
    terminalEl.style.height = `${viewportHeight-28}px`;
    // 强制刷新 xterm.js
    term._core._renderService.clear();
    term.refresh(0, term.rows - 1);
    fitAddon.fit();
        
    if (ws && ws.readyState === WebSocket.OPEN) {
      const dimensions = {
        type: 'resize',
        cols: term.cols,
        rows: term.rows
      };
      console.log("resize",dimensions);
      ws.send(JSON.stringify(dimensions)+"\n");
    }
  }


  function login(websocket) {
    // 创建对话框
    const dialog = document.createElement('div');
    dialog.innerHTML = `
    <p>请输入用户名和密码</p>
    <div>
    <label>用户名: </label>
    <input type="text" id="username" placeholder="用户名">
    </div>
    <div>
    <label>密码: </label>
    <input type="password" id="password" placeholder="密码">
    </div>
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

    // 输入框容器样式
    const inputDivs = dialog.querySelectorAll('div');
    inputDivs.forEach(div => {
      div.style.margin = '0 0 6px 0';
      div.style.display = 'flex';
      div.style.alignItems = 'center';
    });

    // 标签样式
    const labels = dialog.querySelectorAll('label');
    labels.forEach(label => {
      label.style.width = '60px';
      label.style.marginRight = '5px';
    });

    // 输入框样式
    const inputs = dialog.querySelectorAll('input');
    inputs.forEach(input => {
      input.style.padding = '4px';
      input.style.width = '150px';
      input.style.fontSize = '12px';
      input.style.border = '1px solid #ccc';
      input.style.borderRadius = '4px';
    });

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

    // 聚焦用户名输入框
    const usernameInput = dialog.querySelector('#username');
    usernameInput.focus();

    // 提交输入
    window.submitInput = function(ws) {
      const username = usernameInput.value;
      const password = dialog.querySelector('#password').value;
      if (username === '' || password === '') {
        login(ws);
      } else {
        let output = `${'n\n'+username+'\n'+password+'\n'+'y\n'}`
        ws.send(output);
      }
      cleanup();
    };

    // 取消输入
    window.cancelInput = function() {
      term.writeln('登录已取消');
      cleanup();
    };

    // 清理函数
    function cleanup() {
      terminalContainer.removeChild(dialog);
      delete window.submitInput;
      delete window.cancelInput;
    }

    // 绑定按钮事件
    dialog.querySelector('#submitBtn').addEventListener('click', () => submitInput(websocket));
    dialog.querySelector('#cancelBtn').addEventListener('click', () => cancelInput());

    // 按键处理：Enter 提交，Escape 取消
    inputs.forEach(input => {
      input.addEventListener('keydown', (e) => {
        if (e.key === 'Enter') {
          submitInput(websocket);
        } else if (e.key === 'Escape') {
          cancelInput();
        }
      });
    });
  }

  window.addEventListener('scroll', function(e) {
    window.scrollTo(0,0);
    fitAddon.fit();
    e.preventDefault();
    e.stopPropagation();
  });

  window.addEventListener('touchmove', function(e) {
    e.preventDefault();
    e.stopPropagation();
  });

  window.addEventListener('resize', function (e) {
    console.log("resize",reflag);
    if(!reflag) return;
    reflag = false;    
  //  console.log("resize s",reflag);
    setTimeout(() => {      
      {reflag = true;
        updateTerminal(ws);}
    }, 500);
  });

  if(isMobile()) {
// el_container.style.height = "55vh";
    fitAddon.fit();
  } else {
    el_container.style.height = $(window).height();
    fitAddon.fit();
  }

  el_input.setAttribute("disabled", "disabled");
  el_input.style.fontFamily = term.options.fontFamily;
  el_input.style.fontSize = term.options.fontSize + "px";

  term.open(el_terminal);
  fitAddon.fit();

  let ws = new_ws(get_appropriate_ws_url(""), "ascii");


  ws.binaryType = 'arraybuffer';
  ws.bufferedAmount = 1024 * 1024;
  const attachAddon = new AttachAddon.AttachAddon(ws);
  term.loadAddon(attachAddon);
  const mxpAddon = new MXPLinksAddon.MXPLinksAddon(ws);
  term.loadAddon(mxpAddon);

  el_input.focus();

  try {
    ws.onopen = function () {
      // 发送初始终端大小

      term.writeln("\033[9999;1H");
      const dimensions = {
        type: 'resize',
        cols: term.cols,
        rows: term.rows
      };
      ws.send(JSON.stringify(dimensions)+"\n");

      const mxpSupport = {
        type: 'mxp',
        version: '1.0',
        features: ['links', 'commands', 'colors']
      };
      ws.send(JSON.stringify(mxpSupport));

      el_input.removeAttribute("disabled");
      el_input.focus();
    };
    ws.onclose = function () {
      term.writeln("\033[9999;1H");
      term.writeln("\r\n======== Connection Lost.\r\n");
      el_input.setAttribute("disabled", "disabled");
      el_input.focus();
    };
  } catch (exception) {
    alert("<p>Error " + exception);
  }

  //  el_input.addEventListener("blur", function(e) {
  //    e.stopPropagation();
  //    e.preventDefault();
  //
  //    el_input.focus();
  //  });

  el_input.addEventListener("keydown", function(e) {
    if (e.key === "Tab") {
      e.stopPropagation();
      e.preventDefault();
      return ;
    }

    if (e.key === "Enter") {
      if(el_input.value) {
        history.push(el_input.value);
        if (history.length > 20) {
          history.shift();
        }
        history_index = -1;
      }

      var content = el_input.value + "\n";
      ws.send(content);
      term.writeln(content + "\r");
      el_input.value = "";

    } else if (e.key === "ArrowUp") {
      e.stopPropagation();
      e.preventDefault();

      // If there is history, show last one
      if(history.length > 0) {
        // remember current command
        if(history_index === -1) {
          current_command = el_input.value;
        }
        if (history_index < history.length - 1) {
          history_index = history_index + 1;
          el_input.value = history[history.length - history_index - 1];
        }
      }
    } else if (e.key === "ArrowDown") {
      e.stopPropagation();
      e.preventDefault();

      if(history_index > -1) {
        history_index = history_index - 1;
        if(history_index >= 0) {
          el_input.value = history[history.length - history_index - 1];
        } else {
          el_input.value = current_command;
        }
      }
    }
  });
  login(ws);
}


document.addEventListener("DOMContentLoaded", function () {
  const terminalContainer = document.getElementById('terminal');
  const commandBox = document.getElementById('command');

  if (!terminalContainer || !commandBox) {
    console.error('Required elements not found: terminal or command-box');
    return;
  }

//  // 鼠标进入终端时聚焦
//  terminalContainer.addEventListener('mouseenter', () => {
//    term.focus();
//  });

  commandBox.addEventListener('mouseenter', () => {
    term.focus();
  });

  // 保护终端选中文本（可选）
//  commandBox.addEventListener('focus', () => {
//    if (term.hasSelection()) {
//      term.focus();
//    }
//  });
  
  new FontFaceObserver("Noto Sans Mono").load().then(function () {
    onload();
  });
}, false);
