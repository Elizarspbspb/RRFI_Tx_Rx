#include "Golay.h"
#include <math.h>
#include <stdio.h>

double CGolayCodec::sqr(double x)
{
	return x*x;
}
void CGolayCodec::copyPathes(int k)
{
	int j;
	for(int i=0; i<STATES; i++)
	{
		prevPathes[i].dist=pathes[i].dist;
		for(j=0; j<k; j++)
			prevPathes[i].dWord[j]=pathes[i].dWord[j];
	}
}
void CGolayCodec::decode(double *cword, char *mess)
{
	int currStates=2;
	int j=0;
	int n=0;
	int l;
	double bufDist=0;;
	for(j=0; j<STATES; j++)
		pathes[j].dist=0;
	for (int k=0; k<4; k++)
	{
		for(j=currStates-1; j>=0; j--)
		{
			for(l=0; l<k; l++)
				pathes[j].dWord[l]=pathes[j/2].dWord[l];
			pathes[j].dWord[l]=(j%2)+48;
			if (j%2==0)
			{
				pathes[j].dist=sqrt(sqr(pathes[j/2].dist)+sqr(cword[n]-trellis[j/2][k].zero[0]));
				pathes[j].dist=sqrt(sqr(pathes[j].dist)+sqr(cword[n+1]-trellis[j/2][k].zero[1]));
			}
			else
			{
				pathes[j].dist=sqrt(sqr(pathes[j/2].dist)+sqr(cword[n]-trellis[j/2][k].unit[0]));
				pathes[j].dist=sqrt(sqr(pathes[j].dist)+sqr(cword[n+1]-trellis[j/2][k].unit[1]));
			}
		}
		n=n+2;
		currStates=currStates*2;
	}
	for(int k=4; k<LENGTHWORD; k++)
	{
		copyPathes(k);
		for(j=0; j<STATES; j++)
		{
			pathes[j].flag=false;			
		}
		for(j=0; j<STATES; j++)
		{
			if(pathes[trellis[j][k%4].zPath].flag==false)
			{
				for(l=0; l<k; l++)
				{
					pathes[trellis[j][k%4].zPath].dWord[l]=prevPathes[j].dWord[l];	
				}
				pathes[trellis[j][k%4].zPath].dWord[k]='0';
				pathes[trellis[j][k%4].zPath].flag=true;
				pathes[trellis[j][k%4].zPath].dist=sqrt(sqr(prevPathes[j].dist)+sqr(cword[n]-trellis[j][k%4].zero[0]));
				pathes[trellis[j][k%4].zPath].dist=sqrt(sqr(pathes[trellis[j][k%4].zPath].dist)+sqr(cword[n+1]-trellis[j][k%4].zero[1]));
			}
			else
			{
				bufDist=sqrt(sqr(prevPathes[j].dist)+sqr(cword[n]-trellis[j][k%4].zero[0])+sqr(cword[n+1]-trellis[j][k%4].zero[1]));
				if(bufDist<pathes[trellis[j][k%4].zPath].dist)
				{
					pathes[trellis[j][k%4].zPath].dist=bufDist;
					for(l=0; l<k; l++)
					{
						pathes[trellis[j][k%4].zPath].dWord[l]=prevPathes[j].dWord[l];	
					}
					pathes[trellis[j][k%4].zPath].dWord[l]='0';
				}
			}
			if(pathes[trellis[j][k%4].uPath].flag==false)
			{
				pathes[trellis[j][k%4].uPath].dWord[k]='1';
				for(l=0; l<k; l++)
				{
					pathes[trellis[j][k%4].uPath].dWord[l]=prevPathes[j].dWord[l];	
				}
				pathes[trellis[j][k%4].uPath].flag=true;
				pathes[trellis[j][k%4].uPath].dist=sqrt(sqr(prevPathes[j].dist)+sqr(cword[n]-trellis[j][k%4].unit[0]));
				pathes[trellis[j][k%4].uPath].dist=sqrt(sqr(pathes[trellis[j][k%4].uPath].dist)+sqr(cword[n+1]-trellis[j][k%4].unit[1]));
			}
			else
			{
				bufDist=sqrt(sqr(prevPathes[j].dist)+sqr(cword[n]-trellis[j][k%4].unit[0])+sqr(cword[n+1]-trellis[j][k%4].unit[1]));
				if(bufDist<pathes[trellis[j][k%4].uPath].dist)
				{
					pathes[trellis[j][k%4].uPath].dist=bufDist;
					for(l=0; l<k; l++)
					{
						pathes[trellis[j][k%4].uPath].dWord[l]=prevPathes[j].dWord[l];	
					}
					pathes[trellis[j][k%4].uPath].dWord[l]='1';
				}
			}
		}
		n=n+2;
	}
	int index=0;
	double minDist=pathes[0].dist;
	for(l=1; l<STATES; l++)
	{
		if (minDist>pathes[l].dist)
		{
			index=l;
			minDist=pathes[l].dist;
		}
	}
	for(j=0; j<LENGTHWORD; j++)
		mess[j]=pathes[index].dWord[j]-'0';
}
void CGolayCodec::encode(char *mess, char *cword)
{
	int i=0;
	int j;
	int n=0;
	for(int k=0; k<LENGTHWORD; k++)
	{
		j=k%4;
		if (mess[k]==0)
		{
			cword[n]=trellis[i][j].zero[0];
			n++;
			cword[n]=trellis[i][j].zero[1];
			n++;
			i=trellis[i][j].zPath;
		}
		else
		{
			cword[n]=trellis[i][j].unit[0];
			n++;
			cword[n]=trellis[i][j].unit[1];
			n++;
			i=trellis[i][j].uPath;
		}
	}

    for(n=0;n<LENGTHCWORD;n++) if(cword[n]==-1) cword[n]=0;
}
CGolayCodec::CGolayCodec()
{
	FILE *trellisFile;
	int i=0;
	int j=0;
	char ch=' ';
	trellisFile=fopen("t.trellis","r");
	while(ch!=EOF)
	{
		j=0;
		if (ch=='\n') 
			ch=fgetc(trellisFile);		
		while((ch!='\n')&&(ch!=EOF))
		{
			if (ch == ' ')
			{
				ch = fgetc(trellisFile); //error
			}
			trellis[i][j].zero[0]=((ch-48)==0?-1:1);
			ch=fgetc(trellisFile);
			trellis[i][j].zero[1]=((ch-48)==0?-1:1);
			ch=fgetc(trellisFile);		
			ch=fgetc(trellisFile);		
			trellis[i][j].zPath=0;
			while((ch>='0')&&(ch<='9'))
			{
				trellis[i][j].zPath=trellis[i][j].zPath*10+(ch-48);
				ch=fgetc(trellisFile);		
			}
			ch=fgetc(trellisFile);		
			trellis[i][j].unit[0]=((ch-48)==0?-1:1);
			ch=fgetc(trellisFile);		
			trellis[i][j].unit[1]=((ch-48)==0?-1:1);
			ch=fgetc(trellisFile);		
			ch=fgetc(trellisFile);		
			trellis[i][j].uPath=0;
			while((ch>='0')&&(ch<='9'))
			{
				trellis[i][j].uPath=trellis[i][j].uPath*10+(ch-48);
				ch=fgetc(trellisFile);		
			}
			j++;
		}
		i++;
	}
	fclose(trellisFile);
}
