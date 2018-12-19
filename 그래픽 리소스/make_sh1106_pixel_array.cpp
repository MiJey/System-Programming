#include <vector>

using namespace std;

int main() {
	int w, h, n;
	int i, j;
	vector<int> arr;

	scanf("%d %d", &w, &h);
	w /= 4;

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			scanf("%4d", &n);
			switch (n) {
			case 0: arr.push_back(0); break;
			case 1: arr.push_back(1); break;
			case 10: arr.push_back(2); break;
			case 11: arr.push_back(3); break;
			case 100: arr.push_back(4); break;
			case 101: arr.push_back(5); break;
			case 110: arr.push_back(6); break;
			case 111: arr.push_back(7); break;
			case 1000: arr.push_back(8); break;
			case 1001: arr.push_back(9); break;
			case 1010: arr.push_back(10); break;
			case 1011: arr.push_back(11); break;
			case 1100: arr.push_back(12); break;
			case 1101: arr.push_back(13); break;
			case 1110: arr.push_back(14); break;
			case 1111: arr.push_back(15); break;
			}
		}
	}

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j += 2) {
			printf("0x%x%x, ", arr[i*w + j], arr[i*w + j + 1]);
		}
		printf("\n");
	}
}
