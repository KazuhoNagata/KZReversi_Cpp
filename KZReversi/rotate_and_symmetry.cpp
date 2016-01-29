/*
############################################################################
#
#  �ǖʂ̑Ώ̕ϊ��֐�
#  (�ЂƂ̋ǖʂɂ́A��]�E�Ώ̂̊֌W�ɂ���7�̓���ǖʂ��܂܂�Ă���) 
#
############################################################################
*/
#include "stdafx.h"
#include "KZReversi.h"

BitBoard rotate_90(BitBoard board)
{
	/* �����v���90�x�̉�] */
	board = board << 1 & 0xaa00aa00aa00aa00 |
		    board >> 1 & 0x0055005500550055 |
			board >> 8 & 0x00aa00aa00aa00aa |
			board << 8 & 0x5500550055005500;

	board = board << 2 & 0xcccc0000cccc0000 |
			board >> 2 & 0x0000333300003333 |
			board >>16 & 0x0000cccc0000cccc |
			board <<16 & 0x3333000033330000;

	board = board << 4 & 0xf0f0f0f000000000 |
			board >> 4 & 0x000000000f0f0f0f |
			board >>32 & 0x00000000f0f0f0f0 |
			board <<32 & 0x0f0f0f0f00000000;

	return board;
}

BitBoard rotate_180(BitBoard board)
{
	/* �����v���180�x�̉�] */
	board = board << 9 & 0xaa00aa00aa00aa00 |
		    board >> 9 & 0x0055005500550055 |
			board >> 7 & 0x00aa00aa00aa00aa |
			board << 7 & 0x5500550055005500;

	board = board <<18 & 0xcccc0000cccc0000 |
			board >>18 & 0x0000333300003333 |
			board >>14 & 0x0000cccc0000cccc |
			board <<14 & 0x3333000033330000;

	board = board <<36 & 0xf0f0f0f000000000 |
			board >>36 & 0x000000000f0f0f0f |
			board >>28 & 0x00000000f0f0f0f0 |
			board <<28 & 0x0f0f0f0f00000000;

	return board;
}

BitBoard rotate_270(BitBoard board)
{
	/* �����v���270�x�̉�]=���v���90�x�̉�] */
	board = board << 8 & 0xaa00aa00aa00aa00 |
		    board >> 8 & 0x0055005500550055 |
			board << 1 & 0x00aa00aa00aa00aa |
			board >> 1 & 0x5500550055005500;

	board = board <<16 & 0xcccc0000cccc0000 |
			board >>16 & 0x0000333300003333 |
			board << 2 & 0x0000cccc0000cccc |
			board >> 2 & 0x3333000033330000;

	board = board <<32 & 0xf0f0f0f000000000 |
			board >>32 & 0x000000000f0f0f0f |
			board << 4 & 0x00000000f0f0f0f0 |
			board >> 4 & 0x0f0f0f0f00000000;

	return board;
}

BitBoard symmetry_x(BitBoard board)
{
	/* X���ɑ΂��Ώ̕ϊ� */
	board = board << 1 & 0xaaaaaaaaaaaaaaaa |
		    board >> 1 & 0x5555555555555555;

	board = board << 2 & 0xcccccccccccccccc |
			board >> 2 & 0x3333333333333333;

	board = board << 4 & 0xf0f0f0f0f0f0f0f0 |
			board >> 4 & 0x0f0f0f0f0f0f0f0f;

	return board;
}

BitBoard symmetry_y(BitBoard board)
{
	/* Y���ɑ΂��Ώ̕ϊ� */
	board = board << 8 & 0xff00ff00ff00ff00 |
		    board >> 8 & 0x00ff00ff00ff00ff;

	board = board << 16& 0xffff0000ffff0000 |
			board >> 16& 0x0000ffff0000ffff;

	board = board << 32& 0xffffffff00000000 |
			board >> 32& 0x00000000ffffffff;

	return board;
}

BitBoard symmetry_b(BitBoard board)
{
	/* �u���b�N���C���ɑ΂��Ώەϊ� */
	board = board >> 9 & 0x0055005500550055 |
			board << 9 & 0xaa00aa00aa00aa00 |
			board      & 0x55aa55aa55aa55aa;
			
	board = board >>18 & 0x0000333300003333 |
			board <<18 & 0xcccc0000cccc0000 |
			board      & 0x3333cccc3333cccc;

	board = board >>36 & 0x000000000f0f0f0f |
			board <<36 & 0xf0f0f0f000000000 |
			board      & 0x0f0f0f0ff0f0f0f0;

	return board;
}

BitBoard symmetry_w(BitBoard board)
{
	/* �z���C�g���C���ɑ΂��Ώەϊ� */
	board = board >> 7 & 0x00aa00aa00aa00aa |
			board << 7 & 0x5500550055005500 |
			board      & 0xaa55aa55aa55aa55;
			
	board = board >>14 & 0x0000cccc0000cccc |
			board <<14 & 0x3333000033330000 |
			board      & 0xcccc3333cccc3333;

	board = board >>28 & 0x00000000f0f0f0f0 |
			board <<28 & 0x0f0f0f0f00000000 |
			board      & 0xf0f0f0f00f0f0f0f;

	return board;
}