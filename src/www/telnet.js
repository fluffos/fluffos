// telnet.js -- the browser-side telnet client shared by the FluffOS web
// terminals (src/www/index.html over websocket, src/www/wasm/index.html over
// the wasm byte bridge).  Transport-agnostic: bytes in via receive(),
// bytes out via the sendBytes callback.
//
// The driver signals input modes through option negotiation
// (src/net/telnet.cc):
//   WILL/WONT ECHO  server takes over / returns echo (password prompts,
//                   char mode with I_NOECHO)         -> onEcho(on)
//   WILL/WONT SGA   enter/leave char mode (set_charmode/set_linemode)
//                                                    -> onCharMode(on)
//   DO NAWS         please report window size (request_term_size())
//   DO TTYPE        please report terminal type
//
// Anything else is refused unless the page opts in through the hooks:
//   onRemoteOption(opt) -> truthy accepts a server WILL <opt> (DO is sent,
//                          and the return value is invoked if callable)
//   onLocalOption(opt)  -> truthy accepts a server DO <opt> (WILL is sent)
//   onSubneg(opt, bytes)   subnegotiation payloads for those options
//                          (TTYPE is answered internally)

const TelnetClient = (() => {
  'use strict';

  const IAC = 255, DONT = 254, DO = 253, WONT = 252, WILL = 251, SB = 250, SE = 240;
  const OPT_ECHO = 1, OPT_SGA = 3, OPT_TTYPE = 24, OPT_NAWS = 31;
  const TTYPE_IS = 0, TTYPE_SEND = 1;

  class TelnetClient {
    constructor(sendBytes, opts = {}) {
      this.sendBytes = sendBytes;
      this.state = 'data';
      this.decoder = new TextDecoder('utf-8');
      this.onText = () => {};
      this.onEcho = () => {};
      this.onCharMode = () => {};
      this.onSubneg = () => {};
      this.onRemoteOption = () => false;
      this.onLocalOption = () => false;
      this.remote = {};            // options the server announced WILL for
      this.local = {};             // options we announced WILL for
      this.sbBuf = [];
      this.termType = opts.termType || 'xterm-256color';
      this.size = { cols: 80, rows: 24 };
    }

    // application data with IAC bytes doubled
    sendRaw(bytes) {
      const out = [];
      for (const b of bytes) { out.push(b); if (b === IAC) out.push(IAC); }
      this.sendBytes(out);
    }

    // payload bytes with IAC doubled
    sendSubneg(opt, payload) {
      const out = [IAC, SB, opt];
      for (const b of payload) { out.push(b); if (b === IAC) out.push(IAC); }
      out.push(IAC, SE);
      this.sendBytes(out);
    }

    sendNaws() {
      if (!this.local[OPT_NAWS]) return;
      const { cols, rows } = this.size;
      this.sendSubneg(OPT_NAWS, [cols >> 8, cols & 0xff, rows >> 8, rows & 0xff]);
    }

    setSize(cols, rows) {
      this.size = { cols, rows };
      this.sendNaws();
    }

    handleWill(opt) {
      if (opt === OPT_ECHO) {
        if (!this.remote[opt]) { this.remote[opt] = true; this.sendBytes([IAC, DO, opt]); }
        this.onEcho(true);
      } else if (opt === OPT_SGA) {
        if (!this.remote[opt]) { this.remote[opt] = true; this.sendBytes([IAC, DO, opt]); }
        this.onCharMode(true);
      } else {
        const accept = this.onRemoteOption(opt);
        if (accept) {
          if (!this.remote[opt]) {
            this.remote[opt] = true;
            this.sendBytes([IAC, DO, opt]);
            if (typeof accept === 'function') accept(opt);
          }
        } else {
          this.sendBytes([IAC, DONT, opt]);
        }
      }
    }

    handleWont(opt) {
      if (this.remote[opt]) { this.remote[opt] = false; this.sendBytes([IAC, DONT, opt]); }
      if (opt === OPT_ECHO) this.onEcho(false);
      if (opt === OPT_SGA) this.onCharMode(false);
    }

    handleDo(opt) {
      if (opt === OPT_NAWS) {
        if (!this.local[opt]) {
          this.local[opt] = true;
          this.sendBytes([IAC, WILL, opt]);
          this.sendNaws();
        }
      } else if (opt === OPT_TTYPE) {
        if (!this.local[opt]) { this.local[opt] = true; this.sendBytes([IAC, WILL, opt]); }
      } else if (this.onLocalOption(opt)) {
        if (!this.local[opt]) { this.local[opt] = true; this.sendBytes([IAC, WILL, opt]); }
      } else {
        this.sendBytes([IAC, WONT, opt]);
      }
    }

    handleDont(opt) {
      if (this.local[opt]) { this.local[opt] = false; this.sendBytes([IAC, WONT, opt]); }
    }

    handleSubneg(bytes) {
      if (!bytes.length) return;
      const opt = bytes[0];
      if (opt === OPT_TTYPE && bytes[1] === TTYPE_SEND) {
        const name = Array.from(this.termType, (c) => c.charCodeAt(0));
        this.sendSubneg(OPT_TTYPE, [TTYPE_IS, ...name]);
        return;
      }
      this.onSubneg(opt, bytes.slice(1));
    }

    // Feed one received chunk.  The state machine survives sequences (and
    // UTF-8 characters -- stream: true) split across chunks.
    receive(bytes) {
      const text = [];
      for (const b of bytes) {
        switch (this.state) {
          case 'data':
            if (b === IAC) this.state = 'iac';
            else if (b !== 0) text.push(b);
            break;
          case 'iac':
            if (b === IAC) { text.push(b); this.state = 'data'; }
            else if (b === WILL || b === WONT || b === DO || b === DONT) {
              this.verb = b; this.state = 'opt';
            } else if (b === SB) { this.state = 'sb'; this.sbBuf = []; }
            else this.state = 'data'; // GA, NOP, ...
            break;
          case 'opt': {
            if (this.verb === WILL) this.handleWill(b);
            else if (this.verb === WONT) this.handleWont(b);
            else if (this.verb === DO) this.handleDo(b);
            else this.handleDont(b);
            this.state = 'data';
            break;
          }
          case 'sb':
            if (b === IAC) this.state = 'sb-iac';
            else this.sbBuf.push(b);
            break;
          case 'sb-iac':
            if (b === SE) { this.handleSubneg(this.sbBuf); this.sbBuf = []; this.state = 'data'; }
            else { this.sbBuf.push(b); this.state = 'sb'; }   // IAC IAC in payload
            break;
        }
      }
      if (text.length) {
        this.onText(this.decoder.decode(new Uint8Array(text), { stream: true }));
      }
    }
  }

  TelnetClient.OPT = {
    ECHO: OPT_ECHO, SGA: OPT_SGA, TTYPE: OPT_TTYPE, NAWS: OPT_NAWS,
    MSP: 90, MXP: 91, GMCP: 201,
  };
  return TelnetClient;
})();
