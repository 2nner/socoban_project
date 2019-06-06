#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <conio.h> // Cygwin 사용 시 주석 처리
#include <termios.h> // Cygwin 사용 시 주석 해제

int num_map; // 맵 개수
int cnt_undo = 5; // 되돌리기 남은 횟수
char username[11]; // 유저 닉네임
char PATH_MAP[] = "map.txt"; // map 파일이 있는 경로
char map[6][30][30]; // 맵 저장하는 곳
char current_map[30][30]; // 현재 플레이중인 맵
int length_garo[6], length_sero[6]; // Stage 별로 맵 가로 세로 길이 저장
int pwd_g, pwd_s; // pwd_g : 가로 현재 위치, pwd_s : 세로 현재 위치
int current_stage; // 현재 플레이중인 스테이지
int count = 0; // 이동한 횟수 저장
int end_count; // 남은 구멍 개수

char before_map[6][30][30]; // 백업용 맵

int getch() {
	int ch;
	struct termios buf;
	struct termios save;

	tcgetattr(0, &save);
	buf = save;
	buf.c_lflag &= ~(ICANON | ECHO);
	buf.c_cc[VMIN] = 1;
	buf.c_cc[VTIME] = 0;
	tcsetattr(0, TCSAFLUSH, &buf);
	ch = getchar();
	tcsetattr(0, TCSAFLUSH, &save);
	return ch;
}

// 화면 깨끗하게 해주는 함수, Cygwin 사용 시 cls를 clear로 바꿀 것
void clear() {
	printf("\033[H\033[J");
}

// 박스 개수와 보관 장소 개수가 다를 때 출력하는 에러 메세지. 출력 후 프로그램이 종료됨
void error(int i) {
	printf("Stage %d의 박스 개수와 보관장소 개수가 같지 않거나, 옮길 박스 혹은 보관 장소가 존재하지 않습니다. 프로그램을 종료합니다.", i);
}

// 맵 분리 & 스테이지 개수 카운트 & 스테이지 별로 박스($) 개수와 보관 장소(O) 개수가 같지 않거나 아예 없을 경우 에러 출력 후 프로그램 종료
int checkMap() {
	int stage = 1, pos = 0;
	char str[31];

	FILE* f = fopen(PATH_MAP, "r");

	// map 파일에서 스테이지별로 map 2차원 배열로 분리
	if (f != NULL) {
		while (fscanf(f, "%s", str) != EOF) {
			if (strlen(str) == 1) {
				if ((str[0] - '0') >= 1 && (str[0] - '0') <= 5)
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
		return 0;
	}
	fclose(f);

	// Stage 별로 박스와 보관 장수 개수 같은지 비교
	for (int i = 1; i <= num_map; ++i) {
		int num_box = 0, num_place = 0;

		for (int j = 0; j < length_sero[i]; ++j) {
			for (int k = 0; k < length_garo[i]; ++k) {
				if (map[i][j][k] == '$') num_box++;
				else if (map[i][j][k] == 'O') num_place++;
			}
		}
		if (num_box == 0 || num_place == 0 || num_box != num_place) {
			error(i);
			return 0;
		}
	}

	return 1;
}

// 커맨드 출력
void printCommand() {
	printf("h(왼쪽), j(아래), k(위), l(오른쪽)\n");
	printf("u(undo) : 되돌리기 (남은 횟수 : %d번)\n", cnt_undo);
	printf("r(replay) : 현재 맵 처음부터 다시 시작 (움직임 횟수 계속 유지됨)\n");
	printf("n(new) : 첫 번째 맵부터 다시 시작 (움직임 횟수 기록은 삭제됨)\n");
	printf("e(exit) : 게임 종료\n");
	printf("s(save) : 세이브 파일에 저장\n");
	printf("f(file load) : 세이브 파일 로드\n");
	printf("d(display help) : 명령 내용 출력\n");
	printf("t(top) : 게임 순위 출력, t 뒤에 숫자가 오면 해당 맵의 순위 출력\n");
}

// 닉네임 저장
void getNickname() {
	while (1) {
		clear(); // Cygwin에서 화면 깔끔하게 지우고 프로그램 시작하기 위해서 사용
		printf("유저 이름 입력(영문으로 최대 10글자) : ");
		scanf("%s%*c", username);

		if (strlen(username) <= 10)
			break;
		else {
			printf("최대 10글자까지 입력 가능합니다!\n");
			getch();
		}
	}
}

// 움직임이 구현되있는 함수, 사용법 : move(울직이는 세로 칸 수, 움직이는 가로 칸 수, 현재 스테이지 넘버)
void move(int s, int g, int current_stage) {

	//undo를 쓰기위한 이전 맵 데이터 저장 (움직일 때마다 백업맵에다 저장)
	for (int i = 4; i >= 0; i--) {
		for (int j = 0; j < length_sero[current_stage]; ++j) {
			strcpy(before_map[i + 1][j], before_map[i][j]);
		}
	}
	for (int i = 0; i < length_sero[current_stage]; ++i) {
		strcpy(before_map[0][i], current_map[i]);
	}

	//움직임 구현
	if (current_map[pwd_s + s][pwd_g + g] == '.' || current_map[pwd_s + s][pwd_g + g] == 'O') {
		if (map[current_stage][pwd_s][pwd_g] == '@' || map[current_stage][pwd_s][pwd_g] == '$') {
			current_map[pwd_s][pwd_g] = '.';
		}
		else {
			current_map[pwd_s][pwd_g] = map[current_stage][pwd_s][pwd_g];
		}
	}
	else if (current_map[pwd_s + s][pwd_g + g] == '#') return;
	else if (current_map[pwd_s + s][pwd_g + g] == '$') {
		if (current_map[pwd_s + (2 * s)][pwd_g + (2 * g)] == '$' || current_map[pwd_s + (2 * s)][pwd_g + (2 * g)] == '#') return;
		if (current_map[pwd_s + (2 * s)][pwd_g + (2 * g)] == '.') {
			current_map[pwd_s + (2 * s)][pwd_g + (2 * g)] = '$';
			if (map[current_stage][pwd_s][pwd_g] == '@' || map[current_stage][pwd_s][pwd_g] == '$') {
				current_map[pwd_s][pwd_g] = '.';
			}
			else {
				current_map[pwd_s][pwd_g] = map[current_stage][pwd_s][pwd_g];
			}
		}
		else if (current_map[pwd_s + (2 * s)][pwd_g + (2 * g)] == 'O') {
			current_map[pwd_s + (2 * s)][pwd_g + (2 * g)] = '$';
			if (map[current_stage][pwd_s][pwd_g] == '@' || map[current_stage][pwd_s][pwd_g] == '$') {
				current_map[pwd_s][pwd_g] = '.';
			}
			else {
				current_map[pwd_s][pwd_g] = map[current_stage][pwd_s][pwd_g];
			}
		}
	}

	current_map[pwd_s + s][pwd_g + g] = '@';
	pwd_s += s;
	pwd_g += g;

}

// 랭킹 출력
void printRank(int stage) {

	clear();

	FILE* rank;
	rank = fopen("ranking.txt", "rt");
	char input[20];
	int n;
	char ranking[5][6][20];
	int rank_count[5][6];

	if (rank == NULL)
	{
		printf("저장된 랭킹이 없습니다.");
	}

	else {
		clear();

		if (stage == 0) {
			while (fscanf(rank, "%s %d", input, &n) != EOF)
				printf("%s %d\n", input, n);
		}
		else {
			int i = 0; //stage
			int t = 0;
			while (fscanf(rank, "%s %d", input, &n) != EOF) {
				if (input[0] == '-') {
					if (n == stage) {
						printf("%s %d\n", input, n);
					}
					i = n - 1;
					t = 0;
					continue;
				}
				else {
					strcpy(ranking[i][t], input);
					rank_count[i][t] = n;
					if (i == stage - 1) {
						printf("%s %d\n", ranking[i][t], rank_count[i][t]);
					}
					t++;
				}

			}
		}

		fclose(rank);
	}
}

// 저장
void save()
{
	FILE* save;
	save = fopen("sokoban.txt", "w");
	fprintf(save, "%d\n", current_stage); // 현재 스테이지
	fprintf(save, "%d\n", count); // 움직인 횟수
	fprintf(save, "%d\n", end_count); // 남은 구멍의 개수
	fprintf(save, "%s\n", username); // 유저의 이름
	for (int i = 0; i < length_sero[current_stage]; ++i) {
		fprintf(save, "%s\n", current_map[i]);
	} // 맵
	fclose(save);
}

// 파일불러오기
void fileload() {
	FILE *load;
	load = fopen("sokoban.txt", "r");
	if (load == NULL) {
		printf("파일이 존재하지 않습니다.");
	}
	else {
		fscanf(load, "%d", &current_stage); // 현재 스테이지
		fscanf(load, "%d", &count); // 움직인 횟수
		fscanf(load, "%d", &end_count); // 남은 구멍의 개수
		fscanf(load, "%s", &username); // 유저의 이름
		for (int i = 0; i < length_sero[current_stage]; ++i) {
			fscanf(load, "%s", current_map[i]);
		} // 맵
	}
	fclose(load);

	for (int i = 0; i < length_sero[current_stage]; ++i) {
		int flag = 0;
		for (int j = 0; j < length_garo[current_stage]; ++j) {
			if (current_map[i][j] == '@') {
				pwd_s = i, pwd_g = j, flag = 1;
				break;
			}
		}

		if (flag) break;

	}
}

// 현재 맵 처음부터 다시 시작
void replay() {
	clear();
	for (int i = 0; i < length_sero[current_stage]; ++i) {
		strcpy(current_map[i], map[current_stage][i]);
	}

	for (int i = 0; i < length_sero[current_stage]; ++i) {
		int flag = 0;
		for (int j = 0; j < length_garo[current_stage]; ++j) {
			if (current_map[i][j] == '@') {
				pwd_s = i, pwd_g = j, flag = 1;
				break;
			}
		}

		if (flag) break;

	}
}

// u키를 누르면 이전으로 돌아감 (undo 구현, 현재맵을 백업파일로 바꾸고 횟수 1 차감함)
void undo() {
	if (cnt_undo != 0) {
		for (int i = 0; i < length_sero[current_stage]; ++i) {
			strcpy(current_map[i], before_map[0][i]);
		}
		for (int i = 0; i <= 4; i++) {
			for (int j = 0; j < length_sero[current_stage]; ++j) {
				strcpy(before_map[i][j], before_map[i + 1][j]);
			}
		}
		for (int i = 0; i < length_sero[current_stage]; ++i) {
			int flag = 0;
			for (int j = 0; j < length_garo[current_stage]; ++j) {
				if (current_map[i][j] == '@') {
					pwd_s = i, pwd_g = j, flag = 1;
					break;
				}
			}

			if (flag) break;

		}
		cnt_undo -= 1;
	}
}

// n키를 누르면 1라운드부터 재시작함
void newstart() {
	clear();

	current_stage = 1;
	count = 0;
	cnt_undo = 5;

	for (int i = 0; i < length_sero[current_stage]; ++i) {
		strcpy(current_map[i], map[current_stage][i]);
	}


	for (int i = 0; i < length_sero[current_stage]; ++i) {
		int flag = 0;
		for (int j = 0; j < length_garo[current_stage]; ++j) {
			if (current_map[i][j] == '@') {
				pwd_s = i, pwd_g = j, flag = 1;
				break;
			}
		}

		if (flag) break;

	}
}

// 스테이지 클리어 시 랭킹 업데이트
void updateRank() {
	FILE* f = fopen("ranking.txt", "r");

	char input[11];
	int n, pos;
	int fstage; // 파일을 읽으면서 현재 스테이지 저장할 변수
	int num_ranking[5] = { 0, 0, 0, 0, 0 }; // 스테이지 별 랭킹에 몇 명이 있는지 카운트
	char username_ranking[5][5][20]; // 스테이지 별 랭킹에 등록되어 있는 사람들의 유저 네임 저장, [stage_num][ranking : 1st, 2nd..]
	int score_ranking[5][5]; // 스테이지 별 랭킹에 동록되어 있는 사람들의 횟수 저장 [stage_num][ranking]

	while (fscanf(f, "%s %d", input, &n) != EOF) {
		if (input[0] == '-') {
			pos = 0;
			fstage = n - 1;
			continue;
		}
		else {
			strcpy(username_ranking[fstage][pos], input);
			score_ranking[fstage][pos++] = n;
			num_ranking[fstage]++;
		}
	}

	// 새로 랭크에 오를 위치 선정
	for (pos = 0; pos < num_ranking[current_stage - 1]; pos++) {
		if (score_ranking[current_stage - 1][pos] > count)
			break;
	}

	// 랭크될 위치에 랭킹 등록
	if (pos < 5) {
		for (int i = num_ranking[current_stage - 1] - 1; i >= pos; i--) {
			if (i == 4) continue;
			strcpy(username_ranking[current_stage - 1][i + 1], username_ranking[current_stage - 1][i]);
			score_ranking[current_stage - 1][i + 1] = score_ranking[current_stage - 1][i];
		}
		strcpy(username_ranking[current_stage - 1][pos], username);
		score_ranking[current_stage - 1][pos] = count;
	}
	if (num_ranking[current_stage - 1] < 5) num_ranking[current_stage - 1]++;

	// 파일 재작성
	f = fopen("ranking.txt", "w");

	for (int i = 0; i < 5; i++) {
		fprintf(f, "-Stage %d\n", i + 1);
		for (int j = 0; j < num_ranking[i]; j++)
			fprintf(f, "%s %d\n", username_ranking[i][j], score_ranking[i][j]);
	}

	fclose(f);
}

int main(void) {
	if (!checkMap()) return 0;
        getNickname();

        clear();

        // 게임 시작
        for (current_stage = 1; current_stage <= num_map; ++current_stage) {

                // 스테이지 복사
                for (int i = 0; i < length_sero[current_stage]; ++i) {
                        strcpy(current_map[i], map[current_stage][i]);
                }

                // 초기 undo를 위한 맵 백업(현재맵으로 모두초기화) + cnt_undo = 5 초기화
                cnt_undo = 5;
                for (int i = 0; i <= 5; i++) {
                        for (int j = 0; j < length_sero[current_stage]; ++j) {
                                strcpy(before_map[i][j], current_map[j]);
                        }
                }

                // 움직인 횟수 초기화
                count = 0;

                // 현재 위치 탐색
                for (int i = 0; i < length_sero[current_stage]; ++i) {
                        int flag = 0;
                        for (int j = 0; j < length_garo[current_stage]; ++j) {
                                if (current_map[i][j] == '@') {
                                        pwd_s = i, pwd_g = j, flag = 1;
                                        break;
                                }
                        }

                        if (flag) break;
                }

                while (1) {
                        clear();
                        char c;
                        end_count = 0; // 구멍 개수 세는 변수

                        printf("%s님 플레이 중\n", username);
                        printf("Stage %d\n", current_stage);
                        // 맵 출력
                        for (int i = 0; i < length_sero[current_stage]; ++i) {
                                printf("%s\n", current_map[i]);
                        }
                        printf("움직인 횟수 : %d\n", count);
                        printf("남은 undo 횟수 : %d\n", cnt_undo);

                        // 지정된 장소 안에 박스가 다 들어갔는 지 체크하고, 참일 시에 while문 탈출
                        for (int s = 0; s < length_sero[current_stage]; s++) {
                                for (int g = 0; g < length_garo[current_stage]; g++) {
                                        if (current_map[s][g] == 'O')
                                                end_count++;
                                }
                        }
                        if (map[current_stage][pwd_s][pwd_g] == 'O') end_count++;
                        printf("남은 박스 개수 : %d\n", end_count);
                        if (end_count == 0) break;

                        c = getch();
                        switch (c) {
                        case 'h': {
                                count++;
                                move(0, -1, current_stage);
                                break;
                        }

                        case 'j': {
                                count++;
                                move(1, 0, current_stage);
                                break;
                        }

                        case 'k': {
                                count++;
                                move(-1, 0, current_stage);
                                break;
                        }

                        case 'l': {
                                count++;
                                move(0, 1, current_stage);
                                break;
                        }

                        case 's': {
                                save();
                                printf("\n    저장이 완료되었습니다\n(계속하려면 아무키나 누르세요)");
                                getch();
                                break;
                        }

                        case 'r': {
                                replay();
                                break;
                        }
                        case 'f': {
                                fileload();
                                break;
                        }
                        case 'e': {
                                save();
                                printf("\n    저장이 완료되었습니다. 종료합니다.\n");
                                return 0;
                        }

                        case 'n': {
                                newstart();
                                break;
                        }

                        case 'u': {
                                undo();
                                break;
                        }

                        case 't': {
                                printf("t");
                                char a;
                                a = getch();

                                switch (a) {
                                case 10: { // 엔터
                                        printRank(0);
                                        getch();
                                        break;
                                }
                                case 32: {  // 스페이스바
                                        int b;
                                        printf(" ");
                                        scanf("%d%*c", &b);
                                        printRank(b);
                                        getch();
                                        break;
                                }
                                }
                                break;
                        }

                        case 'd': { // d
                                printf("\n");
                                printCommand();
                                getch();
                                break;
                        }
                        }

                }
                printf("Stage %d Clear!", current_stage);
                updateRank();
                getch();
        }
        printf("스테이지를 모두 완료하셨네요 축하합니다. \n");
        getch();
}
