# Kodi PS5 + Overdrive

An **unofficial native Kodi 22 build for PlayStation 5**, with an Overdrive catalog built into Kodi's home screen. Kodi remains the media player and user interface; the Overdrive work is native C++ inside the same application, not a separate app or a Python add-on.

This repository is based on [Kodi](https://github.com/xbmc/xbmc) and the [PS5MC PS5 port](https://github.com/VivaLaVent/PS5MC). The catalog importer uses metadata from the public [MyLuxy/overdrive](https://github.com/MyLuxy/overdrive) project. Credit and licenses remain with their respective authors. This project is not affiliated with Team Kodi, the XBMC Foundation, Sony, or the Overdrive team.

## What is in this build

| Feature | Status |
| --- | --- |
| Native Kodi interface, DualSense navigation, local and network media | Included through the PS5MC port |
| Overdrive entry on Kodi's home screen | Working on a PS5 running firmware 13.60 |
| PS5 Edition Estuary skin | Dark home screen, controller focus styling, and native Overdrive cards |
| Games and Homebrew browsing | Working |
| Title artwork and details | Working when the artwork host is reachable |
| Bundled catalog | 1,845 game and homebrew metadata entries |
| PS5 System Info | Local network and video output values; unavailable values marked `-` |
| Search, downloads, installation, updates, and account features | Not implemented in this preview |

**The catalog is metadata only.** This repository and release contain no games, game archives, download links, archive passwords, firmware, exploit, or Sony SDK. Opening a catalog entry shows its details; it does not download or install the title. The [integration notes](docs/OVERDRIVE-INTEGRATION.md) describe the current code and the remaining work.

The PS5 test on **14 October 2026** confirmed that Kodi launches and that the Overdrive Games, Homebrew, and title-detail views work. Other firmware versions and the rest of Kodi's media features have not been retested for this preview; see the [PS5MC platform notes](ps5/README.md) for the port's broader feature information and limits.

### PS5 interface and System Info

The bundled Estuary skin has a PS5 Edition home screen and a dedicated Overdrive entry. Overdrive remains a native Kodi window; the Games and Homebrew cards lead into the catalog's normal title lists and details. The background and icon are original assets in this repository, with a generator at `addons/skin.estuary/tools/generate_ps5_assets.py`.

Kodi's **Settings → System information** uses PS5 readings where the title environment provides them: output resolution, refresh rate, display HDR/SDR state, high-refresh support, local IP configuration from NetCtl, and the CPU cores visible to the title. The displayed free memory is Kodi's title memory, not the console's total physical RAM. The Network page reports local configuration and marks external internet as **not tested**; it does not contact an outside service. The Storage page displays `Storage: -`, and the Summary page displays `Controller battery: -`. Those are placeholders: Kodi does not make a controller-battery request. CPU and GPU temperatures and the current CPU clock have no verified title-safe reading in this build. Other System Info rows continue to use Kodi's normal platform information.

DualSense navigation reads the current pad state on Kodi's dedicated input thread. Its 8 ms sleep between reads is this port's choice, not a Sony-mandated polling interval. No HID battery query runs in the input path.

## Install on a PS5

You need a PS5 with a working homebrew environment, a compatible HEN/kstuff stack, ShadowMountPlus or another folder-title loader, and a way to copy files to the console such as FTP. The tested setup was firmware **13.60** with the matching etaHEN bundle and ShadowMountPlus. On that setup, load etaHEN before ShadowMountPlus; do not load an unrelated standalone kstuff over etaHEN's bundled copy.

1. Download `Kodi-PS5-Overdrive-PPSA99742.tar.gz` from [Releases](https://github.com/terrafirma2021/kodi-ps5-overdrive/releases).
2. Extract it on your computer. The archive contains a single `PPSA99742` folder.
3. Copy that entire folder to `/data/homebrew/PPSA99742` on the PS5. Keep the `share/kodi` files, `eboot.bin`, `sce_module`, and `sce_sys` together.
4. Run ShadowMountPlus **Scan/Rescan Library** or add `/data/homebrew/PPSA99742` to its manual list, then rescan.
5. Open **Kodi Overdrive Preview** from the PS5 Game Library. In Kodi, choose **Overdrive** on the home menu, then **Games** or **Homebrew**. Select a title to see its details.

The existing `v0.1.0-preview.1` archive predates the PS5 Edition skin and System Info changes on the current source branch. Build from source for those changes until a newer release archive is published.

For your own media, use Kodi's **Videos → Files → Add videos** or **Music → Files → Add music**. Network shares can be added by IP address using SMB or NFS. Kodi's settings and library live in its PS5 save data and normally survive an app update.

**Updating:** close Kodi first, replace the title folder's changed files, and rescan the library if the loader requires it. Replacing a running executable can crash the console.

## Build from source

The PS5 build needs the [ps5-payload-sdk](https://github.com/ps5-payload-dev/sdk), the ported libraries and host tools prepared by `ps5/scripts/00-setup-wsl.sh`, and the PS5 OpenGL build described in [the platform README](ps5/README.md). The setup is substantial; the release archive is the easiest way to try the app. The build scripts target Linux or WSL2, not native Windows.

After the toolchain is installed, from this repository's root:

```bash
export KODI_SRC="$PWD"
export BUILD="$HOME/kodi-ps5-overdrive-build"
export NATIVE="$HOME/kodi-ps5-overdrive-native"
export STAGE="$HOME/kodi-ps5-overdrive-stage"

bash ps5/scripts/20-configure-kodi.sh
cmake --build "$BUILD" -j"$(nproc)"
TITLE_ID=PPSA99742 TITLE_NAME='Kodi Overdrive Preview' \
  bash ps5/scripts/30-deploy.sh
tar -C "$STAGE/app/dist" -czf Kodi-PS5-Overdrive-PPSA99742.tar.gz PPSA99742
```

The build also needs the native app template that `30-deploy.sh` expects; its default is `$HOME/ps5-work/ps5-opengl/build/native-app/PPSA99005`. See the comments in that script and the [PS5MC platform guide](ps5/README.md) before building. Use a clean source tree so the embedded build stamp identifies a real commit.

The bundled catalog is at [`system/overdrive/catalog.json`](system/overdrive/catalog.json). To regenerate it from a compatible Overdrive `catalog.js` checkout:

```bash
node tools/overdrive/import_catalog.mjs /path/to/overdrive/app/data/catalog.js
```

The importer intentionally writes title metadata and artwork URLs only. A replacement catalog can also be placed in Kodi's `special://home/overdrive/catalog.json`; it is loaded when the Overdrive window next opens. The JSON format is `{"schema":1,"titles":[...]}`.

## Source and licensing

Kodi and this fork are distributed under [GPL-2.0-or-later](LICENSE.md), with additional component licenses documented in [`LICENSES`](LICENSES). The PS5 port is credited to the PS5MC contributors and its upstream projects. The Overdrive window and catalog model are new Kodi-side code; the Overdrive project is referenced for catalog data and behavior, not bundled as a second executable. Game names, descriptions, and artwork remain the property of their respective owners.

Issues and contributions are welcome. Please include the PS5 firmware, HEN and loader versions, exact title ID, and a relevant Kodi or kernel log when reporting a failure.
