// Run with Node.js; no packages required. Tests the real HTML installer.
import assert from 'node:assert/strict';
import fs from 'node:fs';
import vm from 'node:vm';

const filename = process.argv[2] || new URL('../sopwith.html', import.meta.url);
if (!filename) throw new Error('Usage: node test-mod-files.mjs path/to/sopwith.html');
const html = fs.readFileSync(filename, 'utf8');
const start = html.indexOf('function installModFiles()');
const end = html.indexOf('function mountFilesystems()', start);
assert(start >= 0 && end > start, 'Could not locate the real mod installer');
for (const contents of [[], ['one'], ['first mission', 'second mission', 'third mission']]) {
  const files = new Map();
  const Module = {arguments: ['-q']};
  const context = {modFiles: contents, Module,
    FS: {writeFile: (name, text) => files.set(name, text)}};
  vm.runInNewContext(html.slice(start, end) + '\ninstallModFiles();', context);
  assert.equal(files.size, contents.length, 'Each mod must retain its own file');
  assert.equal(Module.arguments[0], '-q', 'Existing arguments must be preserved');
  const paths = Module.arguments.slice(1).filter((_, i) => i % 2 === 1);
  assert.deepEqual(paths.map(name => files.get(name)), contents,
    'Loading -m arguments must recover every supplied mod in URL order');
  assert.deepEqual(Module.arguments.slice(1).filter((_, i) => i % 2 === 0),
    contents.map(() => '-m'));
}
console.log('PASS: zero, one and three URL mods retain separate contents in order.');
