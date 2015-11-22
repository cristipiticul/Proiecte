#include <iostream>
#include <fstream>


using namespace std;
ifstream f("date.in");
ofstream g("date.out");

int abs(int nr)
{
    if(nr<0)
        nr=-nr;
    return nr;
}

int prim(int nr)
{
    for(int d=2;d<=nr/2;d++)
        if(nr%d==0)
            return 0;
    return 1;
}

void afisare(int b[],int a[],int k)
{
    for(int i=1;i<=k;i++)
        g<<b[a[i]]<<' ';
    g<<endl;
}


int valid(int a[],int k,int lim,int b[])
{int i;
    for(i=1;i<k;i++)
        if(a[i]>=a[i+1])
            return 0;
    for(i=1;i<=k;i++)
        if(b[a[i]]>lim)
            return 0;
    if(b[a[1]]%2==0)
        return 0;
    for(i=1;i<k;i++)
        if((abs(b[a[i]]-b[a[i+1]]))<2)
            return 0;
    for(i=1;i<k;i++)
        if(b[a[i]]%5==0&&b[a[i+1]]%5==0)
            return 0;

    if(prim(b[a[k]])==0)
            return 0;
    for(i=1;i<=k;i++)
    {

       for(int j=i+1;j<=k;j++)
            if(b[a[i]]==b[a[j]])
                return 0;
    }

return 1;




}



void backt(int i,int a[],int b[],int n,int k,int lim )
{   int j;
    for(j=1;j<=n;j++)
    {a[i]=j;
    if(i==k)
    {

     if(valid(a,k,lim,b))
            afisare(b,a,k);
    }
    else
        backt(i+1,a,b,n,k,lim);
    }
}

int main()
{int b[100],a[100],n,k,lim;
f>>n;
for(int i=1;i<=n;i++)
    f>>b[i];
f>>k;
f>>lim;
backt(1,a,b,n,k,lim);

}
return 1;
