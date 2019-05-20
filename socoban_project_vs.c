#define _CRT_NONSTDC_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <conio.h>

#define clear() printf("\033[H\033[J") // Cygwin���� ȭ�� ������ ���� - ���� : clear()

int num_map; // �� ����
int cnt_undo = 5; // �ǵ����� ���� Ƚ��
int current_stage = 1; // ���� �÷������� �� ��������
char username[11]; // ���� �г���
char PATH_MAP[] = "C:\\Users\\wjdgh\\Desktop\\socoban_project\\map.txt"; // map ������ �ִ� ���
char map[6][30][30]; // �� �����ϴ� ��
char current_map[30][30]; // ���� �÷������� ��
int length_garo[6], length_sero[6]; // Stage ���� �� ���� ���� ���� ����
int count = 0; // �̵��� Ƚ�� ����

// �ڽ� ������ ���� ��� ������ �ٸ� �� ����ϴ� ���� �޼���. ��� �� ���α׷��� �����
void error(int i) {
	printf("Stage %d�� �ڽ� ������ ������� ������ ���� �ʰų�, �ű� �ڽ� Ȥ�� ���� ��Ұ� �������� �ʽ��ϴ�. ���α׷��� �����մϴ�.", i);
	exit(EXIT_FAILURE);
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

// ��ŷ ���
void printRank() {

}

int main(void) {
	checkMap();
	getNickname();

	clear();

	// ���� ����
	for (int stage = 1; stage <= num_map; ++stage) {
		// �������� ����
		for (int i = 0; i < length_sero[current_stage]; ++i) {
			strcpy(current_map[i], map[current_stage][i]);
		}

		int pwd_g, pwd_s; // pwd_g : ���� ���� ��ġ, pwd_s : ���� ���� ��ġ

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
			system("cls");
			char c;
			int end_count = 0; // ���۰��� ���� ����


			// �� ���
			for (int i = 0; i < length_sero[current_stage]; ++i) {
				printf("%s\n", current_map[i]);

			}
			printf("%d", count);

			c = getch();
			switch (c) {
			case 75: { // h
				if (current_map[pwd_s][pwd_g - 1] == '.' || current_map[pwd_s][pwd_g - 1] == 'O') {
					if (map[current_stage][pwd_s][pwd_g] == '@' || map[current_stage][pwd_s][pwd_g] == '$') {
						current_map[pwd_s][pwd_g] = '.';
					}
					else {
						current_map[pwd_s][pwd_g] = map[current_stage][pwd_s][pwd_g];
					}
				}
				else if (current_map[pwd_s][pwd_g - 1] == '#') continue;
				else if (current_map[pwd_s][pwd_g - 1] == '$') {
					if (current_map[pwd_s][pwd_g - 2] == '$' || current_map[pwd_s][pwd_g - 2] == '#') continue;
					if (current_map[pwd_s][pwd_g - 2] == '.') {
						current_map[pwd_s][pwd_g - 2] = '$';
						if (map[current_stage][pwd_s][pwd_g] == '@' || map[current_stage][pwd_s][pwd_g] == '$') {
							current_map[pwd_s][pwd_g] = '.';
						}
						else {
							current_map[pwd_s][pwd_g] = map[current_stage][pwd_s][pwd_g];
						}
					}
					else if (current_map[pwd_s][pwd_g - 2] == 'O') {
						current_map[pwd_s][pwd_g - 2] = '$';
						if (map[current_stage][pwd_s][pwd_g] == '@' || map[current_stage][pwd_s][pwd_g] == '$') {
							current_map[pwd_s][pwd_g] = '.';
						}
						else {
							current_map[pwd_s][pwd_g] = map[current_stage][pwd_s][pwd_g];
						}
					}
				}

				current_map[pwd_s][pwd_g - 1] = '@';
				pwd_g -= 1;
				count++;
				break;
			}

			case 80: { // j
				if (current_map[pwd_s + 1][pwd_g] == '.' || current_map[pwd_s + 1][pwd_g] == 'O') {
					if (map[current_stage][pwd_s][pwd_g] == '@' || map[current_stage][pwd_s][pwd_g] == '$') {
						current_map[pwd_s][pwd_g] = '.';
					}
					else {
						current_map[pwd_s][pwd_g] = map[current_stage][pwd_s][pwd_g];
					}
				}
				else if (current_map[pwd_s + 1][pwd_g] == '#') continue;
				else if (current_map[pwd_s + 1][pwd_g] == '$') {
					if (current_map[pwd_s + 2][pwd_g] == '$' || current_map[pwd_s + 2][pwd_g] == '#') continue;
					if (current_map[pwd_s + 2][pwd_g] == '.') {
						current_map[pwd_s + 2][pwd_g] = '$';
						if (map[current_stage][pwd_s][pwd_g] == '@' || map[current_stage][pwd_s][pwd_g] == '$') {
							current_map[pwd_s][pwd_g] = '.';
						}
						else {
							current_map[pwd_s][pwd_g] = map[current_stage][pwd_s][pwd_g];
						}
					}
					else if (current_map[pwd_s + 2][pwd_g] == 'O') {
						current_map[pwd_s + 2][pwd_g] = '$';
						if (map[current_stage][pwd_s][pwd_g] == '@' || map[current_stage][pwd_s][pwd_g] == '$') {
							current_map[pwd_s][pwd_g] = '.';
						}
						else {
							current_map[pwd_s][pwd_g] = map[current_stage][pwd_s][pwd_g];
						}
					}
				}

				current_map[pwd_s + 1][pwd_g] = '@';
				pwd_s += 1;
				count++;
				break;
			}

			case 72: { // k
				if (current_map[pwd_s - 1][pwd_g] == '.' || current_map[pwd_s - 1][pwd_g] == 'O') {
					if (map[current_stage][pwd_s][pwd_g] == '@' || map[current_stage][pwd_s][pwd_g] == '$') {
						current_map[pwd_s][pwd_g] = '.';
					}
					else {
						current_map[pwd_s][pwd_g] = map[current_stage][pwd_s][pwd_g];
					}
				}
				else if (current_map[pwd_s - 1][pwd_g] == '#') continue;
				else if (current_map[pwd_s - 1][pwd_g] == '$') {
					if (current_map[pwd_s - 2][pwd_g] == '$' || current_map[pwd_s - 2][pwd_g] == '#') continue;
					if (current_map[pwd_s - 2][pwd_g] == '.') {
						current_map[pwd_s - 2][pwd_g] = '$';
						if (map[current_stage][pwd_s][pwd_g] == '@' || map[current_stage][pwd_s][pwd_g] == '$') {
							current_map[pwd_s][pwd_g] = '.';
						}
						else {
							current_map[pwd_s][pwd_g] = map[current_stage][pwd_s][pwd_g];
						}
					}
					else if (current_map[pwd_s - 2][pwd_g] == 'O') {
						current_map[pwd_s - 2][pwd_g] = '$';
						if (map[current_stage][pwd_s][pwd_g] == '@' || map[current_stage][pwd_s][pwd_g] == '$') {
							current_map[pwd_s][pwd_g] = '.';
						}
						else {
							current_map[pwd_s][pwd_g] = map[current_stage][pwd_s][pwd_g];
						}
					}
				}

				current_map[pwd_s - 1][pwd_g] = '@';
				pwd_s -= 1;
				count++;
				break;
			}

			case 77: { // l
				if (current_map[pwd_s][pwd_g + 1] == '.' || current_map[pwd_s][pwd_g + 1] == 'O') {
					if (map[current_stage][pwd_s][pwd_g] == '@' || map[current_stage][pwd_s][pwd_g] == '$') {
						current_map[pwd_s][pwd_g] = '.';
					}
					else {
						current_map[pwd_s][pwd_g] = map[current_stage][pwd_s][pwd_g];
					}
				}
				else if (current_map[pwd_s][pwd_g + 1] == '#') continue;
				else if (current_map[pwd_s][pwd_g + 1] == '$') {
					if (current_map[pwd_s][pwd_g + 2] == '$' || current_map[pwd_s][pwd_g + 2] == '#') continue;
					if (current_map[pwd_s][pwd_g + 2] == '.') {
						current_map[pwd_s][pwd_g + 2] = '$';
						if (map[current_stage][pwd_s][pwd_g] == '@' || map[current_stage][pwd_s][pwd_g] == '$') {
							current_map[pwd_s][pwd_g] = '.';
						}
						else {
							current_map[pwd_s][pwd_g] = map[current_stage][pwd_s][pwd_g];
						}
					}
					else if (current_map[pwd_s][pwd_g + 2] == 'O') {
						current_map[pwd_s][pwd_g + 2] = '$';
						if (map[current_stage][pwd_s][pwd_g] == '@' || map[current_stage][pwd_s][pwd_g] == '$') {
							current_map[pwd_s][pwd_g] = '.';
						}
						else {
							current_map[pwd_s][pwd_g] = map[current_stage][pwd_s][pwd_g];
						}
					}
				}

				current_map[pwd_s][pwd_g + 1] = '@';
				pwd_g += 1;
				count++;
				break;
			}


			}
			for (int s = 0; s < length_sero[1]; s++) {

				for (int g = 0; g < length_garo[1]; g++)
				{
					if (current_map[s][g] == '0')
					{
						end_count++;
					}



				}
			}





			if (end_count = 0 && map[1][pwd_s][pwd_g] != '0')
				break;


		}
	}
	printf("���������� ��� �Ϸ��ϼ̳׿� �����մϴ�. \n");
}