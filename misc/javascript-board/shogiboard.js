// font data is from kbanner2
var font = new Array;
font["FU"]=[0x0100,0x1100,0x11f8,0x1100,0x1100,0xffff,0x0100,0x0910,0x1918,0x310c,0xe117,0x0730,0x0060,0x01c0,0x0700,0x3c00]; // fu
font["KY"]=[0x0018,0x00f0,0x3f80,0x0100,0xffff,0x0520,0x0d30,0x1918,0x310c,0xdffb,0x1008,0x1008,0x1ff8,0x1008,0x1008,0x1ff8]; // kyo
font["KE"]=[0x1020,0x1020,0x11fe,0xfc20,0x1020,0x1020,0x3bff,0x3400,0x3420,0x5020,0x51fe,0x9020,0x1020,0x1020,0x13ff,0x1000]; // kei
font["GI"]=[0x1000,0x38fe,0x2c82,0x6682,0x42fe,0xfc82,0x1082,0x10fe,0xfe91,0x1093,0x9496,0xd498,0x5088,0x1e8c,0x70f6,0xc383]; // gin
font["KI"]=[0x0100,0x0380,0x06c0,0x0c60,0x1830,0x301c,0xeff7,0x0100,0x0100,0x7ffe,0x0100,0x2104,0x310c,0x1918,0x0910,0xffff]; // kin
font["KA"]=[0x0400,0x0ff0,0x1820,0x3040,0xdffc,0x1084,0x1084,0x1ffc,0x1084,0x1084,0x1ffc,0x1004,0x3004,0x2004,0x6004,0xc01c]; // kaku
font["HI"]=[0x0000,0x7ff2,0x0014,0x051c,0x1d12,0x7108,0x1105,0xfff3,0x1110,0x1114,0x1118,0x1114,0x3112,0x2118,0x610d,0xc107]; // hi
font["OU"]=[0x0000,0x7ffe,0x0100,0x0100,0x0100,0x0100,0x0100,0x3ffc,0x0100,0x0100,0x0100,0x0100,0x0100,0x0100,0xffff,0x0000]; // ou
font["TO"]=[0x0000,0x1000,0x1800,0x0800,0x0c0c,0x0438,0x06e0,0x0380,0x0600,0x0c00,0x1800,0x1000,0x1000,0x1800,0x0ffc,0x0000]; // to
font["NK"]=[0x0100,0x0100,0x0100,0xffff,0x0540,0x0d60,0x1930,0x311c,0xe107,0x0000,0x3ffc,0x2004,0x2004,0x2004,0x2004,0x3ffc]; // nari-kyo
font["NG"]=[0x0100,0x0380,0x06c0,0x0c70,0x381c,0xe007,0x1ff8,0x0100,0x0100,0x0100,0x1ff8,0x0100,0x0100,0x0100,0x7ffe,0x0000]; // nari-gin
font["UM"]=[0x0000,0x1ffc,0x1080,0x1080,0x1ff8,0x1080,0x1080,0x1ff8,0x1080,0x1080,0x1ffe,0x0002,0x4922,0xcdb2,0x8496,0x000c]; // uma
font["RY"]=[0x0840,0x0840,0x7f7f,0x2240,0x147e,0xff82,0x007e,0x3e40,0x227e,0x3e40,0x227e,0x3e40,0x227e,0x2241,0x2263,0x263e]; // ryu
font["GYOKU"]=[0x0000,0x7ffe,0x0100,0x0100,0x0100,0x0100,0x0100,0x3ffc,0x0100,0x0100,0x0118,0x010c,0x0104,0x0100,0xffff,0x0000]; // gyoku
font[""]=[0x0000,0x000,0x0000,0x0000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x0000,0x000,0x000,0x000,0x000];

function printPiece(x, y, index, rotate) {
  var cell = document.getElementById("shogiban").rows[y].cells[x];
  var piece;
  if (rotate) {
    piece = printTextCommon(index, rotate180);
  } else {
    piece = printTextCommon(index);
  }
  if (!cell.hasChildNodes()) {
    cell.appendChild(piece);
  } else {
    cell.replaceChild(piece, cell.lastChild);
  }
}

function printBoard(text) {
  var rows = text.split("\n");
  for (var i in rows) {
    var chars = rows[i].split(",");
    for (var j = 0; j < chars.length; j++) {
      printPiece(j, i, chars[j].substr(1), chars[j][0] == '-');
    }
  }
}