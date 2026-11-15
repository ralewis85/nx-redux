# Dev TODO

Work that is **planned but not built** — decided on, scoped, and worth doing, with no code
written yet. Each entry records enough context (why, where, known constraints) that a later
session can start implementing without re-deriving the design.

The two dev to-do files are a pipeline:

| File | Holds | Exit condition |
|---|---|---|
| `DEV_TODO.md` (this file) | designed / requested, nothing written | move the entry to `DEV_CHECKLIST.md` once it builds |
| `DEV_CHECKLIST.md` | built, not yet verified on hardware | delete the section once verified and shipped |

Neither file is a changelog — delete an entry when it lands, don't mark it done and keep it.

---

## DC pre-launch options: no launch transition into the editor (cosmetic)

**Recorded:** 2026-07-30, noted while moving the pre-launch options gotchas into
the flycast README (now `workspace/all/emu-options/README.md`).

Opening "Emulator Options" from the game-list context menu doesn't play the
usual into-game transition. `gamelist.c`'s case 36 (`ctxEmuOptions` →
`openScript`, the pre-launch editor launch) doesn't set nextui's `startgame` flag,
so the screen-blank-out at `nextui.c:577-580` is skipped and the game list stays visible until the editor
draws over it. Cosmetic only — decide on-device how it actually looks before
choosing whether it's worth setting the flag (which would blank the screen on
the way in, matching a normal launch).

- [ ] On device, watch the transition into the editor from the context menu and
      decide whether to set `startgame` in `gamelist.c` case 36.

---

## Credit Kenney for the button-hint glyphs

**Requested:** 2026-09-29 (owner). The button-hint glyphs come from Kenney's **Input Prompts** pack (1.5A, CC0: credit
is not required, but the owner wants to give it). Source pack: `~/Downloads/kenney_input-prompts_1.5/` on the owner's Mac.

- [ ] Add a line to the `## Credits` section of `README.md`, e.g.
      `- [Kenney](https://kenney.nl/assets/input-prompts) for the Input Prompts glyphs used in the button hints (CC0)`.
- [ ] Add the same credit to the docs site page `nx-redux-docs/docs/reference/credits.md`.

---

## Music player: replace fdk-aac with FFmpeg's native AAC decoder (7.1+)

**Decided:** 2026-10-07 (owner). `libfdk-aac.so` (fdk-aac v0.1.6, a TrimUI-SDK prebuilt in
`workspace/all/musicplayer/include/fdk_aac/lib/`) is under the FDK-AAC license, which the FSF
lists as GPL-incompatible, and `musicplayer.elf` + `musicplayerd.elf` (GPLv3) link it
(`-lfdk-aac`, `workspace/all/musicplayer/Makefile`). A GPLv3 linking exception isn't an option:
the binaries also link `common/` code from many other authors (MinUI, NextUI contributors).

Replace it with a static, minimal libavcodec from **FFmpeg ≥ 7.1** (LGPL-2.1+, no
`--enable-gpl`), float `aac` decoder (7.1 added xHE-AAC/USAC, float decoder only; the float
decoder also has AArch64 NEON, `aac_fixed` doesn't).

- [ ] Build script (toolchain image) for a minimal static FFmpeg 7.1+:
      `--disable-everything --enable-decoder=aac --enable-decoder=aac_latm --enable-parser=aac
      --enable-parser=aac_latm --disable-programs --enable-static --disable-shared`, sha256-pinned
      tarball; build it in CI next to ffplay (could share the FFmpeg version with ffplay's build).
- [ ] Port the four fdk call paths (~30 `aacDecoder_*` calls, `player.c` + `radio.c`):
      `.m4a` via minimp4 (DSI → `extradata`, one `avcodec_send_packet` per MP4 sample), `.aac`
      ADTS files, Icecast/ICY AAC/AAC+ streams, and HLS radio (our TS demux → ADTS). ADTS paths
      need `av_parser_parse2` to split frames (fdk took arbitrary byte chunks via `Fill`).
- [ ] Output is planar float (FLTP): add interleave + clamp to our s16 path. Seek =
      `avcodec_flush_buffers`; take rate/channels from `frame->sample_rate`/`ch_layout`.
- [ ] Check HE-AAC (implicit SBR) `.m4a`: today the rate comes from the stsd box, not the
      decoder, which can be half the real output rate.
- [ ] Device-test LC, HE-AAC v1/v2 radio ("aacp"), m4a seek, HLS rate changes; measure the
      size of the two ELFs (estimate +0.5–0.9 MB each).
- [ ] Remove `include/fdk_aac/` and the `libfdk-aac.so*` copy in the root `Makefile`, and
      `licenses/fdk-aac.txt`.

Fallback if this stalls: faad2 (GPL-2.0-or-later, API close to fdk's, no USAC).

---

## Files tool: replace NextCommander with our own copy of od-contrib/commander (MIT)

**Decided:** 2026-10-07 (owner asked; recommendation recorded). The Files tool is
`LoveRetro/NextCommander` (cloned `--depth 1`, unpinned, in `workspace/<plat>/Makefile`, plus
`workspace/all/other/NextCommander.patch`). Nothing in its lineage has a license — LoveRetro ←
OnionUI ← gcwnow ← the original DinguxCommander — so strictly it is all-rights-reserved and we
can't redistribute it. Forking the *original* DinguxCommander doesn't help: it was published
without a license too.

`od-contrib/commander` (OD Commander, Gleb Mazovetskiy) forked DinguxCommander, rewrote most of
the code, replaced all icons, and added an **MIT** `LICENSE.txt` (commit 6023665431: "The
original code and icons were published without a license. Since then, most of the code has
been rewritten and all of the icons have been replaced"). It supports SDL2 and has
controller-button handling.

**Copy, don't GitHub-fork (decided 2026-10-07):** make our own standalone repo (e.g.
`nx-commander` under the owner's account) — `git clone` od-contrib/commander and push it to a new
empty repo, so the history comes along but the repo is **outside their fork network**. A
GitHub "Fork" survives the owner deleting or privatising their repo, but a DMCA notice that
claims the whole fork network can take every fork down at once; a standalone copy is only hit if
it is named itself. The MIT grant we received can't be revoked either way. The remaining risk is
a claim by the original DinguxCommander author over leftover unlicensed code — that hits any copy
that is named, so the more of it we rewrite, the smaller it gets.

- [ ] Create the standalone repo from od-contrib/commander; keep its MIT `LICENSE.txt` and
      copyright line, and say in its README it is based on od-contrib/commander (Gleb Mazovetskiy).
- [ ] Port what NextCommander adds for us (diff NextCommander against its DinguxCommander base +
      our `NextCommander.patch`: SDL2/TrimUI input, NextUI theming/fonts, screen size) as commits
      in our repo.
- [ ] Build from **our** repo at a pinned full commit hash, never from upstream (swap the
      unpinned `--depth 1` clone in `workspace/<plat>/Makefile`, and the copy in
      `workspace/<plat>/platform/Makefile.copy`); keep `Files.pak` paths/behaviour unchanged.
      Optionally attach its source tarball to each GitHub release so shipped code always has its
      source beside it.
- [ ] Replace `licenses/nextcommander.txt` with the OD Commander MIT text; update the README
      credit line (`skeleton/BASE/README.txt`, "Licenses and credits").
- [ ] Device-test on Brick + Smart Pro S (browse, copy/move, text/image viewer, keyboard).
