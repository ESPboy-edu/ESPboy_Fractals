#include "lib/ESPboyInit.h"
#include "lib/ESPboyInit.cpp"
#include <math.h>

/*
#include "lib/ESPboyTerminalGUI.h"
#include "lib/ESPboyTerminalGUI.cpp"
#include "lib/ESPboyOTA2.h"
#include "lib/ESPboyOTA2.cpp"
*/

ESPboyInit myESPboy;

#define C16(c) (uint16_t)(((c) >> 8) | ((c) << 8))

const PROGMEM uint16_t palRed[36] = {
  C16(0x0000), C16(0x2800), C16(0x5000), C16(0x7800), C16(0xA000), C16(0xC800),
  C16(0xE200), C16(0xEC00), C16(0xF500), C16(0xFA00), C16(0xFC00), C16(0xFD00),
  C16(0xFE00), C16(0xFEA0), C16(0xFF40), C16(0xFF80), C16(0xFFA0), C16(0xFFC0),
  C16(0xFFE0), C16(0xFFE0), C16(0xFFE0), C16(0xFFE4), C16(0xFFE8), C16(0xFFEC),
  C16(0xFFF0), C16(0xFFF2), C16(0xFFF5), C16(0xFFF8), C16(0xFFFA), C16(0xFFFC),
  C16(0xFFFD), C16(0xFFFE), C16(0xFFFF), C16(0xFFFF), C16(0xFFFF), C16(0xFFFF)
};

const PROGMEM uint16_t palBlue[36] = {
  C16(0x0000), C16(0x0008), C16(0x0012), C16(0x001A), C16(0x009F), C16(0x011F),
  C16(0x021F), C16(0x031F), C16(0x04DF), C16(0x05DF), C16(0x06BF), C16(0x079F),
  C16(0x07DF), C16(0x07FF), C16(0x17FF), C16(0x37FF), C16(0x57FF), C16(0x77FF),
  C16(0x87FF), C16(0x97FF), C16(0xA7FF), C16(0xB7FF), C16(0xC7FF), C16(0xD7FF),
  C16(0xE7FF), C16(0xF7FF), C16(0xF7FF), C16(0xEFFF), C16(0xF7FF), C16(0xFBFF),
  C16(0xFDFF), C16(0xFEFF), C16(0xFFFF), C16(0xFFFF), C16(0xFFFF), C16(0xFFFF)
};

const PROGMEM uint16_t palGreen[36] = {
  C16(0x0000), C16(0x00C0), C16(0x0180), C16(0x0240), C16(0x0320), C16(0x0420),
  C16(0x0540), C16(0x0640), C16(0x0760), C16(0x1780), C16(0x27A0), C16(0x37C0),
  C16(0x47E0), C16(0x67E0), C16(0x87E0), C16(0xA7E0), C16(0xC7E0), C16(0xE7E0),
  C16(0xF7E0), C16(0xF7E4), C16(0xF7E8), C16(0xF7EC), C16(0xF7F0), C16(0xF7F4),
  C16(0xF7F8), C16(0xF7FC), C16(0xF7FE), C16(0xF7FF), C16(0xE7FF), C16(0xEFFF),
  C16(0xF7FF), C16(0xFBFF), C16(0xFDFF), C16(0xFFFF), C16(0xFFFF), C16(0xFFFF)
};

const uint16_t* palettes[3] = { palRed, palBlue, palGreen };
uint8_t currentPalette = 0;

enum FractalType { 
  MANDELBROT, JULIA, BURNING_SHIP, TRICORN, CELTIC, 
  BUFFALO, CUBIC_MANDELBROT, CUBIC_JULIA, 
  PERPENDICULAR_MANDELBROT, JULIA_BURNING_SHIP 
};
FractalType currentFractal = MANDELBROT;

float offsetX = 0.0, offsetY = 0.0, zoom = 1.0;
int maxIter = 36; 
uint8_t lastKeys = 0;

// История (машина времени) 
#define MAX_HISTORY 15
float histX[MAX_HISTORY], histY[MAX_HISTORY], histZoom[MAX_HISTORY];
int histIter[MAX_HISTORY]; 
int histDepth = 0;

// Рендер и прицел
int crossX = 64, crossY = 64;
const int renderSteps[] = {8, 4, 2, 1}; 
int currentPass = 0;
int renderY = 0; 
bool needsRender = true;

// Управление состояниями
bool isNavigating = false;
uint32_t lastInteractionTime = 0;
uint32_t lastMoveTime = 0;

void resetView() {
  switch(currentFractal) {
    case MANDELBROT:             offsetX = -0.5;  offsetY =  0.0; zoom = 3.0 / 128.0; break;
    case JULIA:                  offsetX =  0.0;  offsetY =  0.0; zoom = 3.0 / 128.0; break;
    case BURNING_SHIP:           offsetX = -0.45; offsetY = -0.5; zoom = 3.5 / 128.0; break;
    case TRICORN:                offsetX =  0.0;  offsetY =  0.0; zoom = 3.0 / 128.0; break;
    case CELTIC:                 offsetX = -0.5;  offsetY =  0.0; zoom = 3.5 / 128.0; break;
    case BUFFALO:                offsetX = -0.5;  offsetY =  0.0; zoom = 3.5 / 128.0; break;
    case CUBIC_MANDELBROT:       offsetX =  0.0;  offsetY =  0.0; zoom = 3.0 / 128.0; break;
    case CUBIC_JULIA:            offsetX =  0.0;  offsetY =  0.0; zoom = 3.0 / 128.0; break;
    case PERPENDICULAR_MANDELBROT: offsetX = -0.5; offsetY = 0.0; zoom = 3.0 / 128.0; break;
    case JULIA_BURNING_SHIP:     offsetX =  0.0;  offsetY =  0.0; zoom = 3.5 / 128.0; break;
  }
  maxIter = 36; 
  crossX = 64; crossY = 64;
  currentPass = 0; renderY = 0;
  needsRender = true;
  isNavigating = false;
}

uint16_t getFractalPixel(int px, int py) {
  float cx = (px - 64) * zoom + offsetX;
  float cy = (py - 64) * zoom + offsetY;
  float zx = cx, zy = cy;
  int i = 0;

  switch (currentFractal) {
    case MANDELBROT:
      for (; i < maxIter; i++) { float zx2 = zx*zx, zy2 = zy*zy; if (zx2+zy2 > 4.0) break; zy = 2.0*zx*zy + cy; zx = zx2-zy2 + cx; } break;
    case JULIA:
      { float kx = -0.7, ky = 0.27015; for (; i < maxIter; i++) { float zx2 = zx*zx, zy2 = zy*zy; if (zx2+zy2 > 4.0) break; zy = 2.0*zx*zy + ky; zx = zx2-zy2 + kx; } } break;
    case BURNING_SHIP:
      for (; i < maxIter; i++) { float zx2 = zx*zx, zy2 = zy*zy; if (zx2+zy2 > 4.0) break; zy = fabs(2.0*zx*zy) + cy; zx = zx2-zy2 + cx; } break;
    case TRICORN:
      for (; i < maxIter; i++) { float zx2 = zx*zx, zy2 = zy*zy; if (zx2+zy2 > 4.0) break; zy = -2.0*zx*zy + cy; zx = zx2-zy2 + cx; } break;
    case CELTIC:
      for (; i < maxIter; i++) { float zx2 = zx*zx, zy2 = zy*zy; if (zx2+zy2 > 4.0) break; zy = 2.0*zx*zy + cy; zx = fabs(zx2-zy2) + cx; } break;
    case BUFFALO:
      for (; i < maxIter; i++) { float zx2 = zx*zx, zy2 = zy*zy; if (zx2+zy2 > 4.0) break; zy = -fabs(2.0*zx*zy) + cy; zx = fabs(zx2-zy2) + cx; } break;
    case CUBIC_MANDELBROT:
      for (; i < maxIter; i++) { float zx2 = zx*zx, zy2 = zy*zy; if (zx2+zy2 > 4.0) break; float nx = zx*(zx2-3.0*zy2) + cx; zy = zy*(3.0*zx2-zy2) + cy; zx = nx; } break;
    case CUBIC_JULIA:
      { float kx = 0.4, ky = 0.0; for (; i < maxIter; i++) { float zx2 = zx*zx, zy2 = zy*zy; if (zx2+zy2 > 4.0) break; float nx = zx*(zx2-3.0*zy2) + kx; zy = zy*(3.0*zx2-zy2) + ky; zx = nx; } } break;
    case PERPENDICULAR_MANDELBROT:
      for (; i < maxIter; i++) { float zx2 = zx*zx, zy2 = zy*zy; if (zx2+zy2 > 4.0) break; zy = -2.0*fabs(zx)*zy + cy; zx = zx2-zy2 + cx; } break;
    case JULIA_BURNING_SHIP:
      { float kx = -0.7, ky = -0.3; for (; i < maxIter; i++) { float zx2 = zx*zx, zy2 = zy*zy; if (zx2+zy2 > 4.0) break; zy = fabs(2.0*zx*zy) + ky; zx = zx2-zy2 + kx; } } break;
  }

  if (i == maxIter) return 0x0000;
  return palettes[currentPalette][i % 36]; 
}

// Отрисовка прицела с черной контрастной обводкой
void processCrosshair(int cx, int cy, bool erase) {
  for (int dy = -4; dy <= 4; dy++) {
    for (int dx = -4; dx <= 4; dx++) {
      
      // Выбиваем ровно 1 пиксель в самом центре, чтобы видеть цель
      if (dx == 0 && dy == 0) continue; 
      
      // Убираем углы, формируя крест
      if (abs(dx) >= 2 && abs(dy) >= 2) continue; 
      
      // Белое ядро прицела
      bool isWhite = (dx == 0 && abs(dy) >= 2 && abs(dy) <= 3) || 
                     (dy == 0 && abs(dx) >= 2 && abs(dx) <= 3);
                     
      int px = cx + dx;
      int py = cy + dy;
      
      if (px >= 0 && px < 128 && py >= 0 && py < 128) {
        if (erase) {
          // Восстанавливаем кусок фрактала
          myESPboy.tft.drawPixel(px, py, getFractalPixel(px, py));
        } else {
          // Рисуем белый пиксель, а всё что вокруг него - черная обводка
          myESPboy.tft.drawPixel(px, py, isWhite ? 0xFFFF : 0x0000);
        }
      }
    }
  }
}

void setup() {
  myESPboy.begin("ESPboy Fractals");
  
/*
  //Check OTA2
  if (myESPboy.getKeys()&PAD_ACT || myESPboy.getKeys()&PAD_ESC) { 
     ESPboyTerminalGUI *terminalGUIobj = new ESPboyTerminalGUI(&myESPboy.tft, &myESPboy.mcp);
     ESPboyOTA2 *OTA2obj = new ESPboyOTA2(terminalGUIobj);
  }
*/
  
  resetView();
  myESPboy.tft.fillScreen(0x0000);
}

void loop() {
  uint8_t keys = myESPboy.getKeys();
  uint8_t pressed = keys & ~lastKeys;

  if (keys & (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT)) {
    lastInteractionTime = millis();
    if (!isNavigating) {
      isNavigating = true;
      processCrosshair(crossX, crossY, false); 
    }
  }

  if (isNavigating && (millis() - lastInteractionTime > 1000)) {
    isNavigating = false;
    processCrosshair(crossX, crossY, true); 
  }

  if (isNavigating && (keys & (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT))) {
    if (millis() - lastMoveTime > 25) { 
      processCrosshair(crossX, crossY, true); 

      if (keys & PAD_UP)    crossY -= 1;
      if (keys & PAD_DOWN)  crossY += 1;
      if (keys & PAD_LEFT)  crossX -= 1;
      if (keys & PAD_RIGHT) crossX += 1;

      crossX = constrain(crossX, 0, 127);
      crossY = constrain(crossY, 0, 127);

      processCrosshair(crossX, crossY, false); 
      lastMoveTime = millis();
    }
  }

  // --- ACT: ЗУМ ВПЕРЕД ---
  if (pressed & PAD_ACT) { 
    if (histDepth < MAX_HISTORY) {
      histX[histDepth] = offsetX; histY[histDepth] = offsetY; histZoom[histDepth] = zoom; histIter[histDepth] = maxIter;
      histDepth++;
    } else {
      for(int i = 1; i < MAX_HISTORY; i++) { 
        histX[i-1] = histX[i]; histY[i-1] = histY[i]; histZoom[i-1] = histZoom[i]; histIter[i-1] = histIter[i];
      }
      histX[MAX_HISTORY-1] = offsetX; histY[MAX_HISTORY-1] = offsetY; histZoom[MAX_HISTORY-1] = zoom; histIter[MAX_HISTORY-1] = maxIter;
    }
    
    offsetX = (crossX - 64) * zoom + offsetX;
    offsetY = (crossY - 64) * zoom + offsetY;
    
    zoom *= 0.5; 
    maxIter += 16; 
    
    crossX = 64; crossY = 64; 
    currentPass = 0; renderY = 0; needsRender = true; isNavigating = false;
  }

  // --- ESC: ЗУМ НАЗАД ПО ИСТОРИИ ---
  if (pressed & PAD_ESC) { 
    if (histDepth > 0) {
      histDepth--;
      offsetX = histX[histDepth]; offsetY = histY[histDepth]; zoom = histZoom[histDepth]; maxIter = histIter[histDepth];
    } else {
      zoom *= 2.0; 
      if (maxIter > 36) maxIter -= 16; 
    }
    crossX = 64; crossY = 64;
    currentPass = 0; renderY = 0; needsRender = true; isNavigating = false;
  }

  if (pressed & PAD_LFT) { 
    currentFractal = (FractalType)((currentFractal + 1) % 10);
    histDepth = 0; 
    resetView();
  }

  if (pressed & PAD_RGT) {
    currentPalette = (currentPalette + 1) % 3;
    currentPass = 0; renderY = 0; needsRender = true; isNavigating = false;
  }
  
  lastKeys = keys;

  if (!isNavigating && needsRender && currentPass <= 3) {
    int step = renderSteps[currentPass];
    int prevStep = (currentPass == 0) ? 128 : renderSteps[currentPass - 1];
    
    uint32_t startTime = millis(); 
    
    while (renderY < 128) {
      for (int x = 0; x < 128; x += step) {
        if (currentPass > 0 && (x % prevStep == 0) && (renderY % prevStep == 0)) continue;
        uint16_t color = getFractalPixel(x, renderY);
        myESPboy.tft.fillRect(x, renderY, step, step, color);
      }
      renderY += step;
      
      if (millis() - startTime > 15) break; 
    }
    
    if (renderY >= 128) {
      renderY = 0;
      currentPass++;
      if (currentPass > 3) needsRender = false;
    }
  }
}
