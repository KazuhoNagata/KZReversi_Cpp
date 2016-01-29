/*#
  # AI�ȂǃX���b�h�œ��삷��@�\���Ǘ����܂�
  #  
  #################################################
*/

#include "stdafx.h"
#include "KZReversi.h"
#include "book.h"
#include "GetPattern.h"
#include "hint.h"
//#include "learn.h"
#include "interface.h"
#include "game.h"
#include "thread.h"
#include "eval.h"

int Flag_Abort;

HANDLE hMutex;
HANDLE SIG_AI_FINISHED;
HANDLE SIG_HINT_FINISHED;

/* �X���b�h�p�\���� */
typedef struct {
	int depth;
	int end_turn;
	int count;
	int situation;
}VAL, *PVAL;

typedef struct {
	HWND hWnd;
	CRITICAL_SECTION cs;
	LPARAM lparam;
	int flag;
}GP_VAL, *GP_PVAL;

typedef struct {
	HDC hdc;
	BitBoard bk;
	BitBoard wh;
	BitBoard moves;
}HINT_VAL, *HINT_PVAL;

typedef struct {
	HWND hWnd;
	CRITICAL_SECTION cs;
} THREADPARAM;

void start_GameProcess_thread(LPVOID val)
{

	GP_PVAL pval = (GP_PVAL)val;
	//srand((unsigned int)time(NULL));

	/* �N���e�B�J���Z�N�V���� */
	EnterCriticalSection(&pval->cs);
	HWND hWnd = pval->hWnd;

	/* ���f�ȊO�̃��j���[���ږ����� */
	int r_id;
	HMENU hMenu = GetMenu(hWnd);

	for(r_id = Veasy; r_id <= print_eval; r_id++)
	{
		EnableMenuItem(hMenu, r_id, MFS_GRAYED);
	}
	/* ���f���j���[�����L�� */
	EnableMenuItem(hMenu, interrupt, MFS_ENABLED);

	/* �{�^�������f�ȊO���� */
	HWND button;
	for(r_id = NEWGAME_BT; r_id <= FINAL_BT; r_id++)
	{
		button = GetDlgItem(hRebar, r_id);
		EnableWindow(button, FALSE);
	}
	/* ���f�{�^�������L�� */
	button = GetDlgItem(hRebar, INTERRUPT_BT);
	EnableWindow(button, TRUE);
	

	hMutex = CreateMutex(NULL, FALSE, NULL);
	WaitForSingleObject(hMutex,INFINITE); //mutex �Ԃ͑��̃X���b�h����ϐ���ύX�ł��Ȃ�
	if(pval->flag == 0)
	{
		OnStartGame(hWnd, hStatus);
	}
	else
	{
		if(chk_range_board(pval->lparam))
		{
			LButtonClick(hWnd, hStatus, pval->lparam);
		}
	}

	/* ���ɖ߂� */
	for(r_id = Veasy; r_id <= print_eval; r_id++)
	{
		EnableMenuItem(hMenu, r_id, MFS_ENABLED);
	}
	for(r_id = NEWGAME_BT; r_id <= FINAL_BT; r_id++)
	{
		button = GetDlgItem(hRebar, r_id);
		EnableWindow(button, TRUE);
	}
	Flag_Abort = FALSE;
	SendMessage(hStatus, SB_SETTEXT, (WPARAM)1 | 0, (LPARAM)turn_str[NowTurn]);
	ReleaseMutex(hMutex);

	LeaveCriticalSection(&pval->cs);
}

void GameThread(LPARAM lparam, int flag)
{
	static GP_VAL val;
	void (*p)(void *) = start_GameProcess_thread;
	//m_FlagHint = FALSE;

	AI_THINKING = TRUE;
	Sleep(50);

	val.lparam = lparam;
	val.flag = flag;
	InitializeCriticalSection(&val.cs);
	val.hWnd = hWnd;

	SIG_AI_FINISHED = (HANDLE)_beginthread(p, 0, &val);
	AI_THINKING = FALSE;
	Sleep(50);

	static THREADPARAM tp;
	tp.hWnd = hWnd;
	InitializeCriticalSection(&tp.cs);

	/*if(Flag_Hint && !m_FlagHint)
	{
		p = start_disp_hint_thread;
		SIG_HINT_FINISHED = (HANDLE)_beginthread(p, 0, &tp);
	}*/
}
/*
void Start_File_Learn_loop()
{
	int i;
	int error;
	if((error = fopen_s(&data_fp[0], "�w�K�f�[�^\\eval_prologue.dat", "a+")) != 0){
		return ;
	}
	if((error = fopen_s(&data_fp[1], "�w�K�f�[�^\\eval_opening1.dat", "a+")) != 0){
		return ;
	}
	if((error = fopen_s(&data_fp[2], "�w�K�f�[�^\\eval_opening2.dat", "a+")) != 0){
		return ;
	}
	if((error = fopen_s(&data_fp[3], "�w�K�f�[�^\\eval_opening3.dat", "a+")) != 0){
		return ;
	}
	if((error = fopen_s(&data_fp[4], "�w�K�f�[�^\\eval_middle1.dat", "a+")) != 0){
		return ;
	}
	if((error = fopen_s(&data_fp[5], "�w�K�f�[�^\\eval_middle2.dat", "a+")) != 0){
		return ;
	}
	if((error = fopen_s(&data_fp[6], "�w�K�f�[�^\\eval_middle3.dat", "a+")) != 0){
		return ;
	}
	if((error = fopen_s(&data_fp[7], "�w�K�f�[�^\\eval_middle4.dat", "a+")) != 0){
		return ;
	}
	if((error = fopen_s(&data_fp[8], "�w�K�f�[�^\\eval_middle5.dat", "a+")) != 0){
		return ;
	}
	if((error = fopen_s(&data_fp[9], "�w�K�f�[�^\\eval_end1.dat", "a+")) != 0){
		return ;
	}
	if((error = fopen_s(&data_fp[10], "�w�K�f�[�^\\eval_end2.dat", "a+")) != 0){
		return ;
	}
	if((error = fopen_s(&data_fp[11], "�w�K�f�[�^\\eval_end3.dat", "a+")) != 0){
		return ;
	}
	if((error = fopen_s(&data_fp[12], "�w�K�f�[�^\\eval_end4.dat", "a+")) != 0){
		return ;
	}
	if((error = fopen_s(&data_fp[13], "�w�K�f�[�^\\eval_end5.dat", "a+")) != 0){
		return ;
	}
	for(i = 0; i < 74; i+=8)
	{
		if(Start_File_Learn(kihu_list[i]) == FALSE)
		{
			MessageBox(hWnd, "error!", "error!!", MB_OK);
		}
	}

	for(i = 0; i < 14; i++)
	{
		fclose(data_fp[i]);
	}
	PostMessage(hStatus, SB_SETTEXT, (WPARAM)0|3, (LPARAM)"END!!!");;
}

void start_learn_thread(void *data)
{
	static PVAL pval = (PVAL)data;
	srand((unsigned int)time(NULL));
	hMutex = CreateMutex(NULL, FALSE, NULL);
	WaitForSingleObject(hMutex,INFINITE); //mutex �Ԃ͑��̃X���b�h����ϐ���ύX�ł��Ȃ�
	//Start_File_Learn_loop();
	CulcPatternEval(2053032);
	ReleaseMutex(hMutex);
	//Eval_Write();
}

void Learn_Thread()
{
	static VAL val;
	void (*p)(void *) = start_learn_thread;
	val.count = 612586;
	val.depth = 8;
	val.end_turn = PROLOGUE;
	val.situation = -1;
	_beginthread(p, 0, &val);
	//WaitForSingleObject(hHandle, INFINITE);
}

*/

//void start_disp_hint_thread(LPVOID pvParam)
//{
//	/* AI�X���b�h�̏I���҂� */
//	WaitForSingleObject(SIG_AI_FINISHED, INFINITE);
//	
//	hMutex = CreateMutex(NULL, FALSE, NULL);
//	WaitForSingleObject(hMutex,INFINITE); //mutex �Ԃ͑��̃X���b�h����ϐ���ύX�ł��Ȃ�
//
//	THREADPARAM *tp = (THREADPARAM *)pvParam;
//	/* �N���e�B�J���Z�N�V���� */
//	EnterCriticalSection(&tp->cs);
//	HDC hdc = GetDC(tp->hWnd);
//
//	int move_cnt;
//	BitBoard moves_b = CreateMoves(black, white, &move_cnt);
//	BitBoard moves_w = CreateMoves(white, black, &move_cnt);
//
//	SetBkColor(hdc, RGB(30, 100, 50));	
//	/* �q���gON�� */
//	/* �Q�[�����A�q���g�T�����܂�����Ă��Ȃ��A�q���g�@�\��ON�A���f���łȂ��ACPU�̃^�[���łȂ��@�ꍇ�ɒʂ� */
//	if(m_FlagInGame && !HINT_DISPED && !m_FlagHint && Flag_Hint && !m_FlagInterrupt && NowTurn != CpuColor)
//	{
//		PostMessage(hStatus, SB_SETTEXT, (WPARAM)0|3, (LPARAM)"Hint processing...");
//		m_FlagHint = TRUE;
//		int cnt = 2, depth = HINT_LEVEL;
//		print_eval_flag = 0;
//		turn = 60 - RemainStone(black, white);
//		while(print_eval_flag == 0 && cnt <= depth && !AI_THINKING && !Flag_Abort)
//		{
//
//			if(NowTurn && moves_w)
//			{
//				disp_hint(hdc, white, black, moves_w, cnt, turn);
//			}
//			else if(!NowTurn && moves_b)
//			{
//				disp_hint(hdc, black, white, moves_b, cnt, turn);
//			}
//			cnt += 2;
//		}
//		/* �\������ */
//		HINT_DISPED = TRUE;
//		/* hint process finished. */
//		m_FlagHint = FALSE;
//		PostMessage(hStatus, SB_SETTEXT, (WPARAM)0|3, (LPARAM)"Hint processing...done.");
//	}
//	//if(Flag_Hint && NowTurn != CpuColor && !Flag_Abort && !m_FlagHint)
//	//{
//	//	/* ��ʂɕ\�� */
//	//	int i = 0;
//	//	while(EvalData[i] != NEGAMAX)
//	//	{
//	//		/* �ۑ�����Ă���]���l��\�� */
//	//		Print_HintEval(hdc, EvalData[i], PosData[i] / 8, PosData[i] % 8, print_eval_flag);
//	//		i++;
//	//	}
//	//}
//	Flag_Abort = FALSE;
//	ReleaseMutex(hMutex);
//	LeaveCriticalSection(&tp->cs);
//}

void auto_game(void *)
{	
	int ret;
	InitBoard();
	CpuColor = BLACK;
	srand((unsigned int)time(NULL));
	while(!Flag_Abort && (ret = CpuProcess(hWnd, hStatus)) == CONTINUE)
	{
		Sleep(0);
		CpuColor = !CpuColor;
	}
	Flag_Abort = FALSE;
}

void auto_game_thread()
{
	static VAL val;
	void (*p)(void *) = auto_game;
	_beginthread(p, 0, NULL);
}

void start_open_book_thread(void *)
{
	/* ��� */
	Sleep(50);

	//makeData();
	//exit(1);

	open_book();
	SetDlgItemText(DlgHandle, IDC_MESG, (LPCTSTR)"loading eval tables..." );
	/* �]���e�[�u���̓ǂݍ��� */
	ReadEvalDat();
	/* MPC�e�[�u���̓ǂݍ��� */
	readMPCInfo();
	/* ���[�f�B���O��ʂ���� */
	PostMessage(DlgHandle, WM_COMMAND, IDOK, NULL);
}

void open_book_thread(HINSTANCE hInst)
{
	static VAL val;
	void (*p)(void *) = start_open_book_thread;
	_beginthread(p, 0, NULL);
	DialogBox(hInst, MAKEINTRESOURCE(IDD_LOAD_BOOKS), hWnd, load_books);
}