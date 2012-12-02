#include <stdio.h>

unsigned hash(char *cmd)
{
unsigned hs=0;
while(*cmd){
	hs = (hs << 4) + hs + *cmd;
	cmd++;
	}
return hs;
}

int main(int argc, char **argv)
{
int i;
for(i=1; i<argc; i++){
	printf("\tcase 0x%04x: // %s\n", hash(argv[i]), argv[i]);
	}
return 0;
}
