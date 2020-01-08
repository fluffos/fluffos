const term = new Terminal({
  rows: 50,
  cols: 120,
  convertEol: true,
  disableStdin: true,
  scrollback: 1024,
  tabStopWidth: 4,
  fontFamily: 'SFMono-Regular,Consolas,Liberation Mono,Menlo,monospace, "Microsoft YaHei", SimSun',
  fontSize: 16
});

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

document.addEventListener("DOMContentLoaded", function () {
  const el_terminal = document.getElementById('terminal');
  const el_input = document.getElementById('command');
  el_input.setAttribute("disabled", "disabled");
  el_input.style.fontFamily = term.getOption("fontFamily");
  el_input.style.fontSize = term.getOption("fontSize") + "px";

  term.open(el_terminal);

  let ws = new_ws(get_appropriate_ws_url(""), "ascii");
  const attachAddon = new AttachAddon.AttachAddon(ws);
  term.loadAddon(attachAddon);

  try {
    ws.onopen = function () {
      term.write("\r\n======== Connected.\r\n");
      el_input.removeAttribute("disabled");
      el_input.focus();
    };
    ws.onclose = function () {
      term.write("\r\n======== Connection Lost.\r\n");
      el_input.setAttribute("disabled", "disabled");
    };
  } catch (exception) {
    alert("<p>Error " + exception);
  }

  el_input.addEventListener("blur", function(e) {
    e.stopPropagation();
    e.preventDefault();

    el_input.focus();
  });

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
      term.write(content + "\r");
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

}, false);
