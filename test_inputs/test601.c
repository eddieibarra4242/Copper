// Maze 2 from https://www.cise.ufl.edu/~manuel/obfuscate/maze2.c

int a[1817];int main(int z,int p,int q,int r){for(p=80;q+p-80;p-=2*a[p])for(z=9;z--;)q=3&(r=time(0)+r*57)/7,q=q?q-1?q-2?1-p%79?-1:0:p%79-77?1:0:p<1659?79:0:p>158?-79:0,q?!a[p+q*2]?a[p+=a[p+=q]=q]=q:0:0;for(;q++-1817;)printf(q%79?"%c":"%c\n"," #"[!a[q-1]]);}