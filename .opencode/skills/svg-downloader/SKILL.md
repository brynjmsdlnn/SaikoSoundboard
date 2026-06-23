---
name: svg-downloader
description: Download SVGs from Lucide icon set, save to src/icons/, and register in icons.qrc
---

# SVG Icon Downloader

Downloads SVG icons from Lucide GitHub, saves to `src/icons/`, and registers in `src/icons/icons.qrc`.

## Usage

```
/icons <icon1> <icon2> ...
```

## Project-specific defaults

- Source: `https://raw.githubusercontent.com/lucide-icons/lucide/main/icons/`
- Icons directory: `src/icons/`
- QRC file: `src/icons/icons.qrc`
- QRC prefix: `/icons`

## Workflow for each icon

1. Fetch the SVG from Lucide's GitHub
2. Save to `src/icons/<name>.svg`
3. Add `<file><name>.svg</file>` to `icons.qrc`
4. Build to verify

## Example

```
/icons play square trash-2 headphones keyboard music check corner-down-left chevron-down save
```
