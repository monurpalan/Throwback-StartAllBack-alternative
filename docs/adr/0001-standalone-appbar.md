# ADR 0001: Standalone AppBar instead of hooking Explorer

**Status:** accepted · 2026-11-02

## Context

Tools like StartAllBack and ExplorerPatcher customize the taskbar by injecting
a DLL into `explorer.exe` and hooking undocumented internal functions. That
gives full fidelity (the real tray, Win+X, notifications keep working) but
couples the tool to Windows internals that change with every feature update,
and injection is routinely flagged by antivirus software.

## Decision

The taskbar and start menu are implemented as our own windows, registered with
the documented AppBar API (`SHAppBarMessage`). Hooking is reserved for features
that cannot be done any other way (classic context menu, Explorer ribbon) and
would live in a separate, opt-in module.

## Consequences

- Windows updates rarely break the core product, since it only uses documented APIs.
- A bug in Throwback cannot crash Explorer.
- We have to reimplement pieces Explorer gives for free, most notably the
  system tray.
- The system taskbar is still present until we add an option to hide it.
