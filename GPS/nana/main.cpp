#include <iostream>
#include <fstream>

using namespace std;
ifstream f("date.in");
ofstream g("date.out");
int n,a[50];
void afisare(int n,int a[],int gr[])
{
    for(int i=1;i<=n;i++)
        g<<gr[a[i]]<<' ';
    g<<'\n';
}



void backt(int i,int n,int a[],int sum,int gr[])
{
    for(int j=a[i-1]+1;j<=n;j++)
    {a[i]=j;
    if(gr[a[i]]<=sum)
        if(gr[a[i]]==sum)
                afisare(i,a,gr);
        else
            if(i<n)
            backt(i+1,n-j,a,sum,gr);

    }
}


int main()
{int sum,gr[100];
f>>n;
f>>sum;
a[0]=1;
for(int i=1;i<=n;i++)
    f>>gr[i];
backt(1,n,a,sum,gr);
return 0;
}
