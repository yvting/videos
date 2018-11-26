/*
	OneLoneCoder.com - Command Line Tetris
	"Put Your Money Where Your Mouth Is" - @Javidx9
	
	License
	~~~~~~~
	Copyright (C) 2018  Javidx9
	This program comes with ABSOLUTELY NO WARRANTY.
	This is free software, and you are welcome to redistribute it
	under certain conditions; See license for details. 
	Original works located at:
	https://www.github.com/onelonecoder
	https://www.onelonecoder.com
	https://www.youtube.com/javidx9

	GNU GPLv3
	https://github.com/OneLoneCoder/videos/blob/master/LICENSE

	From Javidx9 :)
	~~~~~~~~~~~~~~~
	Hello! Ultimately I don't care what you use this for. It's intended to be 
	educational, and perhaps to the oddly minded - a little bit of fun. 
	Please hack this, change it and use it in any way you see fit. You acknowledge 
	that I am not responsible for anything bad that happens as a result of 
	your actions. However this code is protected by GNU GPLv3, see the license in the
	github repo. This means you must attribute me if you use it. You can view this
	license here: https://github.com/OneLoneCoder/videos/blob/master/LICENSE
	Cheers!
	
	Background
	~~~~~~~~~~
	I made a video "8-Bits of advice for new programmers" (https://youtu.be/vVRCJ52g5m4)
	and suggested that building a tetris clone instead of Dark Sould IV might be a better 
	approach to learning to code. Tetris is nice as it makes you think about algorithms. 
	
	Controls are Arrow keys Left, Right & Down. Use Z to rotate the piece. 
	You score 25pts per tetronimo, and 2^(number of lines)*100 when you get lines.
	
	Future Modifications
	~~~~~~~~~~~~~~~~~~~~
	1) Show next block and line counter
	
	Author
	~~~~~~
	Twitter: @javidx9
	Blog: www.onelonecoder.com
	
	Video:
	~~~~~~
	https://youtu.be/8OK8_tHeCIA
	
	Last Updated: 30/03/2017
*/

#include <iostream>
#include <thread>
#include <vector>
using namespace std;

#include <string.h>
#include <stdio.h>
#include <ncurses.h>
// #include <Windows.h>

typedef struct _WIN_struct {
	int startx, starty;
	int height, width;
}WIN;

/* ncurses console */
void init_ncurses ()
{
  initscr();			/* Start curses mode 		*/
  cbreak();			/* Line buffering disabled, Pass on
                                 * everty thing to me 		*/
  keypad(stdscr, TRUE);		/* I need that nifty F1 	*/
  noecho();
  curs_set(0);
  timeout(10);
  start_color();
  init_pair(1,COLOR_WHITE, COLOR_BLACK);
  init_pair(2,COLOR_RED, COLOR_BLACK);
  init_pair(3,COLOR_GREEN, COLOR_BLACK);
  init_pair(4,COLOR_YELLOW, COLOR_BLACK);
  init_pair(5,COLOR_BLUE, COLOR_BLACK);
  init_pair(6,COLOR_MAGENTA, COLOR_BLACK);
  init_pair(7,COLOR_CYAN, COLOR_BLACK);
}

int init_win_params (WIN* p_win, int sx, int sy, int wd, int ht)
{
  p_win->startx = sx;
  p_win->starty = sy;
  p_win->width  = wd;
  p_win->height = ht;

  if (p_win->startx + p_win->width > COLS ||
      p_win->starty + p_win->height > LINES) {
    return 1;
  }
  else 
    return 0;
}

void write_console_output (wchar_t* screen, WIN* p_win)
{
  int sx = p_win->startx, 
      sy = p_win->starty,
      wd = p_win->width,
      ht = p_win->height;
  int y, x, c;
  for (y = 0; y < ht; y++)
    for (x = 0; x < wd; x++) {
      c = screen[y*wd+x];
      mvaddch(sy+y,sx+x,c);
    }
  refresh();
}

int read_input(bool* bKey)
{
  int c, ext;
  c = getch();
  memset(bKey, 0, sizeof(bool)*4);

  ext = 0;
  switch (c) {
    case KEY_RIGHT:
      bKey[0] = TRUE;
      break;
    case KEY_LEFT:
      bKey[1] = TRUE;
      break;
    case KEY_DOWN:
      bKey[2] = TRUE;
      break;
    case KEY_UP:
      bKey[3] = TRUE;
      break;
    case 'Q':
    case 'q':
      ext = 1;
      break;
  }

  return ext;
}

int color_tile(int c) 
{
  if (c == '#' || c == '=') {
    return (c | A_BOLD);
  } else if (c >= 'A' && c <= 'G') {
    return (c | COLOR_PAIR (2 + c%6));
  }
  return c;
}

int nScreenWidth = 80;			// Console Screen Size X (columns)
int nScreenHeight = 30;			// Console Screen Size Y (rows)
wstring tetromino[7];
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char *pField = nullptr;

int Rotate(int px, int py, int r)
{
	int pi = 0;
	switch (r % 4)
	{
	case 0: // 0 degrees			// 0  1  2  3
		pi = py * 4 + px;			// 4  5  6  7
		break;						// 8  9 10 11
									//12 13 14 15

	case 1: // 90 degrees			//12  8  4  0
		pi = 12 + py - (px * 4);	//13  9  5  1
		break;						//14 10  6  2
									//15 11  7  3

	case 2: // 180 degrees			//15 14 13 12
		pi = 15 - (py * 4) - px;	//11 10  9  8
		break;						// 7  6  5  4
									// 3  2  1  0

	case 3: // 270 degrees			// 3  7 11 15
		pi = 3 - py + (px * 4);		// 2  6 10 14
		break;						// 1  5  9 13
	}								// 0  4  8 12

	return pi;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
	// All Field cells >0 are occupied
	for (int px = 0; px < 4; px++)
		for (int py = 0; py < 4; py++)
		{
			// Get index into piece
			int pi = Rotate(px, py, nRotation);

			// Get index into field
			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			// Check that test is in bounds. Note out of bounds does
			// not necessarily mean a fail, as the long vertical piece
			// can have cells that lie outside the boundary, so we'll
			// just ignore them
			if (nPosX + px >= 0 && nPosX + px < nFieldWidth)
			{
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
				{
					// In Bounds so do collision check
					if (tetromino[nTetromino][pi] != L'.' && pField[fi] != 0)
						return false; // fail on first hit
				}
			}
		}

	return true;
}

int main()
{
	// Create Screen Buffer
	wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];
        WIN win;
	for (int i = 0; i < nScreenWidth*nScreenHeight; i++) screen[i] = L' ';
        init_ncurses();
        if (init_win_params(&win, 0, 1, nScreenWidth, nScreenHeight)) {
          endwin();
          delete[] screen;
          cout << "The size of the console window is too small!" << endl;
          return 0;
        }
        attron(A_BLINK | A_BOLD | COLOR_PAIR(1));
        printw("Press 'q' to end the game..." );
        refresh();
        attroff(A_BLINK | A_BOLD | COLOR_PAIR(1));

	// HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	// SetConsoleActiveScreenBuffer(hConsole);
	// DWORD dwBytesWritten = 0;
	
	tetromino[0].append(L"..X...X...X...X."); // Tetronimos 4x4
	tetromino[1].append(L"..X..XX...X.....");
	tetromino[2].append(L".....XX..XX.....");
	tetromino[3].append(L"..X..XX..X......");
	tetromino[4].append(L".X...XX...X.....");
	tetromino[5].append(L".X...X...XX.....");
	tetromino[6].append(L"..X...X..XX.....");

	pField = new unsigned char[nFieldWidth*nFieldHeight]; // Create play field buffer
	for (int x = 0; x < nFieldWidth; x++) // Board Boundary
		for (int y = 0; y < nFieldHeight; y++)
			pField[y*nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

	// Game Logic
	bool bKey[4];
	int nCurrentPiece = 0;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;
	int nCurrentY = 0;
	int nSpeed = 20;
	int nSpeedCount = 0;
	bool bForceDown = false;
	bool bRotateHold = true;
	int nPieceCount = 0;
	int nScore = 0;
	vector<int> vLines;
	bool bGameOver = false;

	while (!bGameOver) // Main Loop
	{
		// Timing =======================
		this_thread::sleep_for(std::chrono::milliseconds(50)); // Small Step = 1 Game Tick
		nSpeedCount++;
		bForceDown = (nSpeedCount == nSpeed);

		// Input ========================
                if (read_input(bKey)) break;
		// for (int k = 0; k < 4; k++)								// R   L   D Z
		// 	bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
		
		// Game Logic ===================

		// Handle player movement
		nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
		nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;		
		nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

		// Rotate, but latch to stop wild spinning
		if (bKey[3])
		{
			nCurrentRotation += (bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHold = false;
		}
		else
			bRotateHold = true;

		// Force the piece down the playfield if it's time
		if (bForceDown)
		{
			// Update difficulty every 50 pieces
			nSpeedCount = 0;
			nPieceCount++;
			if (nPieceCount % 50 == 0)
				if (nSpeed >= 10) nSpeed--;
			
			// Test if piece can be moved down
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
				nCurrentY++; // It can, so do it!
			else
			{
				// It can't! Lock the piece in place
				for (int px = 0; px < 4; px++)
					for (int py = 0; py < 4; py++)
						if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.')
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;

				// Check for lines
				for (int py = 0; py < 4; py++)
					if(nCurrentY + py < nFieldHeight - 1)
					{
						bool bLine = true;
						for (int px = 1; px < nFieldWidth - 1; px++)
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

						if (bLine)
						{
							// Remove Line, set to =
							for (int px = 1; px < nFieldWidth - 1; px++)
								pField[(nCurrentY + py) * nFieldWidth + px] = 8;
							vLines.push_back(nCurrentY + py);
						}						
					}

				nScore += 25;
				if(!vLines.empty())	nScore += (1 << vLines.size()) * 100;

				// Pick New Piece
				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;

				// If piece does not fit straight away, game over!
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}
		}

		// Display ======================

		// Draw Field
		for (int x = 0; x < nFieldWidth; x++)
			for (int y = 0; y < nFieldHeight; y++) {
                          screen[(y + 2)*nScreenWidth + (x + 2)] = 
                            color_tile(L" ABCDEFG=#"[pField[y*nFieldWidth + x]]);
                        }

		// Draw Current Piece
		for (int px = 0; px < 4; px++)
			for (int py = 0; py < 4; py++)
				if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.') {
					screen[(nCurrentY + py + 2)*nScreenWidth + (nCurrentX + px + 2)] = 
                                          color_tile(nCurrentPiece + 65);
                                }

		// Draw Score
		swprintf(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

		// Animate Line Completion
		if (!vLines.empty())
		{
			// Display Frame (cheekily to draw lines)
			// WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
                        write_console_output(screen, &win);
			this_thread::sleep_for(std::chrono::milliseconds(400)); // Delay a bit

			for (auto &v : vLines)
				for (int px = 1; px < nFieldWidth - 1; px++)
				{
					for (int py = v; py > 0; py--)
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					pField[px] = 0;
				}

			vLines.clear();
		}

		// Display Frame
		// WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
                write_console_output(screen, &win);
	}

	// Oh Dear
	// CloseHandle(hConsole);
        endwin();
        delete[] screen;
        delete[] pField;
	cout << "Game Over!! Score:" << nScore << endl;
	return 0;
}

