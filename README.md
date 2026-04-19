## GTK4 Layer Shell App Launcher
A minimalist, high-performance application launcher designed for Wayland compositors (like Sway or Hyprland). This project leverages GTK4 for a modern UI and gtk4-layer-shell to function as a desktop overlay.
## Features
- Wayland Native: Built specifically for Wayland using the Layer Shell protocol.

- Real-time Search: Instant filtering of installed applications as you type.

- Keyboard Focused:

- Enter to launch the selected (or first) application.

- Escape to quit the launcher instantly.

- Modern GTK4 API: Uses GtkFilterListModel and GtkListBox model binding for efficient UI updates and list handling.

- Exclusive Keyboard Focus: Automatically grabs focus upon launch for immediate input.

## Prerequisites
Before building, ensure you have the following libraries installed:
- GTK4 (Development files)

- gtk4-layer-shell

- GLib / GIO

Installation (Fedora/Ubuntu example)
```Bash
# Ubuntu/Debian
sudo apt install libgtk-4-dev libgtk4-layer-shell-dev

# Fedora
sudo dnf install gtk4-devel gtk4-layer-shell-devel
```
## Technical Overview
The application follows a modern GTK4 reactive pattern:
- Data Source: Retrieves all installed apps using g_app_info_get_all() and stores them in a GListStore.

- Filtering: Employs a GtkCustomFilter tied to a GtkFilterListModel. This ensures the UI only renders items matching the search query without destroying the underlying data.

- UI Binding: Uses gtk_list_box_bind_model to dynamically create rows (setup_list_row) as the filter changes.

- Shell Integration: Uses gtk4-layer-shell to set the window as an "overlay" layer, ensuring it appears above other windows and handles keyboard exclusivity correctly.