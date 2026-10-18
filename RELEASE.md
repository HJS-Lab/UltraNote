## UltraNote v1.8.7 — Portable Sticky Notes for Windows

A modern recreation of ATnotes (2005) built with pure Win32 API and C++17. Lightweight, portable, and dependency-free.

### What's New in v1.8.7

**Second code-audit remediation — 77 verified fixes for data safety, alarms, focus handling and hygiene:**
- Data safety: every save now verifies that the bytes really reached the disk, so a full or disconnected drive can no longer replace `notes.json` with an empty file; a failed save keeps retrying and warns via tray balloon; a `notes.json` that exists but is locked (sync client, antivirus) is retried and never overwritten by an empty note set
- Notes: Alt+F4 hides a note instead of destroying it; the editor no longer truncates text at 30,000 characters; a plain click on a note no longer rewrites the notes file; Alt-shortcuts no longer trigger menu mnemonics or beeps
- Alarms: "Snooze" works again; a recurring alarm missed during standby or while the app was closed is caught up once; a "Once" alarm with a stale end date fires again; an open alarm popup closes when its note is deleted; only real WAVE files are played
- Note list: deleting or toggling from the list no longer moves the keyboard focus to another note (the next Del hit the wrong note); the hover preview no longer alters the saved hidden state; selection and scroll position survive refreshes; right-click acts on the folder under the cursor; the maximized state is remembered; umlauts sort correctly
- Dialogs: Enter works in the find dialog and the alarm windows; the find dialog keeps the keyboard focus; file dialogs accept long paths; shortcut assignment checks for conflicts and a shortcut can be cleared
- Tray: sharp small icon, keyboard access (Win+B), robust re-add at autostart
- High-DPI layout for the alarm dialog, alarm popup, find dialog, note list controls and the attachment bar
- Hygiene: dead code removed, locale-correct yearly alarm description, printer copies respected, print font leak fixed

Built on top of v1.8.6; no new dependencies. `notes.json` gains one per-alarm field (`lastFired`); files from older versions load unchanged.

### Download

| Package | Description |
|---------|-------------|
| **UltraNote-Portable.zip** | Standalone EXE + language files — no installation needed |

### Features

- Sticky notes with drag, resize, multi-select, and always-on-top
- Per-note alarms with flexible repetition and snooze
- Note list with sorting, search (with highlighting), hover preview, folder management, and column visibility toggles
- In-note find dialog with case-sensitive and whole-word options
- In-place editor context menu with date/time insertion and file-path picker
- File attachments via drag & drop
- Clickable URLs in note text
- Configurable initial text with date/time variables and live preview
- Import / export of notes (`.unote`) and direct printing via the Windows print dialog
- Settings dialog (Layout, Keyboard, General, Misc, Note List) — configurable shortcuts shown directly in the context menus
- Global hotkeys (Ctrl+Shift+N, Ctrl+Shift+L) plus per-note shortcuts (Delete, Always-on-Top, Hide)
- Optional Windows-startup launch (opt-in, single HKCU\…\Run value)
- English and German localization
- JSON-based portable storage — notes and settings stay in the EXE folder

### System Requirements

- Windows 10 or later
- No external dependencies

### Links

- [Source Code](https://github.com/HJS-Lab/UltraNote)
- [All Releases](https://github.com/HJS-Lab/UltraNote/releases)
