include <iostream>
include <fstream>

using namespace std;
ifstream f("date.in");
ofstream g("date.out");
int n,x[100];
void afisare(int n,int x[])
    {for(int i=1;i<=n;i++)
        g<<x[i]<<' ';
    g<<'\n';

    }
void beck(int x[],int i,int n)
    {for(int j=x[i-1];j<=n;j++)
        {x[i]=j;
        if(j<=n)
            if(j==n)
                afisare(i,x);
            else
                beck(x,i+1,n-j);
        }


    }
int main()
{   f>>n;
    x[0]=1;
    beck(x,1,n);

    return 0;
}



