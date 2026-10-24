# CrystAlien Conflict for Command & Conquer Remastered

**EA has not endorsed and does not support this product.**

Two standalone Tiberian Dawn mods bring the units, buildings, artwork, voices,
music and gameplay elements of **LEGO Mars Mission: CrystAlien Conflict** into
**Command & Conquer Remastered Collection**.

| Variant | Gameplay | Steam Workshop |
| --- | --- | --- |
| **CrystAlien Conflict Factions** | Total conversion: replaces GDI and Nod with the Astros and Aliens. | [Subscribe to Factions](https://steamcommunity.com/sharedfiles/filedetails/?id=3825896336) |
| **CrystAlien Conflict Reinforcements** | Adds the LEGO units and buildings alongside the existing GDI and Nod roster. | [Subscribe to Reinforcements](https://steamcommunity.com/sharedfiles/filedetails/?id=3825897064) |

Enable **one variant at a time**, then restart the game. Each supplies its own
`TiberianDawn.dll`. Play single-player Skirmish or supported missions; online
multiplayer is unsupported. A separate installation of C&C Remastered is required.

## Build the tested releases

Use Python **3.10 or newer** and run:

```powershell
python build.py
```

No asset downloads, extraction tools, installed game files or Python packages
are needed. Every runtime asset and both tested gameplay DLLs are included.
The build reconstructs both complete mod folders and checks every file against
the SHA-256 manifest captured from the corrected Workshop upload payloads.
The DLL source archive shipped inside each mod is retained once as a release
input. Its source files are checked against this repository's browsable source.
Keeping the tested ZIP preserves its exact bytes across Python/zlib versions.

Outputs:

```text
dist/manual/CrystAlienConflict/ccmod.json
dist/manual/CrystAlienConflictPlus/ccmod.json
dist/workshop/CrystAlienConflict/CrystAlienConflict/ccmod.json
dist/workshop/CrystAlienConflictPlus/CrystAlienConflictPlus/ccmod.json
```

Copy a folder from `dist/manual/` into
`Documents\CnCRemastered\Mods\Tiberian_Dawn\`, enable it in Options → Mods,
and restart. Move local copies out of that directory when testing subscriptions.

Workshop content folders require the extra mod subfolder shown above: the game
scans child directories of each installed Workshop item for `ccmod.json`.
No build command installs, subscribes, uploads, or changes Workshop listings.
Workshop titles, descriptions and uploader account data are not stored here.

For one variant or verification without rebuilding:

```powershell
python build.py --variant factions
python build.py --variant reinforcements
python build.py --verify-only
python build.py --verify-output
```

The release assembly uses only byte copies and SHA-256 checks, so it does not
depend on a particular ZIP compressor or operating system.

## Recompile the gameplay source

The complete modified EA DLL source, including generated gameplay tables,
resources and assembler files, is under `src/TiberianDawn/`. The two variants
share this source; Reinforcements defines `CAC_SUPPLEMENTAL`.

On Windows, install Visual Studio C++ build tools with the **v141 toolset**,
**v141 MFC**, and **Windows SDK 10.0.19041.0**. Then run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/rebuild-dlls.ps1
python build.py --rebuilt-dlls
```

Pass `-MSBuild "C:\path\to\MSBuild.exe"` if automatic discovery is unavailable.
Compilation uses Release/Win32 and preserves the project's one-byte packing.
Intermediate files and the newly compiled binaries stay under `.build/`.

Recompiled DLLs can contain different compiler/linker timestamps and debug paths.
`python build.py` uses the checked-in tested DLLs for byte-identical release
assembly; `--rebuilt-dlls` explicitly packages your compiled DLLs instead.
For development builds, `--rebuilt-dlls` also updates the included source ZIP
from your current source files. The default release build rejects source edits
that no longer correspond to its tested DLLs.

## Repository contents

- `assets/objects/`: all finished runtime content, stored once per unique SHA-256
  even where multiple game paths or both variants use identical files.
- `release-manifest.json`: readable mappings from each real mod file path to its
  content object and checksum. This includes both tested DLLs and every asset.
- `src/TiberianDawn/`: complete DLL source and build project.
- `build.py` and `tools/rebuild-dlls.ps1`: release assembly and optional compilation.

There are no raw SWF dumps, extraction intermediates, development logs, symbol
files, screenshots, game installations, uploader binaries or unrelated add-ons.

## Credits and license

CrystAlien Conflict was created for **The LEGO Group** by **4T2 Multimedia**.
Original artwork, sound, music and other LEGO game assets remain the property
of their respective owners. This is an unofficial, non-commercial fan project.
Those assets are not relicensed under the DLL's source-code license.

The modified Tiberian Dawn DLL source retains its original notices and is
distributed under **GPLv3 with EA's additional terms**, reproduced in
[LICENSE-EA-GPLv3.md](LICENSE-EA-GPLv3.md). The release assembly script is
GPL-3.0-only. Both output mods include the source archive and license.
