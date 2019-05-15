#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>

#define clear() printf("\033[H\033[J") // Cygwin에서 화면 깨끗이 해줌 - 사용법 : clear()

int num_map; // 맵 개수
char username[11]; // 유저 닉네임
char PATH_MAP[] = "C:\\Users\\khsbs\\CLionProjects\\project\\map"; // map 파일이 있는 경로
char map[6][30][30]; // 맵 저장하는 곳
int length_garo[6], length_sero[6]; // Stage 별로 맵 가로 세로 길이 저장

int getch(void) {
    int ch;
    struct termios buf;
    struct termios save;

    tcgetattr(0, &save);
    buf = save;
    buf.c_lflag &= ~(ICANON|ECHO);
    buf.c_cc[VMIN] = 1;
    buf.c_cc[VTIME] = 0;
    tcsetattr(0, TCSAFLUSH, &buf);
    ch = getchar();
    tcsetattr(0, TCSAFLUSH, &save);
    return ch;
}

// 박스 개수와 보관 장소 개수가 다를 때 출력하는 에러 메세지. 출력 후 프로그램이 종료됨
void error(int i) {
    printf("Stage %d의 박스 개수와 보관장소 개수가 같지 않거나, 옮길 박스 혹은 보관 장소가 존재하지 않습니다. 프로그램을 종료합니다.", i);
    exit(EXIT_FAILURE);
}

// 맵 분리 & 스테이지 개수 카운트 & 스테이지 별로 박스($) 개수와 보관 장소(O) 개수가 같지 않거나 아예 없을 경우 에러 출력 후 프로그램 종료
void checkMap() {
    int stage = 1, pos = 0;
    char str[31];

    FILE *f = fopen(PATH_MAP, "r");

    // map 파일에서 스테이지별로 map 2차원 배열로 분리
    if (f != NULL) {
        while (fscanf(f, "%s", str) != EOF) {
            if (strlen(str) == 1) {
                if (isdigit(str[0]))
                    stage = str[0] - '0', pos = 0;
                else break;
            }
            else {
                strcpy(map[stage][pos++], str);
                length_garo[stage] = strlen(str);
                length_sero[stage] = pos;
            }
        }
        num_map = stage;
    }
    else {
        printf("파일의 경로가 잘못 지정되었거나, 파일이 없습니다!");
        exit(EXIT_FAILURE);
    }
    fclose(f);

    // Stage 별로 박스와 보관 장수 개수 같은지 비교
    for (int i = 1; i <= num_map ; ++i) {
        int num_box = 0, num_place = 0;

        for (int j = 0; j < length_sero[i]; ++j) {
            for (int k = 0; k < length_garo[i]; ++k) {
                if (map[i][j][k] == '$') num_box++;
                else if (map[i][j][k] == 'O') num_place++;
            }
        }
        if (num_box == 0 || num_place == 0 || num_box != num_place) error(i);
    }
}

// 닉네임 저장
void getNickname() {
    while (1) {
        clear(); // Cygwin에서 화면 깔끔하게 지우고 프로그램 시작하기 위해서 사용
        printf("유저 이름 입력(영문으로 최대 10글자) : ");
        scanf("%s", username);

        if (strlen(username) <= 10)
            break;
        else
            printf("최대 10글자까지 입력 가능합니다!");

        sleep(2);
    }
}

int main(void){
    checkMap();
    getNickname();


}