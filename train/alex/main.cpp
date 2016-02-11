#include "main.hpp"

using namespace std;


int main() {
	int l, h;
	scanf("%d %d\n", &l, &h);

	vector< vector<bool> > data;
	for (int i = 0; i < l; i++) {
		vector<bool> line(h, false);
		for (int j = 0; j < h; j++) {
			char k = getchar();
			if (k == '#') {
				line[j] = true;
			} else if (k != '.') {
				fprintf(stderr, "Fuck you. And excuse my language.\n");
			}
		}
		if (getchar() != '\n') {
			fprintf(stderr, "Fuck you again.\n");
		}
		data.push_back(line);
	}

	auto cmds = solve2(data);
	printf("%i\n", cmds.size());
	for (int i = 0; i < cmds.size(); i++)
		printf("%s\n", cmds[i].c_str());
}
