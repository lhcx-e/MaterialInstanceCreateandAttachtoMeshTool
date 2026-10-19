# Changelog

All notable changes to this project are documented here.
The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and the project uses [Semantic Versioning](https://semver.org/).

## [1.0.0] — 2026-10-19

First public release.

### Added
- Content Browser right-click entry **Create Material Instances...** for Static Mesh and Skeletal Mesh assets; the panel opens with the clicked mesh already loaded.
- Slate panel with target mesh picker, parent material picker, instance name prefix and overwrite toggle.
- One `MaterialInstanceConstant` per material slot, named `<prefix><SlotName>` with illegal characters sanitized to `_`.
- Instances are created in the **same folder as the mesh**; the output folder is derived automatically and shown read-only.
- Generated instances are assigned back to the mesh slots and both mesh and instances are saved.
- Re-running reuses existing instances, or deletes and recreates them when **Overwrite Existing** is checked.
- `Tools → Material Instance Tool` menu entry and toolbar button for the generic, panel-first workflow.
- Editor-only module (`PostEngineInit`), so nothing is included in packaged builds.
- Prebuilt Windows x64 binaries for UE 5.8 distributed as a separate release archive.

### Known limitations
- Processes one mesh at a time (multi-selection uses the first mesh).
- Does not create or modify material parameters — it only gives every slot its own instance.
