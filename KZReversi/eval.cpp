/*#
# ��΃f�[�^�ƕ]���e�[�u���̓ǂݍ��݂��s���܂��D
#  
##############################################
*/

#include "stdafx.h"

#include <stdlib.h>

#include "KZReversi.h"
#include "eval.h"

#define NODE_SIZE 256

char g_fileBuffer[8192];

char charSet[] = 
{
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
	'\n', '.', '-', ';'
};

int convertChar2Index(char chr)
{
	int i;
	for(i = 0; i < 22; i++)
	{
		if(charSet[i] == chr)
		{
			break;
		}
	}

	return i;

}

int CreateHuffmanTree(TreeNode *nodes, int nodeNum)
{
	int freeNode;
	int min1, min2;

	nodes[NODE_SIZE - 1].occurrence = 100000000;          /* �ԕ� */

	for (freeNode = nodeNum + 1; ; freeNode++) {
		min1 = min2 = NODE_SIZE - 1;
		for (int i = NODE_SIZE - 2; i >= 0; i--){
			if (nodes[i].occurrence > 0) {
				if (nodes[i].occurrence < nodes[min1].occurrence) {
					min2 = min1;
					min1 = i;
				} else if (nodes[i].occurrence < nodes[min2].occurrence)
					min2 = i;
			}
		}
		if (min2 == NODE_SIZE - 1) {
			break;
		}
		nodes[freeNode].left = min1;
		nodes[freeNode].right = min2;
		nodes[freeNode].occurrence = nodes[min1].occurrence + nodes[min2].occurrence;
		nodes[min1].parent = freeNode;
		nodes[min2].parent = freeNode;
		nodes[min1].occurrence = 0;
		nodes[min2].occurrence = 0;
	}

	return min1;
}

int writeEncodeData(UCHAR *encodeData, TreeNode *nodes, int root,
	CodeInfo *codeInfo, UCHAR *data, int dataLen){

	int index, div;
	int bitCounter = 0, encodeCounter = 0;
	char bit[16] = {0};
	
	// �n�t�}���؂�ۑ�
	int nodesLen = sizeof(TreeNode) * (root + 1);
	encodeData[encodeCounter++] = (nodesLen & 0xFF00) >> 8;
	encodeData[encodeCounter++] = nodesLen & 0xFF; 

	memcpy_s(&encodeData[encodeCounter], dataLen, nodes, nodesLen);
	encodeCounter += nodesLen;

	for(int i = 0; i < dataLen; i++)
	{
		index = convertChar2Index(data[i]);

		div = codeInfo[index].code;
		for(int j = bitCounter + codeInfo[index].codeSize - 1; j >= bitCounter; j--){

			bit[j] = div % 2;
			div /= 2;
		}

		bitCounter += codeInfo[index].codeSize;

		if(bitCounter >= 8){
			encodeData[encodeCounter++] = (bit[0] << 7) + (bit[1] << 6) + (bit[2] << 5) + 
				(bit[3] << 4) + (bit[4] << 3) + (bit[5] << 2) + (bit[6] << 1) + bit[7];
			bitCounter -= 8;
			for(int j = 0; j < bitCounter; j++){
				bit[j] = bit[8 + j];
			}
		}
	}

	/* bit�]�� */
	memset(&bit[bitCounter], 0, 8 - bitCounter);
	encodeData[encodeCounter++] = (bit[0] << 7) + (bit[1] << 6) + (bit[2] << 5) + 
				(bit[3] << 4) + (bit[4] << 3) + (bit[5] << 2) + (bit[6] << 1) + bit[7];

	return encodeCounter;

}

int generateCode(CodeInfo *codeInfo, TreeNode *node, int nodeIndex, int code, int codeSize) {

	if (node[nodeIndex].left == 0 && node[nodeIndex].right == 0) {
		codeInfo[nodeIndex].code = code;
		codeInfo[nodeIndex].codeSize = codeSize;

		return 1;

	} else {
		int count;
		count = generateCode(codeInfo, node, node[nodeIndex].left, code << 1, codeSize + 1);
		count += generateCode(codeInfo, node, node[nodeIndex].right, (code << 1) + 1, codeSize + 1);

		return count;
	}
}

int encode(UCHAR *encodeData, CodeInfo *codeInfo, UCHAR *data, int dataLen) {

	// �e�o�C�g�l�̔����񐔂𐔂���
	TreeNode nodes[NODE_SIZE];

	memset(nodes, 0, sizeof(nodes));

	for(int i = 0; i < 22; i++)
	{
		nodes[i].chr = charSet[i];
	}

	for(int i = 0; i < dataLen; i++) {

		nodes[convertChar2Index(data[i])].occurrence++;
	}

	// �n�t�}���؂��쐬
	int root = CreateHuffmanTree(nodes, 22 - 1);
	// �[���D��T���Ńo�C�g�l�����������쐬
	int codeInfoNum = generateCode(codeInfo, nodes, root, 0, 0);

	if(codeInfoNum > 22)
	{
		// charSet�Ŏw�肳��Ă��镶���ȊO���܂܂ꂽ�f�[�^��ǂݍ������Ƃ��Ă���
		return -1;
	}

	// ���k�f�[�^���������ɏ�������
	int encodeDataLen = writeEncodeData(encodeData, nodes, root, codeInfo, data, dataLen);

	return encodeDataLen;
}

int decode(UCHAR *decodeData, int maxLen, UCHAR *data, int dataLen, TreeNode *nodes, int root)
{
	int nodeIndex = root;
	int cnt = 0, byteCounter = 0;

	while(byteCounter < dataLen){

		for(int i = 7; i >= 0; i--){

			if(data[byteCounter] & (1 << i))
			{
				nodeIndex = nodes[nodeIndex].right;
			}
			else
			{
				nodeIndex = nodes[nodeIndex].left;
			}
		
			if(nodes[nodeIndex].chr != 0 && cnt < maxLen)
			{
				decodeData[cnt++] = nodes[nodeIndex].chr;
				nodeIndex = root;
			}
		}
		byteCounter++;
	}

	return cnt;

}

int decodeBookData(UCHAR *decodeData)
{
	FILE *fp;
	int readSize, decodeDataLen;

	UCHAR *data = (UCHAR *)malloc(10485760);
	if(fopen_s(&fp, "src\\books.bin", "rb") != 0){
		MessageBox(hWnd, "��΃f�[�^��������܂���", "��΃f�[�^�ǂݍ��݃G���[", MB_OK);
		return -1;
	}

	// open books
	readSize = fread(data, sizeof(UCHAR), 2 * sizeof(int) + 2, fp);
	if(readSize < 2 * sizeof(int) + 2){
		MessageBox(hWnd, "��΃f�[�^�����Ă��܂�", "��΃f�[�^�ǂݍ��݃G���[", MB_OK);
		return -1;
	}

	// �؃f�[�^�̃T�C�Y
	int nodesLen = (data[8] << 8) + data[9];
	// ���k�f�[�^�̃T�C�Y
	int evalDataLen = (data[4] << 24) + (data[5] << 16) + (data[6] << 8) + data[7] - nodesLen - 2;
	// �𓀃f�[�^�̃T�C�Y
	decodeDataLen = (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3];

	TreeNode nodes[43];

	memset(nodes, 0, sizeof(nodes));

	readSize = fread(nodes, sizeof(UCHAR), nodesLen, fp);
	readSize = fread(data, sizeof(UCHAR), evalDataLen, fp);

	int root = nodesLen / sizeof(TreeNode) - 1;

	/* ������ */
    decodeDataLen = decode(decodeData, decodeDataLen, data, readSize, nodes, root);

	fclose(fp);
	
	// debug...
	//FILE *wfp;
	//if(fopen_s(&wfp, "src\\temp.txt", "wb") != 0){
	//	exit(1);
	//}
	//fwrite(decodeData, sizeof(UCHAR), decodeDataLen, wfp);
	//fclose(wfp);

	return decodeDataLen;

}

int decodeEvalData(UCHAR *decodeData, FILE *fp)
{
	int readSize, decodeDataLen;
	UCHAR *data = (UCHAR *)malloc(2097152);

	readSize = fread(data, sizeof(UCHAR), 2 * sizeof(int) + 2, fp);
	// �؃f�[�^�̃T�C�Y
	int nodesLen = (data[8] << 8) + data[9];
	// ���k�f�[�^�̃T�C�Y
	int evalDataLen = (data[4] << 24) + (data[5] << 16) + (data[6] << 8) + data[7] - nodesLen - 2;
	// �𓀃f�[�^�̃T�C�Y
	decodeDataLen = (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3];

	TreeNode nodes[43];

	memset(nodes, 0, sizeof(nodes));

	readSize = fread(nodes, sizeof(UCHAR), nodesLen, fp);
	readSize = fread(data, sizeof(UCHAR), evalDataLen, fp);

	int root = nodesLen / sizeof(TreeNode) - 1;
	/* ������ */
    decodeDataLen = decode(decodeData, decodeDataLen, data, readSize, nodes, root);

	// debug...
	/*FILE *wfp;
	if(fopen_s(&wfp, "src\\temp.txt", "wb") != 0){
		exit(1);
	}
	fwrite(decodeData, sizeof(UCHAR), decodeDataLen, wfp);
	fclose(wfp);*/

	free(data);

	return decodeDataLen;

}


// Ascii�f�[�^����e�[�u���쐬���s�����̂ݎg�p����
void makeData()
{

	FILE *fp, *wfp1, *wfp2;
	char filePath[64], lenData[4];
	int readSize, encodeDataLen;

	UCHAR *data = (UCHAR *)malloc(18160640); // 17735KB

	if(fopen_s(&wfp1, "src\\books.bin", "wb") != 0){
		exit(1);
	}

	if(fopen_s(&wfp2, "src\\eval.bin", "wb") != 0){
		exit(1);
	}
	
	CodeInfo codeInfo[22];
	UCHAR *encodeData = (UCHAR *)malloc(10485760); // 17735KB

	for(int i = -1; i < 60; i++){

		if(i == -1){
			if(fopen_s(&fp, "src\\rawData\\books.dat", "r") != 0){
				exit(1);
			}
		}
		else{
			sprintf_s(filePath, "src\\rawData\\%d.dat", i);
			if(fopen_s(&fp, filePath, "r") != 0){
				exit(1);
			}
		}

		setvbuf(fp, g_fileBuffer, _IOFBF, 16384);
		readSize = fread(data, sizeof(UCHAR), 18160640, fp);

		memset(codeInfo, 0, sizeof(CodeInfo));
		encodeDataLen = encode(encodeData, codeInfo, data, readSize);

		if(encodeDataLen == -1){
			return;
		}

		lenData[0] = (readSize & 0xFF000000) >> 24;
		lenData[1] = (readSize & 0xFF0000) >> 16;
		lenData[2] = (readSize & 0xFF00) >> 8;
		lenData[3] = (readSize & 0xFF);
		if(i == -1){
			fwrite(lenData, sizeof(UCHAR), sizeof(lenData), wfp1);
			lenData[0] = (encodeDataLen & 0xFF000000) >> 24;
			lenData[1] = (encodeDataLen & 0xFF0000) >> 16;
			lenData[2] = (encodeDataLen & 0xFF00) >> 8;
			lenData[3] = (encodeDataLen & 0xFF);
			fwrite(lenData, sizeof(UCHAR), sizeof(lenData), wfp1);
			fwrite(encodeData, sizeof(UCHAR), encodeDataLen, wfp1);
			fclose(wfp1);
		}
		else{
			fwrite(lenData, sizeof(UCHAR), sizeof(lenData), wfp2);
			lenData[0] = (encodeDataLen & 0xFF000000) >> 24;
			lenData[1] = (encodeDataLen & 0xFF0000) >> 16;
			lenData[2] = (encodeDataLen & 0xFF00) >> 8;
			lenData[3] = (encodeDataLen & 0xFF);
			fwrite(lenData, sizeof(UCHAR), sizeof(lenData), wfp2);
			fwrite(encodeData, sizeof(UCHAR), encodeDataLen, wfp2);
		}

		fclose(fp);
	}

	free(data);
	free(encodeData);
	fclose(wfp2);
}