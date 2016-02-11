#include "main.hpp"

using namespace std;

vector<string> solve1(const vector< vector<bool> > &data) {
	vector<string> commands;

	vector< vector<bool> > painted(data.size(), vector<bool>(data[0].size(), false) );

	for (int i = 0; i < data.size(); i++) {
		for (int j = 0; j < data[i].size(); j++) {
			if (data[i][j]) commands.push_back(strfmt("PAINT_SQUARE %d %d 0", i, j));
		}
	}

	return commands;
}
