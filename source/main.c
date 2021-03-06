//Programmation systemes
//TP6
//Heloy Estevanin


#include <gba.h>
#include <stdlib.h>
#include <stdio.h>
#include "font.h"

typedef unsigned short u16;

//macro to change the video mode
#define SetMode(mode) REG_DISPCNT = (mode)

//define some video addresses
#define REG_DISPCNT *(volatile unsigned short*)0x4000000

//create a pointer to the video buffer
unsigned short* videoBuffer = (unsigned short*)0x6000000;

#define BGPaletteMem ((unsigned short*)0x5000000)

#define REG_VCOUNT *(volatile unsigned short*)0x4000006

#define REG_DISPSTAT *(volatile unsigned short *)0x4000004

//define object attribute memory state address
#define SpriteMem ((unsigned short*)0x7000000)

//define object attribute memory image address
#define SpriteData ((unsigned short*)0x6010000)

//define object attribute memory palette address
#define SpritePal ((unsigned short*)0x5000200)

//misc sprite constants
#define OBJ_MAP_2D 0x0
#define OBJ_MAP_1D 0x40
#define OBJ_ENABLE 0x1000

//attribute0 stuff
#define ROTATION_FLAG 0x100
#define SIZE_DOUBLE 0x200
#define MODE_NORMAL 0x0
#define MODE_TRANSPARENT 0x400
#define MODE_WINDOWED 0x800
#define MOSAIC 0x1000
#define COLOR_16 0x0000
#define COLOR_256 0x2000
#define SQUARE 0x0
#define TALL 0x4000
#define WIDE 0x8000

//screen
#define SCREEN_HEIGHT 160
#define SCREEN_WIDTH 240

//attribute1 stuff
#define HORIZONTAL_FLIP 0x1000
#define VERTICAL_FLIP 0x2000
#define SIZE_8 0x0
#define SIZE_16 0x4000
#define SIZE_32 0x8000
#define SIZE_64 0xC000

//attribute2 stuff
#define PRIORITY(n) ((n) << 10)
#define PALETTE(n) ((n) << 12)



#define N_SPRITES 128


//mask for the timer
#define INT_TIMER0 0x0008
#define INT_TIMER1 0x0010
//interrupts
#define REG_IME *(u16 *)0x4000208
#define REG_IE *(u16 *)0x4000200
#define REG_IF *(volatile u16 *)0x4000202
#define REG_INTERRUPT *(u32 *)0x3007FFC

//timers adresses
#define REG_TM0CNT *(volatile u16 *)0x4000102
#define REG_TM1CNT *(volatile u16 *)0x4000106
#define REG_TM2CNT *(volatile u16*)0x400010A
#define REG_TM3CNT *(volatile u16*)0x400010E

//to read the values generated by the timers
#define REG_TM0D *(volatile u16*)0x4000100
#define REG_TM1D *(volatile u16*)0x4000104
#define REG_TM2D *(volatile u16*)0x4000108
#define REG_TM3D *(volatile u16*)0x400010C

#define TIMER_OVERFLOW 0x4
#define TIMER_IRQ_ENABLE 0x40
#define TIMER_ENABLE 0x80

//frequencies
#define TIMER_FREQUENCY_SYSTEM	0x0
#define TIMER_FREQUENCY_64	0x1
#define TIMER_FREQUENCY_256	0x2
#define TIMER_FREQUENCY_1024	0x3



//define boolean
#define bool short
#define true 1
#define false 0

#define INT_BUTTON 0x1000
volatile unsigned int *BUTTONS = (volatile unsigned int *)0x04000130;

#define BUTTON_A 1
#define BUTTON_B 2
#define BUTTON_SELECT 4
#define BUTTON_START 8
#define BUTTON_RIGHT 16
#define BUTTON_LEFT 32
#define BUTTON_UP 64
#define BUTTON_DOWN 128
#define BUTTON_R 256
#define BUTTON_L 512

bool buttons[10];


//sprite structs
typedef struct tagSprite
{
	unsigned short attribute0;
	unsigned short attribute1;
	unsigned short attribute2;
	unsigned short attribute3;
}Sprite,*pSprite;


char paddle[]={
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


char ball[]={
	0,0,1,1,1,1,0,0,
	0,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,0,
	0,0,1,1,1,1,0,0,
};


//create an array of 128 sprites equal to OAM
Sprite sprites[N_SPRITES];

//function prototypes
void WaitForVsync(void);
void UpdateSpriteMemory(void);
void MyHandler(void);
void CheckButtons();
bool Pressed(int);
void PlotPixel(int,int, unsigned short int);
void DrawChar(int, int, char, unsigned short);
void Print(int, int, char *, unsigned short);


signed short x = 140, y = 150, x_ball = 5, y_ball = 15
,y_speed=2,x_speed=2,difficulty=1;
signed int counter=0;
bool start = true;	//affiche ou non l'écran d'accueil


int main(void) {


	int n;

	//set the video mode--mode 2 with sprites
	SetMode(2 | OBJ_ENABLE | OBJ_MAP_1D);


	//move all sprites offscreen to hide them
	for(n = 0; n < N_SPRITES; n++)
	{
		sprites[n].attribute0 = 160;
		sprites[n].attribute1 = 240;
	}

	for(n = 0; n < 64; n++){

		SpritePal[n] = RGB8(255,255,255);
	}



	//copy the sprite image into memory
	for(n = 0; n < 256; n++) {
		SpriteData[n+(0*32)] =  ((short*)paddle)[n];
	}


	//copy the sprite image into memory
	for(n = 0; n < 64; n++) {
		SpriteData[n+(1*128)] =  ((short*)ball)[n];

	}

	//setup the first sprite
	sprites[0].attribute0 = COLOR_256 | y;
	sprites[0].attribute1 = SIZE_16 | x;
	sprites[0].attribute2 = 0;




	sprites[1].attribute0 = COLOR_256 | y_ball;
	sprites[1].attribute1 = SIZE_8 | x_ball ;
	sprites[1].attribute2 = 8;




	REG_IME=0x0;					//Disable the interupts until we setup an interupt
	REG_INTERRUPT=(u32*)MyHandler;	//Setting the interupt handle
	REG_TM0D=0;
	REG_TM0CNT|=TIMER_FREQUENCY_SYSTEM| TIMER_ENABLE|TIMER_IRQ_ENABLE;
	REG_TM1D=0xFFFC;
	REG_TM1CNT|=TIMER_OVERFLOW| TIMER_ENABLE|TIMER_IRQ_ENABLE;;
	REG_IE|= INT_TIMER1;
	REG_IME=0x1;					//Enable the inturupt back again



	while(1)
	{
		int a = (rand()*100)%150;
		PlotPixel(counter,50,RGB8(0,255,255));
		int delay = 500;
		CheckButtons();

		if (Pressed(BUTTON_RIGHT)){
			if (x < 230)
			{
				x+=5;
				sprites[0].attribute1 = SIZE_16 | x;

			}

		}

		if (Pressed(BUTTON_LEFT))
		{
			if (x>3)
			{
				x-=5;
				sprites[0].attribute1 = SIZE_16 | x;
			}

		}

		if(Pressed(BUTTON_START)){
			start=!start;
		}

		delay = 500;
		while (delay--);


		//wait for vertical retrace
		WaitForVsync();

		//display the sprite
		UpdateSpriteMemory();

	}
}


void WaitForVsync(void)
{
	while((REG_DISPSTAT & 1));
}

void UpdateSpriteMemory(void)
{
	int n;
	unsigned short* temp;
	temp = (unsigned short*)sprites;
	for(n = 0; n < 128*4; n++){
		SpriteMem[n] = temp[n];
	}

}


// Function: CheckButtons
// Polls the status of all the buttons
void CheckButtons()
{
	//store the status of the buttons in an array
	buttons[0] = !((*BUTTONS) & BUTTON_A);
	buttons[1] = !((*BUTTONS) & BUTTON_B);
	buttons[2] = !((*BUTTONS) & BUTTON_LEFT);
	buttons[3] = !((*BUTTONS) & BUTTON_RIGHT);
	buttons[4] = !((*BUTTONS) & BUTTON_UP);
	buttons[5] = !((*BUTTONS) & BUTTON_DOWN);
	buttons[6] = !((*BUTTONS) & BUTTON_START);
	buttons[7] = !((*BUTTONS) & BUTTON_SELECT);
	buttons[8] = !((*BUTTONS) & BUTTON_L);
	buttons[9] = !((*BUTTONS) & BUTTON_R);
}

// Function: Pressed
// Returns the status of a button
bool Pressed(int button)
{
	switch(button)
	{
		case BUTTON_A: return buttons[0];
		case BUTTON_B: return buttons[1];
		case BUTTON_LEFT: return buttons[2];
		case BUTTON_RIGHT: return buttons[3];
		case BUTTON_UP: return buttons[4];
		case BUTTON_DOWN: return buttons[5];
		case BUTTON_START: return buttons[6];
		case BUTTON_SELECT: return buttons[7];
		case BUTTON_L: return buttons[8];
		case BUTTON_R: return buttons[9];
	}
	return false;
}


/**
Interupt handler
*/
void MyHandler(void){
	counter+=difficulty;
	u16 Int_Flag;

	REG_IME=0X0;

	Int_Flag = REG_IF;

Print(100, 100, "PONG",RGB8(255,255,255));

	if ((REG_IF & INT_TIMER1)==INT_TIMER1){
		//if(counter>10){
		if(start){



			//CHANGE THIS!

			if(y_ball>y&&(x_ball<x||x_ball>x)){
				y_speed=-y_speed;
			}

			if(y_ball<5)
			y_speed=-y_speed;
			if(x_ball>SCREEN_WIDTH)
			x_speed=-x_speed;
			if(x_ball<0)
			x_speed=-x_speed;

			y_ball+=y_speed;
			x_ball+=x_speed;


			sprites[1].attribute0 = COLOR_256 | y_ball;
			sprites[1].attribute1= SIZE_8|x_ball;
			counter%=100;
		}

	}


	REG_IF = Int_Flag;
	REG_IME = 0X1;
}

void DrawChar(int left, int top, char letter, unsigned short color){

	int x, y;
	int draw;
	for(y = 0; y < 8; y++)
	for (x = 0; x < 8; x++)
	{
		// grab a pixel from the font char
		draw = font[(letter-32) * 64 + y * 8 + x];
		// if pixel = 1, then draw it
		if (draw)
		PlotPixel(left + x, top + y, color);
	}
}

//draw text using characters contained in font.h
void Print(int left, int top, char *str, unsigned short color)
{
    int x, y, draw;
    int pos = 0;
    char letter;

    //look at all characters in this string
    while (*str)
    {
        //get current character ASCII code
        letter = (*str++) - 32;

        //draw the character
        for(y = 0; y < 8; y++)
            for(x = 0; x < 8; x++)
            {
                //grab a pixel from the font character
                draw = font[letter * 64 + y * 8 + x];

                //if pixel = 1, then draw it
                if (draw)
                    PlotPixel(left + pos + x, top + y, color);
            }

        //jump over 8 pixels
        pos += 8;
    }
}


void PlotPixel(int x,int y, unsigned short int couleur) {
	videoBuffer[ y*240 + x] = couleur;
}
