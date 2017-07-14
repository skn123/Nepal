#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <vector>
#include "cmdline.h"
#include "parameter.h"
#include "struct.h"
using std::cout;
using std::endl;
#define INF -999999
#define UNIT 144

std::vector<std::string> splitstring(const std::string &str, char sep);
std::vector<float> splitfloat(const std::string &str, char sep);
void readpssm(std::string &input,inputdata &datai,inputdata &dataj);
void align(inputdata &datai,inputdata &dataj,const float w1[41][UNIT],const float w2[UNIT+1][1],const int cpun);
float max2(const float &x,const float &y);
float max3(const float &x,const float &y,const float &z);
void findmaxindex(int &pi,int &pj,float **H,int &maxi,int &maxj);
void findmaxvalueindexrow(float &rowmax,int &rowmaxindex,float **H,int &maxi,int &maxj);
void findmaxvalueindexcol(float &colmax,int &colmaxindex,float **H,int &maxi,int &maxj);
float max3root(const float &x,const float &y,const float &z,int &root);
void getoption(cmdline::parser &a,int argc,char *argv[])
{
	a.add<std::string>("input",0,"Input pairwise sequence fasta file.",true);
	a.add<int>("cpu",0,"The number of CPU.",false,1);
	a.add("help",0,"Printing help message.");
	a.set_program_name("nepal");
	a.parse_check(argc,argv);
}

int main(int argc,char *argv[])
{
	cmdline::parser option;
	getoption(option,argc,argv);
	
	int cpun=option.get<int>("cpu");
	std::string input=option.get<std::string>("input");
	
	inputdata datai,dataj;
	readpssm(input,datai,dataj);
	
	align(datai,dataj,w1,w2,cpun);
	cout<<">"<<datai.name<<"\n"<<datai.alseq<<"\n"<<">"<<dataj.name<<"\n"<<dataj.alseq<<endl;
}

std::vector<std::string> splitstring(const std::string &str, char sep)
{
	std::vector<std::string> v;
	std::stringstream ss(str);
	std::string buffer;
	while(std::getline(ss,buffer,sep))
	{
		v.push_back(buffer);
	}
	return v;
}
std::vector<float> splitfloat(const std::string &str, char sep)
{
	std::vector<float> v;
	std::stringstream ss(str);
	std::string buffer;
	while(std::getline(ss,buffer,sep))
	{
		float a;
		a=stof(buffer);
		v.push_back(a);
	}
	return v;
}
void readpssm(std::string &input,inputdata &datai,inputdata &dataj)
{
	std::ifstream fin(input);
	std::string line;
	int i=0;
	while(getline(fin,line))
	{
		if((int)line.find("<name>")==0)
		{
			if(i==0)
			{
				datai.name=line.replace(0,6,"");
			}
			else
			{
				dataj.name=line.replace(0,6,"");
			}
		}
		else if((int)line.find("<sequence>")==0)
		{
			if(i==0)
			{
				datai.seq=line.replace(0,10,"");
			}
			else
			{
				dataj.seq=line.replace(0,10,"");
			}
		}
		else if((int)line.find("<pssm>")==0)
		{
			std::vector<std::string> vtmp1=splitstring(line.replace(0,6,""),',');
			int length=vtmp1.size();
			if(i==0)
			{
				for(int i=0;i<length;i++)
				{
					std::vector<float> vtmp2=splitfloat(vtmp1[i],' ');
					datai.pssm.push_back(vtmp2);
				}
			}
			else
			{
				for(int i=0;i<length;i++)
				{
					std::vector<float> vtmp2=splitfloat(vtmp1[i],' ');
					dataj.pssm.push_back(vtmp2);
				}
			}
			i++;
		}
	}
}
void align(inputdata &datai,inputdata &dataj,const float w1[41][UNIT],const float w2[UNIT+1][1],const int cpun)
{
	std::string mode="sg";
	
	int NONE=0,LEFT=1,UP=2,DIAG=3;
	int maxi=datai.pssm.size();
	int maxj=dataj.pssm.size();
	
	float **H=(float**)malloc(sizeof(float*)*(maxi+1));
	for(int i=0;i<maxi+1;i++)
	{
		H[i]=(float*)malloc(sizeof(float)*(maxj+1));
	}
	for(int i=0;i<maxi+1;i++)
	{
		for(int j=0;j<maxj+1;j++)
		{
			H[i][j]=0;
		}
	}
	float **I=(float**)malloc(sizeof(float*)*(maxi+1));
	for(int i=0;i<maxi+1;i++)
	{
		I[i]=(float*)malloc(sizeof(float)*(maxj+1));
	}
	for(int i=0;i<maxi+1;i++)
	{
		for(int j=0;j<maxj+1;j++)
		{
			I[i][j]=INF;
		}
	}
	float **J=(float**)malloc(sizeof(float*)*(maxi+1));
	for(int i=0;i<maxi+1;i++)
	{
		J[i]=(float*)malloc(sizeof(float)*(maxj+1));
	}
	for(int i=0;i<maxi+1;i++)
	{
		for(int j=0;j<maxj+1;j++)
		{
			J[i][j]=INF;
		}
	}
	int **P=(int**)malloc(sizeof(int*)*(maxi+1));
	for(int i=0;i<maxi+1;i++)
	{
		P[i]=(int*)malloc(sizeof(int)*(maxj+1));
	}
	for(int i=0;i<maxi+1;i++)
	{
		for(int j=0;j<maxj+1;j++)
		{
			P[i][j]=0;
		}
	}
	
	if(mode=="sg" ||mode=="nw")
	{
		for(int i=0;i<maxi+1;i++)
		{
			for(int j=0;j<maxj+1;j++)
			{
				if(i==0 && j==0)
				{
					P[i][j]=NONE;
				}
				else if(i==0)
				{
					P[i][j]=LEFT;
				}
				else if(j==0)
				{
					P[i][j]=UP;
				}
			}
		}
		if(mode=="nw")
		{
			for(int i=0;i<maxi+1;i++)
			{
				for(int j=0;j<maxj+1;j++)
				{
					if(i==0 && j==0)
					{
						H[i][j]=0;
					}
					else if(i==0)
					{
						H[i][j]=op+(j-1)*ep;
					}
					else if(j==0)
					{
						H[i][j]=op+(i-1)*ep;
					}
				}
			}
		}
	}
	
	float **middle=(float**)malloc(sizeof(float*)*1);
	for(int i=0;i<1;i++)
	{
		middle[i]=(float*)malloc(sizeof(float)*(UNIT+1));
	}
	float **first=(float**)malloc(sizeof(float*)*1);
	for(int i=0;i<1;i++)
	{
		first[i]=(float*)malloc(sizeof(float)*(41));
	}
	float similarity=0;
	int pi=0,pj=0;
	for(int i=1;i<maxi+1;i++)
	{
		for(int a=0;a<20;a++)
		{
			first[0][a]=datai.pssm[i-1][a];
		}
		for(int j=1;j<maxj+1;j++)
		{
			for(int a=0;a<20;a++)
			{
				first[0][a+20]=dataj.pssm[j-1][a];
			}
			first[0][40]=1;
			for(int i=0;i<1;i++)
			{
				for(int j=0;j<(UNIT+1);j++)
				{
					middle[i][j]=0;
				}
			}
			for(int i=0;i<1;i++)
			{
				for(int t=0;t<41;t++)
				{
					for(int j=0;j<UNIT+1;j++)
					{
						middle[i][j]+=first[i][t]*w1[t][j];
					}
				}
			}
			/* ReLU */
			for(int i=0;i<1;i++)
			{
				for(int j=0;j<UNIT+1;j++)
				{
					if(middle[i][j]<0)
					{
						middle[i][j]=0;
					}
				}
			}
			middle[0][UNIT]=1;
			for(int i=0;i<UNIT+1;i++)
			{
				similarity+=(middle[0][i]*w2[i][0]);
			}
			I[i][j]=max2(H[i-1][j]+op,I[i-1][j]+ep);
			J[i][j]=max2(H[i][j-1]+op,J[i][j-1]+ep);
			float diagscore=max3(H[i-1][j-1],I[i-1][j-1],J[i-1][j-1])+similarity;
			float leftscore=J[i][j];
			float upscore=I[i][j];
			int root=DIAG;
			float maxscore=max3root(diagscore,leftscore,upscore,root);
			
			H[i][j]=maxscore;
			if(mode=="sw")
			{
				H[i][j]=max2(0,maxscore);
			}
			if(mode=="sw")
			{
				if(H[i][j]==0)
				{
					/* Nothing to do */
				}
				else
				{
					P[i][j]=root;
				}
			}
			else
			{
				P[i][j]=root;
			}
			pj=j;
		}
		pi=i;
	}
	
	for(int i=0;i<1;i++)
	{
		free(middle[i]);
	}
	free(middle);
	for(int i=0;i<1;i++)
	{
		free(first[i]);
	}
	free(first);
	
	if(mode=="sw")
	{
		findmaxindex(pi,pj,H,maxi,maxj);
	}
	else if(mode=="sg")
	{
		float rowmax=0;
		int rowmaxindex=0;
		findmaxvalueindexrow(rowmax,rowmaxindex,H,maxi,maxj);
		float colmax=0;
		int colmaxindex=0;
		findmaxvalueindexcol(colmax,colmaxindex,H,maxi,maxj);
		if(rowmax>colmax)
		{
			for(int i=0;i<maxj+1;i++)
			{
				if(i>rowmaxindex)
				{
					P[maxi][i]=LEFT;
				}
			}
		}
		else
		{
			for(int i=0;i<maxi+1;i++)
			{
				if(i>colmaxindex)
				{
					P[i][maxj]=UP;
				}
			}
		}
	}
	
	int pv=P[pi][pj];
	std::string alseqir="",alseqjr="";
	std::string gapstring="-";
	while(pv!=NONE)
	{
		if(pv==DIAG)
		{
			pi--;
			pj--;
			alseqir+=datai.seq[pi];
			alseqjr+=dataj.seq[pj];
		}
		else if(pv==LEFT)
		{
			pj--;
			alseqir+=gapstring;
			alseqjr+=dataj.seq[pj];
		}
		else if(pv==UP)
		{
			pi--;
			alseqir+=datai.seq[pi];
			alseqjr+=gapstring;
		}
		pv=P[pi][pj];
	}
	
	int alength=alseqir.length();
	for(int i=alength-1;i>=0;i--)
	{
		datai.alseq+=alseqir[i];
		dataj.alseq+=alseqjr[i];
	}
	
	for(int i=0;i<maxi+1;i++)
	{
		free(H[i]);
	}
	free(H);
	for(int i=0;i<maxi+1;i++)
	{
		free(I[i]);
	}
	free(I);
	for(int i=0;i<maxi+1;i++)
	{
		free(J[i]);
	}
	free(J);
	for(int i=0;i<maxi+1;i++)
	{
		free(P[i]);
	}
	free(P);
}
void findmaxvalueindexcol(float &colmax,int &colmaxindex,float **H,int &maxi,int &maxj)
{
	colmax=INF;
	for(int i=0;i<maxi+1;i++)
	{
		if(colmax<H[i][maxj])
		{
			colmax=H[i][maxj];
			colmaxindex=i;
		}
	}
}
void findmaxvalueindexrow(float &rowmax,int &rowmaxindex,float **H,int &maxi,int &maxj)
{
	rowmax=INF;
	for(int j=0;j<maxj+1;j++)
	{
		if(rowmax<H[maxi][j])
		{
			rowmax=H[maxi][j];
			rowmaxindex=j;
		}
	}
}
void findmaxindex(int &pi,int &pj,float **H,int &maxi,int &maxj)
{
	int max=INF;
	for(int i=0;i<maxi+1;i++)
	{
		for(int j=0;j<maxj+1;j++)
		{
			if(max<H[i][j])
			{
				max=H[i][j];
				pi=i;
				pj=j;
			}
		}
	}
}
float max2(const float &x,const float &y)
{
	if(x>y)
	{
		return x;
	}
	else
	{
		return y;
	}
}
float max3(const float &x,const float &y,const float &z)
{
	float max=x;
	float tmp[2]={y,z};
	for(int i=0;i<2;i++)
	{
		if(max<tmp[i])
		{
			max=tmp[i];
		}
	}
	return max;
}
float max3root(const float &x,const float &y,const float &z,int &root)
{
	float max=x; // DIAG, root=DIAG
	if(max<y)
	{
		max=y;
		root=1; // LEFT
	}
	if(max<z)
	{
		max=z;
		root=2; // UP
	}
	return max;
}
