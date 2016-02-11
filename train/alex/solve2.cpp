#include <utility>
#include <future>
#include <thread>
#include <mutex>
#include <chrono>

#include "main.hpp"

using namespace std;

class cmd {
public:
	virtual string str() = 0;
	virtual vector<pair<int, int>> pos() = 0;

	void doit(vector< vector<bool> > &done) {
		auto x = pos();
		for (int i = 0; i < x.size(); i++) {
			done[x[i].first][x[i].second] = true;
		}
	}

	virtual int worth(const vector< vector<bool> > &target, const vector< vector<bool> > &done, int p1, int p2) {
		int ret = 0;

		auto x = pos();
		for (int i = 0; i < x.size(); i++) {
			int a = x[i].first, b = x[i].second;
			if (target[a][b] && !done[a][b]) ret += p1;
			if (!target[a][b] && !done[a][b]) ret -= p2;
		}

		return ret;
	}
};

class square : public cmd {
public:
	int x, y, r;

	square(int _x, int _y, int _r) : x(_x), y(_y), r(_r) {}
	string str() {
		return strfmt("PAINT_SQUARE %d %d %d", x, y, r);
	}
	vector< pair<int, int> > pos() {
		vector< pair<int, int> > ret;
		for (int i = x-r; i <= x+r; i++) {
			for (int j = y-r; j <= y+r; j++) {
				ret.push_back(make_pair(i, j));
			}
		}
		return ret;
	}
};

class line : public cmd {
public:
	int x1, y1, x2, y2;

	line(int _x1, int _y1, int _x2, int _y2) : x1(_x1), y1(_y1), x2(_x2), y2(_y2) {
		if (!(x1 == x2 || y1 == y2)) fprintf(stderr, "STOP THIS NOW!\n");
	}
	string str() {
		return strfmt("PAINT_LINE %d %d %d %d", x1, y1, x2, y2);
	}
	vector< pair<int, int> > pos() {
		vector< pair<int, int> > ret;
		if (x1 == x2) {
			for (int y = y1; y <= y2; y++) {
				ret.push_back(make_pair(x1, y));
			}
		} else if (y1 == y2) {
			for (int x = x1; x <= x2; x++) {
				ret.push_back(make_pair(x, y1));
			}
		}
		return ret;
	}
};

vector<string> solve2_int(const vector< vector<bool> > &data, int p1, int p2) {
	vector<string> commands;

	int L = data.size(), C = data[0].size();

	vector< vector<bool> > done(L, vector<bool>(C, false) );

	vector<cmd*> all_cmds;
	// generate all possible commands
	for (int i = 0; i < L; i++) {
		for (int j = 0; j < C; j++) {
			for (int r = 0; r < 6; r++) {
				if (i - r >= 0 && j - r >= 0 && i + r < L && j + r < C)
					all_cmds.push_back(new square(i, j, r));
			}
			for (int i2 = i+1; i2 < min(i+15, L); i2++) {
				all_cmds.push_back(new line(i, j, i2, j));
			}
			for (int j2 = j+1; j2 < min(j+15, C); j2++) {
				all_cmds.push_back(new line(i, j, i, j2));
			}
		}
	}

	// loop and do stuff
	while(true) {
		fprintf(stderr, ".");
		fflush(stderr);

		int best_cmd = -1, best_worth = 0;
		for (int i = 0; i < all_cmds.size(); i++) {
			int w = all_cmds[i]->worth(data, done, p1, p2);
			if (w > p1 && (best_cmd == -1 || w > best_worth)) {
				best_worth = w;
				best_cmd = i;
			} else if (w <= p1) {
				delete all_cmds[i];
				all_cmds[i] = all_cmds.back();
				all_cmds.pop_back();
				i--;
			}
		}
		if (best_cmd == -1) break;
		commands.push_back(all_cmds[best_cmd]->str());
		all_cmds[best_cmd]->doit(done);
	}
	fprintf(stderr, "%d", commands.size());

	while(!all_cmds.empty()) {
		delete all_cmds.back();
		all_cmds.pop_back();
	}

	// complete with trivial paint_square/erase_cell
	for (int i = 0; i < data.size(); i++) {
		for (int j = 0; j < data[i].size(); j++) {
			if (data[i][j] && !done[i][j])
				commands.push_back(strfmt("PAINT_SQUARE %d %d 0", i, j));
			if (done[i][j] && !data[i][j])
				commands.push_back(strfmt("ERASE_CELL %d %d", i, j));
		}
	}
	fprintf(stderr, "(%d %d -> %d)", p1, p2, commands.size());

	return commands;
}

vector<string> solve2(const vector< vector<bool> > &data) {
	

	vector<pair<int, int>> params;
	mutex the_mutex;
	vector<string> best_sol;

	for (int p1 = 2; p1 < 6; p1 += 1) {
		for (int p2 = p1-1; p2 < 12; p2++) {
			params.push_back(make_pair(p1, p2));
		}
	}
	auto internal = [&]{
			while(true) {
				the_mutex.lock();
				if (params.empty()) {
					the_mutex.unlock();
					break;
				}
				auto x = params.back();
				params.pop_back();
				fprintf(stderr, "{%d %d / %d}", x.first, x.second, params.size());
				the_mutex.unlock();

				auto sol = solve2_int(data, x.first, x.second);

				the_mutex.lock();
				if (best_sol.empty() || sol.size() < best_sol.size()) {
					fprintf(stderr, "[!! %d %d -> %d !!]", x.first, x.second, sol.size());
					best_sol = sol;
				}
				the_mutex.unlock();
				}
			};

	vector<thread> threads;
	for (int i = 0; i < 4; i++) {
		threads.push_back(thread(internal));
	}

	for (int i = 0; i < threads.size(); i++) {
		threads[i].join();
	}
	fprintf(stderr, "Done.\n");

	return best_sol;
}
