#pragma once

/* �q���g�t���O */
extern int HINT_LEVEL;
extern BOOL m_FlagHint;
extern double EvalData[64];
extern int PosData[64];

/* �q���g�֘A�֐� */
int Hint_AI(BitBoard, BitBoard, char, int, int, int, int);
int Hint_WinLoss(BitBoard, BitBoard, char, int, int, int, int);
int Hint_Exact(BitBoard, BitBoard, char, int, int, int, int);