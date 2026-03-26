#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#define LANES 5
#define HEIGHT 18

int asteroidHeight(int type)
{
    if(type == 1) return 1;
    if(type == 2) return 2;
    return 3;
}

int asteroidPoints(int type)
{
    if(type == 1) return 1;
    if(type == 2) return 3;
    return 5;
}

int asteroidSpeed(int type)
{
    if(type == 1) return 2;
    if(type == 2) return 3;
    return 4;
}

int asteroidHealthValue(int type)
{
    if(type == 1) return 1;
    if(type == 2) return 2;
    return 3;
}

void clearScreen()
{
    printf("\033[H");
}

int kbhit()
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

char getch()
{
    struct termios oldt, newt;
    char ch;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

int main()
{
    int playerLane = 2;

    int asteroidRow[LANES];
    int asteroidType[LANES];
    int asteroidHealth[LANES];

    int bulletRow[LANES];

    int explosionRow[LANES];
    int explosionTimer[LANES];

    int score = 0;
    int gameOver = 0;
    int frame = 0;

    int i, j;

    srand(time(0));

    for(i = 0; i < LANES; i++)
    {
        asteroidRow[i] = -1;
        asteroidType[i] = 0;
        asteroidHealth[i] = 0;
        bulletRow[i] = -1;
        explosionRow[i] = -1;
        explosionTimer[i] = 0;
    }

    printf("\033[2J");
    printf("\033[?25l");

    while(!gameOver)
    {
        frame++;

        if(kbhit())
        {
            char ch = getch();

            if(ch == 'a' || ch == 'A')
                if(playerLane > 0) playerLane--;

            if(ch == 'd' || ch == 'D')
                if(playerLane < LANES - 1) playerLane++;

            if(ch == ' ')
            {
                if(bulletRow[playerLane] == -1)
                    bulletRow[playerLane] = HEIGHT - 2;
            }

            if(ch == 'q' || ch == 'Q')
                break;
        }

        for(i = 0; i < LANES; i++)
        {
            if(asteroidRow[i] != -1)
            {
                int moveRate = asteroidSpeed(asteroidType[i]);
                if(frame % moveRate == 0)
                    asteroidRow[i]++;
            }
        }

        for(i = 0; i < LANES; i++)
        {
            if(bulletRow[i] != -1)
            {
                bulletRow[i]--;

                if(bulletRow[i] < 0)
                    bulletRow[i] = -1;
            }
        }

        if(rand() % 4 == 0)
        {
            int lane = rand() % LANES;

            if(asteroidRow[lane] == -1)
            {
                asteroidRow[lane] = 0;

                int chance = rand() % 100;

                if(chance < 50)
                    asteroidType[lane] = 1;
                else if(chance < 80)
                    asteroidType[lane] = 2;
                else
                    asteroidType[lane] = 3;

                asteroidHealth[lane] = asteroidHealthValue(asteroidType[lane]);
            }
        }

        for(i = 0; i < LANES; i++)
        {
            if(bulletRow[i] != -1 && asteroidRow[i] != -1)
            {
                int top = asteroidRow[i] - (asteroidHeight(asteroidType[i]) - 1);
                int bottom = asteroidRow[i];

                if(bulletRow[i] >= top && bulletRow[i] <= bottom)
                {
                    asteroidHealth[i]--;

                    if(asteroidHealth[i] <= 0)
                    {
                        score += asteroidPoints(asteroidType[i]);
                        explosionRow[i] = asteroidRow[i];
                        explosionTimer[i] = 2;

                        asteroidRow[i] = -1;
                        asteroidType[i] = 0;
                    }

                    bulletRow[i] = -1;
                }
            }
        }

        for(i = 0; i < LANES; i++)
        {
            if(asteroidRow[i] != -1)
            {
                int top = asteroidRow[i] - (asteroidHeight(asteroidType[i]) - 1);
                int bottom = asteroidRow[i];

                if(i == playerLane &&
                   (HEIGHT - 1) >= top &&
                   (HEIGHT - 1) <= bottom)
                {
                    gameOver = 1;
                }
            }
        }

        for(i = 0; i < LANES; i++)
            if(explosionTimer[i] > 0)
                explosionTimer[i]--;

        clearScreen();

        printf("ASTEROID SHOOTER\n");
        printf("Score: %d\n\n", score);

        for(j = 0; j < HEIGHT; j++)
        {
            for(i = 0; i < LANES; i++)
            {
                int printed = 0;

                if(j == HEIGHT - 1 && i == playerLane)
                {
                    printf("<^>");
                    printed = 1;
                }
                else if(explosionTimer[i] > 0 && explosionRow[i] == j)
                {
                    printf("***");
                    printed = 1;
                }
                else if(bulletRow[i] == j)
                {
                    printf(" | ");
                    printed = 1;
                }
                else if(asteroidRow[i] != -1)
                {
                    int type = asteroidType[i];
                    int bottom = asteroidRow[i];

                    if(type == 1 && j == bottom)
                        printf(" o "), printed = 1;

                    else if(type == 2 && (j == bottom || j == bottom - 1))
                        printf(" O "), printed = 1;

                    else if(type == 3)
                    {
                        if(j == bottom)
                            printf("@@@"), printed = 1;
                        else if(j == bottom - 1)
                            printf(" @ "), printed = 1;
                        else if(j == bottom - 2)
                            printf("@@@"), printed = 1;
                    }
                }

                if(!printed)
                    printf("   ");

                if(i != LANES - 1)
                    printf("|");
            }
            printf("\n");
        }

        printf("------------------------\n");

        fflush(stdout);
        usleep(100000);
    }

    clearScreen();
    printf("\033[?25h");

    printf("GAME OVER!\n");
    printf("Final Score: %d\n", score);

    return 0;
}
