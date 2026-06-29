#include <Adafruit_SSD1306.h>

#define BUTTON_PIN 3
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Constantes do jogo
const int GROUND_Y = 45;
const int GROUND_CAC = 43;
const int GRAVITY = 2;
const int JUMP_FORCE = -13;
const int MAX_SPEED = 10;
const int DINO_WIDTH = 10;
const int DINO_HEIGHT = 13;
const int CACTUS_WIDTH = 7;
const int CACTUS_HEIGHT = 15;
 int MIN_CACTUS_DISTANCE = 70;  // Distância mínima entre cactos

// Estado do jogo
bool isJumping = false;
bool gameOver = false;
int dinoY = GROUND_Y;
int velocity = 0;
int cactusX1 = 128;
int cactusX2 = 200;
int gameSpeed = 3;
unsigned long lastSpeedIncrease = 0;
unsigned long lastFrame = 0;
unsigned long score = 0;
int highscore =0;
int speedCounter=0;

// Debounce do botão
bool buttonPressed = false;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- FUNÇÃO DE COLISÃO ---
bool checkCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 &&
            y1 < y2 + h2 && y1 + h1 > y2);
}

void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        while (1);
    }

    display.clearDisplay();
    display.display();
    randomSeed(analogRead(0));
    
    // Inicializa os cactos com distância adequada
    cactusX1 = SCREEN_WIDTH + random(20, 60);
    cactusX2 = cactusX1 + MIN_CACTUS_DISTANCE + random(10, 40);
}

void loop() {
    unsigned long currentTime = millis();
    if (currentTime - lastFrame < 50) return;
    lastFrame = currentTime;

    int buttonState = digitalRead(BUTTON_PIN);

    // --- GAME OVER ---
    if (gameOver) {
        if (buttonState == HIGH && !buttonPressed) {
            buttonPressed = true;
        }
        if (buttonState == LOW && buttonPressed) {
            buttonPressed = false;
            resetGame();
        }
        return;
    }

    // --- PULO ---
    if (buttonState == HIGH && !buttonPressed && !isJumping && dinoY == GROUND_Y) {
        buttonPressed = true;
        isJumping = true;
        velocity = JUMP_FORCE;
    }
    if (buttonState == LOW) {
        buttonPressed = false;
    }

    // --- FÍSICA ---
    if (isJumping) {
        dinoY += velocity;
        velocity += GRAVITY;
        if (dinoY >= GROUND_Y) {
            dinoY = GROUND_Y;
            isJumping = false;
            velocity = 0;
        }
    }

    // --- MOVIMENTO DOS CACTOS ---
    cactusX1 -= gameSpeed;
    cactusX2 -= gameSpeed;

    // --- SPAWN INDEPENDENTE PARA CADA CACTO ---
    // Cacto 1
    if (cactusX1 < -CACTUS_WIDTH) {
        // O novo cacto deve aparecer depois do cacto 2
        int newX = cactusX2 + MIN_CACTUS_DISTANCE + random(10, 50);
        // Mas não pode aparecer muito longe
        if (newX > SCREEN_WIDTH + 100) newX = SCREEN_WIDTH + random(20, 60);
        cactusX1 = newX;
    }

    // Cacto 2
    if (cactusX2 < -CACTUS_WIDTH) {
        // O novo cacto deve aparecer depois do cacto 1
        int newX = cactusX1 + MIN_CACTUS_DISTANCE + random(10, 50);
        // Mas não pode aparecer muito longe
        if (newX > SCREEN_WIDTH + 100) newX = SCREEN_WIDTH + random(20, 60);
        cactusX2 = newX;
    }

    // --- AUMENTA VELOCIDADE (com limite) ---
    if (currentTime - lastSpeedIncrease > 3000 && gameSpeed < MAX_SPEED) {
      speedCounter++;
      if (speedCounter >= 2) {  // A cada 2 frames
          speedCounter = 0;
          gameSpeed++;
        MIN_CACTUS_DISTANCE+=3;
        lastSpeedIncrease = currentTime;
      }
        
    }
    if(gameSpeed >= MAX_SPEED && MIN_CACTUS_DISTANCE> 70)
    {
      speedCounter ++;
      if(speedCounter >5)
      {
        speedCounter =0;
        MIN_CACTUS_DISTANCE --;

      }
    }

    // --- SCORE ---
    score++;

    // --- DESENHO ---
    display.clearDisplay();
    
    // Dino
    display.fillRect(10, dinoY, DINO_WIDTH, DINO_HEIGHT, SSD1306_WHITE);
    
    // Cactos
    display.fillRect(cactusX1, GROUND_CAC, CACTUS_WIDTH, CACTUS_HEIGHT, SSD1306_WHITE);
    display.fillRect(cactusX2, GROUND_CAC, CACTUS_WIDTH, CACTUS_HEIGHT, SSD1306_WHITE);
    
    // Chão
    display.drawLine(0, 58, SCREEN_WIDTH, 58, SSD1306_WHITE);
    
    // Score
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Pontos: ");
    display.print(score / 10);
    display.display();

    // --- COLISÃO ---
    bool colidiuCacto1 = checkCollision(10, dinoY, DINO_WIDTH, DINO_HEIGHT,
                                        cactusX1, GROUND_Y, CACTUS_WIDTH, CACTUS_HEIGHT);
    bool colidiuCacto2 = checkCollision(10, dinoY, DINO_WIDTH, DINO_HEIGHT,
                                        cactusX2, GROUND_Y, CACTUS_WIDTH, CACTUS_HEIGHT);

    if (colidiuCacto1 || colidiuCacto2) {
      if(score/10 > highscore) highscore = score/10;
        gameOver = true;
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0,0);
        display.print("Highscore: ");
        display.print(highscore);
        display.setTextSize(2);
        display.setCursor(10, 19);
        display.print("Game Over");
        display.setTextSize(1);
        display.setCursor(0, 44);
        display.print("Aperte para recomecar");
        display.display();
    }
}

void resetGame() {
    isJumping = false;
    dinoY = GROUND_Y;
    velocity = 0;
    cactusX1 = SCREEN_WIDTH + random(20, 60);
    cactusX2 = cactusX1 + MIN_CACTUS_DISTANCE + random(10, 40);
    gameSpeed = 3;
    score = 0;
    gameOver = false;
    lastSpeedIncrease = millis();
    lastFrame = millis();
    display.clearDisplay();
    display.display();
}