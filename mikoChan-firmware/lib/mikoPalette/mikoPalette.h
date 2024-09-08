#ifndef MIKOFACE_PALETTE_H_
#define MIKOFACE_PALETTE_H_

#include <M5GFX.h>

typedef struct RGB {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB;

typedef struct HSL {
    int h;
    int s;
    int l;
} HSL;

class colorPalette {
  public:
    colorPalette(uint8_t, uint8_t, uint8_t);
    colorPalette(uint32_t);
    colorPalette(HSL);
    HSL* getHSL();
    RGB* getRGB();
    uint32_t getRGB24();
    uint16_t getRGB16();
    uint8_t getRGB8();
    void set(uint32_t);
    void set(RGB);
    void set(HSL);

  protected:
    uint32_t c24;
    RGB rgb;
    HSL hsl;
    void RGB24toHSL(uint32_t, HSL*);
    uint32_t HSLtoRGB24(HSL);
};

/* namespace mikoPalette { */
/**/
/* extern uint16_t skin; */
/* extern uint16_t eye_upper; */
/* extern uint16_t eye_lower; */
/* extern uint16_t mouth; */
/* extern uint16_t cheek; */
/* extern uint16_t egde; */
/**/
/* } */
namespace mikoPalette {
extern colorPalette skin;
extern colorPalette eye_upper;
extern colorPalette eye_lower;
extern colorPalette mouth;
extern colorPalette cheek;
extern colorPalette edge;
extern colorPalette hot;
extern colorPalette cold;
}  // namespace mikoPalette

#endif  // MIKOFACE_PALETTE_H_
