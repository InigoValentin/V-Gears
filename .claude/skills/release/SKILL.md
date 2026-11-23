---
name: release
description: Cut a V-Gears release using git-flow. Bumps the version, writes the CHANGELOG entry, merges release/vX.Y.Z into master, tags it, merges back into develop, and (after confirmation) pushes. Usage - /release X.Y.Z
disable-model-invocation: true
argument-hint: X.Y.Z
---

# Release V-Gears

Cut release `$ARGUMENTS` (format `X.Y.Z`, no leading `v`). If no argument is
given, read the current version and ask the user which one to use (suggest the
next minor).

The project uses git-flow: `release/vX.Y.Z` is branched from `develop`, merged
with `--no-ff` into `master`, tagged `vX.Y.Z` (annotated), and the tag is merged
back into `develop`. Follow that exactly (see `git log --merges` for examples:
"Merge branch 'release/v0.3.4'" and "Merge tag 'v0.3.4' into develop").

Never push, force anything, or delete branches without the explicit
confirmation in step 8. Stop and report on any failure instead of working
around it.

## 1. Preflight

Run these checks in order.

**Stop conditions** (report and stop, do not continue):

- `git branch --show-current` must be `develop`. If not, ask the user before
  switching.
- `git status --porcelain` must be empty. If there are any uncommitted changes
  (modified, staged or untracked files), list them, warn the user, and stop.
  The user must commit or stash first. Do not stash or commit on their behalf.
- `git fetch --all --tags`, then check `develop` is not behind `origin/develop`.
- The tag `vX.Y.Z` and branch `release/vX.Y.Z` must not already exist.
- The new version must be greater than the current one in `CMakeLists.txt`
  (`project(V-Gears VERSION ...)`).

**Warning** (tell the user and ask whether to continue):

- Unmerged feature branches. List local and remote branches not yet merged into
  `develop`: `git branch -a --no-merged develop`, ignoring `master`, `HEAD`
  aliases and `develop` itself. For each one show the name and the number of
  commits ahead of `develop` (`git rev-list --count develop..<branch>`). Their
  work will not be in this release; ask the user to confirm that is intended.

Find the previous release tag with `git describe --tags --abbrev=0`.

## 2. Create the release branch

```
git checkout -b release/vX.Y.Z develop
```

## 3. Bump the version

Update every place that holds the version:

- `CMakeLists.txt`: `project(V-Gears VERSION X.Y.Z ...)`. This is the source of
  truth; `src/Version.h.in` generates the build's `Version.h` from it.
- `src/Version.h` (tracked, hand-maintained): `VGEARS_VERSION`,
  `VGEARS_VERSION_MAJOR`, `VGEARS_VERSION_MINOR`, `VGEARS_VERSION_PATCH` and
  `VGEARS_VERSION_SIGNATURE` (`"V-Gears vX.Y.Z"`). Set **every** field from the
  new version (the existing values may be stale or inconsistent, e.g. a PATCH
  that doesn't match `VGEARS_VERSION`). After editing, check that all five
  agree with `X.Y.Z` and with `CMakeLists.txt`.

Then `grep -rn "<previous version>" --include=*.md --include=*.txt --include=*.in .`
(excluding `build*`, `_OLD`, `CHANGELOG.md`) and report any other version
mentions to the user; update the ones that are clearly the app version.

## 4. Changelog

Draft a new `## X.Y.Z` section at the top of `doc/CHANGELOG.md`, directly under
the `# Changelog` heading. The file is sorted by version in **decreasing order
(newest first)**: the new section goes above all existing ones, and existing
sections must keep that order. After editing, check the `## ` headings are
strictly descending.

1. Collect changes: `git log <prev-tag>..develop --no-merges --format='%h %s'`
   and `git diff --stat <prev-tag>..develop`. Read diffs where commit messages
   are vague. Do not invent changes that are not in the history.
2. Match the style of the existing entries:
   - First line: `Changes since <prev version>.` followed by one sentence
     summarising where most of the work went.
   - `### Area` headings (e.g. Battle, Field, World map, Installer, Engine and
     scripting), with `**Subgroup**` labels and `- ` bullets where an area is
     big.
   - Plain, factual bullets describing user-visible behaviour, not commit
     messages. No marketing language.
3. Also review `doc/TODO.md`: remove or tick items that this release completed.
   Don't add new items unless the user asks.
4. **Show the drafted changelog (and TODO changes) to the user and wait for
   approval or edits before committing.**

## 5. Build, test and package

Packaging is described in `doc/BUILD.md` ("Make packages"). Use the release
build directory (create it if missing), with the project's build settings from
memory (keep the fallbacks ON). Run `make` with half the system's cores:

```
mkdir -p build-release
cd build-release
cmake ../ -DCMAKE_BUILD_TYPE=Release
make -j"$(( $(nproc) / 2 > 0 ? $(nproc) / 2 : 1 ))" v-gears v-gears-installer
cpack
```

(`doc/BUILD.md` uses a fixed `-j4`; here `-j` is half of `nproc`, with a
minimum of 1, unless the user says otherwise.) The version must have been bumped (step 3) before this, since the
package version comes from `CMakeLists.txt`.

Optionally run the tests (`-DBUILD_TESTS=ON` in a separate `build-tests`
directory, targets `v-gears-installer-tests` and `v-gears-tests`, as in
`doc/BUILD.md`); ask the user whether to run them, as it needs another build.

When `cpack` finishes, `build-release/` must contain these four files for the
new version (older versions' files are also in that directory; **ignore those**):

- `v-gears_X.Y.Z_amd64.deb`
- `v-gears_X.Y.Z_amd64.deb.sha256`
- `v-gears-X.Y.Z-Linux-x86_64.tar.gz`
- `v-gears-X.Y.Z-Linux-x86_64.tar.gz.sha256`

Verify all four exist and are newer than the version bump commit, and verify
the checksums (`sha256sum -c` from inside `build-release`, for the new files
only). Report failures and stop. Note the packages only work on the distribution
and version they were built on; mention this in the final report.

## 6. Commit on the release branch

```
git add CMakeLists.txt src/Version.h doc/CHANGELOG.md doc/TODO.md
git commit -m "vX.Y.Z"
```

(Past release commits were titled just with the version. Stage only the release
files, nothing else.) End the commit message with the standard attribution
lines.

## 7. Merge and tag (local only)

```
git checkout master
git merge --no-ff release/vX.Y.Z -m "Merge branch 'release/vX.Y.Z'"
git tag -a vX.Y.Z -m "vX.Y.Z"
git checkout develop
git merge --no-ff vX.Y.Z -m "Merge tag 'vX.Y.Z' into develop"
git branch -d release/vX.Y.Z
```

If a merge conflicts, stop and show the conflicts. Don't resolve silently.

## 8. Push (requires confirmation)

Show the user `git log --oneline --graph -8 --all` and the tag, then ask which
remotes to push to. Remotes configured: `origin`, `github`, `inigovalentin`.
Push only the ones the user confirms:

```
git push <remote> master develop vX.Y.Z
```

Never use `--force`. Pushing publishes the tag; it can't be cleanly undone.

## 9. GitHub release (ask first)

Pushing the tag to `github` does **not** create a GitHub Release by itself: it
only makes the tag visible. This step creates the Release object from the
existing tag `vX.Y.Z` and uploads the package files to it. It needs the tag to
already be on the `github` remote (step 8) and `gh` to be authenticated
(`gh auth status`).

Ask the user to confirm, then:

1. Extract the new `## X.Y.Z` section of `doc/CHANGELOG.md` (without the heading
   line) into a file in the scratchpad directory.
2. Create the release from the tag and attach the four package files, run from
   the repo root:

```
gh release create vX.Y.Z \
  --repo InigoValentin/V-Gears \
  --verify-tag \
  --title "vX.Y.Z" \
  --notes-file <scratchpad>/release-notes.md \
  build-release/v-gears_X.Y.Z_amd64.deb \
  build-release/v-gears_X.Y.Z_amd64.deb.sha256 \
  build-release/v-gears-X.Y.Z-Linux-x86_64.tar.gz \
  build-release/v-gears-X.Y.Z-Linux-x86_64.tar.gz.sha256
```

`--verify-tag` makes it fail instead of creating the tag if it isn't on the
remote. Pass exact file names, never a glob like `build-release/*`, so old
versions' packages are not uploaded. Afterwards run
`gh release view vX.Y.Z --repo InigoValentin/V-Gears` and confirm the four
assets are listed. Print the release URL.

If the user declines, say the release can be created later with the same
command.

## 10. Report

Finish with a short summary: version, tag, commits created, remotes pushed, and
the four package files, the GitHub release URL if created, and anything
skipped or left for the user.
