/*
 *        Computer Graphics Course - Shenzhen University
 *    Mid-term Assignment - Tetris implementation sample code
 * ============================================================
 *
 *  2020年11月4日更新功能(README)
 *   * 基本功能：
 *		- 绘制棋盘格
 *		- 随机生成随机颜色的方块
 *		- 左右下键控制方块移动，上键旋转方块，空格键直接到底
 *		- 方块自动下滑
 *		- 方块之间、方块与边界之间的碰撞检测
 *		- 满行自动消除
 *	 * 附加功能：
 *		- 消去一行时的success音效，游戏结束时的fail音效
 *		- 实时统计分数面板
 *		- 鼠标右键控制游戏的开始、暂停、结束和难度
 *
 */

#include "Angel.h"

 // #pragma comment(lib, "glew32.lib")

#include <cstdlib>
#include <iostream>
#include <string>
#include <random>
#include <time.h>
#include <stdio.h>
#include <fstream>
#include <Windows.h>
#include <string.h>
#include <mmsystem.h>
using namespace std;

#pragma comment(lib,"winmm.lib")

const int TIMER_STATE_RUN = 1;    // 游戏开始状态常量(与后面点击设置游戏暂停继续功能有关)
const int TIMER_STATE_PAUSE = 2;  // 游戏暂停状态常量

int starttime;			// 控制方块向下移动时间
int rotation = 0;		// 控制当前窗口中的方块旋转
vec2 tile[4];			// 表示当前窗口中的方块
bool gameover = false;	// 游戏结束控制变量
int xsize = 400;		// 窗口大小（尽量不要变动窗口大小！）
int ysize = 720;
int random = 0;
int mainMenu;			// 主菜单
int subMenu;			// 子菜单
int timerState = TIMER_STATE_RUN;  // 游戏状态变量(开始 or 暂停)
int totalScore = 0;		// 游戏当前总得分
int mainWindow;			// 主窗口
int timeGap = 500;		// 自动下落的间隔时间
int deltaDecrease = 50;	// 加大下落的时间变化量
int deltaDecreasePoint = 50;	// 每隔多少个得分点加速下落

// 一个二维数组表示所有可能出现的方块和方向。
vec2 allRotationsLshape[19][4] =
{ {vec2(0, 0), vec2(-1,0), vec2(1, 0), vec2(-1,-1)},  // 下面4个为L型
 {vec2(0, 1), vec2(0, 0), vec2(0,-1), vec2(1, -1)},
 {vec2(1, 1), vec2(-1,0), vec2(0, 0), vec2(1,  0)},
 {vec2(-1,1), vec2(0, 1), vec2(0, 0), vec2(0, -1)},
 {vec2(-1,0), vec2(0, 0), vec2(1, 0), vec2(1, -1)},  // 下面4个为J型
 {vec2(0,-1), vec2(0, 0), vec2(0, 1), vec2(1, 1)},
 {vec2(-1,1), vec2(-1, 0), vec2(0, 0), vec2(1, 0)},
 {vec2(-1,-1), vec2(0, -1), vec2(0, 0), vec2(0, 1)},
 {vec2(-1,0), vec2(0, 0), vec2(1, 0), vec2(0, -1)},  // 下面4个为T型
 {vec2(0,1), vec2(0, 0), vec2(0, -1), vec2(1, 0)},
 {vec2(-1,0), vec2(0, 0), vec2(1, 0), vec2(0, 1)},
 {vec2(-1,0), vec2(0, -1), vec2(0, 0), vec2(0, 1)},
 {vec2(-1,0), vec2(0, 0), vec2(0, -1), vec2(1, -1)},  // 下面2个为Z型
 {vec2(0,-1), vec2(0, 0), vec2(1, 0), vec2(1, 1)},
 {vec2(-1,-1), vec2(0, -1), vec2(0, 0), vec2(1, 0)},  // 下面2个为S型
 {vec2(1,-1), vec2(1, 0), vec2(0, 0), vec2(0, 1)},
 {vec2(-2,0), vec2(-1, 0), vec2(0, 0), vec2(1, 0)},  // 下面2个为I型
 {vec2(0,-2), vec2(0, -1), vec2(0, 0), vec2(0, 1)},
 {vec2(-1,-1), vec2(0, -1), vec2(-1, 0), vec2(0, 0)},  // 这1个为O型
};

// 绘制窗口的颜色变量
const vec4 orange(1.0, 0.5, 0.0, 1.0);
const vec4 white(1.0, 1.0, 1.0, 1.0);
const vec4 black(0.0, 0.0, 0.0, 1.0);


const int MENU_CHOICE_PAUSE = 0;
const int MENU_CHOICE_CONTINUE = 1;
const int MENU_CHOICE_EXIT = 2;
const int MENU_CHOICE_EASY = 3;
const int MENU_CHOICE_SOSO = 4;
const int MENU_CHOICE_HARD = 5;


vec4 COLOR[6] = {
	vec4(1.0, 0.5, 0.0, 1.0),  // orange
	vec4(1.0, 0.0, 0.0, 1.0),  // red
	vec4(0.0, 1.0, 0.0, 1.0),  // green
	vec4(0.0, 0.0, 1.0, 1.0),  // blue
	vec4(1.0, 1.0, 0.0, 1.0),  // yellow
	vec4(0.8, 0.0, 0.8, 1.0)   // purple
};

// 当前方块的位置（以棋盘格的左下角为原点的坐标系）
vec2 tilepos = vec2(5, 19);

// 布尔数组表示棋盘格的某位置是否被方块填充，即board[x][y] = true表示(x,y)处格子被填充。
// （以棋盘格的左下角为原点的坐标系
bool board[10][20];

// 当棋盘格某些位置被方块填充之后，记录这些位置上被填充的颜色
vec4 boardcolours[1200];

GLuint locxsize;
GLuint locysize;

GLuint vaoIDs[3];
GLuint vboIDs[6];

// 游戏调试时用，可注释
void debug() {
	/*for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < 20; ++j) {
			cout << board[i][j] << " ";
		}
		cout << endl;
	}
	cout << "=================================================" << endl;*/
	cout << " *  2020年11月4日更新功能(README)\n\
基本功能：\n\
  -绘制棋盘格 \n\
  -随机生成随机颜色的方块 \n\
  -左右下键控制方块移动，上键旋转方块，空格键直接到底\n\
  -方块自动下滑\n\
  -方块之间、方块与边界之间的碰撞检测\n\
  -满行自动消除\n\
附加功能：\n\
  -消去一行时的success音效，游戏结束时的fail音效\n\
  -实时统计分数面板\n\
  -鼠标右键控制游戏的开始、暂停、结束和难度\n" << endl;
}


// 等概率随机生成俄罗斯方块
int _rand() {  
	int r = rand() % 7;  // 先挑选7种类型里面的任意一种
	if (r == 0) {  // L型
		r = 0 + rand() % 4;  // 生成方块的形式：基址 + rand() % 内部类型总数
	}
	else if (r == 1) {  // J型
		r = 4 + rand() % 4;
	}
	else if (r == 2) {  // T型
		r = 8 + rand() % 4;
	}
	else if (r == 3) {  // Z型
		r = 12 + rand() % 2;
	}
	else if (r == 4) {  // S型
		r = 14 + rand() % 2;
	}
	else if (r == 5) {  // I型
		r = 16 + rand() % 2;
	}
	else if (r == 6) {  // O型
		r = 18;  // 只有一种内部类型
	}
	return r;
}

// 记录(含更新，如果能的话)本轮游戏的得分，保存到文件
void recordScore() {
	fstream file;
	char fileName[100] = "recordMax.txt";
	file.open(fileName, ios::in);
	if (!file) {  // 如果文件不存在，则先创建
		printf("正在创建文件...\n");
		ofstream out(fileName);
		out << 0;
		out.close();
		printf("文件创建成功! \n");
	}
	try {
		ifstream in(fileName);
		int historyMax;
		in >> historyMax;
		if (totalScore > historyMax) {
			// 更新历史最高分
			ofstream out(fileName);
			out << totalScore;
			out.close();
			printf("记录被刷新了! \n");
		}
		else {
			printf("没有刷新纪录! \n");
		}
		in.close();
	}
	catch (const std::exception&)
	{
		printf("An error occurred when reading & writing files!");
	}
}

// 检查在cellpos位置的格子是否被填充或者是否在棋盘格的边界范围内
bool checkvalid(vec2 cellpos)
{
	int xx = cellpos.x;
	int yy = cellpos.y;
	if ((cellpos.x >= 0) && (cellpos.x < 10) && (cellpos.y >= 0)
		&& (cellpos.y < 20) && (board[xx][yy] == false)) {
		// 要在棋盘范围内且没有被别的方块填充
		return true;
	}
	return false;
}

// 修改棋盘格在pos位置的颜色为colour，并且更新对应的VBO
void changecellcolour(vec2 pos, vec4 colour)
{
	// 每个格子是个正方形，包含两个三角形，总共6个定点，并在特定的位置赋上适当的颜色
	for (int i = 0; i < 6; i++) {
		boardcolours[(int)(6 * (10 * pos.y + pos.x) + i)] = colour;
	}

	vec4 newcolours[6] = { colour, colour, colour, colour, colour, colour };

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);

	// 计算偏移量，在适当的位置赋上颜色
	int offset = 6 * sizeof(vec4) * (int)(10 * pos.y + pos.x);
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(newcolours), newcolours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// 当前方块移动或者旋转时，更新VBO
void updatetile()
{
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);

	// 每个方块包含四个格子
	for (int i = 0; i < 4; i++)
	{
		// 计算格子的坐标值
		GLfloat x = tilepos.x + tile[i].x;
		GLfloat y = tilepos.y + tile[i].y;

		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);

		// 每个格子包含两个三角形，所以有6个顶点坐标
		vec4 newpoints[6] = { p1, p2, p3, p2, p3, p4 };
		glBufferSubData(GL_ARRAY_BUFFER, i * 6 * sizeof(vec4), 6 * sizeof(vec4), newpoints);
	}
	#ifdef __APPLE__
		glBindVertexArrayAPPLE(0);
	#else
		glBindVertexArray(0);
	#endif

	// 颜色随着格子移动而改变
	// 上次移动的格子置为黑色
	// 新的格子被方块颜色填充
	vec4 newcolours[24];
	int u = 0;
	for (int i = 0; i < 4; ++i) {
		int t = 6;
		while (t--) {
			if (tilepos.y + tile[i].y >= 20) {
				newcolours[u++] = black;
			}
			else {
				newcolours[u++] = COLOR[random];
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	#ifdef __APPLE__
		glBindVertexArrayAPPLE(0);
	#else
		glBindVertexArray(0);
	#endif
}

// 设置当前方块为下一个即将出现的方块。在游戏开始的时候调用来创建一个初始的方块，
// 在游戏结束的时候判断，没有足够的空间来生成新的方块。
void newtile()
{
	// 将新方块放于棋盘格的最上行中间位置并设置默认的旋转方向
	tilepos = vec2(5, 19);

	// 初始化
	random = _rand(); // 从19种形状中等概率随机选择一种
	rotation = random;  // rotation作为记录旋转类型的变量，后面会用到

	// tile记录当前的方块类型信息
	for (int i = 0; i < 4; i++) {
		tile[i] = allRotationsLshape[random][i];
	}

	// 结束判断条件
	for (int i = 0; i < 4; ++i) {
		int xx = tile[i].x + tilepos.x;
		int yy = tile[i].y + tilepos.y;
		/*cout << "X: " << xx << endl;
		cout << "Y: " << yy << endl;*/
		if (yy < 20 && board[xx][yy] == true) {
			// 如果刚产生时的位置就与其他方块有“交集”，则进入游戏退出逻辑
			// debug();
			recordScore();  // 最高分登记
			PlaySound(TEXT("fail_bgm.wav"), NULL, SND_FILENAME | SND_ASYNC);  // fail音效
			glutMainLoop();
			exit(EXIT_SUCCESS);
		}
	}

	updatetile();

	// 给新方块赋上颜色
	vec4 newcolours[24];
	random = rand() % 6;  // 随机选择一种颜色
	//for (int i = 0; i < 24; i++) {
	//	newcolours[i] = COLOR[random];  // 对24个三角片元顶点赋颜色
	//}
	int u = 0;
	for (int i = 0; i < 4; ++i) {
		int t = 6;
		while (t--) {
			if (tilepos.y + tile[i].y >= 20) {
				newcolours[u++] = white;
			}
			else {
				newcolours[u++] = COLOR[random];
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	#ifdef __APPLE__
		glBindVertexArrayAPPLE(0);
	#else
		glBindVertexArray(0);
	#endif
}

// 游戏和OpenGL初始化
void init()
{
	// 初始化棋盘格，包含64个顶点坐标（总共32条线），并且每个顶点一个颜色值
	vec4 gridpoints[64];
	vec4 gridcolours[64];

	// 纵向线
	for (int i = 0; i < 11; i++)
	{
		gridpoints[2 * i] = vec4((33.0 + (33.0 * i)), 33.0, 0, 1);
		gridpoints[2 * i + 1] = vec4((33.0 + (33.0 * i)), 693.0, 0, 1);
	}

	// 水平线
	for (int i = 0; i < 21; i++)
	{
		gridpoints[22 + 2 * i] = vec4(33.0, (33.0 + (33.0 * i)), 0, 1);
		gridpoints[22 + 2 * i + 1] = vec4(363.0, (33.0 + (33.0 * i)), 0, 1);
	}

	// 将所有线赋成白色
	for (int i = 0; i < 64; i++)
		gridcolours[i] = white;

	// 初始化棋盘格，并将没有被填充的格子设置成黑色
	vec4 boardpoints[1200];
	for (int i = 0; i < 1200; i++)
		boardcolours[i] = black;

	// 对每个格子，初始化6个顶点，表示两个三角形，绘制一个正方形格子
	for (int i = 0; i < 20; i++)
		for (int j = 0; j < 10; j++)
		{
			vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			boardpoints[6 * (10 * i + j)] = p1;
			boardpoints[6 * (10 * i + j) + 1] = p2;
			boardpoints[6 * (10 * i + j) + 2] = p3;
			boardpoints[6 * (10 * i + j) + 3] = p2;
			boardpoints[6 * (10 * i + j) + 4] = p3;
			boardpoints[6 * (10 * i + j) + 5] = p4;
		}

	// 将棋盘格所有位置的填充与否都设置为false（没有被填充）
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++)
			board[i][j] = false;

	debug();  // 输出一点提示信息

	// 载入着色器
	std::string vshader, fshader;
	#ifdef __APPLE__	// for MacOS
		vshader = "shaders/vshader_mac.glsl";
		fshader = "shaders/fshader_mac.glsl";
	#else				// for Windows
		vshader = "shaders/vshader_win.glsl";
		fshader = "shaders/fshader_win.glsl";
	#endif
	GLuint program = InitShader(vshader.c_str(), fshader.c_str());
	glUseProgram(program);

	locxsize = glGetUniformLocation(program, "xsize");
	locysize = glGetUniformLocation(program, "ysize");

	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	GLuint vColor = glGetAttribLocation(program, "vColor");

	#ifdef __APPLE__
		glGenVertexArraysAPPLE(3, &vaoIDs[0]);
		glBindVertexArrayAPPLE(vaoIDs[0]);	// 棋盘格顶点
	#else
		glGenVertexArrays(3, &vaoIDs[0]);
		glBindVertexArray(vaoIDs[0]);		// 棋盘格顶点
	#endif
	glGenBuffers(2, vboIDs);

	// 棋盘格顶点位置
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);
	glBufferData(GL_ARRAY_BUFFER, 64 * sizeof(vec4), gridpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// 棋盘格顶点颜色
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]);
	glBufferData(GL_ARRAY_BUFFER, 64 * sizeof(vec4), gridcolours, GL_STATIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	#ifdef __APPLE__
		glBindVertexArrayAPPLE(vaoIDs[1]);	// 棋盘格每个格子
	#else
		glBindVertexArray(vaoIDs[1]);		// 棋盘格每个格子
	#endif
	glGenBuffers(2, &vboIDs[2]);

	// 棋盘格每个格子顶点位置
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 1200 * sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// 棋盘格每个格子顶点颜色
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200 * sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	#ifdef __APPLE__
		glBindVertexArrayAPPLE(vaoIDs[2]);	// 当前方块
	#else
		glBindVertexArray(vaoIDs[2]);		// 当前方块
	#endif
	glGenBuffers(2, &vboIDs[4]);

	// 当前方块顶点位置
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// 当前方块顶点颜色
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	#ifdef __APPLE__
		glBindVertexArrayAPPLE(0);
	#else
		glBindVertexArray(0);
	#endif
	// 设置外围背景颜色为白色
	glClearColor(1.0, 1.0, 1.0, 0);
	// 以下四行参考百度文库
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, 500.0, 0.0, 500.0);
	glMatrixMode(GL_MODELVIEW);
	// 游戏初始化
	newtile();
}

void drawtext()
{
	float x = 50;  // 左边距为50
	float y = 0;   // 竖直方向不偏置
	char tmp_str[16] = "TotalScore: ";
	int tmp_score = totalScore;
	tmp_str[15] = '\0';
	int uu = 14;
	while (tmp_score) {
		tmp_str[uu--] = tmp_score % 10 + '0';
		tmp_score /= 10;
	}  // 处理得分，拷贝进字符串
	glRasterPos2f(x, y); // 设置文字在viewport起始位置
	// 显示文字
	for (int i = 0; i < 15; ++i) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, tmp_str[i]);
	}
}

// 在棋盘上有足够空间的情况下旋转当前方块
void rotate()
{
	// 计算得到下一个旋转方向
	int nextrotation;
	// 使用基址加偏移的方式，确保旋转之后的方块是同种类型
	if (rotation >= 0 && rotation <= 3) {
		nextrotation = (rotation + 1) % 4;
	}
	else if (rotation >= 4 && rotation <= 7) {
		nextrotation = (rotation + 1 - 4) % 4 + 4;
	}
	else if (rotation >= 8 && rotation <= 11) {
		nextrotation = (rotation + 1 - 8) % 4 + 8;
	}
	else if (rotation >= 12 && rotation <= 13) {
		nextrotation = (rotation + 1 - 12) % 2 + 12;
	}
	else if (rotation >= 14 && rotation <= 15) {
		nextrotation = (rotation + 1 - 14) % 2 + 14;
	}
	else if (rotation >= 16 && rotation <= 17) {
		nextrotation = (rotation + 1 - 16) % 2 + 16;
	}
	else if (rotation == 18) {
		nextrotation = 18;
	}

	// 检查当前旋转之后的位置的有效性
	if (checkvalid((allRotationsLshape[nextrotation][0]) + tilepos)
		&& checkvalid((allRotationsLshape[nextrotation][1]) + tilepos)
		&& checkvalid((allRotationsLshape[nextrotation][2]) + tilepos)
		&& checkvalid((allRotationsLshape[nextrotation][3]) + tilepos))
	{
		// 更新旋转，将当前方块设置为旋转之后的方块
		rotation = nextrotation;
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsLshape[rotation][i];

		updatetile();
	}
}

// 检查棋盘格在row行有没有被填充满
void checkfullrow(int row)
{
	int cnt = 0;
	// 检测一下被填充方格的数量
	for (int i = 0; i < 10; ++i) {
		if (board[i][row] == true) {
			++cnt;
		}
	}
	// 这一行都被填充了
	if (cnt == 10) {
		for (int i = 0; i < 10; ++i) {
			int r = row;
			while (r + 1 < 20 && board[i][r + 1] == true) {
				changecellcolour(vec2(i, r), boardcolours[6 * (10 * (r + 1) + i)]);
				++r;
				// 用上面方格的颜色覆盖下面方格的颜色
			}
			board[i][r] = false;  // 最顶格棋盘格没有方块占领了，因此恢复颜色和board变量
			changecellcolour(vec2(i, r), black);
		}
		// 总得分 +10
		totalScore += 10;
		// 总得分达到一定程度则增加下落速度(通过timeGap减少实现)，但最终下落间隔不会少于100ms
		if (totalScore % deltaDecreasePoint == 0 && timeGap >= 100) {
			timeGap -= deltaDecrease;
		}
		// 消去时的音乐
		PlaySound(TEXT("success_bgm.wav"), NULL, SND_FILENAME | SND_ASYNC);
	}
}

// 放置当前方块，并且更新棋盘格对应位置顶点的颜色VBO
void settile()
{
	// 每个格子
	for (int i = 0; i < 4; i++)
	{
		// 获取格子在棋盘格上的坐标
		int x = (tile[i] + tilepos).x;
		int y = (tile[i] + tilepos).y;
		// 将格子对应在棋盘格上的位置设置为填充
		board[x][y] = true;
		// 并将相应位置的颜色修改
		changecellcolour(vec2(x, y), COLOR[random]);
	}

	// debug();
}

// 给定位置(x,y)，移动方块。有效的移动值为(-1,0)，(1,0)，(0,-1)，分别对应于向
// 左，向下和向右移动。如果移动成功，返回值为true，反之为false
bool movetile(vec2 direction)
{
	// 计算移动之后的方块的位置坐标
	vec2 newtilepos[4];
	for (int i = 0; i < 4; i++)
		newtilepos[i] = tile[i] + tilepos + direction;

	// 检查移动之后的有效性
	if (checkvalid(newtilepos[0])
		&& checkvalid(newtilepos[1])
		&& checkvalid(newtilepos[2])
		&& checkvalid(newtilepos[3]))
	{
		// 有效：移动该方块
		tilepos.x = tilepos.x + direction.x;
		tilepos.y = tilepos.y + direction.y;

		updatetile();

		return true;
	}

	return false;
}

void movetile_version2(vec2 direction)
{
	// 计算移动之后的方块的位置坐标
	vec2 newtilepos[4];
	for (int i = 0; i < 4; i++)
		newtilepos[i] = tile[i] + tilepos + direction;

	// 检查移动之后的有效性
	while (checkvalid(newtilepos[0])
		&& checkvalid(newtilepos[1])
		&& checkvalid(newtilepos[2])
		&& checkvalid(newtilepos[3]))  // 循环试探性地下滑，直到不能再下滑为止
	{
		// 有效：移动该方块
		tilepos.x = tilepos.x + direction.x;
		tilepos.y = tilepos.y + direction.y;

		for (int i = 0; i < 4; i++)
			newtilepos[i] = tile[i] + tilepos + direction;  // 下一轮要判断的位置
	}
	// 注意恢复到非法位置的前个状态（也即是合法位置的最后一个状态）
	tilepos.x = tilepos.x - direction.x;
	tilepos.y = tilepos.y - direction.y;
	movetile(direction);  // 调用movetile完成顶点渲染
}

// 重新启动游戏
void restart()
{
	init();
}

// 游戏渲染部分
void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(locxsize, xsize);
	glUniform1i(locysize, ysize);

	glViewport(0, 0, xsize, ysize - 30);  // 俄罗斯棋盘从(0,0)开始定义，大小为xsize*(ysize-30)

	#ifdef __APPLE__	// for MacOS
		glBindVertexArrayAPPLE(vaoIDs[1]);
		glDrawArrays(GL_TRIANGLES, 0, 1200); // 绘制棋盘格 (10*20*2 = 400 个三角形)
		glBindVertexArrayAPPLE(vaoIDs[2]);
		glDrawArrays(GL_TRIANGLES, 0, 24);	 // 绘制当前方块 (8 个三角形)
		glBindVertexArrayAPPLE(vaoIDs[0]);
		glDrawArrays(GL_LINES, 0, 64);		 // 绘制棋盘格的线
	#else				// for Windows
		glBindVertexArray(vaoIDs[1]);
		glDrawArrays(GL_TRIANGLES, 0, 1200); // 绘制棋盘格 (10*20*2 = 400 个三角形)
		glBindVertexArray(vaoIDs[2]);
		glDrawArrays(GL_TRIANGLES, 0, 24);	 // 绘制当前方块 (8 个三角形)
		glBindVertexArray(vaoIDs[0]);
		glDrawArrays(GL_LINES, 0, 64);		 // 绘制棋盘格的线
	#endif

	glViewport(0, ysize - 30, xsize, 100);  // 文字显示面板从(0, ysize-30)开始绘制，大小为xsize*100
	drawtext();  // 调用文字绘制函数
	glutSwapBuffers();
}

// 在窗口被拉伸的时候，控制棋盘格的大小，使之保持固定的比例。
void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}

// 键盘响应事件中的特殊按键响应
void special(int key, int x, int y)
{
	if (!gameover)
	{
		switch (key)
		{
		case GLUT_KEY_UP:	// 向上按键旋转方块
			rotate();
			break;
		case GLUT_KEY_DOWN: // 向下按键移动方块
			if (!movetile(vec2(0, -1)))  // 尝试往下移动
			{
				settile();  // 不能再往下移动则固定方块
				newtile();  // 同时生成一个新的方块
			}
			break;
		case GLUT_KEY_LEFT:  // 向左按键移动方块
			movetile(vec2(-1, 0));  // 方向向量为(-1, 0)
			break;
		case GLUT_KEY_RIGHT: // 向右按键移动方块
			movetile(vec2(1, 0));  // 方向向量为(1, 0)
			break;
		}
	}
}

void fallfree(int) {
	if (timerState == TIMER_STATE_RUN) {
		if (!movetile(vec2(0, -1))) {  // 尝试向下移动一个方块
			settile();  // 不能再移动，固定住该方块
			newtile();  // 产生新的方块
		}
		glutTimerFunc(timeGap, fallfree, 1);  // 下一个500ms再次触发该定时器
	}
}

// 键盘响应时间中的普通按键响应
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 033: // ESC键 和 'q' 键退出游戏
		exit(EXIT_SUCCESS);
		break;
	case 'q':
		exit(EXIT_SUCCESS);
		break;
	case 'r': // 'r' 键重启游戏
		restart();
		break;
	case ' ': // 空格键
		timerState = TIMER_STATE_PAUSE; // 先停止fallfree自动下滑功能
		movetile_version2(vec2(0, -1)); // 调用直接到底函数
		timerState = TIMER_STATE_RUN;   // 重新开始fallfree
		break;
	}
	glutPostRedisplay();
}

void idle(void)
{
	for (int i = 19; i >= 0; --i) {
		checkfullrow(i);  // 检查是否满行
	}
	glutPostRedisplay();
}

void windowMenuEvent(int op) {
	switch (op) {
	case MENU_CHOICE_PAUSE:  // 暂停游戏
		timerState = TIMER_STATE_PAUSE;
		glutIdleFunc(NULL);
		break;
	case MENU_CHOICE_CONTINUE:  // 继续游戏
		timerState = TIMER_STATE_RUN;
		glutTimerFunc(timeGap, fallfree, timerState);
		glutIdleFunc(idle);
		break;
	case MENU_CHOICE_EXIT:  // 结束游戏
		recordScore();
		exit(EXIT_SUCCESS);
		break;
	case MENU_CHOICE_EASY:	// easy模式
		timeGap = 500;		// 下落间隔500
		deltaDecrease = 50;	// 每隔50个得分点缩短timeGap
		deltaDecreasePoint = 50;	// timeGap下降幅度
		totalScore = 0;		// 重置分数为0
		restart();			// 重新开始新一轮游戏
		break;
	case MENU_CHOICE_SOSO:  // 中等模式
		timeGap = 450;
		deltaDecrease = 50;
		deltaDecreasePoint = 40;
		totalScore = 0;
		restart();
		break;
	case MENU_CHOICE_HARD:  // 困难模式
		timeGap = 400;
		deltaDecrease = 100;
		deltaDecreasePoint = 30;
		totalScore = 0;
		restart();
		break;
	}
}

void setWindowMenu() {
	subMenu = glutCreateMenu(windowMenuEvent);  // 添加一个子菜单（难度控制）
	glutAddMenuEntry("Easy", MENU_CHOICE_EASY);
	glutAddMenuEntry("Soso", MENU_CHOICE_SOSO);
	glutAddMenuEntry("Hard", MENU_CHOICE_HARD);

	mainMenu = glutCreateMenu(windowMenuEvent);  // 添加一个主菜单（运行控制）
	glutAddMenuEntry("Pause", MENU_CHOICE_PAUSE);
	glutAddMenuEntry("Continue", MENU_CHOICE_CONTINUE);
	glutAddMenuEntry("End", MENU_CHOICE_EXIT);

	glutAddSubMenu("LEVEL", subMenu);
	glutAttachMenu(GLUT_RIGHT_BUTTON);  // 右键激活
}

int main(int argc, char** argv)
{
	srand((unsigned)time(nullptr));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(xsize, ysize);
	glutInitWindowPosition(680, 38);
	mainWindow = glutCreateWindow("test");

#ifdef WIN32
	glewExperimental = GL_TRUE;
	glewInit();
#endif

	init();
	setWindowMenu();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);		// 键盘相应事件绑定
	glutIdleFunc(idle);				// 注册空闲回调函数
	glutTimerFunc(timeGap, fallfree, timerState);	// 定时器，下一个timeGap调用fallfree

	glutMainLoop();
	return 0;
}
