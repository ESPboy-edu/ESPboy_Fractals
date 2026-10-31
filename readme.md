# ESPboy Fractal Explorer 🌌

An interactive, highly optimized fractal explorer designed for the [ESPboy](https://www.espboy.com/) platform. Dive deep into the infinite beauty of mathematics right from your pocket! 

This project goes beyond a simple static renderer: it features a responsive UI, progressive multi-pass rendering, a smart targeting crosshair, and a "time machine" history stack for seamless zooming and exploration.

## ✨ Features

* **10 Unique Fractal Worlds:** Explore a diverse collection of mathematical universes:
  * *Mandelbrot, Julia, Burning Ship, Tricorn (Mandelbar), Celtic, Buffalo, Cubic Mandelbrot, Cubic Julia, Perpendicular Mandelbrot, and Julia Burning Ship.*
* **Progressive Multi-Pass Rendering:** Never stare at a frozen screen! The image resolves in 4 progressive passes (8x8, 4x4, 2x2, 1x1 pixels). Combined with a cooperative multitasking loop (15ms yield), the device remains fully responsive even during heavy floating-point calculations.
* **Smart Navigation Crosshair:** Move around using a high-contrast targeting reticle (white core with a black stroke for visibility on any background). The crosshair auto-hides after 1 second of inactivity to let you enjoy the view.
* **Time Machine (History Stack):** The explorer remembers your last 15 navigation steps. Zoomed in too far? Just hit `ESC` to instantly rewind to your exact previous coordinates, zoom level, and iteration depth.
* **Dynamic Detailing:** As you zoom deeper into a fractal, the algorithm automatically increases the `maxIter` limit (adding +16 iterations per zoom level) to reveal hidden micro-structures. The 36-color palettes seamlessly loop to create a hypnotic, psychedelic effect at extreme depths.
* **Custom Palettes:** Switch instantly between 3 atmospheric color schemes (Fire, Ice, and Poison).

## 🎮 Controls

| Button | Action |
| :--- | :--- |
| **D-Pad (Arrows)** | Activate and move the targeting crosshair. |
| **ACT (A Button)** | **Zoom In** at the crosshair's exact location. |
| **ESC (B Button)** | **Zoom Out / Undo** (Rewind time to the previous view). |
| **LFT (Left Side)** | Cycle through the 10 available Fractal types. |
| **RGT (Right Side)**| Cycle through the color palettes. |


## 📄 License
This project is open-source. Feel free to modify, optimize, and share!