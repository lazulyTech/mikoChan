#ifndef FACES_MIKOFACE_H_
#define FACES_MIKOFACE_H_

#include <M5Unified.h>  // TODO(meganetaaan): include only the Sprite function not a whole library
/* #include <Avatar.h> */
#include <BoundingRect.h>
#include <DrawContext.h>
#include <Drawable.h>
#include <mikoPalette.h>

namespace mp = mikoPalette;

namespace m5avatar {
class MikoEye : public Drawable {
  private:
    bool isLeft;

  public:
    MikoEye(bool isLeft = false) {this->isLeft = isLeft;}
    void draw(M5Canvas* spi, BoundingRect rect, DrawContext* ctx) {
        Expression exp = ctx->getExpression();
        uint32_t cx = rect.getCenterX();
        uint32_t cy = rect.getCenterY();
        Gaze g = ctx->getGaze();
        ColorPalette* cp = ctx->getColorPalette();
        uint16_t primaryColor = ctx->getColorDepth() == 1 ? 1 : cp->get(COLOR_PRIMARY);
        uint16_t backgroundColor = ctx->getColorDepth() == 1 ? ERACER_COLOR : cp->get(COLOR_BACKGROUND);
        uint32_t offsetX = g.getHorizontal() * 8;
        uint32_t offsetY = g.getVertical() * 5;
        float eor = ctx->getEyeOpenRatio();

        // cheek
        int x;
        if (!isLeft) x = cx - 30;
        else x = cx + 30;

        if (exp == Expression::Angry) spi->fillEllipse(x, cy + 30, 25, 15, mp::hot.getRGB8());
        else if (exp == Expression::Doubt) spi->fillEllipse(x, cy + 30, 25, 15, mp::cold.getRGB8());
        else spi->fillEllipse(x, cy + 30, 25, 15, mp::cheek.getRGB8());

        // eye closed
        if (eor == 0 || exp == Expression::Happy || exp == Expression::Angry) {
            // eye closed
            int xb, xe, y;
            xb = cx - 20;
            xe = cx + 20;
            y = cy;
            int w = 3;
            int h = 15;
            if (!isLeft) {
                spi->drawWideLine(xb, y - h, xe, y, w, mp::edge.getRGB8());
                spi->drawWideLine(xb, y + h, xe, y, w, mp::edge.getRGB8());
            } else {
                spi->drawWideLine(xb, y, xe, y - h, w, mp::edge.getRGB8());
                spi->drawWideLine(xb, y, xe, y + h, w, mp::edge.getRGB8());

            }
            return;
        } else if (exp == Expression::Sleepy || exp == Expression::Doubt) {
            spi->fillRect(cx - 25, cy - 2, 50, 4, mp::edge.getRGB8());
            return;
        }

        // eye opened
        spi->fillCircle(cx, cy, 30, mp::eye_upper.getRGB8());
        int tp[3][2] = {{5, -2}, {6, 10}, {25, 10}};
        for (int i = 0; i < 3; i++) {
            if (!isLeft) {
                tp[i][0] = cx - 30 + tp[i][0];
                tp[i][1] = cy - 30 + tp[i][1];
            } else {
                tp[i][0] = cx + 30 - tp[i][0];
                tp[i][1] = cy - 30 + tp[i][1];
            }
        }
        spi->fillTriangle(tp[0][0], tp[0][1], tp[1][0], tp[1][1], tp[2][0], tp[2][1], mp::eye_upper.getRGB8());

        M5Canvas low(spi);
        low.createSprite(61, 61);
        low.fillSprite(TFT_BLACK);
        low.fillEllipse(31+offsetX, 31+70, 50, 60, mp::eye_lower.getRGB8());
        M5Canvas tmp(&low);
        tmp.createSprite(61, 61);
        tmp.fillSprite(TFT_BLACK);
        tmp.fillCircle(31, 31, 30, TFT_WHITE);
        tmp.pushSprite(0, 0, TFT_WHITE);
        tmp.deleteSprite();
        low.pushSprite(cx - 31, cy - 31, TFT_BLACK);
        low.deleteSprite();

        /* int eyeSize = 7; */
        /* spi->fillEllipse(cx + offsetX - eyeSize, cy + offsetY - eyeSize + 15, eyeSize, eyeSize, TFT_WHITE); */
    }
};

class MikoMouth : public Drawable {
  private:
    uint16_t minWidth;
    uint16_t maxWidth;
    uint16_t minHeight;
    uint16_t maxHeight;

  public:
    MikoMouth() : MikoMouth(50, 90, 4, 60) {}
    MikoMouth(uint16_t minWidth, uint16_t maxWidth, uint16_t minHeight, uint16_t maxHeight) : minWidth{minWidth}, maxWidth{maxWidth}, minHeight{minHeight}, maxHeight{maxHeight} {}
    void draw(M5Canvas* spi, BoundingRect rect, DrawContext* ctx) {
        uint16_t primaryColor = ctx->getColorDepth() == 1 ? 1 : ctx->getColorPalette()->get(COLOR_PRIMARY);
        uint16_t backgroundColor = ctx->getColorDepth() == 1 ? ERACER_COLOR : ctx->getColorPalette()->get(COLOR_BACKGROUND);
        uint32_t cx = rect.getCenterX();
        uint32_t cy = rect.getCenterY();
        float openRatio = ctx->getMouthOpenRatio();
        uint32_t h = minHeight + (maxHeight - minHeight) * openRatio;
        uint32_t w = minWidth + (maxWidth - minWidth) * (1 - openRatio);
        if (h > minHeight) {
            int mouthR = 10 * openRatio + 5;
            spi->fillCircle(cx, cy - mouthR + 20, mouthR+4, mp::edge.getRGB8());
            spi->fillCircle(cx, cy - mouthR + 20, mouthR, mp::mouth.getRGB8());
        } else {
            int tp[3][2] = {{0, -10}, {-20, 10}, {20, 10}};
            for (int i = 0; i < 3; i++) {
                tp[i][0] = cx + tp[i][0];
                tp[i][1] = cy + tp[i][1];
            }
            spi->fillTriangle(tp[0][0], tp[0][1], tp[1][0], tp[1][1], tp[2][0], tp[2][1], mp::mouth.getRGB8());
            int d = 2;
            spi->drawWideLine(tp[0][0], tp[0][1], tp[1][0], tp[1][1], d, mp::edge.getRGB8());
            spi->drawWideLine(tp[0][0], tp[0][1], tp[2][0], tp[2][1], d, mp::edge.getRGB8());
            spi->drawWideLine(tp[2][0], tp[2][1], tp[1][0], tp[1][1], d, mp::edge.getRGB8());
        }
    }
};

class MikoFace : public Face {
  public:
    MikoFace()
    : Face(
          new MikoMouth(),
          /* new BoundingRect(168, 163), */
          new BoundingRect(200, 160),
          new MikoEye(false),
          new BoundingRect(160, 80),
          new MikoEye(true),
          new BoundingRect(160, 240),
          new Eyeblow(15, 0, false),
          new BoundingRect(67, 96),
          new Eyeblow(15, 0, true),
          new BoundingRect(72, 230)) {}
};

}  // namespace m5avatar

#endif  // FACES_MIKOFACE_H_
