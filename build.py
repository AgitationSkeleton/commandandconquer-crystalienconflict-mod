#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Assemble and verify the two tested mod releases. Never installs or uploads."""
import argparse
import hashlib
import io
import json
import shutil
import sys
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent
NAMES = {'factions': 'CrystAlienConflict', 'reinforcements': 'CrystAlienConflictPlus'}


def sha(data):
    return hashlib.sha256(data).hexdigest()


def source_archive(reference_hash, rebuilt=False):
    reference = (ROOT / 'assets/objects' / reference_hash).read_bytes()
    if sha(reference) != reference_hash:
        raise ValueError('Source archive checksum mismatch')
    output = io.BytesIO()
    with zipfile.ZipFile(io.BytesIO(reference)) as original:
        with zipfile.ZipFile(output, 'w', zipfile.ZIP_DEFLATED) as archive:
            for info in original.infolist():
                data = original.read(info)
                if info.filename.startswith('src/') or info.filename == 'LICENSE-EA-GPLv3.md':
                    current = (ROOT / info.filename).read_bytes()
                    if not rebuilt and current != data:
                        raise ValueError('Source changed: ' + info.filename + '; recompile and use --rebuilt-dlls')
                    data = current
                if rebuilt:
                    archive.writestr(info, data)
    return output.getvalue() if rebuilt else reference


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--variant', choices=['all'] + list(NAMES), default='all')
    parser.add_argument('--verify-only', action='store_true', help='check source and assets without writing output')
    parser.add_argument('--verify-output', action='store_true', help='check the existing dist payloads without rebuilding')
    parser.add_argument('--rebuilt-dlls', action='store_true', help='use .build/bin DLLs produced by tools/rebuild-dlls.ps1; DLL hashes may differ')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'release-manifest.json').read_text(encoding='utf-8'))
    generated_source = source_archive(manifest['source_archive_sha256'], args.rebuilt_dlls)
    variants = list(NAMES.values()) if args.variant == 'all' else [NAMES[args.variant]]
    for name in variants:
        files = manifest['variants'][name]['files']
        payload = {}
        for rel, entry in files.items():
            data = generated_source if entry['source'] == '@source-archive' else (ROOT / entry['source']).read_bytes()
            if sha(data) != entry['sha256'] and not (args.rebuilt_dlls and entry['source'] == '@source-archive'):
                raise ValueError('Input checksum mismatch: ' + rel)
            if args.rebuilt_dlls and rel == 'Data/TiberianDawn.dll':
                data = (ROOT / '.build/bin' / name / 'TiberianDawn.dll').read_bytes()
            payload[rel] = data
        if args.verify_only:
            print(name + ': all source/assets match the reference release (' + str(len(files)) + ' files)')
            continue
        manual = ROOT / 'dist/manual' / name
        workshop = ROOT / 'dist/workshop' / name
        if not args.verify_output:
            for path in (manual, workshop):
                if path.exists():
                    shutil.rmtree(path)
                path.mkdir(parents=True)
            for rel, data in payload.items():
                for base in (manual, workshop / name):
                    target = base / rel
                    target.parent.mkdir(parents=True, exist_ok=True)
                    target.write_bytes(data)
        for base in (manual, workshop / name):
            actual = {p.relative_to(base).as_posix(): sha(p.read_bytes()) for p in base.rglob('*') if p.is_file()}
            expected = {rel: sha(data) for rel, data in payload.items()}
            if actual != expected:
                raise ValueError('Output does not match the release manifest: ' + str(base))
        if (workshop / 'ccmod.json').exists() or not (workshop / name / 'ccmod.json').is_file():
            raise ValueError('Incorrect Workshop nesting')
        print(name + ': manual and Workshop payloads verified, ' + str(len(files)) + ' files each')
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, ValueError, KeyError, zipfile.BadZipFile) as error:
        sys.exit('ERROR: ' + str(error))
