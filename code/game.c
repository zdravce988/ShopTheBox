#include "game.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define M_PI 3.1415

static void DrawRect(float x, float y, float w, float h, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    render_cmd_rect *rect = (render_cmd_rect *)(gameState.commandBuffer.end);
    rect->type = RENDER_RECT;
    rect->x = x - gameState.camX;
    rect->y = y - gameState.camY;
    rect->w = w;
    rect->h = h;
    rect->r = r;
    rect->g = g;
    rect->b = b;
    rect->a = a;
    gameState.commandBuffer.end = rect + 1;
}

static void DrawLine(float x1, float y1, float x2, float y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    render_cmd_line *line = (render_cmd_line *)(gameState.commandBuffer.end);
    line->type = RENDER_LINE;
    line->x1 = x1 - gameState.camX;
    line->x2 = x2 - gameState.camX;
    line->y1 = y1 - gameState.camY;
    line->y2 = y2 - gameState.camY;
    line->r = r;
    line->g = g;
    line->b = b;
    line->a = a;
    gameState.commandBuffer.end = line + 1;
}

void ClearScreen(float r, float g, float b, float a)
{
    render_cmd_clear *clear = (render_cmd_clear *)(gameState.commandBuffer.end);
    clear->type = RENDER_CLEAR;
    clear->r = r;
    clear->g = g;
    clear->b = b;
    clear->a = a;
    gameState.commandBuffer.end = clear + 1;
}

void DrawTexture(TEXTURE_ID textureId, float x, float y, float w, float h)
{
    render_cmd_texture *texture = (render_cmd_texture *)(gameState.commandBuffer.end); 
    texture->type = RENDER_TEXTURE;
    texture->x = x;
    texture->y = y;
    texture->w = w;
    texture->h = h;
    texture->id = textureId;
    gameState.commandBuffer.end = texture + 1;
}

void DrawText(char *message, int fontScale, float x, float y, uint8_t r, uint8_t g, uint8_t b)
{
    render_cmd_text *text = (render_cmd_text *)(gameState.commandBuffer.end);
    text->type = RENDER_TEXT;
    text->x = x;
    text->y = y;
    text->r = r;
    text->g = g;
    text->b = b;
    text->fontScale = fontScale;
    text->message = message;
    gameState.commandBuffer.end = text + 1;
}

#define CARD_LINE 

static int CheckMouseCollisionWithCards(card *cards, float x, float y)
{
    for(int i = 0; i < 10; ++i)
    {
	if(x >= cards[i].x && x < cards[i].x + 140 && y >= cards[i].y && y < cards[i].y + 190)
	{
	    return i;
	}
    }
    return -1;
}

static TEXTURE_ID GetTextureForDice(int dice)
{
    switch(dice)
    {
	case 1: return DICE_1;
	case 2: return DICE_2;
	case 3: return DICE_3;
	case 4: return DICE_4;
	case 5: return DICE_5;
	case 6: return DICE_6;
    }
    return -1;
}

static void DrawDice()
{
    DrawTexture(GetTextureForDice(gameState.dice1), 1400, 600, 64, 64);
    DrawTexture(GetTextureForDice(gameState.dice2), 1500, 600, 64, 64);
}

static void InitCards()
{
	for(int i = 0; i < 10; ++i)
	{
	    gameState.cards[i].used = 0;
	    gameState.cards[i].value = i + 1;
	    gameState.cards[i].textureId = i;

	    if(i == 0)
	    {
		gameState.cards[i].x = 10 + i * 140;
	    }
	    else
	    {
		gameState.cards[i].x = i * 140 + i * 20;

	    }
	    gameState.cards[i].startX = gameState.cards[i].x;
	    gameState.cards[i].y = 250;
	    gameState.cards[i].startY = gameState.cards[i].y;
	    gameState.cards[i].finished = 0;
	    gameState.cards[i].allowed = 0;
	    gameState.cards[i].w = 140;
	    gameState.cards[i].h = 190;
	    gameState.cards[i].animationOffset = gameState.GameRand(6) + 2;
	}
}

static void DrawCards()
{
    card *cards = gameState.cards;
    for(int i = 0; i < 10; ++i)
    {
	DrawRect(cards[i].x + (800 - cards[i].x + cards[i].w / 2.0f) / 70.0f, cards[i].y + (720 - cards[i].y + cards[i].h / 2.0f) / 70.0f, gameState.cards[i].w, gameState.cards[i].h, 0, 0, 0, 80);
	DrawTexture(cards[i].textureId, gameState.cards[i].x, gameState.cards[i].y, gameState.cards[i].w, gameState.cards[i].h);
    }

}

static void ResetCardState()
{
    card *cards = gameState.cards;
    for(int i = 0; i < 10; ++i)
    {
	cards[i].allowed = 0;
    }
}

static void FindAllowedCards(card *cards, int roundSum, int dice1, int dice2)
{
    ResetCardState();
    for(int first = 0; first < 10; ++first)
    {
	if(gameState.cards[first].finished || gameState.cards[first].used)
	{
	    continue;
	}
	int sum = cards[first].value + roundSum;

	if(sum == dice1 + dice2)
	{
	    cards[first].allowed = 1;
	    break;
	}
	else if(sum < dice1 + dice2)
	{
	    for(int second = first + 1; second < 10; ++second)
	    {
		if(cards[second].finished || gameState.cards[second].used)
		{
		    continue;
		}
		int secondSum = sum + cards[second].value;
		if(secondSum == dice1 + dice2)
		{
		    cards[first].allowed = 1;
		    cards[second].allowed = 1;
		    break;
		}
		else if(secondSum < dice1 + dice2)
		{
		    for(int third = second + 1; third < 10; ++third)
		    {
			if(cards[third].finished || gameState.cards[third].used)
			{
			    continue;
			}
			int thirdSum = secondSum + cards[third].value;
			if(thirdSum == dice1 + dice2)
			{
			    cards[first].allowed = 1;
			    cards[second].allowed = 1;
			    cards[third].allowed = 1;
			    break;
			}
			else if(thirdSum < dice1 + dice2)
			{
			    for(int fourth = third + 1; fourth < 10; ++fourth)
			    {
				if(cards[fourth].finished || gameState.cards[fourth].used)
				{
				    continue;
				}
				int fourthSum = thirdSum + cards[fourth].value;
				if(fourthSum == dice1 + dice2)
				{
				    cards[first].allowed = 1;
				    cards[second].allowed = 1;
				    cards[third].allowed = 1;
				    cards[fourth].allowed = 1;
				    break;
				}
				else{
				    break;
				}

			    }
			}
			else
			{
			    break;
			}
		    }
		}
		else
		{
		    break;
		}
	    }
	}
	else
	{
	    break;
	}
    }

}
int cardToRotate = -1;
uint64_t startTime = 0;

static void HandleInput()
{
    game_input gameInput = gameState.gameInput;
    card *cards = gameState.cards;
    if(gameInput.mouseUp)
    {
	int index = CheckMouseCollisionWithCards(gameState.cards, gameInput.mouseX, gameInput.mouseY);
	if(index != -1)
	{
	    if(!cards[index].finished)
	    {
		if(cards[index].used)
		{
		    cardToRotate = index;
		    startTime = gameState.ticks;
		    gameState.roundSum -= cards[index].value;
		    gameState.cards[cardToRotate].used = 0;
		}
		else if(cards[index].allowed)
		{
		    cardToRotate = index;
		    startTime = gameState.ticks;
		    gameState.roundSum += cards[index].value;
		    gameState.cards[cardToRotate].used = 1;
		}
	    }
	}
    }
}


static void AnimateCards()
{
    for(int i = 0; i < 10; ++i)
    {
	gameState.cards[i].y = gameState.cards[i].startY + (gameState.cards[i].animationOffset) * (sin(gameState.ticks / 500.0f));
    }
}

static int GetScore()
{
    int score = 55;
    for(int i = 0; i < 10; ++i)
    {
	if(gameState.cards[i].finished)
	{
	    score -= gameState.cards[i].value;	
	    if(score < gameState.bestScore)
	    {
		gameState.bestScore = score;
	    }
	}
    }
    
    return score;
}

static char *GetBestScoreText()
{
    sprintf(gameState.bestScoreText, "Best Score: %2d", gameState.bestScore);
    return gameState.bestScoreText;
}

static char *GetScoreText()
{
    sprintf(gameState.scoreText, "Score: %2d", GetScore());
    return gameState.scoreText;
}

static int CheckBoxCollision(float px, float py, float boxX, float boxY, float boxW, float boxH)
{
    if(px >= boxX && px <= boxX + boxW && py >= boxY && py < boxY + boxH)
    {
	return 1;
    }
    return 0;
}


int halfway = 0;
uint64_t diceRollAnimationStart = 0;

static void CheckGameFinished()
{
    gameState.gameFinished = 1;
    for(int i = 0; i < 10; ++i)
    {
	if(gameState.cards[i].allowed || gameState.cards[i].used)
	{
	    gameState.gameFinished = 0;
	    return;
	}
    }
}

static void AnimateDiceRoll()
{
    if(gameState.ticks - diceRollAnimationStart > 500)
    {
	gameState.dice1 = gameState.GameRand(6) + 1;
	gameState.dice2 = gameState.GameRand(6) + 1;
	gameState.diceRolled = 1;
	gameState.roundSum = 0;
    }
    else
    {
	gameState.dice1 = gameState.GameRand(6) + 1;
	gameState.dice2 = gameState.GameRand(6) + 1;
    }
}

typedef enum
{
    BUTTON_DEFAULT,
    BUTTON_PRESSED,
    BUTTON_DISABLED,
}BUTTON_STATE;

typedef struct
{
    float x, y, w, h;
    TEXTURE_ID textures[3];
    int numOfTextures;
    BUTTON_STATE state;
    char *text;
    float textOffsetX;
    float textOffsetY;
}button;

static button CreateButton(char *text, float x, float y, float w, float h, float textOffsetX, float textOffsetY, TEXTURE_ID *textures, int numOfTextures, BUTTON_STATE defaultState)
{
    button b;
    b.x = x;
    b.y = y;
    b.w = w;
    b.h = h;
    b.textOffsetX = textOffsetX;
    b.textOffsetY = textOffsetY;
    for(int i = 0; i < numOfTextures; ++i)
    {
	b.textures[i] = textures[i];
    }
    b.state = defaultState;
    b.numOfTextures = numOfTextures;
    b.text = text;

    return b;
}

static void DrawButton(button b)
{
    DrawTexture(b.textures[b.state], b.x, b.y, b.w, b.h);
    DrawText(b.text, 1, b.x + b.textOffsetX, b.y + b.textOffsetY, 0, 0, 0);
}

static int ButtonPressed(button *b)
{
    if(gameState.gameInput.mouseDown && CheckBoxCollision(gameState.gameInput.mouseX, gameState.gameInput.mouseY, b->x, b->y, b->w, b->h))
    {
	return 1;
    }
    return 0;
}

static int ButtonReleased(button *b)
{
    if(gameState.gameInput.mouseUp && CheckBoxCollision(gameState.gameInput.mouseX, gameState.gameInput.mouseY, b->x, b->y, b->w, b->h))
    {
	return 1;
    }
    return 0;
}

static int CheckVictory()
{
    for(int i = 0; i < 10; ++i)
    {
	if(!gameState.cards[i].finished)
	{
	    return 0;
	}
    }
    gameState.gameFinished = 1;
    return 1;
}

static void TurnAllUsedCardsToFinished()
{
    for(int i = 0; i < 10; ++i)
    {
	if(gameState.cards[i].used)
	{
	    gameState.cards[i].finished = 1;
	    gameState.cards[i].used = 0;
	}
    }
}

static void GameUpdateAndRender()
{
    ClearScreen(207, 133, 90, 0);
    if(!gameState.gameInitialized)
    {
	InitCards();
	gameState.gameFinished = 0;
	gameState.diceRolled = 0;
	diceRollAnimationStart = gameState.ticks;
	gameState.gameInitialized = 1;
	gameState.roundSum = 0;
	cardToRotate = -1;
	return;
    }

    AnimateCards();
    DrawCards();
    DrawDice();


    if(!gameState.gameFinished)
    {
	if(cardToRotate != -1)
	{

#define ANIMATION_TICKS 100.0f
	    gameState.cards[cardToRotate].w = 70 * (cosf((gameState.ticks - startTime) / ANIMATION_TICKS * M_PI) + 1.0f);
	    gameState.cards[cardToRotate].x = gameState.cards[cardToRotate].startX + (140 - gameState.cards[cardToRotate].w) / 2.0f;

	    if((gameState.ticks - startTime) >= ANIMATION_TICKS && !halfway)
	    {
		if(gameState.cards[cardToRotate].textureId == CARD_BACK)
		{
		    gameState.cards[cardToRotate].textureId = gameState.cards[cardToRotate].value - 1;
		}
		else
		{
		    gameState.cards[cardToRotate].textureId = CARD_BACK;
		}
		halfway = 1;
	    }
	    if(halfway && (gameState.ticks - startTime) >= 2 * ANIMATION_TICKS)
	    {
		gameState.cards[cardToRotate].w = 140;
		gameState.cards[cardToRotate].x = gameState.cards[cardToRotate].startX;
		cardToRotate = -1;
		halfway = 0;
	    }

	}
	else if(!gameState.diceRolled)
	{
	    AnimateDiceRoll();
	}
	else 
	{
	    FindAllowedCards(gameState.cards, gameState.roundSum, gameState.dice1, gameState.dice2);
	    CheckGameFinished();
	    HandleInput();
	}

	TEXTURE_ID buttonTextures[3] = {GREEN_BUTTON, GREEN_BUTTON_PRESSED, GREEN_BUTTON_DISABLED};
	button greenButton = CreateButton("Confirm", 700, 600, 200, 100, 35, 30, buttonTextures, 3, BUTTON_DEFAULT);

	greenButton.state = BUTTON_DISABLED;
	if(gameState.roundSum == gameState.dice1 + gameState.dice2 && gameState.diceRolled)
	{
	    greenButton.state = BUTTON_DEFAULT;
	    if(ButtonPressed(&greenButton))
	    {
		greenButton.state = BUTTON_PRESSED;
	    }
	    if(ButtonReleased(&greenButton))
	    {
		TurnAllUsedCardsToFinished();
		CheckVictory();
		gameState.diceRolled = 0;
		gameState.roundSum = 0;
		diceRollAnimationStart = gameState.ticks;
	    }
	}

	DrawButton(greenButton);
    }
    else
    {
	DrawRect(0, 0, 1600, 720, 0, 0, 0, 200);
	TEXTURE_ID buttonTextures[2] = {RED_BUTTON, RED_BUTTON_PRESSED};
	button redButton = CreateButton("New Game", 700, 600, 200, 100, 30, 30, buttonTextures, 2, BUTTON_DEFAULT);
	if(ButtonPressed(&redButton))
	{
	    redButton.state = BUTTON_PRESSED;
	}

	if(ButtonReleased(&redButton))
	{
	    gameState.gameInitialized = 0;
	}

	if(!CheckVictory())
	{
	    DrawText("Game Over", 7, 250, 200, 255, 255, 255);
	}
	else
	{
	    DrawText("Congratulations", 5, 125, 250, 255, 255, 255);
	}
	DrawButton(redButton);
    }

    DrawText(GetBestScoreText(), 1, 1322, 20, 255, 255, 255);
    DrawText(GetScoreText(), 1, 1400, 50, 255, 255, 255);
}
