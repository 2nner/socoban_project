#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <termios.h>

#define clear() printf("\033[H\033[J") // Cygwin에서 화면 깨끗이 해줌 - 사용법 : clear()

int num_map; // 맵 개수
int cnt_undo = 5; // 되돌리기 남은 횟수
char username[11]; // 유저 닉네임
char PATH_MAP[] = "C:\\Users\\지석훈\\Desktop\\socoban_project\\map.txt"; // map 파일이 있는 경로
char map[6][30][30]; // 맵 저장하는 곳
char current_map[30][30]; // 현재 플레이중인 맵
int length_garo[6], length_sero[6]; // Stage 별로 맵 가로 세로 길이 저장
int pwd_g, pwd_s; // pwd_g : 가로 현재 위치, pwd_s : 세로 현재 위치
int count = 0; // 이동한 횟수 저장
int current_stage; // 현재 플레이중인 스테이지
int end_count; // 남은 구멍 개수
int save_stage; //저장할 스테이지
int save_count; // 저장할 움직인 횟수
int save_end_count; // 저장할 남은 구멍 개수

int before_map[6][30][30]; // 백업용 맵

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

// 맵 분리 & 스테이지 개수 카운트 & 스테이지 별로 박스($) 개수와 보관 장소(O) 개수가 같지 않거나 아예 없을 경우 에러 출력 후 프로그램 종료
void checkMap() {
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
		exit(EXIT_FAILURE);
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
		if (num_box == 0 || num_place == 0 || num_box != num_place) error(i);
	}
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
		scanf("%s", username);

		if (strlen(username) <= 10)
			break;
		else
			printf("최대 10글자까지 입력 가능합니다!\n");

		getch();
		getch();
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
void printRank() {

}

// 저장
void save(int save_stage, int save_count, int save_end_count)
{
	FILE* save;
	save = fopen("sokoban.txt", "w");
	// fprintf(save, "유저 이름 : %s\n", username);
	fprintf(save, "Stage %d\n", save_stage);
	for (int i = 0; i < length_sero[save_stage]; ++i) {
		fprintf(save, "%s\n", current_map[i]);
	}
	fprintf(save, "움직인 횟수 : %d\n", save_count);
	fprintf(save, "남은 구멍 개수 : %d\n", save_end_count);
	fclose(save);
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

//u키를 누르면 이전으로 돌아감 (undo 구현, 현재맵을 백업파일로 바꾸고 횟수 1 차감함)
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





// e키를 누르면 종료함
void quit() {
	exit(0);
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

int main(void) {
	checkMap();
	getNickname();

	printCommand();
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
			int end_count = 0; // 구멍 개수 세는 변수

			printf("Stage %d\n", current_stage);
			// 맵 출력
			for (int i = 0; i < length_sero[current_stage]; ++i) {
				printf("%s\n", current_map[i]);
			}
			printf("움직인 횟수 : %d\n", count);

			printf("남은 undo 횟수 : %d\n", cnt_undo);

			// 지정된 장소 안에 박스가 다 들어갔는 지 체크하고, 참일 시에 while문 탈출
			for (int s = 0; s < length_sero[1]; s++) {
				for (int g = 0; g < length_garo[1]; g++) {
					if (current_map[s][g] == 'O')
						end_count++;
				}
			}
			if (map[current_stage][pwd_s][pwd_g] == 'O') end_count++;
			printf("남은 박스 개수 : %d\n", end_count);
			if (end_count == 0) break;

			c = getch();
			switch (c) {
			case 'h': { // 75
				count++;
				move(0, -1, current_stage);
				break;
			}

			case 'j': { // 80
				count++;
				move(1, 0, current_stage);
				break;
			}

			case 'k': { // 72
				count++;
				move(-1, 0, current_stage);
				break;
			}

			case 'l': { // 77
				count++;
				move(0, 1, current_stage);
				break;
			}
			case 115: { // s
				save_count = count;
				save_end_count = end_count;
				save(save_stage, save_count, save_end_count);
				printf("\n    저장이 완료되었습니다\n(계속하려면 아무키나 누르세요)");
				getch();
				break;
			}

			case 114: { // r

				replay();
				break;
				break;
			}

			case 'e': { //e
				save_count = count;
				save_end_count = end_count;
				save(save_stage, save_count, save_end_count);
				printf("\n    저장이 완료되었습니다. 종료합니다.\n");
				quit();
				break;
			}

			case 'n': { //n
				newstart();
				break;
			}

			case 'u': { //u
				undo();
				break;
			}
			}

		}
		printf("Stage %d Clear!", current_stage);
		count = 0;
		getch();
		fflush(stdin);
	}
	printf("스테이지를 모두 완료하셨네요 축하합니다. \n");
	getch();
}
