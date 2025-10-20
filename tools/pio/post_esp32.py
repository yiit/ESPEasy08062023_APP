# Part of ESPEasy build toolchain.

Import("env")
from SCons.Script import COMMAND_LINE_TARGETS

platform = env.PioPlatform()

import sys
from os.path import join

# esptool modülünü import et
sys.path.append(join(platform.get_package_dir("tool-esptoolpy")))
import esptool


def _to_str(x):
    try:
        return env.subst(str(x))
    except Exception:
        return str(x)


def _collect_flash_extra_images():
    """
    FLASH_EXTRA_IMAGES şunlardan biri olabilir:
      - [("0x1000", "bootloader.bin"), ("0x8000", "partitions.bin"), ...]
      - [["0x1000", "..."], ...]
      - ["0x1000 path/to/file.bin", ...]
    Bunu daima [(addr, file), ...] listesine çevirir.
    """
    pairs = []
    extra = env.get("FLASH_EXTRA_IMAGES", [])
    if not extra:
        return pairs

    for item in extra:
        if isinstance(item, (list, tuple)) and len(item) == 2:
            addr = _to_str(item[0])
            f = _to_str(item[1])
            pairs.append((addr, f))
        elif isinstance(item, str):
            parts = item.strip().split(None, 1)
            if len(parts) == 2:
                pairs.append((parts[0], parts[1]))
        else:
            # SCons Node vs. beklenmeyen tip: stringe çevirip dene
            s = _to_str(item)
            parts = s.strip().split(None, 1)
            if len(parts) == 2:
                pairs.append((parts[0], parts[1]))
    return pairs


def esp32_create_combined_bin(source, target, env):
    # upload / erase_upload sırasında birleştirme atla (seri upload akışını bozmasın)
    targets = [str(t).lower() for t in COMMAND_LINE_TARGETS]
    if any("upload" in t for t in targets):
        print("Skip factory merge on upload/erase_upload target")
        return

    print("Generating combined binary for serial flashing")

    app_offset = 0x10000  # partitions.csv'den gelen app0 başlangıcı

    new_file_name = env.subst("$BUILD_DIR/${PROGNAME}.factory.bin")
    firmware_name = env.subst("$BUILD_DIR/${PROGNAME}.bin")
    chip = env.get("BOARD_MCU")
    flash_size = env.BoardConfig().get("upload.flash_size")
    flash_freq = env.BoardConfig().get("build.f_flash", "40m").replace("000000L", "m")
    flash_mode = env.BoardConfig().get("build.flash_mode", "dio")
    memory_type = env.BoardConfig().get("build.arduino.memory_type", "qio_qspi")

    # Bazı bellek tiplerinde mod zorunlu değiştirilir
    if flash_mode in ("qio", "qout"):
        flash_mode = "dio"
    if memory_type in ("opi_opi", "opi_qspi"):
        flash_mode = "dout"

    # FLASH_EXTRA_IMAGES'i düzgün [(addr, file), ...] listeye çevir
    sections = _collect_flash_extra_images()

    print("    Offset | File")
    for addr, f in sections:
        print(f" -  {addr} | {f}")
    print(f" - {hex(app_offset)} | {firmware_name}")

    # esptool komut satırı (merge_bin: alt çizgili!)
    argv = [
        "--chip", chip,
        "merge_bin",
        "--output", new_file_name,   # alt çizgi!
        "--flash_mode", flash_mode,  # alt çizgi!
        "--flash_freq", flash_freq,  # alt çizgi!
        "--flash_size", flash_size,  # alt çizgi!
    ]
    for addr, f in sections:
        argv += [addr, f]
    argv += [hex(app_offset), firmware_name]

    print("Using esptool.py arguments: %s" % " ".join(argv))

    # Esptool sürümlerine uyumlu çağırım:
    # - Bazıları _main() bekler ve sys.argv kullanır
    # - Bazıları main(argv) alır ve SystemExit(0) atar
    try:
        if hasattr(esptool, "_main"):
            old_argv = sys.argv
            try:
                sys.argv = ["esptool.py"] + argv
                rc = esptool._main()  # argüman verilmez!
            finally:
                sys.argv = old_argv
            if rc not in (0, None):
                raise RuntimeError(f"esptool returned {rc}")
        else:
            try:
                esptool.main(argv)  # derleyip SystemExit atabilir
            except SystemExit as e:
                if e.code not in (0, None):
                    raise
    except Exception as e:
        print(f"[esptool merge_bin ERROR] {e}")
        raise


env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", esp32_create_combined_bin)
