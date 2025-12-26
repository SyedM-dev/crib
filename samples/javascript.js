/* ===============================
 * JavaScript Syntax Torture Test
 * =============================== */

'use strict';

// === Imports ===
import fs, { readFileSync as rfs } from "fs";
import * as path from "path";
import defaultExport, { named as alias } from "./module.js";

// === Constants ===
const PI = 3.141592653589793;
const HEX = 0xff;
const BIN = 0b101010;
const OCT = 0o755;
const BIG = 123_456_789n;

// === Variables ===
let x = null;
var y = undefined;
let z = NaN;

// === Strings ===
const s1 = "double quotes";
const s2 = 'single quotes';
const s3 = `template literal ${1 + 2}`;
const s4 = `multi
line
template`;
const s5 = String.raw`raw \n string`;

// === Escapes ===
const esc = "\n\t\r\b\f\\\"\'\u00A9\x41";

// === Arrays & Objects ===
const arr = [1, , 3, ...[4, 5], { a: 1, b: { c: 2 } }];
const obj = {
  key: "value",
  "weird-key": 123,
  ['dyn' + 'amic']: true,
  method() {},
  async asyncMethod() {},
  *generator() { yield 1; },
};

// === Destructuring ===
const { a, b: renamed, ...rest } = obj;
const [x1, , x3 = 42] = arr;

// === Functions ===
function normal(a, b = 1, ...rest) {
  return a + b + rest.length;
}

const arrow = (x = 0) => x * x;
const asyncArrow = async () => await Promise.resolve(42);

// === Classes ===
class Example extends Array {
  static staticField = 123;
  #privateField = "secret";

  constructor(...args) {
    super(...args);
  }

  get value() {
    return this.#privateField;
  }

  set value(v) {
    this.#privateField = v;
  }
}

// === Control Flow ===
if (true && !false || null ?? true) {
  console.log("truthy");
} else if (false) {
  console.warn("nope");
} else {
  console.error("never");
}

for (let i = 0; i < 3; i++) {
  continue;
}

for (const k in obj) {}
for (const v of arr) {}

while (false) {}
do {} while (false);

switch (Math.random()) {
  case 0:
    break;
  default:
    break;
}

// === Try / Catch ===
try {
  throw new Error("boom");
} catch (e) {
  console.error(e?.message ?? "unknown");
} finally {
  // cleanup
}

// === Regex ===
const regex1 = /foo|bar/i;
const regex2 = /^<script\b(?![^>]*\btype\s*=\s*"(?!module|text\/javascript)[^"]*")[^>]*>$/;

// === Tagged template ===
function tag(strings, ...values) {
  return strings.raw.join("|") + values.join(",");
}
tag`hello ${42} world`;

// === Optional chaining / nullish ===
const deep = obj?.a?.b ?? "fallback";

// === Bitwise ===
const mask = (1 << 4) | (1 << 8);

// === JSON ===
const json = JSON.stringify({ a: 1, b: [true, false] }, null, 2);

// === Top-level await (if supported) ===
await Promise.resolve("done");

// === JSX-like (should still highlight interestingly) ===
const jsx = (
  <Component prop="value">
    <Child />
  </Component>
);

// === End ===
export default {
  PI,
  arr,
  obj,
  Example,
};
