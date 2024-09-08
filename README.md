# C4LIfe
Game of life console simulation using C language

clang-format -i src/game_of_life.c
---
cppcheck --enable=all --suppress=missingIncludeSystem --check-level=exhaustive src/game_of_life.c
---
gcc -Wall -Werror -Wextra src/game_of_life.c -o game -fsanitize=address -fsanitize=undefined -fsanitize=unreachable -lncurses
