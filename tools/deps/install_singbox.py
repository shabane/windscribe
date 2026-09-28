#!/usr/bin/env python3
# ------------------------------------------------------------------------------
# Windscribe VPN Build System - sing-box Installer
# ------------------------------------------------------------------------------
import os
import shutil
import sys
import subprocess
import urllib.request
import tarfile
import zipfile
import platform

TOOLS_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROOT_DIR = os.path.dirname(TOOLS_DIR)
BUILD_LIBS_DIR = os.path.join(ROOT_DIR, "build-libs", "singbox")

SINGBOX_VERSION = "1.11.4"

def get_os_arch():
    system = platform.system().lower()
    machine = platform.machine().lower()
    
    if system == "darwin":
        os_name = "darwin"
    elif system == "linux":
        os_name = "linux"
    elif system == "windows":
        os_name = "windows"
    else:
        raise ValueError(f"Unsupported OS: {system}")
        
    if machine in ("arm64", "aarch64"):
        arch_name = "arm64"
    elif machine in ("x86_64", "amd64"):
        arch_name = "amd64"
    else:
        raise ValueError(f"Unsupported Arch: {machine}")
        
    return os_name, arch_name

def install():
    os.makedirs(BUILD_LIBS_DIR, exist_ok=True)
    target_exe = os.path.join(BUILD_LIBS_DIR, "sing-box" if platform.system() != "Windows" else "sing-box.exe")
    
    # 1. Check if brew or system sing-box is available
    system_bins = ["/opt/homebrew/bin/sing-box", "/usr/local/bin/sing-box"]
    for sb in system_bins:
        if os.path.exists(sb):
            print(f"Found system sing-box at {sb}, copying to {target_exe}")
            shutil.copy2(sb, target_exe)
            os.chmod(target_exe, 0o755)
            print("sing-box installed successfully.")
            return

    # 2. Check which sing-box
    which_sb = shutil.which("sing-box")
    if which_sb:
        print(f"Found sing-box in PATH at {which_sb}, copying to {target_exe}")
        shutil.copy2(which_sb, target_exe)
        os.chmod(target_exe, 0o755)
        print("sing-box installed successfully.")
        return

    # 3. If Go is available, try go install
    which_go = shutil.which("go")
    if which_go:
        print("Building sing-box using Go...")
        try:
            cmd = ["go", "install", "-v", "-tags", "with_gvisor,with_quic,with_dhcp,with_wireguard,with_ech,with_utls", f"github.com/sagernet/sing-box/cmd/sing-box@v{SINGBOX_VERSION}"]
            subprocess.check_call(cmd)
            gopath = subprocess.check_output(["go", "env", "GOPATH"]).decode().strip()
            built_bin = os.path.join(gopath, "bin", "sing-box")
            if os.path.exists(built_bin):
                shutil.copy2(built_bin, target_exe)
                os.chmod(target_exe, 0o755)
                print(f"sing-box built and installed to {target_exe}")
                return
        except Exception as e:
            print(f"Go build failed: {e}, falling back to release download...")

    # 4. Download release from GitHub
    os_name, arch_name = get_os_arch()
    ext = "zip" if os_name == "windows" else "tar.gz"
    url = f"https://github.com/SagerNet/sing-box/releases/download/v{SINGBOX_VERSION}/sing-box-{SINGBOX_VERSION}-{os_name}-{arch_name}.{ext}"
    print(f"Downloading sing-box from {url}...")
    
    archive_path = os.path.join(BUILD_LIBS_DIR, f"singbox.{ext}")
    urllib.request.urlretrieve(url, archive_path)
    
    if ext == "tar.gz":
        with tarfile.open(archive_path, "r:gz") as tar:
            for member in tar.getmembers():
                if member.name.endswith("sing-box"):
                    f = tar.extractfile(member)
                    with open(target_exe, "wb") as out:
                        out.write(f.read())
                    break
    elif ext == "zip":
        with zipfile.ZipFile(archive_path, 'r') as zip_ref:
            for name in zip_ref.namelist():
                if name.endswith("sing-box.exe"):
                    with open(target_exe, "wb") as out:
                        out.write(zip_ref.read(name))
                    break

    if os.path.exists(archive_path):
        os.remove(archive_path)

    if os.path.exists(target_exe):
        os.chmod(target_exe, 0o755)
        print(f"sing-box downloaded and installed successfully to {target_exe}")
    else:
        raise RuntimeError("Failed to extract sing-box binary")

if __name__ == "__main__":
    install()
