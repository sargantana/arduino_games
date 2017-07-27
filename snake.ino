#include <gfxfont.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
 
// Pin definitions
#define PIN_BUTTON_RIGHT 2
#define PIN_BUTTON_LEFT 3
#define PIN_BUTTON_TOP 4
#define PIN_BUTTON_BOTTOM 5
 
// height and width of playing area
#define MATRIX_HORIZONTAL_LENGTH 8
#define MATRIX_VERTICAL_LENGTH 16
 
#define DIRECTION_TOP 0
#define DIRECTION_RIGHT 1
#define DIRECTION_BOTTOM 2
#define DIRECTION_LEFT 3
 
// checks if value is representing a snake in play area
#define SNAKE(a) (a > 0)
 
int field[MATRIX_HORIZONTAL_LENGTH][MATRIX_VERTICAL_LENGTH] = { 0 };
int snakeHeadX;
int snakeHeadY;
 
Adafruit_8x8matrix matrix = Adafruit_8x8matrix(); 
int direction = DIRECTION_TOP;
int snakeLength = 3;                            
unsigned long prevTime = 0;  
unsigned long delayTime = 400;    
 
int fruitX, fruitY;
unsigned long fruitPrevTime = 0;
unsigned long fruitBlinkTime = 200;
int fruitLed = LED_ON;
 
boolean gameOverShown = false;
boolean gameOver = false;
 
// Setup
void setup()
{
    // LED Matrix initialisieren
    randomSeed(analogRead(0));
    matrix.begin(0x70);
    matrix.setRotation(3);
 
    // initialise game, set random starting point for snake
    snakeHeadX = random(0, MATRIX_HORIZONTAL_LENGTH);
    snakeHeadY = random(0, MATRIX_VERTICAL_LENGTH);
    field[snakeHeadX][snakeHeadY] = snakeLength;
 
    // place first fruit
    makeFruit();
}
 
// check for button press
boolean buttonClicked(int buttonPin)
{
    return digitalRead(buttonPin) == HIGH;
}
 
// loop
void loop()
{
    // ggf. Richtung der Schlange ändern
    checkButtons();
 
    // Game Loop verzögern, damit das Game nicht sofort vorrüber ist...
    unsigned long currentTime = millis();
    if (currentTime - prevTime >= delayTime) {
        nextstep();
        prevTime = currentTime;
    }
 
    // Spielfeld auf Matrix zeichnen
    draw();
}
 
// checks button presses for directions of snake
// + check that snake does not colide with itself from direction presses
void checkButtons()
{
    int currentDirection = direction;
    if (buttonClicked(PIN_BUTTON_LEFT) && currentDirection != DIRECTION_RIGHT) {
        direction = DIRECTION_LEFT;
    }
    else if (buttonClicked(PIN_BUTTON_RIGHT) && currentDirection != DIRECTION_LEFT) {
        direction = DIRECTION_RIGHT;
    }
    else if (buttonClicked(PIN_BUTTON_TOP) && currentDirection != DIRECTION_BOTTOM) {
        direction = DIRECTION_TOP;
    }
    else if (buttonClicked(PIN_BUTTON_BOTTOM) && currentDirection != DIRECTION_TOP) {
        direction = DIRECTION_BOTTOM;
    }
}
 
// give graphical output on led matrix
void draw()
{
    matrix.clear();
    // if game is still running
    if (!gameOver)
    {
        for (int x = 0; x < MATRIX_HORIZONTAL_LENGTH; x++)
        {
            for (int y = 0; y < MATRIX_VERTICAL_LENGTH; y++)
                matrix.drawPixel(x, y, SNAKE(field[x][y]));
        }
 
        drawFruit();
         
        // update led matrix
        matrix.writeDisplay();
    }
    // if game over
    else
    {
        // set matrix to give text output
        matrix.setTextSize(1);
        matrix.setTextWrap(false);
        matrix.setTextColor(LED_ON);
 
        // show game over once
        if (!gameOverShown)
        {
            for (int8_t x = 0; x >= -56; x--) {
                matrix.clear();
                matrix.setCursor(x, 0);
                matrix.print("GAME OVER");
                matrix.writeDisplay();
                delay(100);
            }
 
            gameOverShown = true;
        }
        // show reached score
        else {
            for (int8_t x = 0; x >= -56; x--) {
                matrix.clear();
                matrix.setCursor(x, 0);
                matrix.print("SCORE: ");
                matrix.print(snakeLength);
                matrix.writeDisplay();
                delay(100);
            }
        }
    }
}
 
// draws fruit on matrix and blinks it
void drawFruit()
{
    if (inPlayField(fruitX, fruitY)) {
        unsigned long currenttime = millis();
        if (currenttime - fruitPrevTime >= fruitBlinkTime) {
            fruitLed = (fruitLed == LED_ON) ? LED_OFF : LED_ON;
            fruitPrevTime = currenttime;
        }
        matrix.drawPixel(fruitX, fruitY, fruitLed);
    }
}
 
// check if coordinates are in playing area
boolean inPlayField(int x, int y)
{
    return (x >= 0) && (x<MATRIX_HORIZONTAL_LENGTH) && (y >= 0) && (y<MATRIX_VERTICAL_LENGTH);
}
 
// move snake, check if its still in playing area
void nextstep()
{
    int newX = snakeHeadX;
    int newY = snakeHeadY;
 
    // move snake in direction indicated by button press
    switch (direction) {
        case DIRECTION_TOP:
            newY--;
            break;
        case DIRECTION_RIGHT:
            newX++;
            break;
        case DIRECTION_BOTTOM:
            newY++;
            break;
        case DIRECTION_LEFT:
            newX--;
            break;
    }
 
    // if snake is outside of playing area move to other side of matrix
    if (newY >= MATRIX_VERTICAL_LENGTH)
        newY = 0;
    else if (newY < 0)
        newY = MATRIX_VERTICAL_LENGTH - 1;
 
    if (newX >= MATRIX_HORIZONTAL_LENGTH)
        newX = 0;
    else if (newX < 0)
        newX = MATRIX_HORIZONTAL_LENGTH - 1;
 
    // eating yourself will loose the game
    if (isPartOfSnake(newX, newY))
    {
        gameOver = true;
        delay(3000);
        return;
    }
 
    // if fruit is collected, add length to snake and place new fruit
    if ((newX == fruitX) && (newY == fruitY)) {
        snakeLength++;
        makeFruit();
    }
 
    // configure playing area so snake can be moved
    for (int x = 0; x < MATRIX_HORIZONTAL_LENGTH; x++)
    {
        for (int y = 0; y < MATRIX_VERTICAL_LENGTH; y++)
        {
            int value = field[x][y];
            if (SNAKE(value))
                field[x][y] = value - 1;
        }
    }
 
    snakeHeadX = newX;
    snakeHeadY = newY;
    field[newX][newY] = snakeLength;
}
 
// places fruit on random pixel
void makeFruit()
{
    int x, y = 0;
 
    // exclude snake from random position for new fruit
    do {
        x = random(0, MATRIX_HORIZONTAL_LENGTH);
        y = random(0, MATRIX_VERTICAL_LENGTH);
    } while (isPartOfSnake(x, y));
 
    fruitX = x;
    fruitY = y;
}
 
// check if pixel to place food is part of snake
boolean isPartOfSnake(int x, int y)
{
    return SNAKE(field[x][y]);
}
