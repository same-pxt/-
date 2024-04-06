#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <cstdio>
#include <cmath>
#include <vector>
#include <cassert>

typedef unsigned char uint8;

struct octNode
{
	long long cnt;//本结点以下的像素总数
	long long rSum, gSum, bSum;//红色分量、绿色分量、蓝色分量灰度值的和
	bool isLeaf;//是否是叶子结点
	int depth;//本节点的深度
	octNode* child[8];//8个子结点的指针数组 
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

	void insertColor(uint8 r, uint8 g, uint8 b);						//插入一个颜色
	uint8 generatePalette(RGBQUAD *pal);						//生成调色板
public:
	octNode *root;														//八叉树的根
	int colors;															//当前的颜色总数
	int maxColors;		
	std::vector<octNode*>Leaves;												//最大颜色数
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
//释放八叉树的内存空间
octTree::~octTree()
{
	destroyTree(root);
}

//往八叉树中添加一个像素的颜色
void octTree::insertColor(uint8 r, uint8 g, uint8 b)
{
	printf("keep inserting    ");
	int mask=1;
	uint8 Colors[8];
	//获取每个颜色对应的脚标。
	for (int i = 0; i < 8; i++) {
		
		Colors[i] = (((r >> 7 - i) & 1) << 2) + (((g >> 7 - i) & 1) << 1) + ((b >> 7 - i) & 1);
	}
	//插入
	octNode*temp=this->root;
	int i=0;
	for(;i<8;i++)
	{
		//找到要插入的位置
		if(!temp->isLeaf && temp->child[Colors[i]]!=NULL)
		{
			temp=temp->child[Colors[i]];
		}
		//当前节点是一个已经存在的节点  不增加color，增加该颜色对应的像素数，颜色分量
		else if(temp->isLeaf)
		{
			temp->rSum+=r;
			temp->bSum+=b;
			temp->gSum+=g;
			temp->cnt++;
			break;
		}
		//这条路径没有被创建，不断创建新的节点
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
	//如果顺利的创建到最后，说明是一个新的颜色。
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
//根据现有的八叉树，选择256个颜色作为最终的调色板中的颜色
uint8 octTree::generatePalette(RGBQUAD *pal)
{
		//如果此时超出了maxcolor，进行删除
	while(colors>maxColors)
		deleteColor();
	//依次读取叶节点即可，忽略为NULL的节点
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


//从调色板中选出与给定颜色最接近的颜色
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
	return (uint8)min_idx;//给定某颜色，返回其在调色板中最近似颜色的索引值；
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		printf("using: exe[0], input file[1], output file[2]\n");
		return -1;
	}
	BITMAPFILEHEADER bf, *pbf;//输入、输出文件的文件头
	BITMAPINFOHEADER bi, *pbi;//输入、输出文件的信息头
	RGBQUAD *pRGBQuad;//待生成的调色板指针
	uint8 *pImage;//转换后的图象数据
	DWORD bfSize;//文件大小
	LONG biWidth, biHeight;//图象宽度、高度
	DWORD biSizeImage;//图象的大小，以字节为单位，每行字节数必须是4的整数倍
	unsigned long biFullWidth;//每行字节数必须是4的整数倍

	//打开输入文件
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

	//创建输出文件
	printf("Creating %s ... ", outputName);
	if (!(fpOut = fopen(outputName, "wb")))
	{
		printf("\nCan't create %s!\n", outputName);
		return -1;
	}
	printf("Success!\n");

	//读取输入文件的文件头、信息头
	fread(&bf, sizeof(BITMAPFILEHEADER), 1, fpIn);
	fread(&bi, sizeof(BITMAPINFOHEADER), 1, fpIn);

	//读取文件信息
	biWidth = bi.biWidth;
	biHeight = bi.biHeight;
	biFullWidth = ceil(biWidth / 4.) * 4;//bmp文件每一行的字节数必须是4的整数倍
	biSizeImage = biFullWidth*biHeight;
	bfSize = biFullWidth*biHeight + 54 + 256 * 4;//图象文件的大小，包含文件头、信息头

	//设置输出文件的BITMAPFILEHEADER
	pbf = new BITMAPFILEHEADER;
	pbf->bfType = 19778;
	pbf->bfSize = bfSize;
	pbf->bfReserved1 = 0;
	pbf->bfReserved2 = 0;
	pbf->bfOffBits = 54 + 256 * 4;
	//写出BITMAPFILEHEADER
	if (fwrite(pbf, sizeof(BITMAPFILEHEADER), 1, fpOut) != 1)
	{
		printf("\nCan't write bitmap file header!\n");
		fclose(fpOut);
		return -1;
	}

	//设置输出文件的BITMAPINFOHEADER
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
	//写出BITMAPFILEHEADER
	if (fwrite(pbi, sizeof(BITMAPINFOHEADER), 1, fpOut) != 1)
	{
		printf("\nCan't write bitmap info header!\n");
		fclose(fpOut);
		return -1;
	}
	pImage = new uint8[biSizeImage];
	memset(pImage, 0, biSizeImage);
	//构建颜色八叉树
	printf("Building Color OctTree ...  ");
	octTree *tree;
	tree = new octTree(256);
	uint8 RGB[3];
	//读取图像中每个像素的颜色，并将其插入颜色八叉树
	for (int i = 0; i < bi.biHeight; i++)
	{
		fseek(fpIn, bf.bfOffBits + i*ceil(biWidth * 3 / 4.) * 4, 0);
		for (int j = 0; j < bi.biWidth; j++)
		{
			//读取一个像素的颜色，并将其插入颜色八叉树
			fread(&RGB, 3, 1, fpIn);
			tree->insertColor(RGB[2], RGB[1], RGB[0]);
			printf("%d/%d\n",i*bi.biWidth+j,bi.biHeight*bi.biWidth);
		}
	}
	printf("Success! %d,%d\n",tree->colors,tree->Leaves.size());

	//生成并填充调色板
	printf("Generating palette ... ");
	pRGBQuad = new RGBQUAD[256];
	tree->generatePalette(pRGBQuad);

	//输出256色调色板
	if (fwrite(pRGBQuad, 256 * sizeof(RGBQUAD), 1, fpOut) != 1)
	{
		printf("\nCan't write palette!\n");
		fclose(fpOut);
		return -1;
	}
	printf("Success!\n");

	//填充图像数据
	printf("Generating the output image ... ");
	pImage = new uint8[biSizeImage];
	memset(pImage, 0, biSizeImage);
	for (int i = 0; i < bi.biHeight; i++)
	{		
		fseek(fpIn, bf.bfOffBits + i*ceil(biWidth * 3 / 4.) * 4, 0);
		for (int j = 0; j < bi.biWidth; j++)
		{
			//读取一个像素的颜色，并将其转换位颜色索引值
			fread(&RGB, 3, 1, fpIn);
			pImage[i*biFullWidth + j] = selectClosestColor(RGB[2], RGB[1], RGB[0], pRGBQuad);
		}
	}
	//输出图象数据
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