#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<iostream>
#include<list>
#include<string>
#include<ctime>
#include <climits>
#include <queue>
#include "jsoncpp/json.h"
using namespace std;
int n, m;
const int maxn = 25;
const int dx[4] = { -1, 0, 1, 0 };
const int dy[4] = { 0, 1, 0, -1 };
bool invalid[maxn][maxn];

int score[maxn][maxn]; //代表对下一步每一个点的权值判断

struct point
{
	int x, y;
	point(int _x, int _y)
	{
		x = _x;
		y = _y;
	}
};

list<point> snake[2]; // 0表示自己的蛇，1表示对方的蛇
int possibleDire[10];
int posCount;

bool whetherGrow(int num)  //本回合是否生长
{
	if (num <= 9) return true;
	if ((num - 9) % 3 == 0) return true;
	return false;
}

void deleteEnd(int id)     //删除蛇尾
{
	snake[id].pop_back();
}

void move(int id, int dire, int num)  //编号为id的蛇朝向dire方向移动一步
{
	point p = *(snake[id].begin());
	int x = p.x + dx[dire];
	int y = p.y + dy[dire];
	snake[id].push_front(point(x, y));
	if (!whetherGrow(num))
		deleteEnd(id);
}
void outputSnakeBody(int id)    //调试语句
{
	cout << "Snake No." << id << endl;
	for (list<point>::iterator iter = snake[id].begin(); iter != snake[id].end(); ++iter)
		cout << iter->x << " " << iter->y << endl;
	cout << endl;
}

bool isInBody(int x, int y)   //判断(x,y)位置是否有蛇
{
	for (int id = 0; id <= 1; id++)
	for (list<point>::iterator iter = snake[id].begin(); iter != snake[id].end(); ++iter)
	if (x == iter->x && y == iter->y)
		return true;
	return false;
}

bool validDirection(int id, int k)  //判断当前移动方向的下一格是否合法
{
	point p = *(snake[id].begin());
	int x = p.x + dx[k];
	int y = p.y + dy[k];
	if (x>n || y>m || x<1 || y<1) return false;
	if (invalid[x][y]) return false;
	if (isInBody(x, y)) return false;
	return true;
}

////////////////////////////////////////////////////////////////////算法层面的东西
int Rand(int p)   //随机生成一个0到p的数字
{
	return rand()*rand()*rand() % p;
}

/*
为每一个可走的点生成一个权值：从该点出发到达图中其他点中的最短距离
中的最大值
*/
int gen_greedy_score(int x, int y)
{
	bool temp_invalid[maxn][maxn];
	memcpy(temp_invalid, invalid, sizeof(temp_invalid));
	int temp_score[maxn][maxn];
	memset(temp_score, 0, sizeof(temp_score));
	point start(x, y);
	queue<point> q;
	q.push(start);
	temp_invalid[x][y] = 1;
	while (!q.empty())
	{
		point p = q.front();
		q.pop();
		if (p.x - 1 >= 1 && p.x - 1 <= n && !temp_invalid[p.x - 1][p.y])
		{
			temp_score[p.x - 1][p.y] = temp_score[p.x][p.y] + 1;
			temp_invalid[p.x - 1][p.y] = 1;
			q.push(point(p.x - 1, p.y));
		}
		if (p.x + 1 >= 1 && p.x + 1 <= n && !temp_invalid[p.x + 1][p.y])
		{
			temp_score[p.x + 1][p.y] = temp_score[p.x][p.y] + 1;
			temp_invalid[p.x + 1][p.y] = 1;
			q.push(point(p.x + 1, p.y));
		}
		if (p.y - 1 >= 1 && p.y - 1 <= m && !temp_invalid[p.x][p.y - 1])
		{
			temp_score[p.x][p.y - 1] = temp_score[p.x][p.y] + 1;
			temp_invalid[p.x][p.y - 1] = 1;
			q.push(point(p.x, p.y - 1));
		}
		if (p.y + 1 >= 1 && p.y + 1 <= m && !temp_invalid[p.x][p.y + 1])
		{
			temp_score[p.x][p.y + 1] = temp_score[p.x][p.y] + 1;
			temp_invalid[p.x][p.y + 1] = 1;
			q.push(point(p.x, p.y + 1));
		}
	}
	int max_score = 1;
	for (int i = 0; i < maxn; i++)
	for (int j = 0; j < maxn; j++)
	if (temp_score[i][j] > max_score)
		max_score = temp_score[i][j];
	return max_score;
}
////////////////////////////////////////////////////////////////////

int main()
{
	memset(invalid, 0, sizeof(invalid));
	string str;
	string temp;
	while (getline(cin, temp))
		str += temp;

	Json::Reader reader;
	Json::Value input;
	reader.parse(str, input);

	n = input["requests"][(Json::Value::UInt) 0]["height"].asInt();  //棋盘高度
	m = input["requests"][(Json::Value::UInt) 0]["width"].asInt();   //棋盘宽度

	int x = input["requests"][(Json::Value::UInt) 0]["x"].asInt();  //读蛇初始化的信息
	if (x == 1)
	{
		snake[0].push_front(point(1, 1));
		snake[1].push_front(point(n, m));
	}
	else
	{
		snake[1].push_front(point(1, 1));
		snake[0].push_front(point(n, m));
	}
	//处理地图中的障碍物
	int obsCount = input["requests"][(Json::Value::UInt) 0]["obstacle"].size();

	for (int i = 0; i<obsCount; i++)
	{
		int ox = input["requests"][(Json::Value::UInt) 0]["obstacle"][(Json::Value::UInt) i]["x"].asInt();
		int oy = input["requests"][(Json::Value::UInt) 0]["obstacle"][(Json::Value::UInt) i]["y"].asInt();
		invalid[ox][oy] = 1;
	}

	//根据历史信息恢复现场
	int total = input["responses"].size();

	int dire;
	for (int i = 0; i<total; i++)
	{
		dire = input["responses"][i]["direction"].asInt();
		move(0, dire, i);

		dire = input["requests"][i + 1]["direction"].asInt();
		move(1, dire, i);
	}

	if (!whetherGrow(total)) // 本回合两条蛇生长
	{
		deleteEnd(0);
		deleteEnd(1);
	}

	for (int k = 0; k<4; k++)
	if (validDirection(0, k))
		possibleDire[posCount++] = k;

	srand((unsigned)time(0) + total);

	//做出一个决策
	Json::Value ret;
	//////////////////随机走法//////////////
	//if (posCount)
	//	ret["response"]["direction"] = possibleDire[rand() % posCount];
	//else
	//	ret["response"]["direction"] = 0;//无路可走的时候往北走吧
	////////////////////////////////////////

	/////////////////贪心走法///////////////

	for (int i = 1; i <= n; i++)
	{
		for (int j = 1; j <= m; j++)
		{
			if (isInBody(i, j))
				invalid[i][j] = 1;
		}
	}

	if (!posCount)
	{
		point another_snake_start = *(snake[1].begin());
		point snake_start = *(snake[0].begin());
		//看能不能与对面的蛇同归于尽
		if (snake_start.x - 1 == another_snake_start.x && snake_start.y == another_snake_start.y)
			ret["response"]["direction"] = 0;
		else if (snake_start.x == another_snake_start.x && snake_start.y + 1 == another_snake_start.y)
			ret["response"]["direction"] = 1;
		else if (snake_start.x + 1 == another_snake_start.x && snake_start.y == another_snake_start.y)
			ret["response"]["direction"] = 2;
		else if (snake_start.x == another_snake_start.x && snake_start.y - 1 == another_snake_start.y)
			ret["response"]["direction"] = 3;
		else
			ret["response"]["direction"] = 0;//我选择死亡
	}
	else
	{
		int max_score = 0;
		int next_pos = 0;
		point p = *(snake[0].begin());
		for (int i = 0; i < posCount; i++)
		{
			if (possibleDire[i] == 0 && gen_greedy_score(p.x - 1, p.y) > max_score)
			{
				next_pos = 0;
				max_score = gen_greedy_score(p.x - 1, p.y);
			}
			else if (possibleDire[i] == 1 && gen_greedy_score(p.x, p.y + 1) > max_score)
			{
				next_pos = 1;
				max_score = gen_greedy_score(p.x, p.y + 1);
			}
			else if (possibleDire[i] == 2 && gen_greedy_score(p.x + 1, p.y) > max_score)
			{
				next_pos = 2;
				max_score = gen_greedy_score(p.x + 1, p.y);
			}
			else if (possibleDire[i] == 3 && gen_greedy_score(p.x, p.y - 1) > max_score)
			{
				next_pos = 3;
				max_score = gen_greedy_score(p.x, p.y - 1);
			}
			ret["debug"] = max_score;
		}
		ret["response"]["direction"] = next_pos;
	}
	Json::FastWriter writer;
	cout << writer.write(ret) << endl;

	return 0;
}