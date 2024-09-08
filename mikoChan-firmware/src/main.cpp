#include <Arduino.h>
#include <M5Unified.h>
#include "AudioOutputM5Speaker.h"
#include <AudioFileSourceSD.h>
#include <AudioGeneratorMP3.h>
#include <AudioFileSourceBuffer.h>
#include <Avatar.h>
#include "mikoFace.h"

#include <ServoEasing.hpp>
#include <I2C_Sensor.hpp>
/* #include <gob_unifiedButton.hpp> */
/* goblib::UnifiedButton unifiedButton; */
#define SERVO_PIN_X 2
#define SERVO_PIN_Y 1
MCP3425_Class mcp3425 = MCP3425_Class();
SHT31_Class sht31 = SHT31_Class();

const int maxFile = 20;
String fileList[maxFile];
int fileCount = 0;

static constexpr size_t WAVE_SIZE = 320;
static AudioOutputM5Speaker out(&M5.Speaker, m5spk_virtual_channel);
static AudioGeneratorMP3 mp3;
static AudioFileSourceSD* file = nullptr;
static AudioFileSourceBuffer* buff = nullptr;
const int preallocateBufferSize = 20 * 512;
uint8_t* preallocateBuffer;

using namespace m5avatar;
namespace mp = mikoPalette;

Avatar avatar;
MikoFace mikoFace;
ColorPalette cps;

const Expression expressions[] = {
    Expression::Neutral,  // neutral
    Expression::Happy,    // happy
    Expression::Sleepy,   // sleepy
    Expression::Angry,    // hot
    Expression::Doubt,    // cold
};
const int expressionsSize = sizeof(expressions) / sizeof(Expression);
int idx = 0;

bool isShowingQR = false;

void stop(void);
void play(const char* fname);
void play_num(int num);
void lipSync(void* args);
void servo(void* args);
void Servo_setup();
void file_read();

/* #ifdef USE_SERVO */
/* #define START_DEGREE_VALUE_X 90 */
#define START_DEGREE_VALUE_X 60
#define START_DEGREE_VALUE_Y 80  //
ServoEasing servo_x;
ServoEasing servo_y;
/* #endif */
static fft_t fft;
static int16_t raw_data[WAVE_SIZE * 2];
static float lipsync_level_max = 10.0f;  // リップシンクの上限初期値
float mouth_ratio = 0.0f;

int df;

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Ex_I2C.release();
    M5.Power.begin();

    preallocateBuffer = (uint8_t*)malloc(preallocateBufferSize);
    if (!preallocateBuffer) {
        M5.Display.printf("FATAL ERROR:  Unable to preallocate %d bytes for app\n", preallocateBufferSize);
        for (;;) {
            delay(1000);
        }
    }

    {  /// custom setting
        auto spk_cfg = M5.Speaker.config();
        /// Increasing the sample_rate will improve the sound quality instead of increasing the CPU load.
        spk_cfg.sample_rate = 48000;  // default:64000 (64kHz)  e.g. 48000 , 50000 , 80000 , 96000 , 100000 , 128000 , 144000 , 192000 , 200000
                                      //    spk_cfg.sample_rate = 48000; // default:64000 (64kHz)  e.g. 48000 , 50000 , 80000 , 96000 , 100000 , 128000 , 144000 , 192000 , 200000
        // spk_cfg.task_priority = configMAX_PRIORITIES - 2;
        // 音声が途切れる場合は下記3つのパラメータを調整してみてください。（あまり増やすと動かなくなる場合あり）
        spk_cfg.task_priority = 1;
        spk_cfg.dma_buf_count = 20;
        spk_cfg.dma_buf_len = 512;
        spk_cfg.task_pinned_core = PRO_CPU_NUM;
        M5.Speaker.config(spk_cfg);
    }
    M5.begin(cfg);

    M5.Lcd.clear();
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextSize(2);
    M5.Speaker.begin();
    M5.Speaker.setChannelVolume(m5spk_virtual_channel, 180);
    // M5.Speaker.setVolume(25);
    M5.Speaker.setVolume(10);

    M5.Speaker.tone(2000, 100);
    delay(200);
    M5.Speaker.setVolume(80);
    Servo_setup();
    delay(500);
    file_read();
    delay(100);

    M5.Lcd.setBrightness(100);
    M5.Lcd.clear();

    cps.set(COLOR_BACKGROUND, mp::skin.getRGB16());

    avatar.init(8);
    avatar.setColorPalette(cps);
    avatar.setFace(&mikoFace);
    avatar.setExpression(Expression::Sleepy);

    avatar.addTask(lipSync, "lipSync");
    avatar.addTask(servo, "servo");

    mcp3425.begin(true, 2, 0);
    /* mcp3425.begin(); */
    sht31.SoftReset();
    sht31.Heater(0);

    delay(500);
    /* df = mcp3425.analogRead(); */

    avatar.setExpression(Expression::Happy);
    printf("Finish setup\n");
    play_num(000);
    avatar.setExpression(Expression::Neutral);
}

float add = 0.05;
float ratio = 0;
int count = 0;
unsigned int neutralCount = 0;
unsigned int happyCount = 0;
unsigned int hotCount = 0;
unsigned int coldCount = 0;

bool isSleep = false;

void loop() {
    M5.update();

    if (isSleep && !M5.Power.isCharging()) {

    } else if (M5.Power.getBatteryLevel() <= 25 && !M5.Power.isCharging()) {
        isSleep = true;
        avatar.setExpression(Expression::Sleepy);
        play_num(300);
        servo_x.setEaseTo(START_DEGREE_VALUE_X);
        servo_y.setEaseTo(START_DEGREE_VALUE_Y);
        synchronizeAllServosStartAndWaitForAllServosToStop();
    } else {
        if (isSleep) {
            avatar.setExpression(Expression::Happy);
            play_num(000);
            avatar.setExpression(Expression::Neutral);
        }
        isSleep = false;

        sht31.GetTempHum();
        float di = sht31.DI();
        auto ahoge = mcp3425.analogRead();

        if (di >= 85) {
            /* avatar.setExpression(Expression::Angry); */
            hotCount++;
            coldCount = 0;
            neutralCount = 0;
        }
        else if (di <= 55) {
            /* avatar.setExpression(Expression::Sad); */
            coldCount++;
            hotCount = 0;
            neutralCount = 0;
        }
        else if (ahoge <= 50500) {
            /* avatar.setExpression(Expression::Happy); */
            /* hotCount = 0; */
            /* coldCount = 0; */
            happyCount++;
            neutralCount = 0;
        }
        else {
            avatar.setExpression(Expression::Neutral);
            hotCount = 0;
            coldCount = 0;
            neutralCount++;
        }

        // if neutral
        if (neutralCount == 30) {
            /* String voice = "/mp3/00" + String(random(2)) + ".mp3"; */
            /* play(voice.c_str()); */
            play_num(random(2));
            neutralCount = 0;
        }

        // if happy
        if (happyCount == 2) {
            avatar.setExpression(Expression::Happy);
            play_num(random(2, 7+1));
            happyCount = 0;
        }

        // if hot
        if (hotCount >= 20) avatar.setExpression(Expression::Angry);
        else avatar.setExpression(Expression::Neutral);

        if (hotCount == 100) {
            /* play("/mp3/200.mp3"); */
            play_num(200);
        } else if (hotCount >= 200 || M5.BtnA.wasPressed()) {
            hotCount = 0;
            /* play("/mp3/201.mp3"); */
            play_num(201);
        }

        // if cold
        if (coldCount >= 20) avatar.setExpression(Expression::Doubt);
        else avatar.setExpression(Expression::Neutral);

        if (coldCount == 100) {
            /* play("/mp3/210.mp3"); */
            play_num(210);
        } else if (coldCount >= 200 || M5.BtnC.wasPressed()) {
            coldCount = 0;
            /* play("/mp3/211.mp3"); */
            play_num(211);
        }
    }

    /* printf("Happy: %d, Hot: %d, Cold: %d, Touch: %d\n", happyCount, hotCount, coldCount, (int)M5.Touch.isEnabled()); */
    delay(1);
}

void stop(void) {
    if (file == nullptr) return;
    out.stop();
    mp3.stop();
    //  id3->RegisterMetadataCB(nullptr, nullptr);
    //  id3->close();
    file->close();
    delete file;
    file = nullptr;
}

void play(const char* fname) {
    Serial.printf("play file fname = %s\r\n", fname);
    if (file != nullptr) {
        stop();
    }
    file = new AudioFileSourceSD(fname);
    buff = new AudioFileSourceBuffer(file, preallocateBuffer, preallocateBufferSize);
    //  wav.begin(file, &out);
    mp3.begin(buff, &out);
    delay(10);
    while (mp3.isRunning()) {
        //    while(wav.loop()) {delay(1);}
        while (mp3.loop()) {
        }
        mp3.stop();
        file->close();
        delete file;
        delete buff;
        file = nullptr;
        buff = nullptr;
        /* avatar.setExpression(Expression::Neutral); */
        //    }
    }
}

void play_num(int num) {
    String file = "/mp3/";
    if (num < 10) {
        file += "00" + String(num);
    } else if (num < 100) {
        file += "0" + String(num);
    } else if (num < 1000) {
        file += String(num);
    } else {
        printf("file name error!!\n");
        return;
    }
    file += ".mp3";
    play(file.c_str());
}

void lipSync(void* args) {
    float gazeX, gazeY;
    int level = 0;
    /* DriveContext* ctx = (DriveContext*)args; */
    /* Avatar* avatar = ctx->getAvatar(); */
    for (;;) {
        uint64_t level = 0;
        auto buf = out.getBuffer();
        if (buf) {
            memcpy(raw_data, buf, WAVE_SIZE * 2 * sizeof(int16_t));
            fft.exec(raw_data);
            for (size_t bx = 5; bx <= 60; ++bx) {  // リップシンクで抽出する範囲はここで指定(低音)0〜64（高音）
                int32_t f = fft.get(bx);
                level += abs(f);
                // Serial.printf("bx:%d, f:%d\n", bx, f) ;
            }
            // Serial.printf("level:%d\n", level >> 16);
        }

        // スレッド内でログを出そうとすると不具合が起きる場合があります。
        // Serial.printf("data=%d\n\r", level >> 16);
        mouth_ratio = (float)(level >> 16) / lipsync_level_max;
        if (mouth_ratio > 1.2f) {
            if (mouth_ratio > 1.5f) {
                lipsync_level_max += 10.0f;  // リップシンク上限を大幅に超えるごとに上限を上げていく。
            }
            mouth_ratio = 1.2f;
        }
        /* avatar->setMouthOpenRatio(mouth_ratio); */
        avatar.setMouthOpenRatio(mouth_ratio);

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void servo(void* args) {
    float gazeX, gazeY;
    /* DriveContext* ctx = (DriveContext*)args; */
    /* Avatar* avatar = ctx->getAvatar(); */
    for (;;) {
        /* #ifdef USE_SERVO */
        /* avatar->getGaze(&gazeY, &gazeX); */
        avatar.getGaze(&gazeY, &gazeX);
        servo_x.setEaseTo(START_DEGREE_VALUE_X + (int)(20.0 * gazeX));
        if (gazeY < 0) {
            int tmp = (int)(15.0 * gazeY);
            if (tmp > 15) tmp = 15;
            servo_y.setEaseTo(START_DEGREE_VALUE_Y + tmp);
        } else {
            servo_y.setEaseTo(START_DEGREE_VALUE_Y + (int)(10.0 * gazeY));
        }
        if (!isSleep) {
            synchronizeAllServosStartAndWaitForAllServosToStop();
        }
        /* #endif */
        delay(5000);
    }
}

void Servo_setup() {
    /* #ifdef USE_SERVO */
    if (servo_x.attach(SERVO_PIN_X, START_DEGREE_VALUE_X, DEFAULT_MICROSECONDS_FOR_0_DEGREE, DEFAULT_MICROSECONDS_FOR_180_DEGREE)) {
        Serial.print("Error attaching servo x");
    }
    if (servo_y.attach(SERVO_PIN_Y, START_DEGREE_VALUE_Y, DEFAULT_MICROSECONDS_FOR_0_DEGREE, DEFAULT_MICROSECONDS_FOR_180_DEGREE)) {
        Serial.print("Error attaching servo y");
    }
    servo_x.setEasingType(EASE_QUADRATIC_IN_OUT);
    servo_y.setEasingType(EASE_QUADRATIC_IN_OUT);
    setSpeedForAllServos(30);
    /* #endif */
}

void file_read() {
    // SDカードマウント待ち
    int time_out = 0;
    while (false == SD.begin(GPIO_NUM_4, SPI, 15000000)) {
        if (time_out++ > 10) return;
        Serial.println("SD Wait...");
        M5.Lcd.println("SD Wait...");
        delay(500);
    }
    File root = SD.open("/mp3");
    if (root) {
        File file = root.openNextFile();
        while (file) {
            if (file.isDirectory()) {
                // Dir skip
            } else {
                // File
                String filename = file.name();
                String dirname = "/mp3/";
                Serial.println(filename);
                //        M5.Lcd.println(filename.indexOf(".wav"));
                if (filename.indexOf("mp3") != -1) {
                    // Find
                    fileList[fileCount] = dirname + filename;
                    fileCount++;
                    if (maxFile <= fileCount) {
                        break;
                    }
                }
            }
            file = root.openNextFile();
        }
        root.close();
    }

    Serial.println("File List");
    M5.Lcd.println("File List");
    for (int i = 0; i < fileCount; i++) {
        Serial.println(fileList[i]);
        M5.Lcd.println(fileList[i]);
    }
    delay(2000);
}
