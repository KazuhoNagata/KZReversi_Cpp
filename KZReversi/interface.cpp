/*#
  # ��ʕ`�ʊ֘A:�{�[�h�A�X�e�[�^�X�o�[�Ȃ�
  #  
  ##############################################
*/

#include "stdafx.h"
#include "KZReversi.h"
#include "GetRev.h"
#include "AI.h"
#include "GetPattern.h"
#include "book.h"
#include "hint.h"
#include "game.h"
#include "interface.h"

#define BOARD_COLOR RGB(100, 100, 100)

BitBoard UndoBoard_B[120];
BitBoard UndoBoard_W[120];
int Undo_x[120];
int Undo_y[120];
int Undo_color[120];

int print_eval_flag;
int Flag_Hint;

char Diff_Str[24];

void read_eval_table(int progress)
{
	// nothing to do.
}

void save_undoboard(BitBoard bk, BitBoard wh, int x, int y, int turn)
{
	UndoBoard_B[turn] = bk;
	UndoBoard_W[turn] = wh;
	Undo_x[turn] = x;
	Undo_y[turn] = y;
	Undo_color[turn] = NowTurn;
}

BOOL is_max_eval(double eval)
{
	int i = 0;
	while(EvalData[i] != NEGAMAX)
	{
		if(eval < EvalData[i])
		{
			return FALSE;
		}
		i++;
	}
	return TRUE;
}

int best_x;
int best_y;
char best_eval_str[8];

void Print_HintEval(HDC hdc, double eval, int x, int y, int flag)
{
	char eval_str[8];
	SetBkColor(hdc, RGB(30, 100, 50));

	/* �ׂ����C���^�t�F�[�X(-0.67 �Ƃ� 0.45 ��(+0 ,-0)�̕����t�ŋ�ʂ���) */
	/* �]���l�̕\�� */
	if(flag == 0)
	{
		if(eval == BOOK_MOVE)
		{
			/* ��΂͗� */
			SetTextColor(hdc, RGB(150, 200, 150));
			sprintf_s(eval_str, "Bok", (int )eval);
		}
		else if(is_max_eval(eval))
		{
			/* �O�̐��F�X�R�A�����F�� */
			if(best_x >= 0)
			{
				SetTextColor(hdc, RGB(250, 250, 0));
				TextOut(hdc, best_x * MASS_SIZE + 8, (best_y + 1) * MASS_SIZE + 11, best_eval_str, strlen(best_eval_str));
			}

			/* �ł������X�R�A�͐��F�ɂ��� */
			SetTextColor(hdc, RGB(0, 250, 250));

			/* -0 �ƕ\������ꍇ*/
			if(eval < 0.0 && eval > -1.0)
			{
				sprintf_s(eval_str, "-%d", (int )eval);
			}
			else
			{
				sprintf_s(eval_str, "%+d", (int )eval);
			}
			/* ���F�X�R�A���L�� */
			best_x = x;
			best_y = y;
			strcpy_s(best_eval_str, eval_str);
		}
		else
		{
			/* �ł������X�R�A�ȊO�͉��F */
			SetTextColor(hdc, RGB(250, 250, 0));
			/* -0 �ƕ\������ꍇ*/
			if(eval < 0.0 && eval > -1.0)
			{
				sprintf_s(eval_str, "-%d", (int )eval);
			}
			else
			{
				sprintf_s(eval_str, "%+d", (int )eval);
			}
		}
	}
	else if(flag == 1)
	{
		/* ���s�̕\�� */
		if(eval == WIN)
		{
			SetTextColor(hdc, RGB(0, 250, 250));
			sprintf_s(eval_str, "WIN");
		}
		else if(eval == LOSS)
		{
			SetTextColor(hdc, RGB(250, 0, 100));
			sprintf_s(eval_str, "LOS");
		}
		else
		{
			SetTextColor(hdc, RGB(250, 250, 0));
			sprintf_s(eval_str, "DRW");
		}
	}
	else
	{
		/* �΍��\�� */
		if(is_max_eval(eval))
		{
			/* �ł������X�R�A�͐��F�ɂ��� */
			SetTextColor(hdc, RGB(0, 250, 250));
		}
		else
		{
			/* �ł������X�R�A�ȊO�͉��F */
			SetTextColor(hdc, RGB(250, 250, 0));
		}
		/* �΍��O�̏ꍇ*/
		if(eval == 0)
		{
			sprintf_s(eval_str, "�}%d", (int )eval);
		}
		else
		{
			sprintf_s(eval_str, "%+d", (int )eval);
		}
	}

	/* ��U���� */
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	SelectObject(hdc, hPen);
	HBRUSH hBrush = CreateHatchBrush(HS_CROSS, RGB(30, 100, 50));
	SelectObject(hdc, hBrush);

	Rectangle(hdc, x * MASS_SIZE, (y + 1) * MASS_SIZE, (x + 1) * MASS_SIZE, (y + 2) * MASS_SIZE);
	/* �`�ʏ��� */
	TextOut(hdc, x * MASS_SIZE + 8, (y + 1) * MASS_SIZE + 11, eval_str, lstrlen(eval_str));
}

void disp_eval()
{
	if(AI_THINKING)
	{
		return;
	}
	char msg[128];
	/* ���s�T���� */
	if(!m_FlagPerfectSearch && m_FlagSearchWin)
	{
		if(Flag_Abort && abs(MAXEVAL) > WIN)
		{
			sprintf_s(msg, LEN, "%+0.3f?", (double)MAXEVAL / EVAL_ONE_STONE);
		}
		else if(MAXEVAL == WIN)
		{
			sprintf_s(msg, "%s", "WIN");
		}
		else if(MAXEVAL == LOSS)
		{ 
			sprintf_s(msg, "%s", "LOSS");
		}
		else
		{
			sprintf_s(msg, "%s", "DRAW");
		}
		SendMessage(hStatus, SB_SETTEXT, (WPARAM)2 | 0, (LPARAM)msg);
	}
	/* �΍��T���� */
	else if(m_FlagPerfectSearch)
	{
		if(Flag_Abort && abs(MAXEVAL) > 64)
		{
			sprintf_s(msg, LEN, "%+0.3f?", (double)MAXEVAL / EVAL_ONE_STONE);
		}
		else if(MAXEVAL > 0)
		{
			sprintf_s(msg, LEN, "+%d(WIN)", MAXEVAL);
		}
		else if(MAXEVAL < 0)
		{
			sprintf_s(msg, LEN, "%d(LOSS)", MAXEVAL);
		}
		else
		{
			sprintf_s(msg, LEN, "%d(DRAW)", 0);
		}
		SendMessage(hStatus, SB_SETTEXT, (WPARAM)2 | 0, (LPARAM)msg);
	}
	else if(m_FlagBook)
	{
		sprintf_s(msg, LEN, "%+0.3f(book)", (double)MAXEVAL / EVAL_ONE_STONE);
		SendMessage(hStatus, SB_SETTEXT, (WPARAM)2 | 0, (LPARAM)msg);
	}
	/* ����ȊO */
	else
	{
		if(Flag_Abort)
		{
			sprintf_s(msg, "%+0.3f[%s]?", (double)MAXEVAL / EVAL_ONE_STONE, cordinates_table[MAX_MOVE]);
		}
		else
		{
			sprintf_s(msg, "%+0.3f[%s]", (double)MAXEVAL / EVAL_ONE_STONE, cordinates_table[MAX_MOVE]);
		}
		SendMessage(hStatus, SB_SETTEXT, (WPARAM)2 | 0, (LPARAM)msg);
	}
}

/* ��ʕ`��֐� */
void DrawBoard(HDC hdc, HWND hStatus, int move_x, int move_y)
{
	int x, y;
	int PassFlag = 1;
	HBRUSH green_brush, black_brush, white_brush;
	
	static LOGFONT logfont;  // �t�H���g���̍\����
	HFONT hFont;
	ZeroMemory( &logfont, sizeof(logfont) );  // �t�H���g���\���̂��O�ŏ�����
	logfont.lfCharSet = (BYTE )GetTextCharset(hdc);	  //��������Ȃ��Ɠ��{��\���ł��Ȃ��E�E�E
	logfont.lfQuality = ANTIALIASED_QUALITY;
	logfont.lfHeight = 16;
    logfont.lfWidth = 8; 
	strcpy_s(logfont.lfFaceName, "�l�r ����"); 
	hFont = CreateFontIndirect( &logfont );   // �_���t�H���g���쐬����
	SelectObject( hdc, hFont );

	HPEN pen_black = (HPEN )GetStockObject( BLACK_PEN );
	HPEN pen_red = (HPEN)CreatePen(PS_SOLID, 1, RGB(230, 0, 0) );
	//HPEN pen_yellow = (HPEN)CreatePen(PS_SOLID, 1, RGB(200, 200, 50) );
	white_brush = (HBRUSH )GetStockObject( WHITE_BRUSH );
	//green_brush = CreateSolidBrush( RGB(30, 100, 50) );
	green_brush = CreateSolidBrush( BOARD_COLOR );
	SelectObject(hdc, green_brush);
	black_brush = CreateSolidBrush( RGB(0, 0, 0) );
	SetBkColor(hdc, BOARD_COLOR);
	SetTextColor(hdc, RGB(200, 0, 0));

	int move_cuont;

	BitBoard moves_b = CreateMoves(black, white, &move_cuont);
	BitBoard moves_w = CreateMoves(white, black, &move_cuont);

	for(x = 0; x < MASS_NUM; x++){
		for(y = 0; y < MASS_NUM; y++){
			/* �O�̎�𖾎����� */
			if(x == move_x && y == move_y)
			{
				SelectObject(hdc, pen_red);
				Rectangle(hdc, x * MASS_SIZE, (y + 1) * MASS_SIZE, 
						   (x + 1) * MASS_SIZE, (y + 2) * MASS_SIZE);
				SelectObject(hdc, pen_black);
			}
			else
			{
				SelectObject(hdc, pen_black);
				Rectangle(hdc, x * MASS_SIZE, (y + 1) * MASS_SIZE, 
						   (x + 1) * MASS_SIZE, (y + 2) * MASS_SIZE);
			}

			if(~(black | white) & one << (x * MASS_NUM + y)){
				if(m_FlagInGame && !m_FlagInterrupt){
					/* �q���gOFF�� */
					if(!Flag_Hint)
					{
						if(player[NowTurn] != CPU && NowTurn && moves_w & (one << (x * MASS_NUM + y)))
						{
							PassFlag = 0;
							TextOut(hdc, x * MASS_SIZE + 12, (y + 1) * MASS_SIZE + 11, "��", lstrlen("��"));
						}
						else if(player[NowTurn] != CPU && !NowTurn && moves_b & (one << (x * MASS_NUM + y)))
						{
							PassFlag = 0;
							TextOut(hdc, x * MASS_SIZE + 12, (y + 1) * MASS_SIZE + 11, "��", lstrlen("��"));
						}
					}
				}
				continue;
			}
			else if(black & one << (x * MASS_NUM + y)){
				SelectObject(hdc, black_brush);
			}
			else if(white & one << (x * MASS_NUM + y)){
				SelectObject(hdc, white_brush);
			}
			
			Ellipse(hdc, x * MASS_SIZE + 3, (y + 1) * MASS_SIZE + 3,
				         (x + 1) * MASS_SIZE - 4, (y + 2) * MASS_SIZE - 4);
			SelectObject(hdc, green_brush);
		}
	}

	/* ��ʂɕ\�� */
	int i = 0;
	while(!m_FlagInterrupt && EvalData[i] != NEGAMAX)
	{
		/* �ۑ�����Ă���]���l��\�� */
		Print_HintEval(hdc, EvalData[i], PosData[i] / 8, PosData[i] % 8, print_eval_flag);
		i++;
	}

	SelectObject(hdc, &white_brush);
	SetBkColor(hdc, RGB(255, 255, 255));
	SetTextColor(hdc, RGB(0, 0, 0));

	char move_who[][4] = {"",""};
	/* ���̎�Ԃ�\�� */
	if(m_FlagInGame)
	{
		if(NowTurn == BLACK)
		{
			strcpy_s(move_who[BLACK], "=>");
		}
		else
		{
			strcpy_s(move_who[WHITE], "=>");
		}
	}

	char str[LEN * 2];
	if(Flag_Edit)
	{
		sprintf_s(str, LEN, "Edit Mode");
		SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 0, (LPARAM)str);
		TextOut(hdc, BOARD_SIZE + 10, 100, "��", lstrlen("��"));
		TextOut(hdc, BOARD_SIZE + 10, 125, "��", lstrlen("��"));

	}
	else
	{
		sprintf_s(str, LEN, "%d �^�[����   ", turn + 1);
		TextOut(hdc, BOARD_SIZE + 10, 60, str, strlen(str));

		/*if(CpuColor == BLACK)
		{
			sprintf_s(str, LEN, "%s�� CPU(%s)           ", move_who[BLACK], Diff_Str);
			TextOut(hdc, BOARD_SIZE + 10, 100, str, strlen(str));
			sprintf_s(str, LEN, "%s�� Player           ", move_who[WHITE]);
			TextOut(hdc, BOARD_SIZE + 10, 125, str, strlen(str));
		}
		else
		{
			sprintf_s(str, LEN, "%s�� Player           ", move_who[BLACK]);
			TextOut(hdc, BOARD_SIZE + 10, 100, str, strlen(str));
			sprintf_s(str, LEN, "%s�� CPU(%s)           ", move_who[WHITE], Diff_Str);
			TextOut(hdc, BOARD_SIZE + 10, 125, str, strlen(str));
		}*/
	}

	SetBkColor(hdc, RGB(200, 200, 200));
	SetTextColor(hdc, RGB(100, 0, 0));
	SetBkColor(hdc, RGB(255, 255, 255));

	/* �X�e�[�^�X�o�[�ɕ]���l�̕\�� */
	disp_eval();
}

BOOL CanDropDown(BitBoard bk, BitBoard wh, int x, int y)
{
	/* �󂫃}�X�ł͂Ȃ��ꍇ */
	if((bk | wh) & one << (x * MASS_NUM + y)) return FALSE;

	BitBoard moves;
	int count;

	/* ���@��̈ꗗ���� */
	if(NowTurn == BLACK)
	{
		moves = CreateMoves(bk, wh, &count);
	}
	else
	{
		moves = CreateMoves(wh, bk, &count);
	}
	if(moves & (one << (x * MASS_NUM + y))){ return TRUE;}
	return FALSE;
}

void DropDownStone(BitBoard bk, BitBoard wh, int x, int y)
{
	BitBoard rev;

	if(NowTurn == WHITE)
	{ 
		rev = GetRev[x * MASS_NUM + y](wh, bk);
		wh ^= (one << (x * MASS_NUM + y) | rev);
		/* ���]���� */
		bk ^= rev;
	}
	else
	{
		rev = GetRev[x * MASS_NUM + y](bk, wh);
		bk ^= (one << (x * MASS_NUM + y) | rev);
		/* ���]���� */
		wh ^= rev;
	}

	black = bk;
	white = wh;
}

/* �N���b�N�����ꏊ������\���ǂ����𔻒f */
BOOL chk_range_board(LPARAM lp)
{
	if(!m_FlagInGame)
	{
		if(Flag_Edit)
		{
			Edit_Board(hWnd, hStatus, lp);
		}
		return FALSE;; 
	}

	int x, y;
	x = LOWORD(lp)/ MASS_SIZE;
	y = HIWORD(lp) / MASS_SIZE - 1;

	/* �{�[�h�͈͊O�̏ꍇ */
	if(x < 0 || x >= MASS_NUM || y < 0 || y >= MASS_NUM)
	{
		return FALSE;
	}

	return CanDropDown(black, white, x, y);

}

/* �p�X�ǂ������`�F�b�N */
BOOL IsPass(BitBoard b_board, BitBoard w_board){
	int count;
	CreateMoves(b_board, w_board, &count);
	if(count > 0)
	{
		return FALSE;
	}
	return TRUE;
}

/* ���s����֐� */
int IsWin(BitBoard b_board, BitBoard w_board){
  
  /* ���݂����p�X�Ȃ珟�s������s�� */
  if(IsPass(b_board, w_board) && IsPass(w_board, b_board)){ 
    CNT_B = CountBit(b_board);
    CNT_W = CountBit(w_board);
    if(CNT_B > CNT_W){
      return WIN;
    }
    else if(CNT_B < CNT_W){
      return LOSS;
    }
    else {
      return DRAW;
    }
  }
  return CONTINUE;
}

int RemainStone(BitBoard black, BitBoard white)
{
	BitBoard blank = ~(black | white);
	return CountBit(blank);
}

void OnVeasy(HWND hWnd, int color)
{
	difficult[color] = 1;
	HMENU hMenu = GetMenu(hWnd);
	Force_SearchWin = FALSE;
	Force_SearchSD = FALSE;

	CheckMenuItem(hMenu, Veasy, MF_BYCOMMAND | MF_CHECKED); 

	CheckMenuItem(hMenu, easy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, normal, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, hard, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, Vhard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, Uhard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllWinLoss, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllStoneDiff, MF_BYCOMMAND | MF_UNCHECKED); 
}

void Oneasy(HWND hWnd, int color)
{
	difficult[color] = 2;
	Force_SearchWin = FALSE;
	Force_SearchSD = FALSE;

	HMENU hMenu = GetMenu(hWnd);
	
	CheckMenuItem(hMenu, easy, MF_BYCOMMAND | MF_CHECKED); 

	CheckMenuItem(hMenu, Veasy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, normal, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, hard, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, Vhard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, Uhard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllWinLoss, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllStoneDiff, MF_BYCOMMAND | MF_UNCHECKED); 
}

void Onnormal(HWND hWnd, int color)
{
	difficult[color] = 3;
	Force_SearchWin = FALSE;
	Force_SearchSD = FALSE;

	HMENU hMenu = GetMenu(hWnd);
	
	CheckMenuItem(hMenu, normal, MF_BYCOMMAND | MF_CHECKED); 

	CheckMenuItem(hMenu, Veasy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, easy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, hard, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, Vhard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, Uhard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllWinLoss, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllStoneDiff, MF_BYCOMMAND | MF_UNCHECKED); 
}

void Onhard(HWND hWnd, int color)
{
	difficult[color] = 4;
	Force_SearchWin = FALSE;
	Force_SearchSD = FALSE;

	HMENU hMenu = GetMenu(hWnd);
	
	CheckMenuItem(hMenu, hard, MF_BYCOMMAND | MF_CHECKED); 

	CheckMenuItem(hMenu, Veasy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, easy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, normal, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, Vhard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, Uhard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllWinLoss, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllStoneDiff, MF_BYCOMMAND | MF_UNCHECKED); 
}

void OnVhard(HWND hWnd, int color)
{
	difficult[color] = 5;
	Force_SearchWin = FALSE;
	Force_SearchSD = FALSE;

	HMENU hMenu = GetMenu(hWnd);
	
	CheckMenuItem(hMenu, Vhard, MF_BYCOMMAND | MF_CHECKED); 

	CheckMenuItem(hMenu, Veasy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, easy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, normal, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, hard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, Uhard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllWinLoss, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllStoneDiff, MF_BYCOMMAND | MF_UNCHECKED); 
}

void OnUhard(HWND hWnd, int color)
{
	difficult[color] = 6;
	Force_SearchWin = FALSE;
	Force_SearchSD = FALSE;

	HMENU hMenu = GetMenu(hWnd);
	
	CheckMenuItem(hMenu, Uhard, MF_BYCOMMAND | MF_CHECKED); 

	CheckMenuItem(hMenu, Veasy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, easy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, normal, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, hard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, Vhard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllWinLoss, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllStoneDiff, MF_BYCOMMAND | MF_UNCHECKED); 
}

void OnSUhard(HWND hWnd, int color)
{
	difficult[color] = 7;
	Force_SearchWin = FALSE;
	Force_SearchSD = FALSE;

	HMENU hMenu = GetMenu(hWnd);
	
	CheckMenuItem(hMenu, Uhard, MF_BYCOMMAND | MF_CHECKED); 

	CheckMenuItem(hMenu, Veasy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, easy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, normal, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, hard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, Vhard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllWinLoss, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllStoneDiff, MF_BYCOMMAND | MF_UNCHECKED); 
}

void OnSSUhard(HWND hWnd, int color)
{
	difficult[color] = 8;
	Force_SearchWin = FALSE;
	Force_SearchSD = FALSE;

	HMENU hMenu = GetMenu(hWnd);
	
	CheckMenuItem(hMenu, Uhard, MF_BYCOMMAND | MF_CHECKED); 

	CheckMenuItem(hMenu, Veasy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, easy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, normal, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, hard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, Vhard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllWinLoss, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllStoneDiff, MF_BYCOMMAND | MF_UNCHECKED); 
}

void OnHUhard(HWND hWnd, int color)
{
	difficult[color] = 9;
	Force_SearchWin = FALSE;
	Force_SearchSD = FALSE;

	HMENU hMenu = GetMenu(hWnd);
	
	CheckMenuItem(hMenu, Uhard, MF_BYCOMMAND | MF_CHECKED); 

	CheckMenuItem(hMenu, Veasy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, easy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, normal, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, hard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, Vhard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllWinLoss, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllStoneDiff, MF_BYCOMMAND | MF_UNCHECKED); 
}

void OnUltra(HWND hWnd, int color)
{
	difficult[color] = 10;
	Force_SearchWin = FALSE;
	Force_SearchSD = FALSE;

	HMENU hMenu = GetMenu(hWnd);
	
	CheckMenuItem(hMenu, Uhard, MF_BYCOMMAND | MF_CHECKED); 

	CheckMenuItem(hMenu, Veasy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, easy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, normal, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, hard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, Vhard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllWinLoss, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllStoneDiff, MF_BYCOMMAND | MF_UNCHECKED); 
}

void OnSUltra(HWND hWnd, int color)
{
	difficult[color] = 11;
	Force_SearchWin = FALSE;
	Force_SearchSD = FALSE;

	HMENU hMenu = GetMenu(hWnd);
	
	CheckMenuItem(hMenu, Uhard, MF_BYCOMMAND | MF_CHECKED); 

	CheckMenuItem(hMenu, Veasy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, easy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, normal, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, hard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, Vhard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllWinLoss, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllStoneDiff, MF_BYCOMMAND | MF_UNCHECKED); 
}

void OnSSUltra(HWND hWnd, int color)
{
	difficult[color] = 12;
	Force_SearchWin = FALSE;
	Force_SearchSD = FALSE;

	HMENU hMenu = GetMenu(hWnd);
	
	CheckMenuItem(hMenu, Uhard, MF_BYCOMMAND | MF_CHECKED); 

	CheckMenuItem(hMenu, Veasy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, easy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, normal, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, hard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, Vhard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllWinLoss, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllStoneDiff, MF_BYCOMMAND | MF_UNCHECKED); 
}

void OnAllSearchWin(HWND hWnd, int color)
{
	if(RemainStone(black, white) > 28)
	{
		//char *msg = "�󂫃}�X��28�𒴂��Ă���ƁA\n�T���I���܂ł�1���Ԉȏォ����\��������܂��B";
		//MessageBox(hWnd, msg, "�����Ԃ̒T��", MB_OK);
	}
	
	Force_SearchWin = TRUE;
	Force_SearchSD = FALSE;

	difficult[color] = 6;
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B
	strcpy_s(Diff_Str, LEN, "WinLoss Search");

	HMENU hMenu = GetMenu(hWnd);

	CheckMenuItem(hMenu, AllWinLoss, MF_BYCOMMAND | MF_CHECKED); 

	CheckMenuItem(hMenu, Veasy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, easy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, normal, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, hard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, Vhard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, Uhard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllStoneDiff, MF_BYCOMMAND | MF_UNCHECKED);

}

void OnAllSearchSD(HWND hWnd, int color)
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B
	
	if(RemainStone(black, white) < 26)
	{
		//char *msg = "�󂫃}�X��26�𒴂��Ă���ƁA\n�T���I���܂ł�1���Ԉȏォ����\��������܂��B";
		//MessageBox(hWnd, msg, "�����Ԃ̒T��", MB_OK);
	}

	Force_SearchSD = TRUE;
	Force_SearchWin = FALSE;
	difficult[color] = 6;
	strcpy_s(Diff_Str, LEN, "Exact Search");

	HMENU hMenu = GetMenu(hWnd);

	CheckMenuItem(hMenu, AllStoneDiff, MF_BYCOMMAND | MF_CHECKED); 

	CheckMenuItem(hMenu, Veasy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, easy, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, normal, MF_BYCOMMAND | MF_UNCHECKED); 
	CheckMenuItem(hMenu, hard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, Vhard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, Uhard, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, AllWinLoss, MF_BYCOMMAND | MF_UNCHECKED); 

}

void OnHint_0(HWND hWnd)
{
	Flag_Hint = FALSE;
	CheckMenuItem(GetMenu(hWnd), Hint_0, MFS_CHECKED);

	CheckMenuItem(GetMenu(hWnd), Hint_1, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_2, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_3, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_4, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_5, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_6, MFS_UNCHECKED);
}

void OnHint_1(HWND hWnd)
{
	Flag_Hint = TRUE;
	HINT_LEVEL = 2;
	CheckMenuItem(GetMenu(hWnd), Hint_1, MFS_CHECKED);

	CheckMenuItem(GetMenu(hWnd), Hint_2, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_3, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_0, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_4, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_5, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_6, MFS_UNCHECKED);
}

void OnHint_2(HWND hWnd)
{
	Flag_Hint = TRUE;
	HINT_LEVEL = 4;
	CheckMenuItem(GetMenu(hWnd), Hint_2, MFS_CHECKED);

	CheckMenuItem(GetMenu(hWnd), Hint_0, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_1, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_3, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_4, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_5, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_6, MFS_UNCHECKED);
}

void OnHint_3(HWND hWnd)
{
	Flag_Hint = TRUE;
	HINT_LEVEL = 6;
	CheckMenuItem(GetMenu(hWnd), Hint_3, MFS_CHECKED);

	CheckMenuItem(GetMenu(hWnd), Hint_0, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_1, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_2, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_4, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_5, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_6, MFS_UNCHECKED);
}

void OnHint_4(HWND hWnd)
{
	Flag_Hint = TRUE;
	HINT_LEVEL = 8;
	CheckMenuItem(GetMenu(hWnd), Hint_4, MFS_CHECKED);

	CheckMenuItem(GetMenu(hWnd), Hint_0, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_1, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_2, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_3, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_5, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_6, MFS_UNCHECKED);
}

void OnHint_5(HWND hWnd)
{
	Flag_Hint = TRUE;
	HINT_LEVEL = 10;
	CheckMenuItem(GetMenu(hWnd), Hint_5, MFS_CHECKED);

	CheckMenuItem(GetMenu(hWnd), Hint_0, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_1, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_2, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_3, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_4, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_6, MFS_UNCHECKED);
}

void OnHint_6(HWND hWnd)
{
	Flag_Hint = TRUE;
	HINT_LEVEL = 12;
	CheckMenuItem(GetMenu(hWnd), Hint_6, MFS_CHECKED);

	CheckMenuItem(GetMenu(hWnd), Hint_0, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_1, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_2, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_3, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_4, MFS_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), Hint_5, MFS_UNCHECKED);
}

/* ���s���ʂ̕\���ƃQ�[���̏I������ */
void DispWinLoss(HWND hWnd, HWND hStatus)
{

	m_FlagInGame = FALSE;
	m_FlagInterrupt = TRUE;
	HDC hdc = GetDC(hWnd);
	DrawBoard(hdc, hStatus, move_x, move_y);
	ReleaseDC(hWnd, hdc);
	char msg[64];


	/* ���j���[�o�[�̍��ڂ̗L�������� */
	HMENU hMenu = GetMenu(hWnd);
	EnableMenuItem(hMenu, interrupt, MFS_GRAYED); 
	EnableMenuItem(hMenu, EditBoard, MFS_ENABLED);
	EnableMenuItem(hMenu, ChangeColor, MFS_ENABLED);
	
	if(Flag_Abort == TRUE){
		return;
	}
	
	int count_b = CountBit(black);
	int count_w = CountBit(white);

	if(count_b > count_w)
	{
		sprintf_s(msg, "��%d - ��%d�ŁA���̏����ł��B", CountBit(black), CountBit(white));
		MessageBox(hWnd, msg, "���s", MB_ICONINFORMATION);
	}
	else if(count_w > count_b)
	{
		sprintf_s(msg, "��%d - ��%d�ŁA���̏����ł��B", CountBit(black), CountBit(white));
		MessageBox(hWnd, msg, "���s", MB_ICONINFORMATION);
	}
	else
	{
		MessageBox(hWnd, "���������ł��B", "���s", MB_ICONINFORMATION);
	}
}

