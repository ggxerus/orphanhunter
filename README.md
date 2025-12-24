# OrphanHunter

**OrphanHunter** is a lightweight utility for Arch Linux designed to identify and remove orphaned packages and files that clutter your system. It helps maintain a clean environment by tracking down dependencies that are no longer required.

## Features
- 🔍 **Deep Scanning:** Identifies true orphans that standard tools might miss.
- ⚡ **Fast Execution:** Minimal overhead system scanning.
- 🛡️ **Safe Removal:** Prompts for user confirmation before deletion.

---

## Installation

### From the AUR
orphanhunter is available in the Arch User Repository (AUR). You can install it using your preferred AUR helper.

#### Using yay
```bash
yay -S orphanhunter
```
#### Using paru
paru -S orphanhunter

###Manual Installation

If you prefer not to use a helper, you can build it from source:
```bash
git clone https://aur.archlinux.org/orphanhunter.git
cd orphanhunter
makepkg -si
```
###Usage

To start scanning for orphaned files, simply run the tool from your terminal:

orphanhunter
