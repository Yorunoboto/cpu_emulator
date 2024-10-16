#include<stdio.h>
#include<iostream>
#include<time.h>
#include<conio.h>

int a_reg = 0;				// Aレジスタ
int b_reg = 0;				// Bレジスタ
int output_reg = 0;			// 出力レジスタ
int pc = 0;					// プログラムカウンタ
int outputport = 0;			// 出力ポート
char inputport[100];		// 入力ポート
int inputportdata = 0;		// 入力ポート計算用データ
int carryflag = 0;			// キャリーフラグ
int opcode = 0;				// オペコード
int imdata = 0;				// イミディエイトデータ
int cmd[16][2];				// 命令フォーマット
char rom_address[100];		// ROMアドレス
int select_reg = 0;			// セレクトレジスタ
int buf[2][4];				// 命令バッファ
char pcmoji[] = { "プログラムカウンタ" };
char outputmoji[] = { "出力ポート" };
char inputmoji[] = { "入力ポート" };
char getkey;


typedef enum {
	//手動クロックモード
	MANUAL_CLOCK,
	//自動クロックモード
	AUTO_CLOCK
}CLOCK_MODE_SELECT;

CLOCK_MODE_SELECT clockflag;			// クロックモード選択

//ROMセット関数
void romset() {
	int len = 0;
	int rom_inputflag = 0;
	int count = 0;
	//ROMの0番地から15番地まで順番に命令を入力する
	printf("  ______________________________________________________________________________________\n");
	printf(" |                                                                                      |\n");
	printf(" | 00010000: MOV A,B    (AレジスタにBレジスタの値を書き込む)                            |\n");
	printf(" | 01000000: MOV B,A    (BレジスタにAレジスタの値を書き込む)                            |\n");
	printf(" | 0011xxxx: MOV A,Im   (Aレジスタにイミディエイトデータの値を書き込む)                 |\n");
	printf(" | 0111xxxx: MOV B,Im   (Bレジスタにイミディエイトデータの値を書き込む)                 |\n");
	printf(" | 00100000: IN A       (Aレジスタに入力ポートの値を書き込む)                           |\n");
	printf(" | 01100000: IN B       (Bレジスタに入力ポートの値を書き込む)                           |\n");
	printf(" | 10010000: OUT B      (Bレジスタの値を出力する)                                       |\n");
	printf(" | 1011xxxx: OUT Im     (イミディエイトデータの値を出力する)                            |\n");
	printf(" | 0000xxxx: ADD A,Im   (Aレジスタにイミディエイトデータの値を加算する)                 |\n");
	printf(" | 0101xxxx: ADD B,Im   (Bレジスタにイミディエイトデータの値を加算する)                 |\n");
	printf(" | 1111xxxx: JMP Im　   (イミディエイトデータの値までジャンプする)                      |\n");
	printf(" | 1110xxxx: JNC Im　   (キャリーフラグが0ならイミディエイトデータの値までジャンプする) |\n");
	printf(" |______________________________________________________________________________________|\n\n\n");
	for (int i = 0; i < 16; i++) {
		printf("上記の命令表に従って、%d番地の命令を入力してください\n", i);
		while (0 == rom_inputflag) {
			fgets(rom_address, sizeof(rom_address), stdin);
			len = strlen(rom_address) - 1;
			opcode = (rom_address[0] - '0') << 3 | (rom_address[1] - '0') << 2 | (rom_address[2] - '0') << 1 | (rom_address[3] - '0');
			imdata = (rom_address[4] - '0') << 3 | (rom_address[5] - '0') << 2 | (rom_address[6] - '0') << 1 | (rom_address[7] - '0');
			for (int j = 0; j < 8; j++) {
				rom_address[j] -= '0';
			}
			if (8 == len) {
				for (int k = 0; k < 8; k++) {
					if (0 == rom_address[k] || 1 == rom_address[k]) {
						count++;
					}
				}
			}
			if (8 == count) {
				if (0 == opcode ||
					1 == opcode ||
					2 == opcode ||
					3 == opcode ||
					4 == opcode ||
					5 == opcode ||
					6 == opcode ||
					7 == opcode ||
					9 == opcode ||
					11 == opcode ||
					14 == opcode ||
					15 == opcode) {
					printf("%d番地に命令がセットされました\n\n", i);
					cmd[i][0] = opcode;
					cmd[i][1] = imdata;
					rom_inputflag = 1;
				}
				else {
					printf("該当する命令は存在しません\n");
					printf("再度、%d番地に命令をセットしてください\n", i);
				}
			}
			else {
				printf("該当する命令は存在しません\n");
				printf("再度、%d番地に命令をセットしてください\n", i);
			}
			count = 0;
		}
		rom_inputflag = 0;
	}
}

//リセット関数
void reset() {
	a_reg = 0;
	b_reg = 0;
	output_reg = 0;
	pc = 0;
	carryflag = 0;
	opcode = 0;
	imdata = 0;
	outputport = 0;
	select_reg = 0;
	printf(" ______________________________________________\n");
	printf("|                                              |\n");
	printf("| Aレジスタは%04dに初期化されました            |\n", a_reg);
	printf("| Bレジスタは%04dに初期化されました            |\n", b_reg);
	printf("| 出力レジスタは%04dに初期化されました         |\n", output_reg);
	printf("| プログラムカウンタは%04dに初期化されました   |\n", pc);
	printf("| キャリーフラグは%dに初期化されました          |\n", carryflag);
	printf("| オペコードは%04dに初期化されました           |\n", opcode);
	printf("| イミディエイトデータは%04dに初期化されました |\n", imdata);
	printf("|______________________________________________|\n\n\n");
}

//LEDパターン出力関数
void led_pattern(int data, char* name) {
	char moji[100];
	moji[0] = data / 8;
	moji[1] = (data % 8) / 4;
	moji[2] = ((data % 8) % 4) / 2;
	moji[3] = (((data % 8) % 4) % 2) / 1;
	printf("%s:", name);
	for (int i = 0; i < 4; i++) {
		if (1 == moji[i]) {
			printf("○");
		}
		else
		{
			printf("●");
		}
	}
}


///イミディエイトデータパターン出力関数
void Imdata_pattern(int data) {
	char moji[100];
	moji[3] = data / 8;
	moji[2] = (data % 8) / 4;
	moji[1] = ((data % 8) % 4) / 2;
	moji[0] = (((data % 8) % 4) % 2) / 1;
	for (int i = 3; i >= 0; i--) {
		if (1 == moji[i]) {
			printf("bit%d = ○\t\n", i);
		}
		else
		{
			printf("bit%d = ●\t\n", i);
		}
	}
}


//入力ポートセット関数
void inputset() {
	int inputflag = 0;
	int len = 0;
	int count = 0;
	//入力ポートに値をセットする
	while (0 == inputflag) {
		printf("入力ポートに値(0000～1111)をセットした後に、Enterキーを押してください。\n");
		fgets(inputport, sizeof(inputport), stdin);
		inputportdata = (inputport[0] - '0') << 3 | (inputport[1] - '0') << 2 | (inputport[2] - '0') << 1 | (inputport[3] - '0');
		len = strlen(inputport) - 1;
		for (int i = 0; i < 4; i++) {
			inputport[i] -= '0';
		}
		if (4 == len) {
			for (int j = 0; j < 4; j++) {
				if (0 == inputport[j] || 1 == inputport[j]) {
					count++;
				}
			}
			if (4 == count) {
				led_pattern(inputportdata, inputmoji);
				inputflag = 1;
			}
			else {
				printf("異常値がセットされました。再度、入力ポートに数字をセットしてください\n");
			}
		}
		else {
			printf("異常値がセットされました。再度、入力ポートに数字をセットしてください\n");
		}
		count = 0;
	}
}


//クロックモード選択関数
void clockmode() {
	int flag = 0;
	printf("クロックモードを下記選択肢より選択してください\n");
	while (0 == flag) {
		printf("%d：自動クロックモード or %d：手動クロックモード\n", AUTO_CLOCK, MANUAL_CLOCK);
		scanf_s("%d", &clockflag);

		if (AUTO_CLOCK == clockflag) {
			printf("自動クロックモードを実行します\n");
			flag = 1;
		}
		else if (MANUAL_CLOCK == clockflag) {
			printf("手動クロックモードを実行します\n");
			flag = 1;
		}
		else {
			printf("異常値がセットされました\n");
			printf("再度、クロックモードを下記選択肢より選択してください\n");
		}
	}
}

//データセレクト関数
void dataselect(int i) {
	select_reg = i;
}

//ALU関数
int alu(int i) {
	int alu_calc_data = 0;		// ALU演算用データ
	int alu_calc_result = 0;	// ALU演算結果
	alu_calc_data = select_reg;
	alu_calc_result = alu_calc_data + i;
	return alu_calc_result;
}

//MOV A, Im関数
void MOV_A_Im() {
	printf("select[0] = ●\t\nselect[1] = ○\t\nload[0] = ●\t\nload[1] = ○\t\nload[2] = ○\t\nload[3] = ○\n");
	printf("bit7 = ●\t\nbit6 = ●\t\nbit5 = ○\t\nbit4 = ○\n");
	Imdata_pattern(imdata);
	dataselect(0);
	a_reg = alu(imdata);
}

//MOV B, Im関数
void MOV_B_Im() {
	printf("select[0] = ○\t\nselect[1] = ○\t\nload[0] = ○\t\nload[1] = ●\t\nload[2] = ○\t\nload[3] = ○\n");
	printf("bit7 = ●\t\nbit6 = ○\t\nbit5 = ○\t\nbit4 = ○\n");
	Imdata_pattern(imdata);
	dataselect(0);
	b_reg = alu(imdata);
}

//MOV A, B関数
void MOV_A_B() {
	printf("select[0] = ○\t\nselect[1] = ●\t\nload[0] = ●\t\nload[1] = ○\t\nload[2] = ○\t\nload[3] = ○\n");
	printf("bit7 = ●\t\nbit6 = ●\t\nbit5 = ●\t\nbit4 = ○\t\nbit3 = ●\t\nbit2 = ●\t\nbit1 = ●\t\nbit0 = ●\n");
	dataselect(0);
	a_reg = alu(b_reg);
}

//MOV B, A関数
void MOV_B_A() {
	printf("select[0] = ●\t\nselect[1] = ●\t\nload[0] = ○\t\nload[1] = ●\t\nload[2] = ○\t\nload[3] = ○\n");
	printf("bit7 = ●\t\nbit6 = ○\t\nbit5 = ●\t\nbit4 = ●\t\nbit3 = ●\t\nbit2 = ●\t\nbit1 = ●\t\nbit0 = ●\n");
	dataselect(0);
	b_reg = alu(a_reg);
}

//ADD A, Im関数
void ADD_A_Im() {
	printf("select[0] = ●\t\nselect[1] = ●\t\nload[0] = ●\t\nload[1] = ○\t\nload[2] = ○\t\nload[3] = ○\n");
	printf("bit7 = ●\t\nbit6 = ●\t\nbit5 = ●\t\nbit4 = ●\n");
	Imdata_pattern(imdata);
	dataselect(a_reg);
	int buf = alu(imdata);
	if (16 <= buf) {
		a_reg = buf - 15;
		carryflag = 1;
	}
	else {
		a_reg = buf;
	}
}

//ADD B, Im関数
void ADD_B_Im() {
	printf("select[0] = ○\t\nselect[1] = ●\t\nload[0] = ○\t\nload[1] = ●\t\nload[2] = ○\t\nload[3] = ○\n");
	printf("bit7 = ●\t\nbit6 = ○\t\nbit5 = ●\t\nbit4 = ○\n");
	Imdata_pattern(imdata);
	dataselect(b_reg);
	int buf = alu(imdata);
	if (16 <= buf) {
		b_reg = buf - 15;
		carryflag = 1;
	}
	else {
		b_reg = buf;
	}
}

//IN A関数
void IN_A() {
	printf("select[0] = ●\t\nselect[1] = ○\t\nload[0] = ●\t\nload[1] = ○\t\nload[2] = ○\t\nload[3] = ○\n");
	printf("bit7 = ●\t\nbit6 = ●\t\nbit5 = ○\t\nbit4 = ●\t\nbit3 = ●\t\nbit2 = ●\t\nbit1 = ●\t\nbit0 = ●\n");
	dataselect(inputportdata);
	a_reg = alu(0);
}

//IN B関数
void IN_B() {
	printf("select[0] = ●\t\nselect[1] = ○\t\nload[0] = ○\t\nload[1] = ●\t\nload[2] = ○\t\nload[3] = ○\n");
	printf("bit7 = ●\t\nbit6 = ○\t\nbit5 =○\t\nbit4 = ●\t\nbit3 = ●\t\nbit2 = ●\t\nbit1 = ●\t\nbit0 = ●\n");
	dataselect(inputportdata);
	b_reg = alu(0);
}

//OUT Im関数
void OUT_Im() {
	printf("select[0] = ○\t\nselect[1] = ○\t\nload[0] = ○\t\nload[1] = ○\t\nload[2] = ●\t\nload[3] = ○\n");
	printf("bit7 = ○\t\nbit6 = ●\t\nbit5 = ○\t\nbit4 = ○\n");
	Imdata_pattern(imdata);
	dataselect(0);
	output_reg = alu(imdata);
	//LEDを点灯させる
	outputport = output_reg;
	led_pattern(outputport, outputmoji);
}

//OUT B関数
void OUT_B() {
	printf("select[0] = ○\t\nselect[1] = ●\t\nload[0] = ○\t\nload[1] = ○\t\nload[2] = ●\t\nload[3] = ○\n");
	printf("bit7 = ○\t\nbit6 = ●\t\nbit5 = ●\t\nbit4 = ○\t\nbit3 = ●\t\nbit2 = ●\t\nbit1 = ●\t\nbit0 = ●\n");
	dataselect(0);
	output_reg = alu(b_reg);
	//LEDを点灯させる
	outputport = output_reg;
	led_pattern(outputport, outputmoji);
}

//JMP Im関数
void JMP_Im() {
	printf("select[0] = ○○\t\nselect[1] = ○○\t\nload[0] = ○\t\nload[1] = ○\t\nload[2] = ○\t\nload[3] = ●\n");
	printf("bit7 = ○\t\nbit6 = ○\t\nbit5 = ○\t\nbit4 = ○\n");
	Imdata_pattern(imdata);
	dataselect(0);
	pc = alu(imdata);
	printf("%d番地にジャンプします\n", pc);
	pc--;
}

//JNC Im関数
void JNC_Im() {
	if (0 == carryflag) {
		printf("select[0] = ○○\t\nselect[1] = ○●\t\nload[0] = ○\t\nload[1] = ○\t\nload[2] = ○\t\nload[3] = ●\n");
		printf("bit7 = ○\t\nbit6 = ○\t\nbit5 = ○\t\nbit4 = ●\n");
		Imdata_pattern(imdata);
		dataselect(0);
		pc = alu(imdata);
		printf("%d番地にジャンプします\n", pc);
		pc--;
	}
	else {
		printf("select[0] = ○●\t\nselect[1] = ○●\t\nload[0] = ○\t\nload[1] = ○\t\nload[2] = ○\t\nload[3] = ○\n");
		carryflag = 0;
	}
}

//デコード関数
void decode() {
	//オペコード&イミディエイトデータ取得
	opcode = cmd[pc][0];
	imdata = cmd[pc][1];
	//オペコードから命令を読み取る
	switch (opcode) {
	case 3:
		printf("%d番地の命令として「MOV A, Im」を実行します\n", pc);
		MOV_A_Im();
		break;
	case 7:
		printf("%d番地の命令として「MOV B, Im」を実行します\n", pc);
		MOV_B_Im();
		break;
	case 1:
		printf("%d番地の命令として「MOV A, B」を実行します\n", pc);
		MOV_A_B();
		break;
	case 4:
		printf("%d番地の命令として「MOV B, A」を実行します\n", pc);
		MOV_B_A();
		break;
	case 0:
		printf("%d番地の命令として「ADD A, Im」を実行します\n", pc);
		ADD_A_Im();
		break;
	case 5:
		printf("%d番地の命令として「ADD B, Im」を実行します\n", pc);
		ADD_B_Im();
		break;
	case 2:
		printf("%d番地の命令として「IN A」を実行します\n", pc);
		IN_A();
		break;
	case 6:
		printf("%d番地の命令として「IN B」を実行します\n", pc);
		IN_B();
		break;
	case 11:
		printf("%d番地の命令として「OUT Im」を実行します\n", pc);
		OUT_Im();
		break;
	case 9:
		printf("%d番地の命令として「OUT B」を実行します\n", pc);
		OUT_B();
		break;
	case 15:
		printf("%d番地の命令として「JMP Im」を実行します\n", pc);
		JMP_Im();
		break;
	case 14:
		printf("%d番地の命令として「JNC Im」を実行します\n", pc);
		JNC_Im();
		break;
	default:
		break;
	}
}

//1秒待つ関数
void delay(int milliseconds) {
	clock_t start_time = clock();
	while (clock() < start_time + milliseconds);
}

int main() {
	printf("|||||||||||||||||||||       ||||||||||||||||||                     |||||||||||\n");
	printf("|||||||||||||||||||||       |||||||||||||||||||                   ||||||||||||\n");
	printf("|||||||||||||||||||||       |||||         ||||||                 |||||   |||||\n");
	printf("       |||||||              |||||          ||||||               |||||    |||||\n");
	printf("       |||||||              |||||           ||||||             |||||     |||||\n");
	printf("       |||||||              |||||            ||||||           |||||      |||||\n");
	printf("       |||||||              |||||            ||||||          |||||       |||||\n");
	printf("       |||||||              |||||            ||||||         |||||        |||||\n");
	printf("       |||||||              |||||            ||||||        |||||         |||||\n");
	printf("       |||||||              |||||           ||||||        |||||||||||||||||||||||||\n");
	printf("       |||||||              |||||          ||||||        ||||||||||||||||||||||||||\n");
	printf("       |||||||              |||||         ||||||                         |||||\n");
	printf("       |||||||              |||||||||||||||||||                          |||||\n");
	printf("       |||||||              ||||||||||||||||||                           |||||\n\n\n\n\n\n\n");
	reset();
	romset();
	inputset();
	clockmode();

	//手動クロックモード
	if (MANUAL_CLOCK == clockflag) {
		while (1) {
			for (pc = 0; pc < 16; pc++) {
				getkey = getchar();
				switch (getkey) {
				case '\n':
					printf("クロック：○\n");
					led_pattern(pc, pcmoji);
					decode();
					break;
				case 'r':
					reset();
					pc--;
					break;
				default:
					break;
				}
			}
		}
	}
	//自動クロックモード
	else if (AUTO_CLOCK == clockflag) {
		while (true) {
			for (pc = 0; pc < 16; pc++) {
				printf("クロック：○\n");
				led_pattern(pc, pcmoji);
				decode();
				delay(1000);
				if (_kbhit()) {
					getkey = _getch();
					if ('r' == getkey) {
						reset();
						break;
					}
				}
			}
		}
	}
}
