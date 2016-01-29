/*#
  # ��΃f�[�^�̓ǂݍ��݁A�T�����s���܂�
  #  
  ##############################################
*/

#include "stdafx.h"
#include "KZReversi.h"
#include "hint.h"
#include "rotate_and_symmetry.h"
#include "GetRev.h"
#include "book.h"
#include "game.h"
#include "eval.h"

int ConvertToMove(char *);
BOOL UseBook;
BOOL b_Flag_not_change;
BOOL b_Flag_little_change;
BOOL b_Flag_change;
int TRANCE_MOVE;
int m_FlagBook;

int max_change_num[2];

/* ��΃c���[ */

BooksNode *BeforeNode;
BooksNode *Undo_Node[120];
BooksNode book_tree;

/* �m�[�h��ڑ� */
void append(BooksNode *parent, BooksNode *node)
{
	//node->book_name = name;
	//node->move = move;
	node->child = NULL;
	node->next = NULL;
	if(parent == NULL)
	{
		return;
	}
	if(parent->child == NULL)
	{
		parent->child = node;
	}
	else
	{
		BooksNode *last = parent->child;
		while(last->next != NULL)
		{
			last = last->next;
		}
		last->next = node;
	}
}
//
//int count_depth(char *str)
//{
//	int cnt = 0;
//	while(str[cnt] == '-')
//	{
//		cnt++;
//	}
//	return cnt;
//}

///* ��΂̖؍\�����쐬 */
//BooksNode *struction(BooksNode *head, int depth, char *data, char *str, FILE *fp)
//{
//	/* ��΃t�@�C����EOF */
//	if(!strcmp(data, "END"))
//	{
//		return head;
//	}
//
//	/* �܂��̓m�[�h�̍쐬 */
//	BooksNode *node = (BooksNode *)malloc(sizeof(BooksNode));
//	/* ��΂̖��O�̏ꍇ */
//	if(data[0] != '-')
//	{
//		//book_name_flag = 1;
//		strcpy_s(str, LEN, data);
//		/* ���s�폜 */
//		char *p = strchr(str, '\n');
//		if(p != NULL)
//		{
//			*p = '\0';
//		}
//		/* ���̍s�͒�΃f�[�^ */
//		fgets(data, LEN, fp);
//	}
//	/* ��΃f�[�^�̏ꍇ */
//	/* �T�����Ă���[���ƈ�v����Ȃ� */
//	if(count_depth(data) == depth)
//	{
//		/*  ���݂̃w�b�_�ɂȂ���*/
//		append(head, node);
//		data[0] = data[depth];
//		data[1] = data[depth + 1];
//		data[2] = '\0';
//		int move = ConvertToMove(data);
//		node->move = move;
//		node->depth = depth;
//		//if(book_name_flag == 1)
//		//{
//		//strcpy_s(node->book_name, LEN, str);
//		//book_name_flag = 0;
//		//}
//		//else 
//		//{
//		//	strcpy_s(node->book_name, LEN, "");
//		//}
//		/* �ǖʏ����i�[ */
//		node->bk = black;
//		node->wh = white;
//		if(depth % 2)
//		{
//			BitBoard rev = GetRev[move](white, black, one << move);
//			white ^= (one << move) | rev;
//			black ^= rev;
//		}
//		else
//		{
//			BitBoard rev = GetRev[move](black, white, one << move);
//			black ^= (one << move) | rev;
//			white ^= rev;
//			
//		}
//		fgets(data, LEN, fp);
//		//HDC hdc = GetDC(hWnd);
//		//DrawBoard(hWnd, hdc, hStatus, move / MASS_NUM, move % MASS_NUM);
//		//ReleaseDC(hWnd, hdc);
//		//Sleep(0);
//		/* ���̒�΃f�[�^�� */
//		return struction(node, depth + 1, data, str, fp);
//	}
//	else
//	{
//		black = head->bk;
//		white = head->wh;
//		/* �[������v���Ȃ��Ȃ�O�̃m�[�h�ɖ߂��č\�z�����݂� */
//		return struction(head->prev, depth - 1, data, str, fp);
//	}
//}

///* �f�o�b�O�֐� */
//void output_data(BooksNode *node, FILE *fp)
//{
//	int i;
//	if(strcmp(node->book_name, ""))
//	{
//		fprintf(fp, "%s\n", node->book_name);
//	}
//	for(i = 0; i < node->depth;i++)
//	{
//		fprintf(fp, "-");
//	}
//	fprintf(fp, ":%s\n%llx\n%llx\n", cordinates_table[node->move], node->bk, node->wh);
//}
///* �f�o�b�O�֐� */
//void print_tree(BooksNode *node, FILE *fp)
//{
//	BitBoard temp_b = black;
//	BitBoard temp_w = white;
//	if(node == NULL)
//	{
//		return;
//	}
//	output_data(node, fp);
//	print_tree(node->child, fp);
//	black = temp_b;
//	white = temp_w;
//	print_tree(node->next, fp);
//}

BooksNode *search_child(BooksNode *head, int move)
{
	if(head->child == NULL)
	{
		return NULL;
	}
	head = head->child;
	while(head != NULL)
	{
		if(head->move == move)
		{
			return head;
		}
		head = head->next;
	}

	return NULL;
}

void st_tree_from_line(BooksNode *head, char *line, int eval)
{
	short depth = 0;
	char move_str[3];
	BitBoard bk = FIRST_BK, wh = FIRST_WH;
	BitBoard rev;
	int line_len = strlen(line);
	BooksNode *head_child;

	while(depth < line_len)
	{
		//move_str[0] = 'a' + line[depth] - 'A';
		move_str[0] = line[depth];
		move_str[1] = line[depth + 1];
		move_str[2] = '\0';
		/* a1��0 �Ȃ� ���l�ɕϊ� */
		int move = (move_str[0] - 'a') * 8 + move_str[1] - '1';

		/* ���łɓo�^����Ă���m�[�h�����邩�T�� */
		head_child = search_child(head, move);

		/* �V�K�̏ꍇ */
		if(head_child == NULL)
		{
			BooksNode *node = (BooksNode *)malloc(sizeof(BooksNode));
			node->move = (short )move;
			if(depth % 4)
			{
				rev = GetRev[move](wh, bk);
				node->bk = bk;
				node->wh = wh;
				wh ^= (one << move) | rev;
				bk ^= rev;
			}
			else
			{
				rev = GetRev[move](bk, wh);
				node->bk = bk;
				node->wh = wh;
				bk ^= (one << move) | rev;
				wh ^= rev;
			}
			node->eval = eval;
			node->depth = depth / 2;
			append(head, node);
			head = node;
		}
		/* ���o�̏ꍇ */
		else
		{
			if(depth % 4)
			{
				rev = GetRev[move](wh, bk);
				wh ^= (one << move) | rev;
				bk ^= rev;
			}
			else
			{
				rev = GetRev[move](bk, wh);
				bk ^= (one << move) | rev;
				wh ^= rev;
			}
			head = head_child;
		}
		depth += 2;
	}
}

void struction_book_tree(BooksNode *head)
{
	int count = 0;
	char *decode_sep, *line_data, *eval_str;
	char *next_str, *next_line;
	HWND progress_handle = GetDlgItem(DlgHandle, IDC_PROGRESS1);

	UCHAR *decodeData = (UCHAR *)malloc(18160640);
	int decodeDataLen = decodeBookData(decodeData);

	if(decodeDataLen == -1)
	{
		return;
	}

	decode_sep = strtok_s((char *)decodeData, "\n", &next_line);
	do
	{
		/* �t�@�C������1�s�Âǂݍ���Ŗ؍\�����쐬 */
		line_data = strtok_s(decode_sep, ";", &next_str);
		eval_str = strtok_s(next_str, ";", &next_str);
		st_tree_from_line(head, line_data, (int)(atof(eval_str) * EVAL_ONE_STONE));
		count++;
		/* �v���O���X�o�[�̍X�V */
		if(count % 1080 == 0)
		{
			PostMessage(progress_handle, PBM_STEPIT, 0, 0);
		}

	}while((decode_sep = strtok_s(next_line, "\n", &next_line)) != NULL);

}

void open_book()
{
	/* ��΃c���[���\�� */
	//book_name_flag = 1;
	BooksNode *root = &book_tree;
	root->bk = FIRST_BK;
	root->wh = FIRST_WH;
	root->move = 64;
	root->eval = 0;
	root->depth = 0;
	struction_book_tree(root);

	//struction(&book_tree, 0, data, str, fp);
	//fclose(fp);

	//black = book_tree.bk;
	//white = book_tree.wh;
	/*fopen_s(&fp, "C:\\output_books.dat", "w");
	print_tree(book_tree.child, fp);
	fclose(fp);*/
}

/* �ǖʏ�񂩂獷��������� */
BooksNode *search_book_info(BooksNode *book_header, BooksNode *before_book_header, BitBoard bk, BitBoard wh, int turn)
{
	/* �t�m�[�h�܂Ō������Č�����Ȃ��ꍇ */
	if(book_header == NULL)
	{
		return NULL;
	}
	if(book_header->depth > turn)
	{
		return NULL;
	}
	if(book_header->depth == turn)
	{
		if(turn == 0)
		{
			before_book_header = &book_tree;
		}
		/* �Y���̒�΂𔭌�(��]�E�Ώ̂��l����) */
		if(book_header->bk == bk && book_header->wh == wh)
		{
			/* �w����̉�]�E�Ώ̕ϊ��Ȃ� */
			TRANCE_MOVE = 0;
			return before_book_header;
		}
		/* 90�x�̉�]�` */
		if(book_header->bk == rotate_90(bk) && book_header->wh == rotate_90(wh))
		{
			TRANCE_MOVE = 1;
			return before_book_header;
		}
		/* 180�x�̉�]�` */
		if(book_header->bk == rotate_180(bk) && book_header->wh == rotate_180(wh))
		{
			TRANCE_MOVE = 2;
			return before_book_header;
		}
		/* 270�x�̉�]�` */
		if(book_header->bk == rotate_270(bk) && book_header->wh == rotate_270(wh))
		{
			TRANCE_MOVE = 3;
			return before_book_header;
		}
		/* X���̑Ώ̌` */
		if(book_header->bk == symmetry_x(bk) && book_header->wh == symmetry_x(wh))
		{
			TRANCE_MOVE = 4;
			return before_book_header;
		}
		/* Y���̑Ώ̌` */
		if(book_header->bk == symmetry_y(bk) && book_header->wh == symmetry_y(wh))
		{
			TRANCE_MOVE = 5;
			return before_book_header;
		}
		/* �u���b�N���C���̑Ώ̌` */
		if(book_header->bk == symmetry_b(bk) && book_header->wh == symmetry_b(wh))
		{
			TRANCE_MOVE = 6;
			return before_book_header;
		}
		/* �z���C�g���C���̑Ώ̌` */
		if(book_header->bk == symmetry_w(bk) && book_header->wh == symmetry_w(wh))
		{
			TRANCE_MOVE = 7;
			return before_book_header;
		}
	}
	BooksNode *ret;
	if((ret = search_book_info(book_header->child, book_header, bk, wh, turn)) != NULL)
	{
		return ret;
	}
	if((ret = search_book_info(book_header->next, book_header, bk, wh, turn)) != NULL)
	{
		return ret;
	}
	return NULL;
}

/* book������őP�菇�Ƀ\�[�g */
void sort_book_node(BooksNode *best_node[], int e_list[], int cnt)
{
	int i = 0;
	int h = cnt * 10 / 13;
	int swaps;
	BooksNode *temp;
	int int_temp;
	if(cnt == 1){ return; }
	while(1)
	{
		swaps = 0;
		for(i = 0; i + h < cnt; i++)
		{
			if(e_list[i] < e_list[i + h])
			{
				temp = best_node[i];
				best_node[i] = best_node[i + h];
				best_node[i + h] = temp;
				int_temp = e_list[i];
				e_list[i] = e_list[i + h];
				e_list[i + h] = int_temp;
				swaps++;
			}
		}
		if(h == 1)
		{
			if(swaps == 0)
			{
				break;
			}
		}
		else
		{
			h = h * 10 / 13;
		}
	}
}

/* book�ω��x�ɂ���Ďw�����I�� */
int select_node(int e_list[], int cnt)
{
	int ret;
	if(b_Flag_not_change)
	{
		int count;
		int max = e_list[0];
		for(count = 1; count < cnt; count++)
		{
			if(e_list[count] < max)
			{
				break;
			}
		}
		ret = rand() % count;
	}
	/* ���P�肩��-2�ȏ�܂ŋ��� */
	else if(b_Flag_little_change)
	{
		int count;
		int flag = 0;
		int max = e_list[0];
		for(count = 1; count < cnt; count++)
		{
			/* ��x���P���I��ł���A
			   �܂��͎��P�肪���̕]���l���2000�ȏ�Ⴂ�Ƃ���break */
			if(max_change_num[NowTurn] || max - e_list[count] >= 2000)
			{
				break;
			}
			if(e_list[count] < max)
			{
				if(flag)
				{
					break;
				}
				flag++;
			}

		}
		ret = rand() % count;
		/* ���P���I�� */
		if(e_list[ret] != max)
		{
			max_change_num[NowTurn]++;
		}
	}
	else if(b_Flag_change)
	{
		int count;
		int flag = 0;
		int max = e_list[0];
		for(count = 1; count < cnt; count++)
		{
			/* 2�x���P���I��ł���A
			   �܂��͎��P�肪���̕]���l���4000�ȏ�Ⴂ�Ƃ���break */
			if(max_change_num[NowTurn] > 1 || max - e_list[count] >= 4000)
			{
				break;
			}
			if(e_list[count] < max)
			{
				if(flag > 2)
				{
					break;
				}
				flag++;
			}
		}
		ret = rand() % count;
		/* ���P���I�� */
		if(ret != 0 && e_list[ret] != max)
		{
			max_change_num[NowTurn]++;
		}
	}
	else
	{
		ret = rand() % cnt;
	}

	return ret;
}

BooksNode *BestNode;

/* ����̕]���l���Z�o */
int book_alphabeta(BooksNode *book_header, int depth, int alpha, int beta, int color)
{
	if(book_header->child == NULL)
	{
		if(color == WHITE)
		{
			return -book_header->eval;
		}
		return book_header->eval;
	}

	if(depth == 0)
	{
		int i = 0, e_list[24] = 
		{
			NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, 
			NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN
		};
		int eval, max = NEGAMIN - 1;
		BooksNode *best_node[24] = 
		{
			NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
			NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
		};
		do{
			if(i == 0)
			{
				book_header = book_header->child;
			}
			else
			{
				book_header = book_header->next;
			}
			eval = -book_alphabeta(book_header, depth + 1, -beta, -alpha, color ^ 1);
			e_list[i] = eval;
			best_node[i] = book_header;
			if(eval > max)
			{
				max = eval;
			}
			i++;
		}while(book_header->next != NULL);
		sort_book_node(best_node, e_list, i);
		/* �m�[�h�ԍ����Z�o */
		int node_num = select_node(e_list, i);
		BestNode = best_node[node_num];
		return e_list[node_num];
	}
	else
	{
		int i = 0;
		int eval, max = NEGAMIN - 1;
		do{
			if(i == 0)
			{
				book_header = book_header->child;
			}
			else
			{
				book_header = book_header->next;
			}
			eval = -book_alphabeta(book_header, depth + 1, -beta, -alpha, color ^ 1);
			if(eval > max)
			{
				max = eval;
				if(max > alpha)
				{
					alpha = max;   //�����l���X�V
					/* �A���t�@�J�b�g */
					if(beta <= alpha)
					{
						break;
					}
				}
			}
			i++;
		}while(book_header->next != NULL);
		return max;
	}
}

/* �ǖʏ������ɒ�΂����� */
int search_books(BooksNode *book_root, BitBoard bk, BitBoard wh, int turn)
{
	int move = -1;
	srand((unsigned int )time(NULL));

	/* �ǖʂ���Y���̒�΂�T�� */
	BooksNode *book_header = search_book_info(book_root, NULL, bk, wh, turn);
	if(book_header != NULL)
	{
		//if(!m_FlagHint)
		//{
			/* �]���l�ɂ�莟�̎���΂���I�� */
			MAXEVAL = book_alphabeta(book_header, 0, NEGAMIN, NEGAMAX, NowTurn);
			book_header = BestNode;
		//}
		/*if(book_header->book_name != "" && !m_FlagHint)
		{
			SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | 0, (LPARAM)book_header->book_name);
		}*/

		/* �w����̑Ώ̉�]�ϊ��̏ꍇ���� */
		switch(TRANCE_MOVE)
		{
		case 0:
			move = book_header->move;
			break;
		case 1:
			{
				BitBoard t_move = rotate_90(one << book_header->move);
				move =  CountBit((t_move & (-(INT64)t_move)) - 1);
			}
			break;
		case 2:
			{
				BitBoard t_move = rotate_180(one << book_header->move);
				move =  CountBit((t_move & (-(INT64)t_move)) - 1);
			}
			break;
		case 3:
			{
				BitBoard t_move = rotate_270(one << book_header->move);
				move =  CountBit((t_move & (-(INT64)t_move)) - 1);
			}
			break;
		case 4:
			{
				BitBoard t_move = symmetry_x(one << book_header->move);
				move =  CountBit((t_move & (-(INT64)t_move)) - 1);
			}
			break;
		case 5:
			{
				BitBoard t_move = symmetry_y(one << book_header->move);
				move =  CountBit((t_move & (-(INT64)t_move)) - 1);
			}
			break;
		case 6:
			{
				BitBoard t_move = symmetry_b(one << book_header->move);
				move =  CountBit((t_move & (-(INT64)t_move)) - 1);
			}
			break;
		case 7:
			{
				BitBoard t_move = symmetry_w(one << book_header->move);
				move =  CountBit((t_move & (-(INT64)t_move)) - 1);
			}
			break;
		default:
			move = book_header->move;
			break;
		}
	}
	/* save global node. */
	Undo_Node[turn] = book_header;
	return move;
}