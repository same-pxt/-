#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <cstdio>
#include <cmath>
#include <vector>
#include <cassert>

typedef unsigned char uint8;

struct octNode
{
	long long cnt;//��������µ���������
	long long rSum, gSum, bSum;//��ɫ��������ɫ��������ɫ�����Ҷ�ֵ�ĺ�
	bool isLeaf;//�Ƿ���Ҷ�ӽ��
	int depth;//���ڵ�����
	octNode* child[8];//8���ӽ���ָ������ 
	octNode* parent;
	int childNum;
	octNode() : cnt(0), rSum(0), gSum(0), bSum(0), isLeaf(false), depth(0),childNum(0) {
		this->parent = NULL;
		for(int i=0;i<8;i++)
		{
			this->child[i]=NULL;
		}
	}
};

class octTree
{
public:
	octTree(){};
	octTree(int maxColorNum)
	{
		colors = 0;
		maxColors = maxColorNum;
		root = new octNode;
		root->depth = 0;
	}
	~octTree();

	void insertColor(uint8 r, uint8 g, uint8 b);						//����һ����ɫ
	uint8 generatePalette(RGBQUAD *pal);						//���ɵ�ɫ��
public:
	octNode *root;														//�˲����ĸ�
	int colors;															//��ǰ����ɫ����
	int maxColors;		
	std::vector<octNode*>Leaves;												//�����ɫ��
	void destroyTree(octNode *node);
	void deleteColor();
};

VOID octTree::destroyTree(octNode *node)
{
	if(node==NULL)
		return ;
	if(node->isLeaf)
	{
		delete node;
		return;
	}
	if (node)
	{
		for (int i = 0; i < 8; ++i)
		{
			destroyTree(node->child[i]);
		}
		delete node;
	}
}
//�ͷŰ˲������ڴ�ռ�
octTree::~octTree()
{
	destroyTree(root);
}

//���˲��������һ�����ص���ɫ
void octTree::insertColor(uint8 r, uint8 g, uint8 b)
{
	printf("keep inserting    ");
	int mask=1;
	uint8 Colors[8];
	//��ȡÿ����ɫ��Ӧ�Ľűꡣ
	for (int i = 0; i < 8; i++) {
		
		Colors[i] = (((r >> 7 - i) & 1) << 2) + (((g >> 7 - i) & 1) << 1) + ((b >> 7 - i) & 1);
	}
	//����
	octNode*temp=this->root;
	int i=0;
	for(;i<8;i++)
	{
		//�ҵ�Ҫ�����λ��
		if(!temp->isLeaf && temp->child[Colors[i]]!=NULL)
		{
			temp=temp->child[Colors[i]];
		}
		//��ǰ�ڵ���һ���Ѿ����ڵĽڵ�  ������color�����Ӹ���ɫ��Ӧ������������ɫ����
		else if(temp->isLeaf)
		{
			temp->rSum+=r;
			temp->bSum+=b;
			temp->gSum+=g;
			temp->cnt++;
			break;
		}
		//����·��û�б����������ϴ����µĽڵ�
		else 
		{
			temp->childNum++;
			temp->cnt++;
			octNode*newNode=new octNode();
			newNode->parent=temp;
			newNode->depth=temp->depth+1;
			temp->child[Colors[i]]=newNode;
			temp=temp->child[Colors[i]];
		}
	}
	//���˳���Ĵ��������˵����һ���µ���ɫ��
	if(i==8 && temp->isLeaf==0)
	{
		temp->cnt++;
		temp->isLeaf=1;
		temp->rSum+=r;
		temp->bSum+=b;
		temp->gSum+=g;
		colors++;
		Leaves.push_back(temp);
	}

}

void octTree::deleteColor()
{
	printf("keep deleting left:%d   %d\n",colors,Leaves.size());
	octNode*cur=NULL;

	int maxdepth=0;
	int len=Leaves.size();
	for(int i=0;i<Leaves.size();i++)
	{
		//printf("here  %d/%d---%d\n",i,len,Leaves.size());
		if(Leaves[i]!=NULL)
		{
			octNode*par=Leaves[i];
			while (par->childNum<2)
			{
				par=par->parent;
			}
			if(par->depth>maxdepth)
			{
				cur=par;
				maxdepth=par->depth;
			}
			else if(par->depth==maxdepth)
			{
				if(par->cnt < cur->cnt)
					cur=par;//change
			}
		}
	}
	if(cur==NULL)
	{
		printf("not find a new Node  %d\n",maxdepth);
		return ;
	}
	int cnt=0;
	octNode*tmp;
	for(int i=0;i<8;i++)
	{
		if(cur->child[i]==NULL)
			continue;
		
		tmp=cur->child[i];
		while (!tmp->isLeaf)
		{
			int t=0;
			while(tmp->child[t]==NULL)
				t++;
			tmp=tmp->child[t];
			delete tmp->parent;
			tmp->parent=NULL;
			cnt++;
		}
		cur->bSum+=tmp->bSum;
		cur->rSum+=tmp->rSum;
		cur->gSum+=tmp->gSum;

		for(auto it=Leaves.begin();it!=Leaves.end();it++)
		{
			if(*it==tmp)
			{
				Leaves.erase(it);
				break;
			}
		}
		delete tmp;
		tmp=NULL;
	}
	cur->isLeaf=true;
	this->colors=this->colors-(cur->childNum)+1;
	cur->childNum=0;
	Leaves.push_back(cur);
	
}
//�������еİ˲�����ѡ��256����ɫ��Ϊ���յĵ�ɫ���е���ɫ
uint8 octTree::generatePalette(RGBQUAD *pal)
{
		//�����ʱ������maxcolor������ɾ��
	while(colors>maxColors)
		deleteColor();
	//���ζ�ȡҶ�ڵ㼴�ɣ�����ΪNULL�Ľڵ�
	int k = 0;
	for (int i = 0; i < Leaves.size()&&k<=256; i++) {
		if (Leaves[i] != NULL) {
			pal[k].rgbRed = Leaves[i]->rSum / Leaves[i]->cnt;
			pal[k].rgbGreen = Leaves[i]->gSum / Leaves[i]->cnt;
			pal[k].rgbBlue = Leaves[i]->bSum / Leaves[i]->cnt;
			k++;
		}
	}
	printf("%d\n",k);
	for (; k < 256; k++) {
		pal[k].rgbRed = 0;
		pal[k].rgbGreen = 0;
		pal[k].rgbBlue = 0;
	}
	return 0;
}


//�ӵ�ɫ����ѡ���������ɫ��ӽ�����ɫ
uint8 selectClosestColor(uint8 r, uint8 g, uint8 b, RGBQUAD *pal)
{
	int min = 256 * 256 * 3 + 1;
	int min_idx = -1;
	for (int i = 0; i < 256; i++) {
		int x = (int)(pal[i].rgbRed - r) * (int)(pal[i].rgbRed - r) + (int)(pal[i].rgbGreen - g) * (int)(pal[i].rgbGreen - g) + (int)(pal[i].rgbBlue - b) * (int)(pal[i].rgbBlue - b);
		if (x < min) {
			min_idx = i;
			min = x;
		}
	}
	return (uint8)min_idx;//����ĳ��ɫ���������ڵ�ɫ�����������ɫ������ֵ��
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		printf("using: exe[0], input file[1], output file[2]\n");
		return -1;
	}
	BITMAPFILEHEADER bf, *pbf;//���롢����ļ����ļ�ͷ
	BITMAPINFOHEADER bi, *pbi;//���롢����ļ�����Ϣͷ
	RGBQUAD *pRGBQuad;//�����ɵĵ�ɫ��ָ��
	uint8 *pImage;//ת�����ͼ������
	DWORD bfSize;//�ļ���С
	LONG biWidth, biHeight;//ͼ���ȡ��߶�
	DWORD biSizeImage;//ͼ��Ĵ�С�����ֽ�Ϊ��λ��ÿ���ֽ���������4��������
	unsigned long biFullWidth;//ÿ���ֽ���������4��������

	//�������ļ�
	char *inputName, *outputName;
	FILE *fpIn, *fpOut;
	inputName = argv[1];
	outputName = argv[2];
	printf("Opening %s ... ", inputName);
	if (!(fpIn = fopen(inputName, "rb")))
	{
		printf("\nCan't open %s!\n", inputName);
		return -1;
	}
	printf("Success!\n");

	//��������ļ�
	printf("Creating %s ... ", outputName);
	if (!(fpOut = fopen(outputName, "wb")))
	{
		printf("\nCan't create %s!\n", outputName);
		return -1;
	}
	printf("Success!\n");

	//��ȡ�����ļ����ļ�ͷ����Ϣͷ
	fread(&bf, sizeof(BITMAPFILEHEADER), 1, fpIn);
	fread(&bi, sizeof(BITMAPINFOHEADER), 1, fpIn);

	//��ȡ�ļ���Ϣ
	biWidth = bi.biWidth;
	biHeight = bi.biHeight;
	biFullWidth = ceil(biWidth / 4.) * 4;//bmp�ļ�ÿһ�е��ֽ���������4��������
	biSizeImage = biFullWidth*biHeight;
	bfSize = biFullWidth*biHeight + 54 + 256 * 4;//ͼ���ļ��Ĵ�С�������ļ�ͷ����Ϣͷ

	//��������ļ���BITMAPFILEHEADER
	pbf = new BITMAPFILEHEADER;
	pbf->bfType = 19778;
	pbf->bfSize = bfSize;
	pbf->bfReserved1 = 0;
	pbf->bfReserved2 = 0;
	pbf->bfOffBits = 54 + 256 * 4;
	//д��BITMAPFILEHEADER
	if (fwrite(pbf, sizeof(BITMAPFILEHEADER), 1, fpOut) != 1)
	{
		printf("\nCan't write bitmap file header!\n");
		fclose(fpOut);
		return -1;
	}

	//��������ļ���BITMAPINFOHEADER
	pbi = new BITMAPINFOHEADER;
	pbi->biSize = 40;
	pbi->biWidth = biWidth;
	pbi->biHeight = biHeight;
	pbi->biPlanes = 1;
	pbi->biBitCount = 8;
	pbi->biCompression = 0;
	pbi->biSizeImage = biSizeImage;
	pbi->biXPelsPerMeter = 0;
	pbi->biYPelsPerMeter = 0;
	pbi->biClrUsed = 0;
	pbi->biClrImportant = 0;
	//д��BITMAPFILEHEADER
	if (fwrite(pbi, sizeof(BITMAPINFOHEADER), 1, fpOut) != 1)
	{
		printf("\nCan't write bitmap info header!\n");
		fclose(fpOut);
		return -1;
	}
	pImage = new uint8[biSizeImage];
	memset(pImage, 0, biSizeImage);
	//������ɫ�˲���
	printf("Building Color OctTree ...  ");
	octTree *tree;
	tree = new octTree(256);
	uint8 RGB[3];
	//��ȡͼ����ÿ�����ص���ɫ�������������ɫ�˲���
	for (int i = 0; i < bi.biHeight; i++)
	{
		fseek(fpIn, bf.bfOffBits + i*ceil(biWidth * 3 / 4.) * 4, 0);
		for (int j = 0; j < bi.biWidth; j++)
		{
			//��ȡһ�����ص���ɫ�������������ɫ�˲���
			fread(&RGB, 3, 1, fpIn);
			tree->insertColor(RGB[2], RGB[1], RGB[0]);
			printf("%d/%d\n",i*bi.biWidth+j,bi.biHeight*bi.biWidth);
		}
	}
	printf("Success! %d,%d\n",tree->colors,tree->Leaves.size());

	//���ɲ�����ɫ��
	printf("Generating palette ... ");
	pRGBQuad = new RGBQUAD[256];
	tree->generatePalette(pRGBQuad);

	//���256ɫ��ɫ��
	if (fwrite(pRGBQuad, 256 * sizeof(RGBQUAD), 1, fpOut) != 1)
	{
		printf("\nCan't write palette!\n");
		fclose(fpOut);
		return -1;
	}
	printf("Success!\n");

	//���ͼ������
	printf("Generating the output image ... ");
	pImage = new uint8[biSizeImage];
	memset(pImage, 0, biSizeImage);
	for (int i = 0; i < bi.biHeight; i++)
	{		
		fseek(fpIn, bf.bfOffBits + i*ceil(biWidth * 3 / 4.) * 4, 0);
		for (int j = 0; j < bi.biWidth; j++)
		{
			//��ȡһ�����ص���ɫ��������ת��λ��ɫ����ֵ
			fread(&RGB, 3, 1, fpIn);
			pImage[i*biFullWidth + j] = selectClosestColor(RGB[2], RGB[1], RGB[0], pRGBQuad);
		}
	}
	//���ͼ������
	if (fwrite(pImage, biSizeImage, 1, fpOut) != 1)
	{
		printf("\nCan't write image data!\n");
		fclose(fpOut);

		return -1;
	}
	printf("Success!\n");


	delete tree;
	delete pbf;
	delete pbi;
	delete[] pRGBQuad;
	delete[] pImage;
	fclose(fpIn);
	fclose(fpOut);
	printf("All done!\n");
	system("pause");
	return 0;
}