#include <windows.h>
#include <windowsx.h>
#include <ddraw.h>
#include<stdio.h>
#include<time.h>
#include "ddutil.h"
#include<math.h>

#include <dsound.h>
#include "dsutil.h"

#define _GetKeyState( vkey ) HIBYTE(GetAsyncKeyState( vkey ))
#define _GetKeyPush( vkey )  LOBYTE(GetAsyncKeyState( vkey ))
#define PI 3.141592
typedef struct Data
{
	float Xpos, Ypos, Xvel, Yvel;
};

int tempFrame;
static int Frame = 0;
HWND MainHwnd;
float angle = 0;
LPDIRECTDRAW         DirectOBJ;
LPDIRECTDRAWSURFACE  RealScreen;
LPDIRECTDRAWSURFACE  BackScreen;
LPDIRECTDRAWSURFACE  SpriteImage;
LPDIRECTDRAWSURFACE  BackGround;
LPDIRECTDRAWSURFACE  Bullet;
LPDIRECTDRAWSURFACE  Explosion;


LPDIRECTDRAWCLIPPER	ClipScreen;

int gFullScreen = 0, Click = 0;
int gWidth = 500 * 2, gHeight = 800;
float MouseX = 100.0f, MouseY = (float)(gHeight / 2);
bool alive = true;
bool otheralive = true;
float PrevTime = 0;
float NowTIme = 0;
float DeltaTime = 0;
////////////////////
float SC = 0;
bool falselogin = false;
LPDIRECTSOUND       SoundOBJ = NULL;
LPDIRECTSOUNDBUFFER SoundDSB = NULL;
DSBUFFERDESC        DSB_desc;

HSNDOBJ Sound[10];

float term = 0;
bool GameSet = false;
bool Damaged = false;
int arrOBJ[20];
float Xvel = 0;
float Yvel = 0;  //내 캐릭터의 속도
float RealPosX = 850;
float RealPosY = 100;		//내 캐릭터의 포지션
int id;
char DataType;
bool Init = false;
bool occur = false;
Data Other;
float BulletPos[3][2] = { { 200.0f,600.0f },{ 800.0f,300.0f },{ 500.0f,300.0f } };
RECT EX = { 0,0,48,48 };

float occurx;
float occury;

int tempfr = 0;
int fr = 24;

void PlayExplosion(float x, float y, int fr)
{
	BackScreen->BltFast(x, y, Explosion, &EX, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
	EX.left = 48 * (fr);
	EX.top = 0;
	EX.right = EX.left + 48;
	EX.bottom = EX.top + 48;
}

BOOL _InitDirectSound(void)
{
	if (DirectSoundCreate(NULL, &SoundOBJ, NULL) == DS_OK)
	{
		if (SoundOBJ->SetCooperativeLevel(MainHwnd, DSSCL_PRIORITY) != DS_OK) return FALSE;

		memset(&DSB_desc, 0, sizeof(DSBUFFERDESC));
		DSB_desc.dwSize = sizeof(DSBUFFERDESC);
		DSB_desc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN;

		if (SoundOBJ->CreateSoundBuffer(&DSB_desc, &SoundDSB, NULL) != DS_OK) return FALSE;
		SoundDSB->SetVolume(DSBVOLUME_MAX); // DSBVOLUME_MIN
		SoundDSB->SetPan(DSBPAN_RIGHT);
		return TRUE;
	}
	return FALSE;
}

float ABS(float a)
{
	if (a < 0)
		return -a;
	else return a;
}

void _Play(int num)
{
	SndObjPlay(Sound[num], NULL);
}

BOOL Fail(HWND hwnd)
{
	ShowWindow(hwnd, SW_HIDE);
	MessageBox(hwnd, "DIRECT X 초기화에 실패했습니다.", "게임 디자인", MB_OK);
	DestroyWindow(hwnd);
	return FALSE;
}

void _ReleaseAll(void)
{
	if (DirectOBJ != NULL)
	{
		if (RealScreen != NULL)
		{
			RealScreen->Release();
			RealScreen = NULL;
		}
		if (SpriteImage != NULL)
		{
			SpriteImage->Release();
			SpriteImage = NULL;
		}
		if (BackGround != NULL)
		{
			BackGround->Release();
			BackGround = NULL;
		}
		DirectOBJ->Release();
		DirectOBJ = NULL;
	}
}

long FAR PASCAL WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int Step = 5;


	switch (message)
	{

	case	WM_LBUTTONDOWN: 	Click = 1;
		_Play(3);
		break;
	case    WM_DESTROY:  _ReleaseAll();
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

BOOL _GameMode(HINSTANCE hInstance, int nCmdShow, int x, int y, int bpp)
{
	HRESULT result;
	WNDCLASS wc;
	DDSURFACEDESC ddsd;
	DDSCAPS ddscaps;
	LPDIRECTDRAW pdd;

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetStockBrush(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "TEST";
	RegisterClass(&wc);


	if (gFullScreen) {
		if ((MainHwnd = CreateWindowEx(0, "TEST", NULL, WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN),
			GetSystemMetrics(SM_CYSCREEN), NULL, NULL, hInstance, NULL)) == NULL)
			ExitProcess(1);
	}
	else {
		if ((MainHwnd = CreateWindow("TEST", "TEST", WS_OVERLAPPEDWINDOW, 0, 0, x,
			y, NULL, NULL, hInstance, NULL)) == NULL)
			ExitProcess(1);
		//SetWindowPos(MainHwnd, NULL, 100, 100, x, y, SWP_NOZORDER);
	}

	//SetFocus(MainHwnd);
	ShowWindow(MainHwnd, nCmdShow);
	UpdateWindow(MainHwnd);
	ShowCursor(TRUE);

	result = DirectDrawCreate(NULL, &pdd, NULL);
	if (result != DD_OK) return Fail(MainHwnd);

	result = pdd->QueryInterface(IID_IDirectDraw, (LPVOID *)&DirectOBJ);
	if (result != DD_OK) return Fail(MainHwnd);


	// 윈도우 핸들의 협력 단계를 설정한다.
	if (gFullScreen) {
		result = DirectOBJ->SetCooperativeLevel(MainHwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
		if (result != DD_OK) return Fail(MainHwnd);

		result = DirectOBJ->SetDisplayMode(x, y, bpp);
		if (result != DD_OK) return Fail(MainHwnd);

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount = 1;

		result = DirectOBJ->CreateSurface(&ddsd, &RealScreen, NULL);
		if (result != DD_OK) return Fail(MainHwnd);

		memset(&ddscaps, 0, sizeof(ddscaps));
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		result = RealScreen->GetAttachedSurface(&ddscaps, &BackScreen);
		if (result != DD_OK) return Fail(MainHwnd);
	}
	else {
		result = DirectOBJ->SetCooperativeLevel(MainHwnd, DDSCL_NORMAL);
		if (result != DD_OK) return Fail(MainHwnd);

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		ddsd.dwBackBufferCount = 0;

		result = DirectOBJ->CreateSurface(&ddsd, &RealScreen, NULL);
		if (result != DD_OK) return Fail(MainHwnd);

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		ddsd.dwWidth = x;
		ddsd.dwHeight = y;
		result = DirectOBJ->CreateSurface(&ddsd, &BackScreen, NULL);
		if (result != DD_OK) return Fail(MainHwnd);

		result = DirectOBJ->CreateClipper(0, &ClipScreen, NULL);
		if (result != DD_OK) return Fail(MainHwnd);

		result = ClipScreen->SetHWnd(0, MainHwnd);
		if (result != DD_OK) return Fail(MainHwnd);

		result = RealScreen->SetClipper(ClipScreen);
		if (result != DD_OK) return Fail(MainHwnd);

		//	SetWindowPos(MainHwnd, NULL, 100, 100, x, y, SWP_NOZORDER | SWP_NOACTIVATE);
	}


	return TRUE;
}



extern void CommInit(int argc, char **argv);
extern void CommSend(char *sending);
extern void CommRecv(char *recvData);
char SendBuffer[127];

RECT PLAYER[4][4];
RECT PLAYERrect = PLAYER[0][0];
RECT OTHERSrect = PLAYER[0][0];

bool Collision(float Ax, float Ay, float Bx, float By, RECT A, RECT B)
{
	float aXleng = A.right - A.left;
	float aYleng = A.bottom - A.top;

	float bXleng = B.right - B.left;
	float bYleng = B.bottom - B.top;

	if ((Ax <= Bx && Bx <= (Ax + aXleng)) && Ay <= By && By <= Ay + aYleng)
		return true;

	if ((Ax <= (Bx + bXleng) && (Bx + bXleng) <= (Ax + aXleng)) && Ay <= By && By <= Ay + aYleng)
		return true;

	if ((Ax <= Bx && Bx <= (Ax + aXleng)) && Ay <= (By + bYleng) && (By + bYleng) <= Ay + aYleng)
		return true;

	if ((Ax <= (Bx + bXleng) && (Bx + bXleng) <= (Ax + aXleng)) && Ay <= (By + bYleng) && (By + bYleng) <= Ay + aYleng)
		return true;

	return false;
}

void Animation()
{
	PLAYER[0][0].left = 0 + Frame * 36;
	PLAYER[0][0].top = 0;
	PLAYER[0][0].right = PLAYER[0][0].left + 36;
	PLAYER[0][0].bottom = PLAYER[0][0].top + 36;

	PLAYER[0][1].left = 0 + Frame * 36;
	PLAYER[0][1].top = 36 * 3;
	PLAYER[0][1].right = PLAYER[0][1].left + 36;
	PLAYER[0][1].bottom = PLAYER[0][1].top + 36;

	PLAYER[0][2].left = 0 + Frame * 36;
	PLAYER[0][2].top = 36;
	PLAYER[0][2].right = PLAYER[0][2].left + 36;
	PLAYER[0][2].bottom = PLAYER[0][2].top + 36;

	PLAYER[0][3].left = 0 + Frame * 36;
	PLAYER[0][3].top = 36 * 2;
	PLAYER[0][3].right = PLAYER[0][3].left + 36;
	PLAYER[0][3].bottom = PLAYER[0][3].top + 36;

	PLAYER[1][0].left = 36 * 3 + Frame * 36;
	PLAYER[1][0].top = 0;
	PLAYER[1][0].right = PLAYER[1][0].left + 36;
	PLAYER[1][0].bottom = PLAYER[1][0].top + 36;

	PLAYER[1][1].left = 36 * 3 + Frame * 36;
	PLAYER[1][1].top = 36 * 3;
	PLAYER[1][1].right = PLAYER[1][1].left + 36;
	PLAYER[1][1].bottom = PLAYER[1][1].top + 36;

	PLAYER[1][2].left = 36 * 3 + Frame * 36;
	PLAYER[1][2].top = 36;
	PLAYER[1][2].right = PLAYER[1][2].left + 36;
	PLAYER[1][2].bottom = PLAYER[1][2].top + 36;

	PLAYER[1][3].left = 36 * 3 + Frame * 36;
	PLAYER[1][3].top = 36 * 2;
	PLAYER[1][3].right = PLAYER[1][3].left + 36;
	PLAYER[1][3].bottom = PLAYER[1][3].top + 36;

	PLAYER[2][0].left = 36 * 6 + Frame * 36;
	PLAYER[2][0].top = 0;
	PLAYER[2][0].right = PLAYER[2][0].left + 36;
	PLAYER[2][0].bottom = PLAYER[2][0].top + 36;

	PLAYER[2][1].left = 36 * 6 + Frame * 36;
	PLAYER[2][1].top = 36 * 3;
	PLAYER[2][1].right = PLAYER[2][1].left + 36;
	PLAYER[2][1].bottom = PLAYER[2][1].top + 36;

	PLAYER[2][2].left = 36 * 6 + Frame * 36;
	PLAYER[2][2].top = 36;
	PLAYER[2][2].right = PLAYER[2][2].left + 36;
	PLAYER[2][2].bottom = PLAYER[2][2].top + 36;

	PLAYER[2][3].left = 36 * 6 + Frame * 36;
	PLAYER[2][3].top = 36 * 2;
	PLAYER[2][3].right = PLAYER[2][3].left + 36;
	PLAYER[2][3].bottom = PLAYER[2][3].top + 36;

	PLAYER[3][0].left = 36 * 9 + Frame * 36;
	PLAYER[3][0].top = 0;
	PLAYER[3][0].right = PLAYER[3][0].left + 36;
	PLAYER[3][0].bottom = PLAYER[3][0].top + 36;

	PLAYER[3][1].left = 36 * 9 + Frame * 36;
	PLAYER[3][1].top = 36 * 3;
	PLAYER[3][1].right = PLAYER[3][1].left + 36;
	PLAYER[3][1].bottom = PLAYER[3][1].top + 36;

	PLAYER[3][2].left = 36 * 9 + Frame * 36;
	PLAYER[3][2].top = 36;
	PLAYER[3][2].right = PLAYER[3][2].left + 36;
	PLAYER[3][2].bottom = PLAYER[3][2].top + 36;

	PLAYER[3][3].left = 36 * 9 + Frame * 36;
	PLAYER[3][3].top = 36 * 2;
	PLAYER[3][3].right = PLAYER[3][3].left + 36;
	PLAYER[3][3].bottom = PLAYER[3][3].top + 36;

}

void DecidePlayerRect(RECT &dest, int id, float Xvel, float Yvel)
{
	if (ABS(Xvel) > ABS(Yvel))
	{
		if (Xvel > 0)
		{
			dest = PLAYER[id][3];
		}
		else {
			dest = PLAYER[id][2];
		}
	}
	else if (ABS(Xvel) < ABS(Yvel))
	{
		if (Yvel > 0)
		{
			dest = PLAYER[id][0];
		}
		else {
			dest = PLAYER[id][1];
		}
	}
	else {
		dest = PLAYER[id][0];
	}
}

void PositionOffset()
{
	if (RealPosX < 25)
	{
		RealPosX = 25;
		Xvel *= -0.7;
	}
	if (RealPosX > 951)
	{
		RealPosX = 950;
		Xvel *= -0.7;
	}
	if (RealPosY < 60)
	{
		RealPosY = 60;
		Yvel *= -0.7;
	}
	if (RealPosY > 756)
	{
		RealPosY = 755;
		Yvel *= -0.7;
	}

}

void Positioninit(int id)
{
	if (id == 0)
	{
		RealPosX = 100;
		RealPosY = 100;
	}
	else if (id == 1)
	{
		RealPosX = 900;
		RealPosY = 730;
	}

}

bool Connecting[4] = { false };
int condex = 0;
bool initialed = false;

//Data type Q는 자기아이디, C는 다른사람 아이디, 위치, 속도
void _GameProcDraw(char* recvBuffer)
{
	RECT BackRect = { 0, 35, 1000, 835 };		//배경 스프라이트의 픽셀위치.
	RECT DispRect = { 0, 0, gWidth, gHeight }; //스크린에 그려줄 좌표.
	RECT BulletRect = { 0,0,26,26 };
	RECT WinRect;


	if (recvBuffer[0] == 'Q')
	{
		sscanf(recvBuffer, "%c,%d", &DataType, &id);
		DecidePlayerRect(PLAYERrect, id, Xvel, Yvel);	//초기 캐릭터 지정.
		if (!Init)		//id 받아서 포지션 초기화(초기 포지션 결정)
		{
			Positioninit(id);
			Init = true;

			sprintf(SendBuffer, "R,%d,%f,%f,%f,%f,%f,%f,%f,%d", id, 200.0f, 600.0f, 800.0f, 200.0f, 400.0f, 300.0f, 10.0f, 0);//아이디 까지 받음. 
			CommSend(SendBuffer);		//내 위치, 속도 전송해 줌.//정보 보낼때 아이디도 보낼수 있게 지정하자
		}
	}

	if (recvBuffer[0] == 'C')
	{
		int tempid;
		float tempX, tempY, tvelX, tvelY;
		//sscanf(recvBuffer, "%c,%d,%f,%f,%f,%f,%d", &DataType, &tempid,&tempX,&tempY,&tvelX,&tvelY,&term);
		sscanf(recvBuffer, "%c,%d,%f,%f,%f,%f,%d", &DataType, &tempid, &Other.Xpos, &Other.Ypos, &Other.Xvel, &Other.Yvel, &term);
		Connecting[tempid] = true;

		//Other.Xpos = tempX;
		//Other.Ypos = tempY;
		//Other.Xvel = tvelX;
		//Other.Yvel = tvelY;
	}


	if (recvBuffer[0] == 'B')
		sscanf(recvBuffer, "%C,%d", &DataType, &term);

	PositionOffset();  //내 캐릭터가 벽에 닿는 판정, 위치제한 정해주는 함수
	Animation();

	term += 3;
	if (term > 1000)
		term = 0;
	// 변경  (Frame 변화에 따라)
	sprintf(SendBuffer, "C,%d,%f,%f,%f,%f,%d", id, RealPosX, RealPosY, Xvel, Yvel, (term + 1));//아이디 까지 받음. 
	CommSend(SendBuffer);		//내 위치, 속도 전송해 줌.//정보 보낼때 아이디도 보낼수 있게 지정하자.



	BackScreen->BltFast(0, 0, BackGround, &BackRect, DDBLTFAST_WAIT | DDBLTFAST_NOCOLORKEY);

	if (tempFrame++ > 5)
	{
		tempFrame = 0;
		Frame++;
	}

	if (Frame >= 2)
		Frame = 0;

	RealPosX += Xvel;
	Xvel /= 1.001f;
	RealPosY += Yvel;
	Yvel /= 1.001f;


	if (otheralive&&alive)
	{
		if (Collision(RealPosX, RealPosY, Other.Xpos, Other.Ypos, PLAYERrect, OTHERSrect))
		{
			occurx = ((RealPosX + (RealPosX + 36)) / 2 + (Other.Xpos + Other.Xpos + 36) / 2) / 2;
			occury = ((RealPosY + (RealPosY + 36)) / 2 + (Other.Ypos + Other.Ypos + 36) / 2) / 2;
			_Play(1);
			occur = true;
			float V1 = (Xvel)*(Xvel)+(Yvel)*(Yvel);
			float V2 = (Other.Xvel)*(Other.Xvel) + (Other.Yvel)*(Other.Yvel);

			if (V1 > V2)
			{
				otheralive = false;
				sprintf(SendBuffer, "D,%d,%f,%f,%f,%f,%d", id, RealPosX, RealPosY, Xvel, Yvel, (term + 1));//아이디 까지 받음. 
				CommSend(SendBuffer);		//내 위치, 속도 전송해 줌.//정보 보낼때 아이디도 보낼수 있게 지정하자.
			}
			else {
				alive = false;
				sprintf(SendBuffer, "K,%d,%f,%f,%f,%f,%d", id, RealPosX, RealPosY, Xvel, Yvel, (term + 1));//아이디 까지 받음. 
				CommSend(SendBuffer);		//내 위치, 속도 전송해 줌.//정보 보낼때 아이디도 보낼수 있게 지정하자.
			}
		}
	}

	if (recvBuffer[0] == 'R')
	{
		otheralive = true;
		int z;

		sscanf(recvBuffer, "%c,&d,%f,%f,%f,%f,%f,%f,%f,%d", &DataType, &z, &BulletPos[0][0], &BulletPos[0][1], &BulletPos[1][0], &BulletPos[1][1], &BulletPos[2][0], &BulletPos[2][1], &angle, &term);
	}
	int eX;
	int eY;
	if (recvBuffer[0] == 'K')
	{
		otheralive = false;
		occur = true;
		_Play(1);
		sscanf(recvBuffer, "K,%d,%f,%f", &DataType, &occurx, &occury);

	}
	for (int i = 0; i < 2; i++)		//i가 아이디와 같다.
	{
		if (i != id && Connecting[i])
		{
			DecidePlayerRect(OTHERSrect, i, Other.Xvel, Other.Yvel);
			if (otheralive)
				BackScreen->BltFast(Other.Xpos, Other.Ypos, SpriteImage, &OTHERSrect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
		}
	}

	DecidePlayerRect(PLAYERrect, id, Xvel, Yvel);       //속도로 애니메이션 정해줌. 그 애니메이션으로 다시 내 캐릭터 그림

	if (alive)
		BackScreen->BltFast(RealPosX, RealPosY, SpriteImage, &PLAYERrect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);

	///////////////////////////////////////////////////////총알///////////////////////////////////////////////////////////
	BackScreen->BltFast(BulletPos[0][0], BulletPos[0][1], Bullet, &BulletRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
	BackScreen->BltFast(BulletPos[1][0], BulletPos[1][1], Bullet, &BulletRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
	BackScreen->BltFast(BulletPos[2][0], BulletPos[2][1], Bullet, &BulletRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);

	angle += term / 4;
	BulletPos[0][0] -= term / 5;
	BulletPos[0][1] = 600 + (float)cos(((angle * 2) / 1000 * PI))*70.0f;

	BulletPos[1][0] += term / 5;
	BulletPos[1][1] = 200 + (float)cos(((angle * 2) / 1000 * PI))*70.0f;

	BulletPos[2][1] += term / 5;
	BulletPos[2][0] = 400 + (float)cos(((angle * 2) / 1000 * PI))*70.0f;

	if (BulletPos[0][0] > 1000)
		BulletPos[0][0] = 1;
	if (BulletPos[2][1] > 1000)
		BulletPos[2][1] = 1;

	if (BulletPos[0][0] <= 0)
		BulletPos[0][0] = 1000;
	if (BulletPos[1][0] >= 1000)
		BulletPos[1][0] = 1;
	if (otheralive&&alive)
	{
		if (Collision(RealPosX, RealPosY, BulletPos[0][0], BulletPos[0][1], PLAYERrect, BulletRect))
		{
			occurx = ((RealPosX + (RealPosX + 36)) / 2 + (BulletPos[0][0] + BulletPos[0][0] + 13) / 2) / 2;
			occury = ((RealPosY + (RealPosY + 36)) / 2 + (BulletPos[0][1] + BulletPos[0][1] + 13) / 2) / 2;
			_Play(1);
			occur = true;
			alive = false;
			sprintf(SendBuffer, "K,%d,%f,%f,%f,%f,%d", id, RealPosX, RealPosY, Xvel, Yvel, (term + 1));//아이디 까지 받음. 
			CommSend(SendBuffer);		//내 위치, 속도 전송해 줌.//정보 보낼때 아이디도 보낼수 있게 지정하자.

		}
		if (Collision(RealPosX, RealPosY, BulletPos[1][0], BulletPos[1][1], PLAYERrect, BulletRect))
		{
			occurx = ((RealPosX + (RealPosX + 36)) / 2 + (BulletPos[1][0] + BulletPos[1][0] + 13) / 2) / 2;
			occury = ((RealPosY + (RealPosY + 36)) / 2 + (BulletPos[1][1] + BulletPos[1][1] + 13) / 2) / 2;
			_Play(1);
			occur = true;
			alive = false;
			sprintf(SendBuffer, "K,%d,%f,%f,%f,%f,%d", id, RealPosX, RealPosY, Xvel, Yvel, (term + 1));//아이디 까지 받음. 
			CommSend(SendBuffer);		//내 위치, 속도 전송해 줌.//정보 보낼때 아이디도 보낼수 있게 지정하자.
		}
		if (Collision(RealPosX, RealPosY, BulletPos[2][0], BulletPos[2][1], PLAYERrect, BulletRect))
		{
			occurx = ((RealPosX + (RealPosX + 36)) / 2 + (BulletPos[2][0] + BulletPos[2][0] + 13) / 2) / 2;
			occury = ((RealPosY + (RealPosY + 36)) / 2 + (BulletPos[2][1] + BulletPos[2][1] + 13) / 2) / 2;
			_Play(1);
			occur = true;
			alive = false;
			sprintf(SendBuffer, "K,%d,%f,%f,%f,%f,%d", id, RealPosX, RealPosY, Xvel, Yvel, (term + 1));//아이디 까지 받음. 
			CommSend(SendBuffer);		//내 위치, 속도 전송해 줌.//정보 보낼때 아이디도 보낼수 있게 지정하자.
		}
	}
	///////////////////////////////////////////////////////////총알/////////////////////////////////////////////////////////////

	//////////////////////////////////////////////폭발//////////////////////////////////////////

	if (occur)
	{
		tempfr++;
		if (tempfr > 3)
		{
			tempfr = 0;
			fr--;
		}
		PlayExplosion(occurx, occury, fr);
	}
	if (fr <= 0)
	{
		fr = 24;
		EX.left = 0;
		EX.top = 0;
		EX.right = 32;
		EX.bottom = 32;
		occur = false;
	}
	/////////////////////////////////////폭발////////////////////////////////////////
	if (gFullScreen)
		RealScreen->Flip(NULL, DDFLIP_WAIT);
	else {
		GetWindowRect(MainHwnd, &WinRect);
		RealScreen->Blt(&WinRect, BackScreen, &DispRect, DDBLT_WAIT, NULL);
	}
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;

	if (!_GameMode(hInstance, nCmdShow, gWidth, gHeight, 32)) return FALSE;
	memset(arrOBJ, 0, sizeof(arrOBJ));
	SpriteImage = DDLoadBitmap(DirectOBJ, "sprite_char.BMP", 0, 0);
	BackGround = DDLoadBitmap(DirectOBJ, "Icerink2.BMP", 0, 0);
	Bullet = DDLoadBitmap(DirectOBJ, "Bullet.BMP", 0, 0);
	Explosion = DDLoadBitmap(DirectOBJ, "explosion.BMP", 0, 0);
	DDSetColorKey(SpriteImage, RGB(0, 0, 0));
	DDSetColorKey(Bullet, RGB(0, 0, 0));
	DDSetColorKey(Explosion, RGB(0, 0, 0));

	CommInit(NULL, NULL); //서버에서 아이디 받아온다.
	char senddata[200] = "a";
	CommSend(senddata);

	if (_InitDirectSound())
	{
		Sound[0] = SndObjCreate(SoundOBJ, "NetBGM.WAV", 1);
		Sound[1] = SndObjCreate(SoundOBJ, "Explosion.WAV", 1);
	}

	while (!_GetKeyState(VK_ESCAPE))
	{

		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, NULL, 0, 0)) return msg.wParam;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//        else _GameProc();
		_Play(0);

		if (SC >2)
		{
			SndObjStop(Sound[4]);
			Damaged = false;
			SC = 0;
		}

		if (_GetKeyState(VK_UP))
			Yvel -= 0.00001;
		if (_GetKeyState(VK_RIGHT))
			Xvel += 0.00001;
		if (_GetKeyState(VK_DOWN))
			Yvel += 0.00001;
		if (_GetKeyState(VK_LEFT))
			Xvel -= 0.00001;
	}
	DestroyWindow(MainHwnd);

	return TRUE;
}
