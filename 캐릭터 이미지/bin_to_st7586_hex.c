#include <stdio.h>

int main(){
	int w = 6;
	int h = 40;
	int n;
	int arr[40][6];
	int i, j;

	for(i = 0; i < h; i++){
		for(j = 0; j < w; j++){
			scanf("%d", &n);
			switch(n){
			case 0: arr[i][j] = 0; break;
			case 1: arr[i][j] = 3; break;
			case 10: arr[i][j] = 28; break;
			case 11: arr[i][j] = 31; break;
			case 100: arr[i][j] = 224; break;
			case 101: arr[i][j] = 227; break;
			case 110: arr[i][j] = 252; break;
			case 111: arr[i][j] = 255; break;
			}
		}
	}

	for(i = 0; i < h; i++){
		for(j = 0; j < w; j++){
			printf("0x%02x, ", arr[i][j]);
		}
		printf("\n");
	}
}
