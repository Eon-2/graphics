// include the basic windows header files and the Direct3D header file
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <iostream>

// define the screen resolution and keyboard macros
#define SCREEN_WIDTH  1366
#define SCREEN_HEIGHT 768
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

#define ENEMY_NUM 10 


// include the Direct3D Library file
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

// global declarations
LPDIRECT3D9 d3d;    // the pointer to our Direct3D interface
LPDIRECT3DDEVICE9 d3ddev;    // the pointer to the device class
LPD3DXSPRITE d3dspt;    // the pointer to our Direct3D Sprite interface
LPD3DXFONT dxfont;
LPD3DXFONT dxfont2; // the pointer to the font object

int score = 0;


// sprite declarations
LPDIRECT3DTEXTURE9 sprite;    // the pointer to the sprite
LPDIRECT3DTEXTURE9 sprite_hero;    // the pointer to the sprite
LPDIRECT3DTEXTURE9 sprite_enemy;    // the pointer to the sprite
LPDIRECT3DTEXTURE9 sprite_bullet;    // the pointer to the sprite
LPDIRECT3DTEXTURE9 DisplayTexture;    // the pointer to the texture



// function prototypes
void initD3D(HWND hWnd);    // sets up and initializes Direct3D
void render_frame(void);    // renders a single frame
void cleanD3D(void);		// closes Direct3D and releases memory

void draw_display();
void LoadTexture(LPDIRECT3DTEXTURE9* texture, LPCTSTR filename);
void DrawTexture(LPDIRECT3DTEXTURE9 texture, RECT texcoords, float x, float y, int a);

void init_game(void);
void do_game_logic(void); 


// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

using namespace std; 


enum {MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT}; 


//기본 클래스 
class entity{

public: 
float x_pos; 
float y_pos; 
int status; 
int HP; 

};


//주인공 클래스 
class Hero:public entity{

public: 
void fire(); 
void super_fire(); 
void move(int i); 
void init(float x, float y); 


}; 

void Hero::init( float x, float y)
{

	x_pos = x; 
	y_pos = y; 
	
}

void Hero::move(int i)
{
	switch(i)
	{
	case MOVE_UP:
		y_pos -= 3; 
		break; 

	case MOVE_DOWN:
		y_pos += 3; 
		break; 


	case MOVE_LEFT:
		x_pos -= 5; 
		break; 


	case MOVE_RIGHT:
		x_pos += 5; 
		break; 
			
	}
	
}




// 적 클래스 
class Enemy:public entity{
	
public: 
void fire(); 
void init(float x, float y); 
void move(); 

}; 

void Enemy::init(float x, float y)
{

	x_pos = x; 
	y_pos = y; 

}


void Enemy::move()
{
	y_pos += 2; 

}


// 총알 클래스 
class Bullet:public entity{
	
public: 
bool bShow; 

void init(float x, float y); 
void move(); 
bool show(); 
void hide(); 
void active(); 

}; 

void Bullet::init(float x, float y)
{
	x_pos = x;
	y_pos = y; 

}



bool Bullet::show()
{
	return bShow; 

}


void Bullet::active()
{
	bShow = true; 

}



void Bullet::move()
{
	y_pos -= 15; 
}

void Bullet::hide()
{
	bShow = false; 

}




//객체 생성 
Hero hero; 
Enemy enemy[ENEMY_NUM]; 
Bullet bullet; 



// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    HWND hWnd;
    WNDCLASSEX wc; //윈도우 정보 커서,스타일, 사이즈, 뒷배경 등

    ZeroMemory(&wc, sizeof(WNDCLASSEX)); //전체블록 NULL

    wc.cbSize = sizeof(WNDCLASSEX); //윈도우 크기할당
    wc.style = CS_HREDRAW | CS_VREDRAW; //윈도우 스타일
    wc.lpfnWndProc = (WNDPROC)WindowProc; //키보드, 마우스 등 행동
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); //마우스커서
    wc.lpszClassName = L"WindowClass";

    RegisterClassEx(&wc);

    hWnd = CreateWindowEx(NULL, L"WindowClass", L"Our Direct3D Program",
                          WS_EX_TOPMOST | WS_POPUP, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                          NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);

    // set up and initialize Direct3D
    initD3D(hWnd);


	//게임 오브젝트 초기화 
	init_game();

    // enter the main loop:

    MSG msg;

    while(TRUE)
    {
        DWORD starting_point = GetTickCount();

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

		do_game_logic();


        render_frame();

        // check the 'escape' key
        if(KEY_DOWN(VK_ESCAPE))
            PostMessage(hWnd, WM_DESTROY, 0, 0);




        while ((GetTickCount() - starting_point) < 25);
    }

    // clean up DirectX and COM
    cleanD3D();

    return msg.wParam;
}


// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            } break;
    }

    return DefWindowProc (hWnd, message, wParam, lParam);
}


// this function initializes and prepares Direct3D for use
void initD3D(HWND hWnd)
{
    d3d = Direct3DCreate9(D3D_SDK_VERSION);

    D3DPRESENT_PARAMETERS d3dpp;

    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = FALSE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = hWnd;
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    d3dpp.BackBufferWidth = SCREEN_WIDTH;
    d3dpp.BackBufferHeight = SCREEN_HEIGHT;


    // create a device class using this information and the info from the d3dpp stuct
    d3d->CreateDevice(D3DADAPTER_DEFAULT,
                      D3DDEVTYPE_HAL,
                      hWnd,
                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                      &d3dpp,
                      &d3ddev);

    D3DXCreateSprite(d3ddev, &d3dspt);    // create the Direct3D Sprite object

    


    D3DXCreateTextureFromFileEx(d3ddev,    // the device pointer
                                L"hero.png",    // the file name
                                D3DX_DEFAULT,    // default width
                                D3DX_DEFAULT,    // default height
                                D3DX_DEFAULT,    // no mip mapping
                                NULL,    // regular usage
                                D3DFMT_A8R8G8B8,    // 32-bit pixels with alpha
                                D3DPOOL_MANAGED,    // typical memory handling
                                D3DX_DEFAULT,    // no filtering
                                D3DX_DEFAULT,    // no mip filtering
                                D3DCOLOR_XRGB(255, 0, 255),    // the hot-pink color key
                                NULL,    // no image info struct
                                NULL,    // not using 256 colors
                                &sprite_hero);    // load to sprite

    D3DXCreateTextureFromFileEx(d3ddev,    // the device pointer
                                L"enemy.png",    // the file name
                                D3DX_DEFAULT,    // default width
                                D3DX_DEFAULT,    // default height
                                D3DX_DEFAULT,    // no mip mapping
                                NULL,    // regular usage
                                D3DFMT_A8R8G8B8,    // 32-bit pixels with alpha
                                D3DPOOL_MANAGED,    // typical memory handling
                                D3DX_DEFAULT,    // no filtering
                                D3DX_DEFAULT,    // no mip filtering
                                D3DCOLOR_XRGB(255, 0, 255),    // the hot-pink color key
                                NULL,    // no image info struct
                                NULL,    // not using 256 colors
                                &sprite_enemy);    // load to sprite


	D3DXCreateTextureFromFileEx(d3ddev,    // the device pointer
                                L"bullet.png",    // the file name
                                D3DX_DEFAULT,    // default width
                                D3DX_DEFAULT,    // default height
                                D3DX_DEFAULT,    // no mip mapping
                                NULL,    // regular usage
                                D3DFMT_A8R8G8B8,    // 32-bit pixels with alpha
                                D3DPOOL_MANAGED,    // typical memory handling
                                D3DX_DEFAULT,    // no filtering
                                D3DX_DEFAULT,    // no mip filtering
                                D3DCOLOR_XRGB(255, 0, 255),    // the hot-pink color key
                                NULL,    // no image info struct
                                NULL,    // not using 256 colors
                                &sprite_bullet);    // load to sprite

	D3DXCreateFont(d3ddev,    // the D3D Device
					30,    // font height of 30
					0,    // default font width
					FW_NORMAL,    // font weight
					1,    // not using MipLevels
					true,    // italic font
					DEFAULT_CHARSET,    // default character set
					OUT_DEFAULT_PRECIS,    // default OutputPrecision,
					DEFAULT_QUALITY,    // default Quality
					DEFAULT_PITCH | FF_DONTCARE,    // default pitch and family
					L"Arial",    // use Facename Arial
					&dxfont);    // the font object

	D3DXCreateFont(d3ddev,    // the D3D Device
					30,    // font height of 30
					0,    // default font width
					FW_NORMAL,    // font weight
					1,    // not using MipLevels
					true,    // italic font
					DEFAULT_CHARSET,    // default character set
					OUT_DEFAULT_PRECIS,    // default OutputPrecision,
					DEFAULT_QUALITY,    // default Quality
					DEFAULT_PITCH | FF_DONTCARE,    // default pitch and family
					L"Arial",    // use Facename Arial
					&dxfont2);    // the font object

	


    return;
}


void init_game(void)
{
	//객체 초기화 
	hero.init(150, 500); 

	//적들 초기화 
	for (int i=0; i<ENEMY_NUM; i++)
	{

		enemy[i].init((float) (rand()%300), rand()%200 - 300); 
	}

	//총알 초기화 
	bullet.init(hero.x_pos, hero.y_pos); 

}


void do_game_logic(void)
{

	//주인공 처리 
	/*if(KEY_DOWN(VK_UP))
		hero.move(MOVE_UP); 

	if(KEY_DOWN(VK_DOWN))
		hero.move(MOVE_DOWN); */

	if(KEY_DOWN(VK_LEFT))
		hero.move(MOVE_LEFT); 

	if(KEY_DOWN(VK_RIGHT))
		hero.move(MOVE_RIGHT); 


	//적들 처리 
	for (int i=0; i<ENEMY_NUM; i++)
	{
		if(enemy[i].y_pos > 500)
			enemy[i].init((float) (rand()%500), rand()%200 - 300);

		else
			enemy[i].move();

		if ( bullet.y_pos < enemy[i].y_pos+55 )
		{
			if (bullet.x_pos > enemy[i].x_pos-1 && bullet.x_pos < enemy[i].x_pos + 125)
			{
				bullet.hide();
				enemy[i].init((float)(rand() % 500), rand() % 200 - 300);
				score += 1;

			}
		}


		

		
	}


	//총알 처리 
	if(bullet.show() == false)
	{
		if(KEY_DOWN(VK_SPACE))
		{
			bullet.active(); 
			bullet.init(hero.x_pos+20, hero.y_pos);
		}


	}


	if( bullet.show() == true )
	{
		if(bullet.y_pos < -70)
			bullet.hide();
		else
			bullet.move();


	}
	
	


}

// this is the function used to render a single frame
void render_frame(void)
{
    // clear the window to a deep blue
    d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

    d3ddev->BeginScene();    // begins the 3D scene

    d3dspt->Begin(D3DXSPRITE_ALPHABLEND);    // // begin sprite drawing with transparency

	draw_display();

	// create a RECT to contain the text
	static RECT textbox; SetRect(&textbox, 0, 460, 250, 768);

	// draw the Hello World text
	dxfont->DrawTextA(NULL,
		"Score: ",
		20,
		&textbox,
		DT_CENTER | DT_VCENTER,
		D3DCOLOR_ARGB(255, 255, 255, 255));

	//UI 창 렌더링 


	/*
	static int frame = 21;    // start the program on the final frame
    if(KEY_DOWN(VK_SPACE)) frame=0;     // when the space key is pressed, start at frame 0
    if(frame < 21) frame++;     // if we aren't on the last frame, go to the next frame

    // calculate the x-position
    int xpos = frame * 182 + 1;

	RECT part;
    SetRect(&part, xpos, 0, xpos + 181, 128);
    D3DXVECTOR3 center(0.0f, 0.0f, 0.0f);    // center at the upper-left corner
    D3DXVECTOR3 position(150.0f, 50.0f, 0.0f);    // position at 50, 50 with no depth
    d3dspt->Draw(sprite, &part, &center, &position, D3DCOLOR_ARGB(127, 255, 255, 255));
	*/

	//주인공 
	RECT part;
	SetRect(&part, 0, 0, 64, 64);
    D3DXVECTOR3 center(0.0f, 0.0f, 0.0f);    // center at the upper-left corner
	D3DXVECTOR3 position(hero.x_pos, hero.y_pos, 0.0f);    // position at 50, 50 with no depth
	d3dspt->Draw(sprite_hero, &part, &center, &position, D3DCOLOR_ARGB(255, 255, 255, 255));

	////총알 
	if(bullet.bShow == true)
	{
		RECT part1;
		SetRect(&part1, 0, 0, 20, 23);
		D3DXVECTOR3 center1(0.0f, 0.0f, 0.0f);    // center at the upper-left corner
		D3DXVECTOR3 position1(bullet.x_pos, bullet.y_pos, 0.0f);    // position at 50, 50 with no depth
		d3dspt->Draw(sprite_bullet, &part1, &center1, &position1, D3DCOLOR_ARGB(255, 255, 255, 255));
	}


	////적 
	
	RECT part2;
	SetRect(&part2, 0, 0, 125, 45);
    D3DXVECTOR3 center2(0.0f, 0.0f, 0.0f);    // center at the upper-left corner

	for (int i=0; i<ENEMY_NUM; i++)
	{
		
		D3DXVECTOR3 position2(enemy[i].x_pos, enemy[i].y_pos, 0.0f);    // position at 50, 50 with no depth
		d3dspt->Draw(sprite_enemy, &part2, &center2, &position2, D3DCOLOR_ARGB(255, 255, 255, 255));
	}



    d3dspt->End();    // end sprite drawing

    d3ddev->EndScene();    // ends the 3D scene

    d3ddev->Present(NULL, NULL, NULL, NULL);

    return;
}


// this is the function that cleans up Direct3D and COM
void cleanD3D(void)
{
    sprite->Release();
    d3ddev->Release();
    d3d->Release();
	dxfont->Release();
	dxfont2->Release();

	//객체 해제 
    sprite_hero->Release();
	sprite_enemy->Release();
	sprite_bullet->Release();
	DisplayTexture->Release();

    return;
}



// this draws the display
void draw_display()
{
	RECT Part;


	// DRAW THE AMMO INDICATOR
	// display the backdrop
	SetRect(&Part, 351, 14, 456, 40);
	DrawTexture(DisplayTexture, Part, 530, 449, 127);

	SetRect(&Part, 351, 14, 456, 40);
	DrawTexture(DisplayTexture, Part, 530, 449, 127);

	// display the font
	SetRect(&Part, 50, 600, 100, 670);
	static char strAmmoText[10];
	_itoa_s(score, strAmmoText, 10);
	dxfont2->DrawTextA(NULL,
		(LPCSTR)&strAmmoText,
		strlen((LPCSTR)&strAmmoText),
		&Part,
		DT_RIGHT,
		D3DCOLOR_ARGB(255, 120, 120, 255));

	return;
}


// this loads a texture from a file
void LoadTexture(LPDIRECT3DTEXTURE9* texture, LPCTSTR filename)
{
	
		(D3DX_DEFAULT, NULL, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_DEFAULT,
		D3DX_DEFAULT, D3DCOLOR_XRGB(255, 0, 255), NULL, NULL, texture);

	return;
}


// this draws a portion of the specified texture
void DrawTexture(LPDIRECT3DTEXTURE9 texture, RECT texcoords, float x, float y, int a)
{
	D3DXVECTOR3 center(0.0f, 0.0f, 0.0f), position(x, y, 0.0f);
	d3dspt->Draw(texture, &texcoords, &center, &position, D3DCOLOR_ARGB(a, 255, 255, 255));

	return;
}