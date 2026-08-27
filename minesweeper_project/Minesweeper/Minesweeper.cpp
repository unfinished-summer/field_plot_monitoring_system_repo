#include <graphics.h>
#include <time.h>
#include <stdio.h>

const int ROW = 10;
const int COL = 10;
const int MINE_COUNT = 12;
const int CELL_SIZE = 50;
HWND hwnd;
int openCellCnt;
IMAGE img[12];

void Game();
void DrawMap(int Map[ROW + 2][COL + 2], IMAGE* img);
void OpenBlank(int Map[ROW + 2][COL + 2], int x, int y);
int Play(int Map[ROW + 2][COL + 2]);

int main() {
	while (1) {
		Game();
		int ret = MessageBox(hwnd, L"Play again?", L"Quit", MB_RETRYCANCEL);
		if (ret == IDCANCEL) {
			break;
		}
	}
	return 0;
}

void Game() {
	initgraph(COL * CELL_SIZE, ROW * CELL_SIZE);
	hwnd = GetHWnd();
	openCellCnt = 0;

	int Map[ROW + 2][COL + 2] = { 0 };
	srand((unsigned)time(NULL));

	for (int mineNum = 0;mineNum < MINE_COUNT;) {
		int x = rand() % ROW + 1;
		int y = rand() % COL + 1;
		if (Map[x][y] == 0) {
			Map[x][y] = -1;
			mineNum++;
		}
	}

	for (int i = 1;i <= ROW;i++) {
		for (int j = 1;j <= COL;j++) {
			if (Map[i][j] != -1) {
				for (int m = i - 1;m <= i + 1;m++) {
					for (int n = j - 1;n <= j + 1;n++) {
						if (Map[m][n] == -1) {
							Map[i][j]++;
						}
					}
				}
			}
		}
	}
	

	loadimage(&img[0], L"res/0.png", CELL_SIZE, CELL_SIZE);
	loadimage(&img[1], L"res/1.png", CELL_SIZE, CELL_SIZE);
	loadimage(&img[2], L"res/2.png", CELL_SIZE, CELL_SIZE);
	loadimage(&img[3], L"res/3.png", CELL_SIZE, CELL_SIZE);
	loadimage(&img[4], L"res/4.png", CELL_SIZE, CELL_SIZE);
	loadimage(&img[5], L"res/5.png", CELL_SIZE, CELL_SIZE);
	loadimage(&img[6], L"res/6.png", CELL_SIZE, CELL_SIZE);
	loadimage(&img[7], L"res/7.png", CELL_SIZE, CELL_SIZE);
	loadimage(&img[8], L"res/8.png", CELL_SIZE, CELL_SIZE);
	loadimage(&img[9], L"res/Mine.png", CELL_SIZE, CELL_SIZE);
	loadimage(&img[10], L"res/block.png", CELL_SIZE, CELL_SIZE);
	loadimage(&img[11], L"res/flag.png", CELL_SIZE, CELL_SIZE);
	


	int gameOverFlag = 0; // 0正常运行 1踩雷 2胜利
	while (1) {
		// 双缓冲绘图，消除画面基础闪烁
		BeginBatchDraw();
		DrawMap(Map, img);
		EndBatchDraw();

		Sleep(10); // 限制刷新帧率，减少无效重绘
		int state = Play(Map);
		if (state == 1) {
			gameOverFlag = 1;
			break; // 踩雷，先退出刷新循环
		}
		if (openCellCnt == ROW * COL - MINE_COUNT) {
			gameOverFlag = 2;
			break; // 胜利，先退出刷新循环
		}
	}
	// 游戏循环停止，最后渲染一次最终画面再弹窗，弹窗时画面静止无刷新
	BeginBatchDraw();
	DrawMap(Map, img);
	EndBatchDraw();

	// 弹窗移到刷新循环外部，不会持续重绘窗口
	if (gameOverFlag == 1)
	{
		MessageBox(hwnd, L"You hit a mine, game over!", L"Game Over", MB_OK);
	}
	else if (gameOverFlag == 2)
	{
		MessageBox(hwnd, L"Congratulations, you win!", L"Well PLAYED", MB_OK);
	}
	closegraph();
}

void DrawMap(int Map[ROW + 2][COL + 2], IMAGE* img) {
	cleardevice();
	for (int i = 1;i <= ROW;i++) {
		for (int j = 1;j <= COL;j++) {
			int val = Map[i][j];
			// 第一优先级：游戏结束才显示的地雷(-10)
			if (val == -10) {
				putimage((j - 1) * CELL_SIZE, (i - 1) * CELL_SIZE, &img[9]);
				continue;
			}
			// 第二优先级：插旗
			if (val >= 19) {
				putimage((j - 1) * CELL_SIZE, (i - 1) * CELL_SIZE, &img[11]);
				continue;
			}
			// 正常游戏里原生地雷(-1)，隐藏成未开方块
			if (val == -1) {
				putimage((j - 1) * CELL_SIZE, (i - 1) * CELL_SIZE, &img[10]);
				continue;
			}
			// 普通未翻开格子(0~8)
			if (val >= 0 && val <= 8) {
				putimage((j - 1) * CELL_SIZE, (i - 1) * CELL_SIZE, &img[10]);
				continue;
			}
			// 已翻开
			switch (val) {
			case 10:putimage((j-1)*CELL_SIZE,(i-1)*CELL_SIZE, &img[0]);break;
			case 11:putimage((j-1)*CELL_SIZE,(i-1)*CELL_SIZE, &img[1]);break;
			case 12:putimage((j-1)*CELL_SIZE,(i-1)*CELL_SIZE, &img[2]);break;
			case 13:putimage((j-1)*CELL_SIZE,(i-1)*CELL_SIZE, &img[3]);break;
			case 14:putimage((j-1)*CELL_SIZE,(i-1)*CELL_SIZE, &img[4]);break;
			case 15:putimage((j-1)*CELL_SIZE,(i-1)*CELL_SIZE, &img[5]);break;
			case 16:putimage((j-1)*CELL_SIZE,(i-1)*CELL_SIZE, &img[6]);break;
			case 17:putimage((j-1)*CELL_SIZE,(i-1)*CELL_SIZE, &img[7]);break;
			case 18:putimage((j-1)*CELL_SIZE,(i-1)*CELL_SIZE, &img[8]);break;
			default:putimage((j - 1) * CELL_SIZE, (i - 1) * CELL_SIZE, &img[10]);
			}
		}
	}
	// ========== 控制台打印整张地图 ==========
	system("cls"); // 清空控制台，避免刷屏堆积
	printf("===== 扫雷地图实时数值(ROW:行,COL:列) =====\n");
	for (int i = 1; i <= ROW; i++)
	{
		for (int j = 1; j <= COL; j++)
		{
			printf("%3d ", Map[i][j]); // %3d 对齐排版
		}
		printf("\n");
	}
	printf("==========================================\n");
}
int Play(int Map[ROW + 2][COL + 2]) {
	MOUSEMSG msg;
	while (MouseHit()) { // 有鼠标消息才处理，不阻塞
		msg = GetMouseMsg();
		// 修复坐标换算：除以格子大小CELL_SIZE
		int col = msg.x / CELL_SIZE + 1;
		int row = msg.y / CELL_SIZE + 1;
		if (row < 1 || row > ROW || col < 1 || col > COL)
			continue;

		switch (msg.uMsg) {
		case WM_LBUTTONDOWN:
		{
			int v = Map[row][col];
			// 已翻开 / 插旗 不能点击
			if (v >= 19) break;  // 插旗格子，禁止左键点击
			if (v >= 10) break;  // 已翻开格子，禁止左键点击
			// 踩到地雷，返回失败1
			if (v == -1) {
				// 全部地雷暴露
				for (int i = 1; i <= ROW; i++)
					for (int j = 1; j <= COL; j++)
						if (Map[i][j] == -1 || Map[i][j] == 19) Map[i][j] = -10;
				return 1;
			}
			// 递归展开空白区域
			OpenBlank(Map, row, col);
			break;
		}
		case WM_RBUTTONDOWN: // 补全右键插旗逻辑
		{
			int v = Map[row][col];
			if (v >= 19)
				Map[row][col] -= 20; // 取消旗子
			else
				Map[row][col] += 20; // 插上旗子
			break;
			if (v >= 10) break; // 已翻开格子，禁止插旗
		}
		}
	}
	return 0;
}
void OpenBlank(int Map[ROW + 2][COL + 2], int x, int y) {
	if (x < 1 || x > ROW || y < 1 || y > COL) return;
	int val = Map[x][y];
	// 已点开 / 地雷 / 插旗 直接返回
	if (val >= 10 || val == -1 || val >= 20) return;

	// 标记为已点开 +10
	Map[x][y] += 10;
	openCellCnt++;

	// 当前是空白0，递归周围8格
	if (val == 0) {
		for (int di = -1; di <= 1; di++)
			for (int dj = -1; dj <= 1; dj++)
				OpenBlank(Map, x + di, y + dj);
	}
}