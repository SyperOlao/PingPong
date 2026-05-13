# Contributing

Contributions are welcome when they make the project easier to read, easier to build, better documented, or slightly richer in examples, rendering, or debug tooling. This repository is a **small educational C++20 / DirectX 11 mini-engine** for students and beginner-to-intermediate graphics and game programmers. The goal is to keep the codebase **understandable**, not to grow it into a production engine.

## Good contribution types

- Documentation improvements (README-adjacent docs, diagrams, clarifications).
- Build fixes (CMake, vcpkg triplet hints, clearer error paths for common mistakes).
- Small, focused bug fixes with a clear repro or rationale.
- Simple, student-friendly examples that fit existing demos or patterns.
- Rendering or debug improvements (forward/deferred, GBuffer debug, shadows, cascaded shadow debug, GPU particles) when changes stay localized and explainable.
- UI or debug overlays (ImGui / ImGuizmo) that help learning or iteration.
- Small gameplay systems that extend **existing** demos (Main Menu, Pong, Solar System, Katamari, Lighting Test).
- Short comments where DirectX, CMake, or engine architecture is genuinely non-obvious.
- Small refactors that improve readability **without** rewriting large subsystems.

## What not to contribute

- Large rewrites of core architecture or rendering without prior discussion.
- Unrelated frameworks or dependencies that do not serve the educational scope.
- Unreal Engine, Unity, or other engine integrations.
- Massive asset dumps or binary blobs that bloat the repo.
- Low-value or misleading “AI slop” documentation (vague, unverified, or copy-pasted walls of text).
- Uncompressed huge GIFs or videos in the repository.
- Non-Windows platform rewrites or broad portability refactors unless agreed in an issue first.
- Changes that break existing demos or the default student workflow without a strong, documented reason.

## Before opening a pull request

Use this as a quick self-check:

- [ ] Build succeeds on **Windows x64**.
- [ ] Toolchain: **Visual Studio 2022 / MSVC**.
- [ ] **CMake 3.21+** is used as the project expects.
- [ ] **vcpkg** is used with a consistent triplet (e.g. `x64-windows`).
- [ ] **MiniEngineDemo** runs after your change.
- [ ] **Main Menu** navigation still works.
- [ ] You manually tested **at least every demo touched** by your change (and any related debug views).
- [ ] No `cmake-build-*`, `out/`, `build/`, or other generated build trees are committed.
- [ ] No local IDE or cache junk (e.g. `.vs/`, user-specific files) is committed.
- [ ] Any screenshots or GIFs in the PR or docs are **reasonably small** and purposeful.

If you are unsure whether an idea fits the project, open an issue first with a short plan and the demos you expect to touch.

## Build instructions

Short reference only; for full context see the repository README.

```powershell
git clone https://github.com/SyperOlao/Mini-Engine-With-Games.git
cd Mini-Engine-With-Games

cmake -S . -B cmake-build-debug `
  -G "Visual Studio 17 2022" `
  -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DVCPKG_TARGET_TRIPLET=x64-windows

cmake --build cmake-build-debug --config Debug
.\cmake-build-debug\MiniEngineDemo.exe
