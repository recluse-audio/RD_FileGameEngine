"""
view_data_layout.py — visualize the DATA/ folder as nested boxes in a GUI.

Usage:
  python SCRIPTS/view_data_layout.py                        # opens repo DATA/
  python SCRIPTS/view_data_layout.py --project /path/to/DATA
  python SCRIPTS/view_data_layout.py -p /path/to/MyGame    # auto-detects DATA/ subdir

Left-click  a file (.md / .json / .png) to preview it in the right panel.
Right-click any folder header for context-aware actions:
  - LEVELS dir   → Add Level
  - LEVEL_N dir  → Add Scene
  - SCENE_N dir  → Add Scene (child scene)
  - Any folder   → New File / New Folder / Delete
  - Any file     → Delete
"""

import argparse
import importlib.util
import json
import os
import shutil
import sys
import tkinter as tk
from tkinter import font as tkfont, simpledialog, messagebox, filedialog

try:
    from tkinterdnd2 import TkinterDnD, DND_FILES
    _TkRoot = TkinterDnD.Tk
    HAS_DND = True
except ImportError:
    _TkRoot = tk.Tk
    HAS_DND = False

# --- Config ---
REPO_ROOT       = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
DATA_ROOT       = os.path.join(REPO_ROOT, "DATA")
LEVELS_DIR      = os.path.join(REPO_ROOT, "DATA", "LEVELS")
REFRESH_SCRIPT  = os.path.join(REPO_ROOT, "SCRIPTS", "refresh_game_data.py")
RESIZE_SCRIPT   = os.path.join(REPO_ROOT, "SCRIPTS", "resize_png.py")


def _load_refresh_module():
    """Import refresh_game_data as a module (cached after first load)."""
    if _load_refresh_module._mod is None:
        spec = importlib.util.spec_from_file_location("refresh_game_data", REFRESH_SCRIPT)
        mod  = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(mod)
        _load_refresh_module._mod = mod
    return _load_refresh_module._mod

_load_refresh_module._mod = None


def _load_resize_module():
    """Import SCRIPTS/resize_png as a module (cached after first load)."""
    if _load_resize_module._mod is None:
        spec = importlib.util.spec_from_file_location("resize_png", RESIZE_SCRIPT)
        mod  = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(mod)
        _load_resize_module._mod = mod
    return _load_resize_module._mod

_load_resize_module._mod = None


def run_refresh(data_root: str | None = None):
    """Regenerate level_info.json / scene_info.json / Default_Game_State.json."""
    try:
        _load_refresh_module().run(data_root)
    except Exception as e:
        print(f"[run_refresh] error: {e}", file=sys.stderr)

PAD      = 10
GAP      = 8
HEADER_H = 22
FILE_H   = 18
MIN_W    = 120

COLORS = {
    "folder_bg":   "#1e2a3a",
    "folder_hdr":  "#2e4a6a",
    "folder_text": "#a8c8f0",
    "file_bg":     "#263040",
    "file_text":   "#d0e8ff",
    "ext_json":    "#f0c040",
    "ext_png":     "#80e080",
    "ext_md":      "#e08060",
    "ext_ttf":     "#c080e0",
    "ext_other":   "#8090a0",
    "border":      "#4a6a8a",
    "canvas_bg":   "#121820",
    "preview_bg":  "#161e2c",
    "preview_hdr": "#1a2438",
    "preview_txt": "#c8dff0",
}

EXT_COLORS = {
    ".json": COLORS["ext_json"],
    ".png":  COLORS["ext_png"],
    ".md":   COLORS["ext_md"],
    ".ttf":  COLORS["ext_ttf"],
}

PREVIEW_EXTS = {".md", ".json", ".png"}

LORES_W, LORES_H = 320, 240


def gen_lores_pngs(data_root: str) -> tuple[int, list[str]]:
    """
    Walk all LEVEL_*/SCENE_* dirs and find PNGs whose native resolution exceeds
    320×240. For each one, write a 320×240 crop-to-fill version named
    <original>_320x240.png into the same scene directory as the source PNG.
    The lores PNG inherits the scene's friendly name from scene_info.json.

    Returns (count_generated, list_of_warning_strings).
    """
    try:
        resize_mod = _load_resize_module()
        from PIL import Image
    except Exception as e:
        return 0, [f"Failed to load resize module: {e}. Ensure Pillow is installed (pip install Pillow)."]

    levels_dir = os.path.join(data_root, "LEVELS")
    count      = 0
    warnings   = []

    if not os.path.isdir(levels_dir):
        return 0, [f"LEVELS directory not found: {levels_dir}"]

    for level_name in sorted(os.listdir(levels_dir)):
        level_path = os.path.join(levels_dir, level_name)
        if not os.path.isdir(level_path):
            continue

        for scene_name in sorted(os.listdir(level_path)):
            if not scene_name.startswith("SCENE_"):
                continue
            scene_path = os.path.join(level_path, scene_name)
            if not os.path.isdir(scene_path):
                continue

            info_path = os.path.join(scene_path, "scene_info.json")
            try:
                info = json.loads(open(info_path, encoding="utf-8").read()) if os.path.isfile(info_path) else {}
            except Exception:
                info = {}

            png_file = info.get("png", "")
            if not png_file:
                continue

            png_path = os.path.join(scene_path, png_file)
            if not os.path.isfile(png_path):
                warnings.append(f"Missing PNG: {png_path}")
                continue

            try:
                with Image.open(png_path) as img:
                    if img.width <= LORES_W and img.height <= LORES_H:
                        continue
                    src = img if img.mode == "RGB" else img.convert("RGB")
                    resized = resize_mod.resize_crop(src, (LORES_W, LORES_H))
                    base, _ = os.path.splitext(png_file)
                    lores_path = os.path.join(scene_path, f"{base}_{LORES_W}x{LORES_H}.png")
                    resized.save(lores_path, "PNG")
                    count += 1
            except Exception as e:
                warnings.append(f"Error processing {png_path}: {e}")

    return count, warnings


def ext_color(name):
    _, ext = os.path.splitext(name)
    return EXT_COLORS.get(ext.lower(), COLORS["ext_other"])


# --- Node classification ---

def node_kind(node) -> str:
    if node.name == "LEVELS":
        return "levels_dir"
    if node.name.startswith("LEVEL_"):
        return "level"
    if node.name.startswith("SCENE_"):
        return "scene"
    return "other"


# --- Next-number helpers ---

def _next_numbered_dir(parent_path: str, prefix: str) -> tuple[str, int]:
    existing = []
    for entry in os.scandir(parent_path):
        if entry.is_dir() and entry.name.startswith(prefix):
            try:
                existing.append(int(entry.name[len(prefix):]))
            except ValueError:
                pass
    n = max(existing, default=0) + 1
    return f"{prefix}{n}", n


# --- Tree builder ---

class Node:
    def __init__(self, name, path, is_dir):
        self.name   = name
        self.path   = path
        self.is_dir = is_dir
        self.children: list["Node"] = []

    def add(self, child):
        self.children.append(child)


def build_tree(path, name=None) -> Node:
    name = name or os.path.basename(path)
    node = Node(name, path, os.path.isdir(path))
    if node.is_dir:
        try:
            entries = sorted(os.scandir(path), key=lambda e: (not e.is_dir(), e.name.lower()))
            for entry in entries:
                node.add(build_tree(entry.path, entry.name))
        except PermissionError:
            pass
    return node


# --- Layout engine ---

def measure(node: Node, canvas_font, file_font) -> tuple[int, int]:
    if not node.is_dir:
        tw = file_font.measure(node.name) + 24
        return max(tw, MIN_W), FILE_H

    child_sizes = [measure(c, canvas_font, file_font) for c in node.children]
    inner_w = max((cw for cw, _ in child_sizes), default=0)
    inner_w = max(inner_w, canvas_font.measure(node.name) + 8)
    inner_h = sum(ch for _, ch in child_sizes) + GAP * max(len(node.children) - 1, 0)
    return max(inner_w + PAD * 2, MIN_W), HEADER_H + PAD + inner_h + PAD


def draw_node(canvas, node: Node, x, y, w, h, canvas_font, file_font, hit_areas, depth=0):
    if not node.is_dir:
        canvas.create_rectangle(x, y, x + w, y + h,
                                 fill=COLORS["file_bg"], outline=COLORS["border"], width=1)
        dot_color = ext_color(node.name)
        canvas.create_oval(x + 6, y + h // 2 - 4, x + 14, y + h // 2 + 4,
                            fill=dot_color, outline="")
        canvas.create_text(x + 20, y + h // 2, text=node.name,
                            anchor="w", fill=COLORS["file_text"], font=file_font)
        hit_areas.append((x, y, x + w, y + h, node))
        return

    canvas.create_rectangle(x, y, x + w, y + h,
                             fill=COLORS["folder_bg"], outline=COLORS["border"], width=1)
    canvas.create_rectangle(x, y, x + w, y + HEADER_H,
                             fill=COLORS["folder_hdr"], outline="", width=0)
    canvas.create_text(x + PAD, y + HEADER_H // 2, text=node.name,
                        anchor="w", fill=COLORS["folder_text"], font=canvas_font)
    canvas.create_text(x + w - PAD, y + HEADER_H // 2, text="+",
                        anchor="e", fill="#6a9abf", font=canvas_font)

    hit_areas.append((x, y, x + w, y + HEADER_H, node))

    cx = x + PAD
    cy = y + HEADER_H + PAD
    child_sizes = [measure(c, canvas_font, file_font) for c in node.children]
    inner_w = w - PAD * 2
    for child, (cw, ch) in zip(node.children, child_sizes):
        draw_node(canvas, child, cx, cy, inner_w, ch, canvas_font, file_font, hit_areas, depth + 1)
        cy += ch + GAP


# --- New Scene dialog ---

class NewSceneDialog(tk.Toplevel):
    """
    Modal dialog to create a new scene.
    Supports Browse buttons and (if tkinterdnd2 is installed) drag-and-drop
    of .md / .png files onto the dialog.

    result dict keys after OK:
      name, md, md_source, png, png_source
    *_source is the full path of an existing file to copy (None = create blank).
    """

    def __init__(self, parent, default_name: str, bg: str, fg: str, font):
        super().__init__(parent)
        self.title("New Scene")
        self.configure(bg=bg)
        self.resizable(False, False)
        self.grab_set()

        self.result: dict | None = None
        self._md_source  = None   # full path to copy from, or None
        self._png_source = None

        lbl_opts = dict(bg=bg, fg=fg, font=font, anchor="w")
        ent_opts = dict(bg="#263040", fg=fg, font=font, insertbackground=fg,
                        relief=tk.FLAT, bd=4)
        btn_opts = dict(bg="#263040", fg=fg, font=font,
                        activebackground="#303848", relief=tk.FLAT, padx=6)

        # Row 0 – scene name (spans all columns)
        tk.Label(self, text="Scene name:", **lbl_opts).grid(
            row=0, column=0, sticky="w", padx=(12, 4), pady=(12, 4))
        self.e_name = tk.Entry(self, width=28, **ent_opts)
        self.e_name.insert(0, default_name)
        self.e_name.grid(row=0, column=1, columnspan=2, padx=(0, 12), pady=(12, 4))

        # Rows 1-2 – md / png with Browse button
        for row, (label, attr, src_attr, ext_label, filetypes) in enumerate([
            ("MD file:",  "e_md",  "_md_source",
             ".md",  [("Markdown", "*.md"), ("All files", "*.*")]),
            ("PNG file:", "e_png", "_png_source",
             ".png", [("PNG image", "*.png"), ("All files", "*.*")]),
        ], start=1):
            tk.Label(self, text=label, **lbl_opts).grid(
                row=row, column=0, sticky="w", padx=(12, 4), pady=4)

            entry = tk.Entry(self, width=22, **ent_opts)
            entry.grid(row=row, column=1, pady=4)
            setattr(self, attr, entry)

            def _browse(a=attr, sa=src_attr, ft=filetypes):
                path = filedialog.askopenfilename(parent=self, filetypes=ft)
                if not path:
                    return
                getattr(self, a).delete(0, tk.END)
                getattr(self, a).insert(0, os.path.basename(path))
                setattr(self, sa, path)

            tk.Button(self, text="Browse…", command=_browse, **btn_opts).grid(
                row=row, column=2, padx=(4, 12), pady=4)

        # Drop-zone hint + DND wiring
        if HAS_DND:
            hint = tk.Label(self, text="or drop a .md / .png file anywhere here",
                            bg=bg, fg="#507090", font=font)
            hint.grid(row=3, column=0, columnspan=3, pady=(0, 4))
            self._wire_dnd()
        else:
            tk.Frame(self, bg=bg, height=4).grid(row=3, column=0)

        # Action buttons
        btn_frame = tk.Frame(self, bg=bg)
        btn_frame.grid(row=4, column=0, columnspan=3, pady=10)
        tk.Button(btn_frame, text="Create", font=font,
                  bg="#2e4a6a", fg=fg, activebackground="#3a5a7a", relief=tk.FLAT,
                  command=self._ok).pack(side=tk.LEFT, padx=6)
        tk.Button(btn_frame, text="Cancel", font=font,
                  bg="#263040", fg=fg, activebackground="#303848", relief=tk.FLAT,
                  command=self.destroy).pack(side=tk.LEFT, padx=6)

        self.e_name.focus_set()
        self.bind("<Return>", lambda _: self._ok())
        self.bind("<Escape>", lambda _: self.destroy())

        self.update_idletasks()
        px, py = self.master.winfo_rootx(), self.master.winfo_rooty()
        pw, ph = self.master.winfo_width(), self.master.winfo_height()
        self.geometry(f"+{px + pw//2 - self.winfo_width()//2}"
                      f"+{py + ph//2 - self.winfo_height()//2}")
        self.wait_window()

    def _wire_dnd(self):
        """Register every child widget as a drop target."""
        def _on_drop(event):
            # tkinterdnd2 wraps paths with braces when they contain spaces
            raw = event.data.strip()
            paths = self.tk.splitlist(raw)
            for path in paths:
                _, ext = os.path.splitext(path)
                ext = ext.lower()
                if ext == ".md":
                    self.e_md.delete(0, tk.END)
                    self.e_md.insert(0, os.path.basename(path))
                    self._md_source = path
                elif ext == ".png":
                    self.e_png.delete(0, tk.END)
                    self.e_png.insert(0, os.path.basename(path))
                    self._png_source = path

        for widget in (self, *self.winfo_children()):
            try:
                widget.drop_target_register(DND_FILES)
                widget.dnd_bind("<<Drop>>", _on_drop)
            except Exception:
                pass

    def _ok(self):
        name = self.e_name.get().strip()
        if not name:
            return
        self.result = {
            "name":       name,
            "md":         self.e_md.get().strip(),
            "md_source":  self._md_source,
            "png":        self.e_png.get().strip(),
            "png_source": self._png_source,
        }
        self.destroy()


# --- Main GUI ---

def _resolve_project_root(path: str) -> str:
    """
    Accept either a DATA folder or a project root that contains a DATA/ subfolder.
    Returns the absolute path to the DATA folder, or raises SystemExit on failure.
    """
    path = os.path.abspath(path)
    # If the path itself contains LEVELS/, treat it as DATA/
    if os.path.isdir(os.path.join(path, "LEVELS")):
        return path
    # If it contains DATA/LEVELS/, descend one level
    candidate = os.path.join(path, "DATA")
    if os.path.isdir(os.path.join(candidate, "LEVELS")):
        return candidate
    print(f"error: '{path}' is not a valid project path.\n"
          f"  Expected a DATA folder (containing LEVELS/) or a project root "
          f"(containing DATA/LEVELS/).")
    sys.exit(1)


def main():
    parser = argparse.ArgumentParser(description="Visualize a game project DATA folder.")
    parser.add_argument(
        "-p", "--project", metavar="PATH",
        help="Path to the project DATA folder (or project root containing DATA/).",
    )
    args = parser.parse_args()

    if args.project:
        initial_root = _resolve_project_root(args.project)
        initial_name = os.path.basename(os.path.dirname(initial_root)) or os.path.basename(initial_root)
    else:
        initial_root = os.path.abspath(DATA_ROOT)
        initial_name = os.path.basename(REPO_ROOT)

    _project = {
        "root":   initial_root,
        "levels": os.path.join(initial_root, "LEVELS"),
        "name":   initial_name,
    }

    if not os.path.isdir(_project["root"]):
        print(f"DATA folder not found: {_project['root']}")
        sys.exit(1)

    win = _TkRoot()
    win.title(f"DATA Layout  —  {_project['name']}")
    win.configure(bg=COLORS["canvas_bg"])

    folder_font   = tkfont.Font(family="Consolas", size=9, weight="bold")
    file_font     = tkfont.Font(family="Consolas", size=8)
    preview_font  = tkfont.Font(family="Consolas", size=9)
    preview_hdr_font = tkfont.Font(family="Consolas", size=9, weight="bold")

    margin = 20

    # --- Toolbar ---
    toolbar = tk.Frame(win, bg=COLORS["preview_hdr"], pady=3)
    toolbar.pack(side=tk.TOP, fill=tk.X)

    _tb_btn = dict(bg="#263040", fg=COLORS["folder_text"], font=file_font,
                   activebackground="#2e4a6a", activeforeground="#ffffff",
                   relief=tk.FLAT, padx=10, pady=2)

    save_all_btn = tk.Button(toolbar, text="Save All", **_tb_btn)
    save_all_btn.pack(side=tk.LEFT, padx=(6, 2), pady=2)

    refresh_btn = tk.Button(toolbar, text="Refresh", **_tb_btn)
    refresh_btn.pack(side=tk.LEFT, padx=2, pady=2)

    lores_btn = tk.Button(toolbar, text="Refresh Lo-Res PNGs", **_tb_btn)
    lores_btn.pack(side=tk.LEFT, padx=2, pady=2)

    # --- Top-level paned split: tree left, preview right ---
    paned = tk.PanedWindow(win, orient=tk.HORIZONTAL, bg=COLORS["canvas_bg"],
                            sashwidth=5, sashrelief=tk.FLAT)
    paned.pack(fill=tk.BOTH, expand=True)

    # Left: scrollable canvas
    left_frame = tk.Frame(paned, bg=COLORS["canvas_bg"])
    paned.add(left_frame, stretch="always")

    vscroll = tk.Scrollbar(left_frame, orient=tk.VERTICAL)
    hscroll = tk.Scrollbar(left_frame, orient=tk.HORIZONTAL)
    canvas  = tk.Canvas(left_frame, bg=COLORS["canvas_bg"],
                         yscrollcommand=vscroll.set,
                         xscrollcommand=hscroll.set,
                         highlightthickness=0)
    vscroll.config(command=canvas.yview)
    hscroll.config(command=canvas.xview)
    vscroll.pack(side=tk.RIGHT, fill=tk.Y)
    hscroll.pack(side=tk.BOTTOM, fill=tk.X)
    canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

    canvas.bind("<MouseWheel>", lambda e: canvas.yview_scroll(int(-1 * (e.delta / 120)), "units"))

    # Right: preview panel
    right_frame = tk.Frame(paned, bg=COLORS["preview_bg"], width=340)
    paned.add(right_frame, stretch="never")

    # Header row: filename label + save button
    preview_hdr_frame = tk.Frame(right_frame, bg=COLORS["preview_hdr"])
    preview_hdr_frame.pack(side=tk.TOP, fill=tk.X)

    preview_header = tk.Label(preview_hdr_frame, text="No file selected",
                               bg=COLORS["preview_hdr"], fg=COLORS["folder_text"],
                               font=preview_hdr_font, anchor="w", padx=8, pady=4)
    preview_header.pack(side=tk.LEFT, fill=tk.X, expand=True)

    save_btn = tk.Button(preview_hdr_frame, text="Save", font=file_font,
                         bg="#2a5a3a", fg="#80e8a0", activebackground="#3a7a50",
                         relief=tk.FLAT, padx=8, pady=2, state=tk.DISABLED)
    save_btn.pack(side=tk.RIGHT, padx=4, pady=2)

    preview_text_scroll = tk.Scrollbar(right_frame, orient=tk.VERTICAL)
    preview_text = tk.Text(right_frame, bg=COLORS["preview_bg"], fg=COLORS["preview_txt"],
                            font=preview_font, wrap=tk.WORD, state=tk.DISABLED,
                            relief=tk.FLAT, borderwidth=0, padx=8, pady=6,
                            yscrollcommand=preview_text_scroll.set,
                            insertbackground=COLORS["preview_txt"],
                            undo=True)
    preview_text_scroll.config(command=preview_text.yview)
    preview_text_scroll.pack(side=tk.RIGHT, fill=tk.Y)
    preview_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

    # Mutable preview state
    _preview_image_ref = [None]
    _preview_path      = [None]   # path of currently loaded text file, or None
    _is_modified       = [False]

    def _mark_modified(event=None):
        if not _is_modified[0]:
            _is_modified[0] = True
            name = os.path.basename(_preview_path[0]) if _preview_path[0] else ""
            preview_header.config(text=f"{name}  •")

    def _save_preview():
        path = _preview_path[0]
        if not path:
            return
        try:
            content = preview_text.get("1.0", tk.END)
            # tk.Text always appends a trailing newline; strip exactly one
            if content.endswith("\n"):
                content = content[:-1]
            with open(path, "w", encoding="utf-8") as f:
                f.write(content)
        except OSError as e:
            messagebox.showerror("Save Error", str(e), parent=win)
            return
        _is_modified[0] = False
        preview_header.config(text=os.path.basename(path))

    save_btn.config(command=_save_preview)
    win.bind("<Control-s>", lambda _: _save_preview())

    def _do_save_all():
        _save_preview()

    def _do_refresh():
        run_refresh(_project["root"])
        redraw()

    def _do_refresh_lores():
        count, warnings = gen_lores_pngs(_project["root"])
        if warnings:
            messagebox.showwarning("Refresh Lo-Res PNGs",
                                   "\n".join(warnings), parent=win)
        if count:
            redraw()

    save_all_btn.config(command=_do_save_all)
    refresh_btn.config(command=_do_refresh)
    lores_btn.config(command=_do_refresh_lores)

    def show_preview(node: Node):
        _, ext = os.path.splitext(node.name)
        ext = ext.lower()

        _preview_path[0]  = None
        _is_modified[0]   = False
        _preview_image_ref[0] = None

        preview_header.config(text=node.name)
        preview_text.config(state=tk.NORMAL)
        preview_text.delete("1.0", tk.END)

        if ext == ".png":
            save_btn.config(state=tk.DISABLED)
            preview_text.edit_reset()
            try:
                img = tk.PhotoImage(file=node.path)
                _preview_image_ref[0] = img
                preview_text.image_create(tk.END, image=img)
            except Exception as e:
                preview_text.insert(tk.END, f"[Could not load image]\n{e}")
            preview_text.config(state=tk.DISABLED)

        elif ext in (".md", ".json"):
            try:
                with open(node.path, encoding="utf-8", errors="replace") as f:
                    content = f.read()
                preview_text.insert(tk.END, content)
                preview_text.edit_reset()          # clear undo stack for fresh file
                preview_text.config(state=tk.NORMAL)
                _preview_path[0] = node.path
                save_btn.config(state=tk.NORMAL)
                preview_text.bind("<<Modified>>", _on_text_modified)
            except Exception as e:
                preview_text.insert(tk.END, f"[Could not read file]\n{e}")
                preview_text.config(state=tk.DISABLED)
                save_btn.config(state=tk.DISABLED)
        else:
            preview_text.config(state=tk.DISABLED)
            save_btn.config(state=tk.DISABLED)

    def _on_text_modified(event=None):
        if preview_text.edit_modified():
            _mark_modified()
            preview_text.edit_modified(False)   # reset flag so next change fires again

    def clear_preview():
        _preview_path[0]  = None
        _is_modified[0]   = False
        _preview_image_ref[0] = None
        preview_header.config(text="No file selected")
        preview_text.config(state=tk.NORMAL)
        preview_text.delete("1.0", tk.END)
        preview_text.config(state=tk.DISABLED)
        save_btn.config(state=tk.DISABLED)

    # --- Hit areas & redraw ---

    hit_areas: list[tuple[int, int, int, int, Node]] = []

    def redraw():
        nonlocal hit_areas
        canvas.delete("all")
        hit_areas = []
        tree = build_tree(_project["root"], "DATA")
        tw, th = measure(tree, folder_font, file_font)
        canvas_w = tw + margin * 2
        canvas_h = th + margin * 2
        canvas.config(scrollregion=(0, 0, canvas_w, canvas_h))
        draw_node(canvas, tree, margin, margin, tw, th, folder_font, file_font, hit_areas)
        screen_w = win.winfo_screenwidth()
        screen_h = win.winfo_screenheight()
        preview_w = 420
        total_w = min(canvas_w + preview_w, screen_w - 60)
        total_h = min(canvas_h + 20, screen_h - 100)
        win.geometry(f"{total_w}x{total_h}")
        win.update_idletasks()
        paned.sash_place(0, total_w - preview_w, 0)

    def canvas_to_world(event):
        return canvas.canvasx(event.x), canvas.canvasy(event.y)

    def find_node_at(cx, cy) -> Node | None:
        for x1, y1, x2, y2, node in hit_areas:
            if x1 <= cx <= x2 and y1 <= cy <= y2:
                return node
        return None

    # --- Context actions ---

    def add_scene(parent_node: Node):
        dir_name, n = _next_numbered_dir(parent_node.path, "SCENE_")

        dlg = NewSceneDialog(win, f"Scene {n}",
                             bg=COLORS["canvas_bg"], fg=COLORS["folder_text"],
                             font=file_font)
        if dlg.result is None:
            return

        dest = os.path.join(parent_node.path, dir_name)
        try:
            os.makedirs(dest)

            info: dict = {"name": dlg.result["name"], "zones": []}

            for key, src_key in (("md", "md_source"), ("png", "png_source")):
                fname  = dlg.result[key]
                source = dlg.result[src_key]
                if not fname:
                    continue
                info[key] = fname
                dest_file = os.path.join(dest, fname)
                if source and os.path.isfile(source):
                    shutil.copy2(source, dest_file)
                else:
                    open(dest_file, "w").close()

            with open(os.path.join(dest, "scene_info.json"), "w", encoding="utf-8") as f:
                json.dump(info, f, indent=2)
        except OSError as e:
            messagebox.showerror("Error", str(e), parent=win)
            return
        run_refresh(_project["root"])
        redraw()

    def add_level():
        dir_name, n = _next_numbered_dir(_project["levels"], "LEVEL_")
        dest = os.path.join(_project["levels"], dir_name)
        try:
            os.makedirs(dest)
            with open(os.path.join(dest, "level_info.json"), "w", encoding="utf-8") as f:
                json.dump({"name": f"Level {n}"}, f, indent=2)
        except OSError as e:
            messagebox.showerror("Error", str(e), parent=win)
            return
        run_refresh(_project["root"])
        redraw()

    def add_file(parent_node: Node):
        name = simpledialog.askstring("New File", f"File name (inside {parent_node.name}):", parent=win)
        if not name or not name.strip():
            return
        dest = os.path.join(parent_node.path, name.strip())
        if os.path.exists(dest):
            messagebox.showerror("Error", f"Already exists:\n{dest}", parent=win)
            return
        try:
            open(dest, "w").close()
        except OSError as e:
            messagebox.showerror("Error", str(e), parent=win)
            return
        redraw()

    def add_folder(parent_node: Node):
        name = simpledialog.askstring("New Folder", f"Folder name (inside {parent_node.name}):", parent=win)
        if not name or not name.strip():
            return
        dest = os.path.join(parent_node.path, name.strip())
        if os.path.exists(dest):
            messagebox.showerror("Error", f"Already exists:\n{dest}", parent=win)
            return
        try:
            os.makedirs(dest)
        except OSError as e:
            messagebox.showerror("Error", str(e), parent=win)
            return
        run_refresh(_project["root"])
        redraw()

    _PROTECTED: set = set()

    def _refresh_protected():
        _PROTECTED.clear()
        _PROTECTED.add(_project["root"])
        _PROTECTED.add(_project["levels"])

    _refresh_protected()

    def delete_node(node: Node):
        if os.path.abspath(node.path) in _PROTECTED:
            messagebox.showerror("Protected", f"{node.name} cannot be deleted.", parent=win)
            return
        msg = (f"Delete '{node.name}' and all its contents?\n\n{node.path}"
               if node.is_dir else f"Delete '{node.name}'?\n\n{node.path}")
        if not messagebox.askyesno("Confirm Delete", msg, icon="warning", parent=win):
            return
        try:
            shutil.rmtree(node.path) if node.is_dir else os.remove(node.path)
        except OSError as e:
            messagebox.showerror("Error", str(e), parent=win)
            return
        if node.is_dir:
            run_refresh(_project["root"])
        clear_preview()
        redraw()

    # --- Project management ---

    def set_project(new_root: str, name: str | None = None):
        """Switch the active project; updates _project and redraws."""
        _project["root"]   = os.path.abspath(new_root)
        _project["levels"] = os.path.join(_project["root"], "LEVELS")
        _project["name"]   = name or os.path.basename(os.path.dirname(new_root)) or os.path.basename(new_root)
        _refresh_protected()
        win.title(f"DATA Layout  —  {_project['name']}")
        clear_preview()
        redraw()

    def new_project():
        name = simpledialog.askstring("New Project", "Project name:", parent=win)
        if not name or not name.strip():
            return
        name = name.strip()
        dest_parent = filedialog.askdirectory(title="Choose location for new project", parent=win)
        if not dest_parent:
            return
        project_root = os.path.join(dest_parent, name)
        data_dir     = os.path.join(project_root, "DATA")
        if os.path.exists(project_root):
            messagebox.showerror("Error", f"Already exists:\n{project_root}", parent=win)
            return
        try:
            os.makedirs(os.path.join(data_dir, "LEVELS"),     exist_ok=True)
            gui_dir = os.path.join(data_dir, "GUI")
            os.makedirs(gui_dir,                               exist_ok=True)
            gs_dir  = os.path.join(data_dir, "GAME_STATE")
            os.makedirs(gs_dir,                                exist_ok=True)
            # Copy GUI assets (layout + fonts) from the current project
            curr_gui = os.path.join(_project["root"], "GUI")
            if os.path.isdir(curr_gui):
                for fname in os.listdir(curr_gui):
                    src = os.path.join(curr_gui, fname)
                    if os.path.isfile(src):
                        shutil.copy2(src, os.path.join(gui_dir, fname))
            # Write minimal default game state
            default_state = {
                "current_level": "",
                "current_scene": "",
                "levels":        {},
            }
            gs_path = os.path.join(gs_dir, "Default_Game_State.json")
            with open(gs_path, "w", encoding="utf-8") as f:
                json.dump(default_state, f, indent=2)
        except OSError as e:
            messagebox.showerror("Error", str(e), parent=win)
            return
        set_project(data_dir, name)

    def open_project():
        path = filedialog.askdirectory(
            title="Open project — select the DATA folder or project root", parent=win
        )
        if not path:
            return
        path = os.path.abspath(path)
        # Accept DATA folder directly (contains LEVELS/) or project root (contains DATA/LEVELS/)
        if os.path.isdir(os.path.join(path, "LEVELS")):
            data_path = path
        elif os.path.isdir(os.path.join(path, "DATA", "LEVELS")):
            data_path = os.path.join(path, "DATA")
        else:
            messagebox.showerror(
                "Invalid Project",
                "Could not find a LEVELS folder.\n\n"
                "Select either:\n"
                "  • the DATA folder (containing LEVELS/)\n"
                "  • the project root (containing DATA/LEVELS/)",
                parent=win,
            )
            return
        parent_name = os.path.basename(os.path.dirname(data_path))
        name = parent_name if parent_name else os.path.basename(data_path)
        set_project(data_path, name)

    def save_project_as():
        name = simpledialog.askstring(
            "Save Project As", "Project name:",
            initialvalue=_project["name"], parent=win,
        )
        if not name or not name.strip():
            return
        name = name.strip()
        dest_parent = filedialog.askdirectory(title="Choose destination folder", parent=win)
        if not dest_parent:
            return
        target = os.path.join(dest_parent, name)
        if os.path.exists(target):
            if not messagebox.askyesno(
                "Overwrite?", f"'{target}'\nalready exists. Overwrite?", parent=win
            ):
                return
            shutil.rmtree(target)
        try:
            shutil.copytree(_project["root"], target)
        except OSError as e:
            messagebox.showerror("Save Error", str(e), parent=win)
            return
        messagebox.showinfo("Saved", f"Project saved to:\n{target}", parent=win)

    # --- Menu bar ---

    _menu_opts = dict(bg="#1e2a3a", fg="#a8c8f0",
                      activebackground="#2e4a6a", activeforeground="#ffffff",
                      font=file_font)
    menubar   = tk.Menu(win, **_menu_opts)
    file_menu = tk.Menu(menubar, tearoff=0, **_menu_opts)
    file_menu.add_command(label="New Project…",      command=new_project)
    file_menu.add_command(label="Open Project…",     command=open_project)
    file_menu.add_separator()
    file_menu.add_command(label="Save Project As…",  command=save_project_as)
    menubar.add_cascade(label="File", menu=file_menu)
    win.config(menu=menubar)

    # --- Click handlers ---

    def on_left_click(event):
        cx, cy = canvas_to_world(event)
        node = find_node_at(cx, cy)
        if node is None or node.is_dir:
            return
        _, ext = os.path.splitext(node.name)
        if ext.lower() in PREVIEW_EXTS:
            show_preview(node)

    def on_right_click(event):
        cx, cy = canvas_to_world(event)
        node = find_node_at(cx, cy)
        if node is None:
            return

        kind = node_kind(node)
        menu = tk.Menu(win, tearoff=0,
                       bg="#1e2a3a", fg="#a8c8f0",
                       activebackground="#2e4a6a", activeforeground="#ffffff",
                       font=file_font)

        if kind == "levels_dir":
            menu.add_command(label="Add Level", command=add_level)
            menu.add_separator()

        if kind in ("level", "scene"):
            menu.add_command(label=f"Add Scene to {node.name}",
                             command=lambda n=node: add_scene(n))
            menu.add_separator()

        if node.is_dir:
            menu.add_command(label="New File...",   command=lambda n=node: add_file(n))
            menu.add_command(label="New Folder...", command=lambda n=node: add_folder(n))
            menu.add_separator()

        menu.add_command(label=f"Delete {node.name}...",
                         command=lambda n=node: delete_node(n),
                         foreground="#e07070", activeforeground="#ff9090")
        menu.tk_popup(event.x_root, event.y_root)

    canvas.bind("<Button-1>", on_left_click)
    canvas.bind("<Button-3>", on_right_click)

    redraw()
    win.mainloop()


if __name__ == "__main__":
    main()
