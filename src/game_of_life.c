#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>  // Для nanosleep()

// Размеры поля
#define ROWS 25
#define COLS 80

typedef struct {
    int row;
    int column;
} Cell;

// Getting all 8 of cell's neighbors
void findNeighbors(Cell cell, Cell neighborsArray[8]);

void countAliveNeighbors(const char prevFrame[ROWS][COLS], Cell neighbors[8], int *counter);

// Updating value of doubled buffer (copy from next_frame to prev_frame)
void copy_buffer(char prevFrame[ROWS][COLS], char nextFrame[ROWS][COLS]);

// Field cell update function
void updateCell(char prevFrame[ROWS][COLS], char nextFrame[ROWS][COLS], Cell cell);

void iterateField(char prevFrame[ROWS][COLS], char nextFrame[ROWS][COLS]);

void generateTextField(char prevField[ROWS][COLS]);

bool readAndValidateInput(char field[ROWS][COLS]);

void clearInputBuffer();

int main(int argc, const char *argv[]) {
    bool isPressedQuit = false;
    int speed = 1;
    struct timespec ts = {0, 100000000L};  // 100 миллисекунд
    char prevField[ROWS][COLS];
    char nextField[ROWS][COLS];

    if (argc == 1) {
        bool isValid = readAndValidateInput(prevField);
        clearInputBuffer();

        if (!isValid) {
            // printf("Go fuck urself!\n");
            return 0;
        }
        // printf("vse good\n");
        //  Я не знаю почему, но в этом случае не работает скорость
    } else if (argc > 1 && strcmp(argv[1], "-g") == 0) {
        generateTextField(prevField);
    } else {
        // Нам передали какую-то херотень
    }

    if (freopen("/dev/tty", "r", stdin)) initscr();  // Инициализация экрана
    raw();  // Отключаем буферизацию и специальные символы
    keypad(stdscr,
           TRUE);  // Включаем поддержку функциональных клавиш, включая стрелки
    noecho();      // Отключаем отображение вводимых символов
    nodelay(stdscr, TRUE);  // getch() больше не блокирует выполнение программы
    // scrollok(stdscr, TRUE); // Включаем автопрокрутку

    while (1) {
        int ch = getch();

        if (ch == 'q')
            isPressedQuit = true;
        else if (ch == KEY_LEFT)
            speed = (speed > 1) ? speed - 1 : 1;
        else if (ch == KEY_RIGHT)
            speed = (speed < 4) ? speed + 1 : 4;

        clear();

        iterateField(prevField, nextField);
        refresh();
        // Updating the buffer
        copy_buffer(prevField, nextField);

        for (int i = 0; i < 10 / speed; i++) {
            nanosleep(&ts, NULL);  // Спим 100 миллисекунд
            ch = getch();
            if (ch == 'q')
                isPressedQuit = true;
            else if (ch == KEY_LEFT)
                speed = (speed > 1) ? speed - 1 : 1;
            else if (ch == KEY_RIGHT)
                speed = (speed < 4) ? speed + 1 : 4;
        }

        if (isPressedQuit) break;
    }

    endwin();
    return 0;
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != EOF && c != '\n') {
        // printf("cleaning!");
    }
    // Очищаем до конца строки
}

bool readAndValidateInput(char field[ROWS][COLS]) {
    char line[COLS + 2];
    int lineCount = 0;
    bool valid = true;

    while (fgets(line, sizeof(line), stdin)) {
        if (strlen(line) != COLS + 1)  // Проверка на длину строки
        {
            valid = false;
            break;
        }
        strncpy(field[lineCount], line, COLS);
        lineCount++;
    }

    if (lineCount != ROWS)  // Проверка на количество строк
    {
        valid = false;
    }

    return valid;
}

void generateTextField(char prevField[ROWS][COLS]) {
    FILE *file = fopen("field.txt", "w");
    if (file == NULL) {
        perror("Unable to open file");
        return;
    }

    srand(time(NULL));  // Для случайных значений

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            // Заполняем символами '#' и пробелами случайным образом
            char ch = (rand() % 2) ? '#' : ' ';
            prevField[i][j] = ch;
            fputc(ch, file);
        }
        // Добавляем перенос строки после каждой строки
        fputc('\n', file);
    }

    fclose(file);
}

void iterateField(char prevFrame[ROWS][COLS], char nextFrame[ROWS][COLS]) {
    for (int row = 0; row < ROWS; ++row) {
        for (int column = 0; column < COLS; ++column) {
            Cell currentCell = {row, column};
            updateCell(prevFrame, nextFrame, currentCell);

            // Render field
            printw("%c", nextFrame[row][column]);
        }
        printw("\n");
    }
}

void copy_buffer(char prevFrame[ROWS][COLS], char nextFrame[ROWS][COLS]) {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; j++) {
            prevFrame[i][j] = nextFrame[i][j];
        }
    }
}

// Expecting to encounter only symbols of '#' or ' '
void updateCell(char prevFrame[ROWS][COLS], char nextFrame[ROWS][COLS], Cell cell) {
    int counterOfAliveNeighbors = 0;

    // Here I will add all 8 neighbors of current cell
    Cell cellNeighbors[8];

    // Filling the array above with neighbors
    findNeighbors(cell, cellNeighbors);

    // Checking if there are symbols of '#' among the neighbors
    countAliveNeighbors(prevFrame, cellNeighbors, &counterOfAliveNeighbors);

    if (prevFrame[cell.row][cell.column] == '#') {
        nextFrame[cell.row][cell.column] =
            (counterOfAliveNeighbors > 3 || counterOfAliveNeighbors < 2) ? ' ' : '#';
    } else {
        // Vot tut dobavit obrabotku probela
        nextFrame[cell.row][cell.column] = (counterOfAliveNeighbors == 3) ? '#' : ' ';
    }
}

// bool isHorizontalBorder(int column);
void findNeighbors(Cell cell, Cell neighborsArray[8]) {
    int index = 0;
    for (int i = cell.row - 1; i <= cell.row + 1; ++i) {
        for (int j = cell.column - 1; j <= cell.column + 1; ++j) {
            if (i == cell.row && j == cell.column) {
                continue;
            }

            neighborsArray[index].row = i;
            neighborsArray[index].column = j;

            ++index;
        }
    }
}

void countAliveNeighbors(const char prevFrame[ROWS][COLS], Cell neighbors[8], int *counter) {
    for (int i = 0; i < 8; ++i) {
        Cell validCell = {neighbors[i].row, neighbors[i].column};

        // Here I am checking for oveflow (% 25 and % 80) and underflow (+ 25 and +
        // 80)
        validCell.row = (validCell.row < 0) ? validCell.row + ROWS : validCell.row % ROWS;
        validCell.column = (validCell.column < 0) ? validCell.column + COLS : validCell.column % COLS;

        if (prevFrame[validCell.row][validCell.column] == '#') {
            ++(*counter);
        }
    }
}