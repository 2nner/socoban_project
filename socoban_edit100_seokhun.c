#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <termios.h>

#define clear() printf("\033[H\033[J") // Cygwin���� ȭ�� ������ ���� - ���� : clear()

int num_map; // �� ����
int cnt_undo = 5; // �ǵ����� ���� Ƚ��
char username[11]; // ���� �г���
char PATH_MAP[] = "/cygdrive/c/users/������/desktop/sokoban_project/map.txt"; // map ������ �ִ� ���
char map[6][30][30]; // �� �����ϴ� ��
char current_map[30][30]; // ���� �÷������� ��
int length_garo[6], length_sero[6]; // Stage ���� �� ���� ���� ���� ����
int pwd_g, pwd_s; // pwd_g : ���� ���� ��ġ, pwd_s : ���� ���� ��ġ
int count = 0; // �̵��� Ƚ�� ����
int current_stage; // ���� �÷������� ��������
int end_count; // ���� ���� ����

int before_map[6][30][30]; // ����� ��

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

// �� �и� & �������� ���� ī��Ʈ & �������� ���� �ڽ�($) ������ ���� ���(O) ������ ���� �ʰų� �ƿ� ���� ��� ���� ��� �� ���α׷� ����
void checkMap() {
	int stage = 1, pos = 0;
	char str[31];

	FILE* f = fopen(PATH_MAP, "r");

	// map ���Ͽ��� ������������ map 2���� �迭�� �и�
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
		printf("������ ��ΰ� �߸� �����Ǿ��ų�, ������ �����ϴ�!");
		exit(EXIT_FAILURE);
	}
	fclose(f);

	// Stage ���� �ڽ��� ���� ��� ���� ������ ��
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

// Ŀ�ǵ� ���
void printCommand() {
	printf("h(����), j(�Ʒ�), k(��), l(������)\n");
	printf("u(undo) : �ǵ����� (���� Ƚ�� : %d��)\n", cnt_undo);
	printf("r(replay) : ���� �� ó������ �ٽ� ���� (������ Ƚ�� ��� ������)\n");
	printf("n(new) : ù ��° �ʺ��� �ٽ� ���� (������ Ƚ�� ����� ������)\n");
	printf("e(exit) : ���� ����\n");
	printf("s(save) : ���̺� ���Ͽ� ����\n");
	printf("f(file load) : ���̺� ���� �ε�\n");
	printf("d(display help) : ��� ���� ���\n");
	printf("t(top) : ���� ���� ���, t �ڿ� ���ڰ� ���� �ش� ���� ���� ���\n");
}

// �г��� ����
void getNickname() {
	while (1) {
		clear(); // Cygwin���� ȭ�� ����ϰ� ����� ���α׷� �����ϱ� ���ؼ� ���
		printf("���� �̸� �Է�(�������� �ִ� 10����) : ");
		scanf("%s", username);

		if (strlen(username) <= 10)
			break;
		else
			printf("�ִ� 10���ڱ��� �Է� �����մϴ�!\n");

		getch();
		getch();
	}
}

// �������� �������ִ� �Լ�, ���� : move(�����̴� ���� ĭ ��, �����̴� ���� ĭ ��, ���� �������� �ѹ�)
void move(int s, int g, int current_stage) {

	//undo�� �������� ���� �� ������ ���� (������ ������ ����ʿ��� ����)
	for (int i = 4; i >= 0; i--) {
		for (int j = 0; j < length_sero[current_stage]; ++j) {
			strcpy(before_map[i + 1][j], before_map[i][j]);
		}
	}
	for (int i = 0; i < length_sero[current_stage]; ++i) {
		strcpy(before_map[0][i], current_map[i]);
	}

	//������ ����
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

// ����
void save() {
	FILE* save;
	save = fopen("sokoban.txt", "w");
	fprintf(save, "%d\n", current_stage); // ���� ��������
	fprintf(save, "%d\n", count); // ������ Ƚ��
	fprintf(save, "%d\n", end_count); // ���� ������ ����
	fprintf(save, "%s\n", username); // ������ �̸�
	for (int i = 0; i < length_sero[current_stage]; ++i) {
		fprintf(save, "%s\n", current_map[i]);
	} // ��
	fclose(save);
}

// ���Ϻҷ�����
void fileload() {
	FILE *load;
	load = fopen("sokoban.txt", "r");
	if (load == NULL) {
		printf("������ �������� �ʽ��ϴ�.");
	}
	else {
		fscanf(load, "%d", &current_stage); // ���� ��������
		fscanf(load, "%d", &count); // ������ Ƚ��
		fscanf(load, "%d", &end_count); // ���� ������ ����
		fscanf(load, "%s", &username); // ������ �̸�
		for (int i = 0; i < length_sero[current_stage]; ++i) {
			fscanf(load, "%s", current_map[i]);
		} // ��
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

// ���� �� ó������ �ٽ� ����
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

//uŰ�� ������ �������� ���ư� (undo ����, ������� ������Ϸ� �ٲٰ� Ƚ�� 1 ������)
void undo() {
	count += 1;
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


// nŰ�� ������ 1������� �������
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

	// ���� ����
	for (current_stage = 1; current_stage <= num_map; ++current_stage) {


		// �������� ����
		for (int i = 0; i < length_sero[current_stage]; ++i) {
			strcpy(current_map[i], map[current_stage][i]);
		}

		// �ʱ� undo�� ���� �� ���(��������� ����ʱ�ȭ) + cnt_undo = 5 �ʱ�ȭ
		cnt_undo = 5;
		for (int i = 0; i <= 5; i++) {
			for (int j = 0; j < length_sero[current_stage]; ++j) {
				strcpy(before_map[i][j], current_map[j]);
			}
		}

		count = 0;

		// ���� ��ġ Ž��
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
			int end_count = 0; // ���� ���� ���� ����

			printf("Stage %d\n", current_stage);
			// �� ���
			for (int i = 0; i < length_sero[current_stage]; ++i) {
				printf("%s\n", current_map[i]);
			}
			printf("������ Ƚ�� : %d\n", count);

			printf("���� undo Ƚ�� : %d\n", cnt_undo);

			// ������ ��� �ȿ� �ڽ��� �� ���� �� üũ�ϰ�, ���� �ÿ� while�� Ż��
			for (int s = 0; s < length_sero[1]; s++) {
				for (int g = 0; g < length_garo[1]; g++) {
					if (current_map[s][g] == 'O')
						end_count++;
				}
			}
			if (map[current_stage][pwd_s][pwd_g] == 'O') end_count++;
			printf("���� �ڽ� ���� : %d\n", end_count);
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
			case 's': { // s
				save();
				printf("\n    ������ �Ϸ�Ǿ����ϴ�\n(����Ϸ��� �ƹ�Ű�� ��������)");
				getch();
				break;
			}

			case 'r': { // r
				replay();
				break;
			}

			case 'f': { //f
				fileload();
				break;
			}

			case 'e': { //e
				printf("\n    ������ �Ϸ�Ǿ����ϴ�. �����մϴ�.\n");
				save();
				return 0;
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
	printf("���������� ��� �Ϸ��ϼ̳׿� �����մϴ�. \n");
	getch();
}