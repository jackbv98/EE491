import numpy as np
from PIL import Image, ImageEnhance

NUM_LEDS = 144
DIVISIONS_PER_ROTATION = 120
STRIPS = 3

def translate_image_three_strips(img):
    img = img.convert("RGB")
    width, height = img.size
    max_dim = max(width, height)
    new_size = (int((width / max_dim) * 600), int((height / max_dim) * 600))
    img = img.resize(new_size)
    img = ImageEnhance.Color(img).enhance(2.0)
    arr = np.asarray(img)
    h, w, _ = arr.shape
    min_dim = min(w, h)

    data = np.zeros((DIVISIONS_PER_ROTATION, STRIPS, NUM_LEDS, 3), dtype=np.uint8)
    offset = -np.pi / 2
    strip_offsets = [0, 2*np.pi/3, 4*np.pi/3]

    for t in range(DIVISIONS_PER_ROTATION):
        base_theta = t * (2*np.pi / DIVISIONS_PER_ROTATION) + offset
        for s, so in enumerate(strip_offsets):
            theta = base_theta + so
            cos_t, sin_t = np.cos(theta), np.sin(theta)
            for l in range(NUM_LEDS):
                r = l / NUM_LEDS
                x_raw = cos_t * r
                y_raw = sin_t * r
                x = np.interp(-x_raw, [-1, 1], [(w/2)-(min_dim/2), (w/2)+(min_dim/2)])
                y = np.interp(y_raw, [-1, 1], [(h/2)-(min_dim/2), (h/2)+(min_dim/2)])
                xi = int(np.clip(x, 0, w-1))
                yi = int(np.clip(y, 0, h-1))
                data[t, s, l] = arr[yi, xi]
    return data

if __name__ == "__main__":
    img = Image.open("bear.bmp")
    data = translate_image_three_strips(img)
    flat = data.flatten()

    with open("image_data.h", "w") as f:
        f.write("#pragma once\n")
        f.write(f"#define NUM_LEDS {NUM_LEDS}\n")
        f.write(f"#define DIVISIONS {DIVISIONS_PER_ROTATION}\n")
        f.write(f"#define STRIPS {STRIPS}\n")
        f.write("const uint8_t PROGMEM image_data[] = {\n")
        for i, val in enumerate(flat):
            f.write(f"{val},")
            if (i+1) % 12 == 0:
                f.write("\n")
        f.write("};\n")

