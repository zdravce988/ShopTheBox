#ifndef GAME_H
#define GAME_H
typedef enum
{
    RENDER_CLEAR,
    RENDER_RECT,
    RENDER_LINE,
    RENDER_CIRCLE,
    RENDER_TRIANGLE,
    RENDER_TEXTURE,
    RENDER_TEXT,
}RENDER_CMD;

typedef enum
{
    CARD_1,
    CARD_2,
    CARD_3,
    CARD_4,
    CARD_5,
    CARD_6,
    CARD_7,
    CARD_8,
    CARD_9,
    CARD_10,
    CARD_BACK,
    DICE_1,
    DICE_2,
    DICE_3,
    DICE_4,
    DICE_5,
    DICE_6,
    RED_BUTTON,
    RED_BUTTON_PRESSED,
    GREEN_BUTTON,
    GREEN_BUTTON_PRESSED,
    GREEN_BUTTON_DISABLED,
}TEXTURE_ID;

typedef struct
{
    RENDER_CMD type;
    uint8_t r, g, b, a;
}render_cmd_clear;


typedef struct
{
    RENDER_CMD type;
    float x, y, w, h;
    uint8_t r, g, b, a;
}render_cmd_rect;

typedef struct
{
    RENDER_CMD type;
    float x1, y1, x2, y2;
    uint8_t r, g, b, a;
}render_cmd_line;

typedef struct
{
    RENDER_CMD type;
    TEXTURE_ID id;
    float x, y, w, h;
}render_cmd_texture;

typedef struct
{
    RENDER_CMD type;
    float x, y;
    uint8_t r, g, b;
    int fontScale;
    char *message;
}render_cmd_text;

typedef struct
{
    int length;
    void *data;
    void *base;
    void *end;
}cmd_buf;

typedef struct
{
    uint8_t up;
    uint8_t down;
    uint8_t right;
    uint8_t left;
    float mouseX;
    float mouseY;
    uint32_t mouseUp;
    uint32_t mouseDown;
}game_input;

typedef struct
{
    TEXTURE_ID textureId;
    float x;
    float y;
    float w;
    float h;
    float animationOffset;
    float startX;
    float startY;
    int used;
    int finished;
    int allowed;
    int value;
}card;

typedef struct
{
    game_input gameInput;
    cmd_buf commandBuffer;
    float camX;
    float camY;
    int diceRolled;
    int dice1;
    int dice2;
    int roundSum;
    int gameFinished;
    int gameInitialized;
    int bestScore;
    char *scoreText;
    char *bestScoreText;
    float delta;
    int64_t ticks;
    int32_t (*GameRand)(int32_t);
    card cards[10];
}game_state;

game_state gameState;
#endif
