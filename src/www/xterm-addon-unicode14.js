const UnicodeV14 = /** @class */ (function () {
  function UnicodeV14() {
    this.version = '14';
  }

  UnicodeV14.prototype.wcwidth = function (num) {
    let x = widechar_wcwidth(num);
    if (x === widechar_widened_in_9) x = 2;
    if (x === widechar_private_use) x = 1;
    if (x === widechar_ambiguous) x = 1;
    if (x < 0) x = 0;
    return x;
  }
  return UnicodeV14;
}());

const Unicode14Addon = /** @class */ (function () {
  function Unicode14Addon() {
  }

  Unicode14Addon.prototype.activate = function (terminal) {
    terminal.unicode.register(new UnicodeV14());
  };
  Unicode14Addon.prototype.dispose = function () {
  };
  return Unicode14Addon;
}());
