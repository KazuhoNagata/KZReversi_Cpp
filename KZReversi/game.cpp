/*#
# �Q�[���S�̂̐i�s���Ǘ����܂�
#  
##############################################
*/

#include "stdafx.h"
#include "KZReversi.h"
#include "AI.h"
#include "book.h"
#include "hint.h"
#include "interface.h"
#include "game.h"
#include "thread.h"
#include "hash.h"
#include <crtdbg.h>

/* winlose���ʋL�^�p(�΍��T���Ŏg��) */
int wld;
double start_t;
double end_t;
int difficult[2];
char msg[64];
int AI_result, AI_thinking;
char CountNodeTime_str[64];
int MAX_MOVE;
int line[30];
int OnTheWay;
int AI_THINKING;
int move_x, move_y;
int NowTurn;
int difficult_table[12];
int Force_SearchWin, Force_SearchSD;
int m_FlagSearchWin, m_FlagPerfectSearch;
int HINT_DISPED;
int m_FlagInGame, m_FlagInterrupt = TRUE;
int Flag_Edit;
int PassCnt;
int m_FlagForWhite;

Hash *ghash;

/* player */
int player[2];

void process_abort(int move, char *predict_msg)
{
	char abort_msg[128];
	sprintf_s(abort_msg, "[Incompleted serach]���s�\�z[%s:%s]", cordinates_table[move], predict_msg);
	SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 3, (LPARAM)abort_msg);
}

int winlossdraw_process(BitBoard bk, BitBoard wh, Hash *hash)
{
	int move;
	char msg[64];
	int max_score, max_move;
	int key = KEY_HASH_MACRO(bk, wh);
	start_t = timeGetTime();
	char winlossdraw_str[][16] = {"LOSS", "DRAW", "WIN"};

	COUNT_NODE = 0;
	COUNT_NODE2 = 0;

	INF_DEPTH = RemainStone(bk ,wh);
	LIMIT_DEPTH = INF_DEPTH - 2;
	if(LIMIT_DEPTH > 24)
	{
		LIMIT_DEPTH = 24;
	}
	start_t = timeGetTime();
	int g;
	/* ���O�T�� */
	move = middle_process(bk, wh, LIMIT_DEPTH, hash);

	max_score = MAXEVAL;
	max_move = move;

	LIMIT_DEPTH = INF_DEPTH;

	if(MAXEVAL > 2000)
	{
		wld = 2;
	}
	else if(MAXEVAL < -2000)
	{
		wld = 0;
	}
	else
	{
		wld = 1;
	}

	g = MAXEVAL;
	g /= EVAL_ONE_STONE;
	g += (g % 2);

	/* �A�{�[�g */
	if(Flag_Abort == TRUE)
	{
		process_abort(move, winlossdraw_str[wld]);
		return move;
	}
	/* �A���t�@�x�[�^�l�������� */
	//init_substitution_table_exact();
	/* �΍��T���J�n */
	if(AI_thinking)
	{
		//move =  EndSolverExact_test( bk, wh, 0, g - 8, g + 8, turn, 0, line_stack);
	}
	else
	{
		/* �]���l���珟�s�T���̒T�����̌��� */
		int window[2];
		if(g > 2)
		{
			/* ���\�������� */
			window[0] = DRAW;
			window[1] = WIN;
		}
		else if(g < -2)
		{
			/* ������\�������� */
			window[0] = LOSS;
			window[1] = DRAW;
		}
		else{
			/* ���������̉\�������� */
			window[0] = LOSS;
			window[1] = WIN;
		}

		sprintf_s(msg, "�\�z���s[%s:(CPU %s)]", cordinates_table[move], winlossdraw_str[wld]);
		SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 3, (LPARAM)msg);

		/* ���s�T���p�ɒu���\�̓��_���v�Z */
		init_substitution_table_winloss(hash);
		LIMIT_DEPTH = INF_DEPTH;
		/* ���s�T�� */
		//MPC_FLAG = TRUE;
		MAXEVAL = EndSolverWinLoss(bk, wh, INF_DEPTH, LOSS, WIN, NowTurn, hash, 0);

		/* �����������o���ꍇ�́A���������ȉ��̉\��������̂ŁA�����Đݒ肵�ĒT�� */
		//if(g > 2 && MAXEVAL == DRAW)
		//{
		//	/* �������������� */
		//	window[0] = LOSS;
		//	window[1] = DRAW;
		//	/* ���s�T�� */
		//	MAXEVAL = EndSolverWinLoss(bk, wh, 0, window[0], window[1], turn, 0);
		//}
		///* �����������o���ꍇ�́A���������ȏ�̉\��������̂ŁA�����Đݒ肵�ĒT�� */
		//else if(g < -2 && MAXEVAL == DRAW)
		//{
		//	/* �������������� */
		//	window[0] = DRAW;
		//	window[1] = LOSS;
		//	/* ���s�T�� */
		//	MAXEVAL = EndSolverWinLoss(bk, wh, 0, window[0], window[1], turn, 0);
		//}

		/* ���݂̋ǖʂ̍őP���u���\���璊�o */
		move = hash->data[key].bestmove;

		/* �A�{�[�g */
		if(Flag_Abort == TRUE)
		{
			end_t = timeGetTime();
			sprintf_s(CountNodeTime_str, "node:%lld, time:%.4f[sec]", 
			COUNT_NODE, (end_t - start_t) / 1000);
			process_abort(move, winlossdraw_str[wld]);
			if(MAXEVAL > max_score)
			{
				return move;
			}
			else
			{
				MAXEVAL = max_score;
				return max_move;
			}
		}

		sprintf_s(msg, "@(%s)", winlossdraw_str[MAXEVAL + 1], winlossdraw_str[MAXEVAL + 1]);
		SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 2, (LPARAM)msg);

		/* �΍��T���p�ɒu���\�̓��_���v�Z */
		init_substitution_table_exact(hash);

		if(MAXEVAL == WIN){
			if(g < 9){
				g = 9;
			}
			else if(g > 56){

				g = 56;
			}
			/* �����m��(�]���l - 8 < �΍� < �]���l + 8 �ɂ��邱�Ƃ�����) */
			window[0] = g - 8;
			window[1] = g + 8;
		}
		else if(MAXEVAL == LOSS){
			/* �����m�� */
			if(g > -9){
				g = -9;
			}
			else if(g < -56){

				g = -56;
			}
			window[0] = g - 8;
			window[1] = g + 8;
		}
		else{
			/* ���������m��(�����O�Ȃ̂ł�����DRAW���Ԃ邪�ꉞ�B�B) */
			window[0] = DRAW;
			window[1] = DRAW;
		}

		end_t = timeGetTime();
		sprintf_s(CountNodeTime_str, "node:%lld, time:%.4f[sec]", 
			COUNT_NODE, (end_t - start_t) / 1000);
		SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 0, (LPARAM)CountNodeTime_str);
		
		sprintf_s(msg, "���s�m��[%s:%s][%+d�`%+d]", cordinates_table[move], winlossdraw_str[MAXEVAL + 1], window[0], window[1]);
		SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 3, (LPARAM)msg);
	}

	return move;
}

int exact_process(BitBoard bk, BitBoard wh, Hash *hash)
{
	int move;
	char msg[64];
	int max_score, max_move;
	int key = KEY_HASH_MACRO(bk, wh);

	//MTD-f �I�Ղ��Ƃ���܂�E�E�E�ۗ�
	//if(AI_result && INF_DEPTH > 12)
	//{
	//	COUNT_NODE = 0;
	//	LIMIT_DEPTH = INF_DEPTH - 12;
	//	if(LIMIT_DEPTH > 14)
	//	{
	//		LIMIT_DEPTH = 14;
	//	}
	//	read_eval_table(turn + LIMIT_DEPTH);
	//	start_t = timeGetTime();
	//	move = CPU_AI_SHARROW(bk, wh, 0, NEGAMIN - 1, NEGAMAX + 1, turn);

	//	end_t = timeGetTime();
	//	double search_time = (end_t - start_t) / 1000;
	//	MAXEVAL *= -1;
	//	MAXEVAL /= EVAL_ONE_STONE;
	//	int g, beta;
	//	/* �A���t�@�x�[�^�l�������� */
	//	init_substitution_table_exact();
	//	g = MAXEVAL;
	//	g += (g % 2);
	//	if(g > 60)
	//	{
	//		g = 60;
	//	}
	//	int lower = -65, upper = 65;
	//	int temp_move;
	//	while(1)
	//	{
	//		if ( g == lower )
	//		{
	//			beta = g + 1;
	//			if(beta > 64)
	//			{
	//				beta = 64;
	//			}
	//		}
	//		else
	//		{
	//			beta = g - 1;
	//		}
	//		sprintf_s(msg, "�\�z�΍�[%s:%+d�`%+d]", cordinates_table[move], (beta - 1), beta);
	//		SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 3, (LPARAM)msg);
	//		start_t = timeGetTime();
	//		temp_move = EndSolverExact(bk, wh, 0, beta - 1, beta, turn, 0);
	//		end_t = timeGetTime();
	//		search_time += (end_t - start_t) / 1000;
	//		g = -MAXEVAL;
	//		if(g < beta)
	//		{
	//			upper = g;
	//		}
	//		else
	//		{
	//			lower = g;
	//			move = temp_move;
	//		}
	//		if(lower >= upper || g == 64)
	//		{
	//			/* minmax�l�𔭌� */
	//			sprintf_s(msg, "�m��΍�[%s:%+d]", cordinates_table[move], lower);
	//			SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 3, (LPARAM)msg);
	//			break;
	//		}
	//	}
	//	if(AI_result)
	//	{
	//		sprintf_s(CountNodeTime_str, "node:%lld, time:%.4f[sec]", COUNT_NODE, search_time);
	//	}
	//}
	//else
	//{
	INF_DEPTH = RemainStone(bk ,wh);
	start_t = timeGetTime();

	if(INF_DEPTH <= 12)
	{
		MAXEVAL = EndSolverExact( bk, wh, 0, -64, 64, NowTurn, hash, 0);
		/* ���݂̋ǖʂ̍őP���u���\���璊�o */

		move = hash->data[key].bestmove;

		end_t = timeGetTime();
		sprintf_s(CountNodeTime_str, "node:%lld, time:%.4f[sec]", 
			COUNT_NODE, (end_t - start_t) / 1000);
	}
	else
	{
		char winlossdraw_str[][16] = {"LOSS", "DRAW", "WIN"};
		COUNT_NODE = 0;
		LIMIT_DEPTH = INF_DEPTH - 2;
		if(LIMIT_DEPTH % 2 == 1){
			LIMIT_DEPTH++;
		}
		if(LIMIT_DEPTH > 24)
		{
			LIMIT_DEPTH = 24;
		}
		start_t = timeGetTime();
		int g;
		/* ���O�T�� */
		move = middle_process(bk, wh, LIMIT_DEPTH, hash);

		max_score = MAXEVAL;
		max_move = move;

		LIMIT_DEPTH = INF_DEPTH;

		if(MAXEVAL > 2000)
		{
			wld = 2;
		}
		else if(MAXEVAL < -2000)
		{
			wld = 0;
		}
		else
		{
			wld = 1;
		}

		g = MAXEVAL;
		g /= EVAL_ONE_STONE;
		g += (g % 2);

		/* �A�{�[�g */
		if(Flag_Abort == TRUE)
		{
			process_abort(move, winlossdraw_str[wld]);
			return move;
		}
		/* �A���t�@�x�[�^�l�������� */
		//init_substitution_table_exact();
		int line_stack[30] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,};
		LIMIT_DEPTH = INF_DEPTH;
		/* �΍��T���J�n */
		if(AI_thinking)
		{
			//move =  EndSolverExact_test( bk, wh, 0, g - 8, g + 8, turn, 0, line_stack);
		}
		else
		{
			/* �]���l���珟�s�T���̒T�����̌��� */
			int window[2];
			if(g > 2)
			{
				/* ���\�������� */
				window[0] = DRAW;
				window[1] = WIN;
			}
			else if(g < -2)
			{
				/* ������\�������� */
				window[0] = LOSS;
				window[1] = DRAW;
			}
			else{
				/* ���������̉\�������� */
				window[0] = LOSS;
				window[1] = WIN;
			}

			sprintf_s(msg, "�\�z���s[%s:(CPU %s)]", cordinates_table[move], winlossdraw_str[wld]);
			SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 3, (LPARAM)msg);

			/* ���s�T���p�ɒu���\�̓��_���v�Z */
			init_substitution_table_winloss(hash);
			/* ���s�T�� */
					MPC_FLAG = TRUE;
			MAXEVAL = EndSolverWinLoss(bk, wh, INF_DEPTH, LOSS, WIN, NowTurn, hash, 0);

			///* �����������o���ꍇ�́A���������ȉ��̉\��������̂ŁA�����Đݒ肵�ĒT�� */
			//if(g > 2 && MAXEVAL == DRAW)
			//{
			//	/* �������������� */
			//	window[0] = LOSS;
			//	window[1] = DRAW;
			//	/* ���s�T�� */
			//	MAXEVAL = EndSolverWinLoss(bk, wh, 0, window[0], window[1], turn, 0);
			//}
			///* �����������o���ꍇ�́A���������ȏ�̉\��������̂ŁA�����Đݒ肵�ĒT�� */
			//else if(g < -2 && MAXEVAL == DRAW)
			//{
			//	/* �������������� */
			//	window[0] = DRAW;
			//	window[1] = LOSS;
			//	/* ���s�T�� */
			//	MAXEVAL = EndSolverWinLoss(bk, wh, 0, window[0], window[1], turn, 0);
			//}

			/* ���݂̋ǖʂ̍őP���u���\���璊�o */
			move = hash->data[key].bestmove;

			sprintf_s(msg, "@(%s)", winlossdraw_str[MAXEVAL + 1], winlossdraw_str[MAXEVAL + 1]);
			SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 2, (LPARAM)msg);

			/* �΍��T���p�ɒu���\�̓��_���v�Z */
			init_substitution_table_exact(hash);

			if(MAXEVAL == WIN){
				wld = WIN;
				if(g < 9){
					g = 9;
				}
				else if(g > 56){

					g = 56;
				}
				/* �����m��(�]���l - 8 < �΍� < �]���l + 8 �ɂ��邱�Ƃ�����) */
				window[0] = g - 8;
				window[1] = g + 8;
			}
			else if(MAXEVAL == LOSS){
				wld = LOSS;
				/* �����m�� */
				if(g > -9){
					g = -9;
				}
				else if(g < -56){

					g = -56;
				}
				window[0] = g - 8;
				window[1] = g + 8;
			}
			else{
				wld = DRAW;
				/* ���������m��(�����O�Ȃ̂ł�����DRAW���Ԃ邪�ꉞ�B�B) */
				window[0] = DRAW;
				window[1] = DRAW;
			}

			sprintf_s(msg, "���s�m��[%s:%s][%+d�`%+d]", cordinates_table[move], winlossdraw_str[MAXEVAL + 1], window[0], window[1]);
			SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 3, (LPARAM)msg);

			/* �΍��T�� */
			//MPC_FLAG = TRUE;
			MAXEVAL =  EndSolverExact(bk, wh, INF_DEPTH, window[0], window[1], NowTurn, hash, 0);
			//MPC_FLAG = FALSE;

			//init_substitution_table_exact(hash);
			//MAXEVAL =  EndSolverExact(bk, wh, INF_DEPTH, MAXEVAL - 4, MAXEVAL + 4, hash, 0);


			/* fail high */
			if(MAXEVAL >= window[1] && MAXEVAL != 64)
			{
				sprintf_s(msg, "fail high �΍� >= %+d", MAXEVAL);
				SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 3, (LPARAM)msg);

				if(AI_thinking)
				{
					//move =  EndSolverExact_test( bk, wh, INF_DEPTH, window[1], 64, turn, 0, line_stack);

				}
				else
				{
					/* �O�̒T���̏���������ɐݒ肵�āA+64�܂ł��Č��� */
					MAXEVAL = EndSolverExact( bk, wh, INF_DEPTH, window[1], 64, NowTurn, hash, 0);
					move = hash->data[key].bestmove;
				}
			}
			/* fail low */
			else if(MAXEVAL <= window[0] && MAXEVAL != -64)
			{
				sprintf_s(msg, "fail low �΍� <= %+d", MAXEVAL);
				SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 3, (LPARAM)msg);

				if(AI_thinking)
				{
					//move =  EndSolverExact_test( bk, wh, INF_DEPTH, -64, window[0], turn, 0, line_stack);
				}
				else
				{
					/* �O�̒T���̉���������ɐݒ肵�āA-64�܂ł��Č��� */
					MAXEVAL =  EndSolverExact( bk, wh, INF_DEPTH, -64, window[0], NowTurn, hash, 0);
					move = hash->data[key].bestmove;
				}
			}
			/* ���݂̋ǖʂ̍őP���u���\���璊�o */
			move = hash->data[key].bestmove;
		}

		/* �A�{�[�g */
		if(Flag_Abort == TRUE)
		{
			end_t = timeGetTime();
			sprintf_s(CountNodeTime_str, "node:%lld, time:%.4f[sec]", 
			COUNT_NODE, (end_t - start_t) / 1000);
			process_abort(move, winlossdraw_str[wld]);
			if(MAXEVAL > max_score)
			{
				return move;
			}
			else
			{
				MAXEVAL = max_score;
				return max_move;
			}
		}

		max_score = MAXEVAL;
		max_move = move;

		g = MAXEVAL;

		if(AI_thinking)
		{
			line_stack[INF_DEPTH] = -1;
			line_stack[INF_DEPTH + 1] = -2;
			print_line(line_stack);
			cpy_stack(line, line_stack, 0);
		}

		end_t = timeGetTime();
		sprintf_s(CountNodeTime_str, "node:%lld, time:%.4f[sec]", 
			COUNT_NODE, (end_t - start_t) / 1000);
		/* �A�{�[�g */
		if(Flag_Abort == TRUE)
		{
			process_abort(move, winlossdraw_str[wld]);
			if(MAXEVAL > max_score)
			{
				return move;
			}
			else
			{
				MAXEVAL = max_score;
				return max_move;
			}
		}


		sprintf_s(msg, "%+d(%s)", MAXEVAL, winlossdraw_str[wld + 1]);
		SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 2, (LPARAM)msg);

	}

	if(!AI_thinking)
	{
		sprintf_s(msg, "�m��΍�[%s:%+d]", cordinates_table[move], MAXEVAL);
		SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 3, (LPARAM)msg);
	}


	return move;
}

int middle_process(BitBoard bk, BitBoard wh, int depth, Hash *hash)
{
	int move;
	/* �����[�������l */
	int cnt = 2;
	char msg[64];
	int max_score = ABORT, max_move = ABORT;
	int score = 0;
	int alpha = NEGAMIN, beta = NEGAMAX;
	int key = KEY_HASH_MACRO(bk, wh);
	bestLineHeadKey = key;
	BASE_NODE_NUM = 0;
	init_bestline();

	//depth = 18;
	if(AI_result && !(OnTheWay || AI_thinking))
	{
		COUNT_NODE = 0;
		MAXEVAL = NEGAMIN;
		start_t = timeGetTime();
		INF_DEPTH = RemainStone(bk, wh);
		TurnNum = 60 - INF_DEPTH;

		do{
			LIMIT_DEPTH = cnt;
			max_score = score;
			//read_eval_table(turn + LIMIT_DEPTH - 1);
			
			score = pv_search_middle(bk, wh, LIMIT_DEPTH, alpha, beta, NowTurn, hash, 0);

			move = hash->data[key].bestmove;
			//print_bestline(hash, LIMIT_DEPTH);

			if(score == ABORT){
				break;
			}
			/* �T�����s */
			if(score <= alpha)
			{
				sprintf_s(msg, "(eval <= %d)[Fail low]", score);
				SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 3, (LPARAM)msg);
				score = pv_search_middle(bk, wh, LIMIT_DEPTH, NEGAMIN, alpha, NowTurn, hash, 0);
				move = hash->data[key].bestmove;
				//print_bestline(hash, LIMIT_DEPTH);
			}
			else if(score >= beta)
			{
				sprintf_s(msg, "(eval >= %d)[Fail high]", score);
				score = pv_search_middle(bk, wh, LIMIT_DEPTH, beta, NEGAMAX, NowTurn, hash, 0);
				move = hash->data[key].bestmove;
				//print_bestline(hash, LIMIT_DEPTH);
			}

			if(score > 1400000 || score < -1400000)
			{
				if(score > 1400000)
				{
					sprintf_s(msg, "%s[%+d](depth = %d)", cordinates_table[move], (score - 1400000), cnt);
				}
				else if(score < -1400000)
				{
					sprintf_s(msg, "%s[%d](depth = %d)", cordinates_table[move], (score + 1400000), cnt);
				}
				else
				{

					sprintf_s(msg, "%s[0](depth = %d)", cordinates_table[move], cnt);
				}
				break;
			}
			else
			{
				if(NowTurn == BLACK){
					sprintf_s(msg, "%s[%+d](depth = %d)", cordinates_table[move], score, cnt);
				}
				else{
					sprintf_s(msg, "%s[%+d](depth = %d)", cordinates_table[move], score, cnt);
				}
			}

			alpha = score - 40000;
			beta = score + 40000;

			SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 3, (LPARAM)msg);
			cnt+=2;
		}while(cnt <= depth && !Flag_Abort);

		end_t = timeGetTime();
		sprintf_s(CountNodeTime_str, "node:%lld, time:%.4f[sec]", 
			COUNT_NODE, (end_t - start_t) / 1000);

		max_move = move;

		MAXEVAL = score;
		MAX_MOVE = max_move;
		sprintf_s(msg, "%+0.3f[%s]", (double)score / EVAL_ONE_STONE, cordinates_table[max_move]);
		SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 2, (LPARAM)msg);
	}
	else
	{
		return 0;
	}

	if(max_move == ABORT)
	{
		if(move != -1){
			max_move = move;
		}
		return max_move;
	}
	if(Flag_Abort)
	{
		sprintf_s(msg, "%+0.3f[%s]", (double)max_score / EVAL_ONE_STONE, cordinates_table[max_move]);
		SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 2, (LPARAM)msg);
		if(MAXEVAL > max_score)
		{
			sprintf_s(msg, "[interrupted]%s[%+.3f](depth = %d)", cordinates_table[move], (double)max_score / EVAL_ONE_STONE, cnt - 2);
			SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 3, (LPARAM)msg);
			MAXEVAL = max_score;
			return move;
		}
		else
		{
			sprintf_s(msg, "[interrupted]%s[%+.3f](depth = %d)", cordinates_table[max_move], (double)max_score / EVAL_ONE_STONE, cnt - 2);
			SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 3, (LPARAM)msg);
			MAXEVAL = max_score;
			return max_move;
		}
	}

	return move;
}

int CpuProcess(HWND hWnd, HWND hStatus)
{

	AI_THINKING = TRUE;
	char AI_msg[FILE_LEN];
	int move;
	BitBoard bk = black, wh = white;
	COUNT_NODE = 0;
	COUNT_NODE2 = 0;
	/* �q���g�p�̕]���f�[�^�j�� */
	//for(int i = 0; i < 64; i++){ EvalData[i] = NEGAMAX;}

	/* ��΂̒T�� */

	turn = 60 - RemainStone(black, white);
	if(UseBook)
	{
		m_FlagBook = TRUE;
		if(turn == 0)
		{
			int cnt;
			BitBoard moves = CreateMoves(black, white, &cnt);
			int rnd = rand() % cnt;

			while(rnd)
			{
				moves &= moves - 1;
				rnd--;
			}
			move = CountBit((moves & (-(INT64)moves)) - 1);
		}
		else 
		{
			if(Undo_Node[turn + 1] == NULL)
			{
				move = search_books(book_tree.child, bk, wh, turn);
			}
			else
			{
				move = search_books(Undo_Node[turn + 1], bk, wh, turn);
			}
		}
		if(move != -1)
		{
			/* ���X�g�r���[�ɒ����ǉ� */
			ClearListBoxItem(turn);
			sprintf_s(msg, "%s%s", turn_str[NowTurn], cordinates_table[move]);
			SendMessage(hListBox, LB_ADDSTRING, (WPARAM)turn, (LPARAM)msg);
			SendMessage(hListBox, LB_SETCURSEL, (WPARAM)turn, 0);
			m_FlagBook = TRUE;
			move_x = move / MASS_NUM;
			move_y = move % MASS_NUM;
			DropDownStone(bk, wh, move_x, move_y);
			NowTurn = !NowTurn;
			turn++;
			HDC hdc = GetDC(hWnd);
			DrawBoard( hdc, hStatus, move_x, move_y);
			ReleaseDC(hWnd, hdc);
			//get_book_name(&book_tree, black, white);
			//SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 0, (LPARAM)book_msg);
			/* ���̋ǖʂ��AUNDO�@�\�̂��߂ɕۑ� */
			save_undoboard(black, white, move_x, move_y, turn);
			AI_THINKING = FALSE;
			disp_eval();
			return IsWin(black, white);
		}
		m_FlagBook = FALSE;
	}

	/* �^�[�����ɑΉ������]���e�[�u���̓ǂݍ��� */
	read_eval_table(turn + (difficult[NowTurn] * 2) - 1);
	SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 0, (LPARAM)msg);

	int end_turn = 60 - difficult_table[difficult[NowTurn] - 1];

	if(NowTurn == WHITE)
	{
		bk = white;
		wh = black;
	}
	else
	{
		bk = black;
		wh = white;
	}

	
	/* �u���\�̎擾 */
	//Hash *hash;
	//hash = HashNew(21);
	//HashClear(hash);

	if (ghash == NULL)
	{
		ghash = HashNew(21);
		HashClear(ghash);
	}

	int key = KEY_HASH_MACRO(bk, wh);
	ghash->data[key].bestLocked = PREPARE_LOCKED;
	//init_substitution_table(ghash);

	/* AI���s */
	do{


		/* WinLossDraw�T�� */
		if(Force_SearchWin || (!Force_SearchSD && (turn >= end_turn && turn < end_turn + 2)))
		{
			init_substitution_table_winloss(ghash);
			m_FlagSearchWin = TRUE;
			m_FlagPerfectSearch = FALSE;
			move = winlossdraw_process(bk, wh, ghash);
		}
		/* Exact�T�� */
		else if(Force_SearchSD || turn >= end_turn + 2)
		{
			init_substitution_table_exact(ghash);
			m_FlagPerfectSearch = TRUE;
			m_FlagSearchWin = FALSE;
			move = exact_process(bk, wh, ghash);
		}
		/* ���Ւ��Ղ̍őP��̒T�� */
		else
		{
			/* �u���\�̏����� */
			init_substitution_table(ghash);
			m_FlagPerfectSearch = FALSE;
			m_FlagSearchWin = FALSE;
			/* ��Փx�ɂ��ǂސ[�������� */
			move = middle_process(bk, wh, difficult[NowTurn] * 2, ghash);
		}

		/* �A�{�[�g */
		if(move == ABORT)
		{
			AI_THINKING = FALSE;
			HashDelete(ghash);
			return 0;
		}

		/* ���X�g�r���[�ɒ����ǉ� */
		ClearListBoxItem(turn);
		sprintf_s(msg, "%s%s", turn_str[turn % 2], cordinates_table[move]);
		SendMessage(hListBox, LB_INSERTSTRING, (WPARAM)turn, (LPARAM)msg);
		SendMessage(hListBox, LB_SETCURSEL, (WPARAM)turn, 0);
		bk = black;
		wh = white;
		/* �{�[�h�ɒ��� */
		move_x = move / MASS_NUM;
		move_y = move % MASS_NUM;
		DropDownStone(bk, wh, move_x, move_y);
		turn++;
		NowTurn = !NowTurn;

		if(Flag_Abort)
		{
			int score = abs(MAXEVAL);
			if(m_FlagPerfectSearch && score > 64)
			{
				if(score == 65)
				{
					MAXEVAL = ABORT;
				}
				else
				{
					m_FlagPerfectSearch = FALSE;
				}
			}
		}

		/* �ĕ`�� */
		HDC hdc = GetDC(hWnd);
		SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 0, (LPARAM)"");
		DrawBoard( hdc, hStatus, move_x, move_y);
		ReleaseDC(hWnd, hdc);
		/* ���̋ǖʂ��AUNDO�@�\�̂��߂ɕۑ� */
		save_undoboard(black, white, move_x, move_y, turn);

		/* �f�o�b�O���b�Z�[�W���� */
		if(AI_result || OnTheWay || AI_thinking)
		{
			/* �T�����ʂ̕\�� */
			if(AI_result)
			{
				SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 0, (LPARAM)CountNodeTime_str);
			}
			if(AI_thinking)
			{
				SendMessage(hStatus, SB_SETTEXT, (WPARAM)3 | 0, (LPARAM)AI_msg);
			}
			int i = 0;
			while(i < FILE_LEN - 1)
			{
				AI_msg[i] = '\0';
				i++;
			}
		}

		if(NowTurn == BLACK)
		{
			bk = white;
			wh = black;
		}
		else
		{
			bk = black;
			wh = white;
		}

		/* CPU���ł�����A�v���C���[�̓p�X�̋ǖʂ��H */
		/* �p�X�ł���΁A�ēx��2���� */
		if(IsPass(wh, bk))
		{
			/* CPU���p�X�Ȃ珟�s���� */
			if(IsPass(bk, wh))
			{
				AI_THINKING = FALSE;
				//HashDelete(ghash);
				return IsWin(bk, wh);
			}
			MessageBox(hWnd, "player pass","CPU message", MB_OK);
			NowTurn = !NowTurn;
		}
		else
		{
			/* �p�X�łȂ��Ȃ�A���̊֐����I�� */
			break;
		}
	}while(1);
	AI_THINKING = FALSE;

	//HashDelete(ghash);

	return CONTINUE;
}

int GameProcess(HWND hWnd, HWND hStatus)
{
	HDC hdc = GetDC(hWnd);

	int ret;
	int pass_cnt = 0;

	do
	{
		/* ���̋ǖʂ��AUNDO�@�\�̂��߂ɕۑ� */
		save_undoboard(black, white, move_x, move_y, turn);

		/* CPU�̃^�[�� */
		if(player[NowTurn] == CPU)
		{
			/* �p�X�łȂ����AI�N�� */
			if((NowTurn == BLACK && !IsPass(black, white)) || (NowTurn == WHITE && !IsPass(white, black)))
			{
				pass_cnt = 0;
				ret = CpuProcess(hWnd, hStatus);
				disp_eval();
			}
			else
			{
				NowTurn = !NowTurn;
				pass_cnt++;
				if(pass_cnt == 2)
				{
					ret = IsWin(black, white);
					break;
				}
				ret = CONTINUE;
			}
		}
		/* �l�Ԃ̃^�[�� */
		else
		{
			/* �p�X���ǂ��� */
			if((NowTurn == BLACK && IsPass(black, white)) || (NowTurn == WHITE && IsPass(white, black)))
			{
				NowTurn = !NowTurn;
				pass_cnt++;
				ret = CONTINUE;
			}
			else
			{
				/* �{�[�h�̕\���Ƃ��̊֐��̏I�� */
				DrawBoard( hdc, hStatus, move_x, move_y);
				ReleaseDC(hWnd, hdc);
				ret = CONTINUE;
				break;
			}
		}
		if(player[BLACK] == CPU && player[WHITE] == CPU){ Sleep(50); }
		if(Flag_Abort == TRUE){
			return 0;
		}
	}while(ret == CONTINUE);

	//HashDelete(ghash);

	return ret;
}

void OnStartGame(HWND hWnd, HWND hStatus)
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B

	if(IsPass(black, white) && IsPass(white, black))
	{
		MessageBox(hWnd, "���҃p�X�̋ǖʂł��B", "�J�n�G���[", MB_OK);
		return;
	}
	else if(IsPass(black, white) && NowTurn == BLACK)
	{
		MessageBox(hWnd, "���̓p�X�̋ǖʂł��B", "����", MB_OK);
		NowTurn = WHITE;
	}
	else if(IsPass(white, black) && NowTurn == WHITE)
	{
		MessageBox(hWnd, "���̓p�X�̋ǖʂł��B", "����", MB_OK);
		NowTurn = BLACK;
	}
	HINT_DISPED = FALSE;
	/* �ǖʂ���^�[�������擾 */
	turn = 60 - RemainStone(black, white);

	m_FlagInGame = TRUE;
	m_FlagInterrupt = FALSE;
	Flag_Edit = FALSE;

	/* ���j���[�o�[�̊e���ڂ̗L�������� */
	HMENU hMenu = GetMenu(hWnd);
	EnableMenuItem(hMenu, interrupt, MFS_ENABLED); //���f���ڗL��
	EnableMenuItem(hMenu, EditBoard, MFS_GRAYED); //�ՖʕҏW����
	EnableMenuItem(hMenu, ChangeColor, MFS_GRAYED);

	/* �Q�[���X�^�[�g */
	int result = GameProcess(hWnd, hStatus);	


	if(result != CONTINUE)
	{
		/* ���s���ʂ̕\���ƃQ�[���̏I������ */
		DispWinLoss(hWnd, hStatus);
	}

}

void LButtonClick(HWND hWnd, HWND hStatus, LPARAM lp)
{
	BitBoard temp;
	BitBoard bk = black, wh = white;
	int x, y;
	x = LOWORD(lp)/ MASS_SIZE;
	y = HIWORD(lp) / MASS_SIZE - 1;

	/* ���̋ǖʂ��AUNDO�@�\�̂��߂ɕۑ� */
	save_undoboard(bk, wh, move_x, move_y, turn);
	//EnableMenuItem(GetMenu(hWnd), Undo, MFS_ENABLED);

	/* �v���C���[�͐��H���H */
	/* ���Ȃ�black��white�����ւ���B */
	if(CpuColor == BLACK)
	{
		temp = bk;
		bk = wh;
		wh = temp;
	}

	/* �q���g�p�̕]���f�[�^�j�� */
	//int i;
	//for(i = 0; i < 64; i++){ EvalData[i] = NEGAMAX;}
	PassCnt = 0;
	/* ����ւ������ɖ߂��ĉ�ʍX�V*/
	if(CpuColor == BLACK)
	{
		temp = bk;
		bk = wh;
		wh = temp;
	}

	/* ���X�g�r���[�ɒ����ǉ� */
	ClearListBoxItem(turn);
	sprintf_s(msg, "%s%s", turn_str[turn % 2], cordinates_table[x * MASS_NUM + y % MASS_NUM]);
	SendMessage(hListBox, LB_INSERTSTRING, (WPARAM)turn, (LPARAM)msg);
	SendMessage(hListBox, LB_SETCURSEL, (WPARAM)turn, 0);
	turn++;
	move_x = x;
	move_y = y;
	DropDownStone(bk, wh, x, y);
	m_FlagForWhite = !m_FlagForWhite;
	NowTurn = !NowTurn;

	HDC hdc = GetDC(hWnd);
	DrawBoard( hdc, hStatus, x, y);
	ReleaseDC(hWnd, hdc);

	/* ���̋ǖʂ��AUNDO�@�\�̂��߂ɕۑ� */
	save_undoboard(black, white, move_x, move_y, turn);
	bk = black;
	wh = white;

	SendMessage(hStatus, SB_SETTEXT, (WPARAM)1 | 0, (LPARAM)turn_str[NowTurn]);

	/* �l�ԓ��m */
	if(player[NowTurn] == HUMAN)
	{
		HDC hdc = GetDC(hWnd);
		DrawBoard( hdc, hStatus, x, y);
		ReleaseDC(hWnd, hdc);
		return;
	}

	if(CpuColor == BLACK && IsPass(bk, wh) || CpuColor == WHITE && IsPass(wh, bk))
	{
		if(CpuColor == BLACK && IsPass(wh, bk) || CpuColor == WHITE && IsPass(bk, wh))
		{
			/* ���s���ʂ̕\�� */
			DispWinLoss(hWnd, hStatus);
			return;
		}
		else
		{
			NowTurn = !NowTurn;
			m_FlagForWhite = !m_FlagForWhite;
			HDC hdc = GetDC(hWnd);
			DrawBoard( hdc, hStatus, x, y);
			ReleaseDC(hWnd, hdc);
			GameThread(0, STARTGAME);
			return;
		}
	}

	/* CPU��AI�N�� */
	if (ghash == NULL)
	{
		ghash = HashNew(21);
		HashClear(ghash);
	}
	int result = CpuProcess(hWnd, hStatus);
	if(result != CONTINUE)
	{
		/* ���s���ʂ̕\�� */
		DispWinLoss(hWnd, hStatus);
	}
	/* ���̋ǖʂ��AUNDO�@�\�̂��߂ɕۑ� */
	save_undoboard(black, white, move_x, move_y, turn);
	HINT_DISPED = FALSE;
}

void Edit_Board(HWND hWnd, HWND hStatus, LPARAM lp)
{
	int x, y;

	x = LOWORD(lp) / MASS_SIZE;
	y = HIWORD(lp) / MASS_SIZE - 1;

	/* �͈͊O�̏ꍇ */
	if(x >= MASS_NUM || y >= MASS_NUM)
	{
		return;
	}

	black ^= one << (x * MASS_NUM + y);
	if(white & one << (x * MASS_NUM + y))
	{
		white ^= one << (x * MASS_NUM + y);
	}
	HDC hdc = GetDC(hWnd);
	DrawBoard( hdc, hStatus, -1, -1);
	ReleaseDC(hWnd, hdc);
}

void RButtonClick(HWND hWnd, HWND hStatus, LPARAM lp)
{
	if(m_FlagInGame || !Flag_Edit)
	{
		return;
	}

	int x, y;

	x = LOWORD(lp) / MASS_SIZE;
	y = HIWORD(lp) / MASS_SIZE - 1;

	/* �͈͊O�̏ꍇ */
	if(x >= MASS_NUM || y >= MASS_NUM)
	{
		return;
	}

	white ^= one << (x * MASS_NUM + y);

	if(black & one << (x * MASS_NUM + y))
	{
		black ^= one << (x * MASS_NUM + y);
	}
	HDC hdc = GetDC(hWnd);
	DrawBoard( hdc, hStatus, -1, -1);
	ReleaseDC(hWnd, hdc);
}