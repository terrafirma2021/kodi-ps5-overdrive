"""Generate the small original assets used by Estuary's PS5 layout.

Requires Pillow. Run from the repository root with:
    python addons/skin.estuary/tools/generate_ps5_assets.py
"""

from pathlib import Path
import math

from PIL import Image, ImageDraw


SKIN = Path(__file__).resolve().parents[1]


def background() -> None:
    width, height = 1920, 1080
    image = Image.new("RGB", (width, height))
    pixels = image.load()
    for y in range(height):
        fy = y / height
        for x in range(width):
            fx = x / width
            blue_glow = math.exp(-(((fx - 0.80) / 0.47) ** 2 + ((fy - 0.21) / 0.60) ** 2) * 2.2)
            violet_glow = math.exp(-(((fx - 0.22) / 0.38) ** 2 + ((fy - 1.04) / 0.60) ** 2) * 3.0)
            band = max(0.0, 1.0 - abs(fy - (0.18 + fx * 0.48)) / 0.27)
            pixels[x, y] = (
                int(5 + blue_glow * 8 + violet_glow * 6 + band * 2),
                int(10 + blue_glow * 21 + violet_glow * 11 + band * 4),
                int(24 + blue_glow * 48 + violet_glow * 35 + band * 9),
            )
    output = SKIN / "extras" / "backgrounds" / "ps5-home.png"
    image.save(output, optimize=True)


def overdrive_icon() -> None:
    scale = 4
    size = 128
    image = Image.new("RGBA", (size * scale, size * scale), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    s = lambda value: value * scale
    draw.rounded_rectangle((s(13), s(13), s(115), s(115)), radius=s(28),
                           fill=(15, 34, 73, 255), outline=(88, 189, 255, 255), width=s(3))
    draw.arc((s(31), s(31), s(97), s(97)), 204, 513, fill=(146, 222, 255, 255), width=s(6))
    draw.polygon([(s(61), s(39)), (s(88), s(64)), (s(61), s(89)),
                  (s(61), s(74)), (s(41), s(74)), (s(41), s(54)), (s(61), s(54))],
                 fill=(245, 250, 255, 255))
    output = SKIN / "media" / "icons" / "sidemenu" / "overdrive.png"
    image.resize((size, size), Image.Resampling.LANCZOS).save(output, optimize=True)


if __name__ == "__main__":
    background()
    overdrive_icon()
