#!/usr/bin/env node
// Convert the recovered Overdrive catalog to Kodi's versioned, data-only format.
// The generated file contains metadata and art URLs; it never contains download URLs.

import { readFile, writeFile, mkdir } from 'node:fs/promises';
import path from 'node:path';
import { pathToFileURL } from 'node:url';

const [source, destination = 'system/overdrive/catalog.json'] = process.argv.slice(2);
if (!source) {
  console.error('Usage: node import_catalog.mjs <Overdrive app/data/catalog.js> [output.json]');
  process.exit(2);
}

const sourcePath = path.resolve(source);
const { games } = await import(pathToFileURL(sourcePath).href);
if (!Array.isArray(games)) throw new Error('Source module does not export games[]');

const seen = new Set();
const titles = [];
for (const game of games) {
  const id = String(game.id ?? '').trim();
  const title = String(game.title ?? '').trim();
  if (!id || !title || seen.has(id)) continue;
  seen.add(id);

  let summary = '';
  try {
    const pagePath = path.join(path.dirname(sourcePath), 'pages', `${id}.json`);
    const page = JSON.parse(await readFile(pagePath, 'utf8'));
    summary = typeof page.summary === 'string' ? page.summary : '';
  } catch (error) {
    if (error.code !== 'ENOENT') throw error;
  }

  titles.push({
    id,
    title,
    platform: String(game.platform ?? ''),
    contentKind: String(game.contentKind ?? 'game'),
    developer: String(game.developer ?? ''),
    summary,
    cover: String(game.art?.cover ?? ''),
  });
}

const output = path.resolve(destination);
await mkdir(path.dirname(output), { recursive: true });
await writeFile(output, JSON.stringify({ schema: 1, titles }) + '\n');
console.log(`Imported ${titles.length} titles to ${output}`);
