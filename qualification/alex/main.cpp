#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>
#include <cmath>

#include "../../util.hpp"

using namespace std;	// pour la concision, bordel

// =========== THE PROBLEM ====================

struct warehouse {
	int x, y;
	vector<int> qp;			// quantity for each product
};

struct order {
	int x, y;
	int P;					// number of products to be delivered
	vector<int> ip;			// products to be delivered
};

struct problem {
	int X, Y;		// size of the map
	int T;			// total time
	int U;			// max payload for a drone

	int D;			// number of drones

	int P;			// number of products
	vector<int> up;	// weight of each product

	int W;			// the number of warehouses
	vector<warehouse> wh;	// the content of the warehouses

	int O;			// the number of orders
	vector<order> orders;	// each order
};

problem read_problem() {
	problem p;

	scanf("%d %d %d %d %d\n", &p.X, &p.Y, &p.D, &p.T, &p.U);

	scanf("%d\n", &p.P);
	p.up.resize(p.P);
	for (int i = 0; i < p.P; i++) {
		scanf("%d ", &p.up[i]);
	}

	scanf("%d\n", &p.W);
	for (int i = 0; i < p.W; i++) {
		warehouse w;
		scanf("%d %d\n", &w.x, &w.y);
		w.qp.resize(p.P);
		for (int j = 0; j < p.P; j++) {
			scanf("%d ", &w.qp[j]);
		}
	}

	scanf("%d\n", &p.O);
	for (int i = 0; i < p.O; i++) {
		order o;
		scanf("%d %d\n%d\n", &o.x, &o.y, &o.P);
		o.ip.resize(o.P);
		for (int j = 0; j < o.P; j++) {
			scanf("%d ", &o.ip[j]);
		}
	}

	return p;
};


// =========== DEFINITION OF A SOLUTION ====================

struct cmd {
	enum {
		Load,
		Unload,
		Deliver,
		Wait
	} c;
	int i;		// number of the warehouse (for load/unload), or id of the order (for deliver), or time (for wait)
	int pi;			// type of product
	int np;			// number of the products (for load/unload/deliver)

	static cmd load(int wi, int pi, int np) {
		cmd x = { Load, wi, pi, np };
		return x;
	}
	static cmd unload(int wi, int pi, int np) {
		cmd x = { Unload, wi, pi, np};
		return x;
	}
	static cmd deliver(int oi, int pi, int np) {
		cmd x = { Deliver, oi, pi, np };
		return x;
	}
	static cmd wait(int t) {
		cmd x = { Wait, t, 0, 0 };
		return x;
	}
};

struct solution {
	const problem &p;

	vector< vector<cmd> > dcmd;		// for each drone, a list of the commands it has to do

	solution(const problem &_p) : p(_p) {}

	void print() {
		int s = 0;
		for (auto& x: dcmd) s += x.size();
		printf("%d\n", s);

		for (int i = 0; i < p.D; i++) {
			for (auto& c: dcmd[i]) {
				if (c.c == cmd::Load) printf("%d L %d %d %d\n", i, c.i, c.pi, c.np);
				if (c.c == cmd::Unload) printf("%d U %d %d %d\n", i, c.i, c.pi, c.np);
				if (c.c == cmd::Deliver) printf("%d D %d %d %d\n", i, c.i, c.pi, c.np);
				if (c.c == cmd::Wait) printf("%d W %d\n", i, c.i);
			}
		}
	}
};

// =========== VERIFY A SOLUTION IS CORRECT & CALCULATE SCORE ===========

struct drone_status {
	int x, y;			// position for beginning next operation
	int t;				// time of next operation
	int i;				// id of next operation in queue
	vector<int> qp;		// quantity of each product loaded
	int u;				// current load

	bool operator<(const drone_status& o) { return t < o.t; }
	drone_status(int _x, int _y, int _t, int _i, const vector<int> &_qp, int _u)
	: x(_x), y(_y), t(_t), i(_i), qp(_qp), u(_u) {}

	void move(int xx, int yy) {
		int dx = x-xx, dy = y-yy;
		t += ceil(sqrt(dx*dx+dy*dy));
		x = xx;
		y = yy;
	}
	void load(const problem &p, int pi, int q) {
		// weight verification
		u += q * p.up[pi];
		if (u > p.U) fprintf(stderr, "Too much load (%d/%d)\n", u, p.U);
		// do stuff
		t++;
		qp[pi] += q;
	}
	void unload(const problem &p, int pi, int q) {
		u -= q * p.up[pi];
		if (q > qp[pi]) fprintf(stderr, "Not enough %d on drone (have %d/%d)\n", pi, qp[pi], q);

		t++;
		qp[pi] -= q;
	}
};

struct drone_status_compare {
	const vector<drone_status> &ds;
	drone_status_compare(const vector<drone_status> &_ds) : ds(_ds) {}
	bool operator() (int i, int j) const { return ds[i].t < ds[j].t; }
};

struct wh_status {
	vector<int> qp;		// quantity of each product in store
	wh_status(const vector<int> &_qp) : qp(_qp) {}
	void rm(int pi, int q) {
		if (q > qp[pi]) fprintf(stderr, "Not enough %d in warehouse", pi);
		qp[pi] -= q;
	}
	void add(int pi, int q) {
		qp[pi] += q;
	}
};

struct order_status {
	vector<int> qp;		// quantity of each product remaining to be delivered
	order_status(const vector<int> &_qp) : qp(_qp) {}
	int deliver(int t, const problem &p, int pi, int n) {
		qp[pi] -= n;
		// check if order is satisfied
		for (auto x: qp) if (x != 0) return 0;
		return ceil(100.*((float)p.T - t)/((float)p.T));
	}
};

struct status {
	const problem &p;
	const solution &s;

	vector<wh_status> wh;
	vector<order_status> order;
	vector<drone_status> drone;

	int score;

	status(const problem &_p, const solution &_s) : p(_p), s(_s) {
		// initialize warehouse status
		for (int i = 0; i < p.W; i++) {
			wh.push_back(wh_status(p.wh[i].qp));
		}

		// initialize order status
		for (int i = 0; i < p.O; i++) {
			order.push_back(order_status(vector<int>(p.P, 0)));
			for (auto pi: p.orders[i].ip) {
				order.back().qp[pi]++;
			}
		}

		// initialize drone status
		for (int i = 0; i < p.D; i++) {
			drone.push_back(drone_status(p.wh[0].x, p.wh[0].y, 0, 0, vector<int>(p.P, 0), 0));
		}

		//initialize score 
		score = 0;
	}

	int simu() {
		auto qu = priority_queue<int, std::vector<int>, drone_status_compare>(drone_status_compare(drone));
		for (int i = 0; i < p.D; i++) {
			if (drone[i].i < s.dcmd[i].size()) qu.push(i);
		}

		while (!qu.empty()) {
			int t = drone[qu.top()].t;
			vector<int> di;
			while (drone[qu.top()].t == t) {
				di.push_back(qu.top());
				qu.pop();
			}

			// Do all the unloads
			for (auto i: di) {
				const cmd &c = s.dcmd[i][drone[i].i];
				if (c.c == cmd::Unload) {
					if (drone[i].x == p.wh[c.i].x && drone[i].y == p.wh[c.i].y) {
						wh[c.i].add(c.pi, c.np);
						drone[i].unload(p, c.pi, c.np);
						drone[i].i++;
					} else {
						drone[i].move(p.wh[c.i].x, p.wh[c.i].y);
					}
				}
			}

			// Do all the other things
			for (auto i: di) {
				const cmd &c = s.dcmd[i][drone[i].i];
				if (c.c == cmd::Load) {
					if (drone[i].x == p.wh[c.i].x && drone[i].y == p.wh[c.i].y) {
						wh[c.i].rm(c.pi, c.np);
						drone[i].load(p, c.pi, c.np);
						drone[i].i++;
					} else {
						drone[i].move(p.wh[c.i].x, p.wh[c.i].y);
					}
				} else if (c.c == cmd::Deliver) {
					if (drone[i].x == p.orders[c.i].x && drone[i].y == p.orders[c.i].y) {
						score += order[c.i].deliver(t, p, c.pi, c.np);
						drone[i].unload(p, c.pi, c.np);
						drone[i].i++;
					} else {
						drone[i].move(p.orders[c.i].x, p.orders[c.i].y);
					}
				} else if (c.c == cmd::Wait) {
					drone[i].t += c.i;
					drone[i].i++;
				}
			}

			// Put all the drones back on the queue
			for (auto i: di) {
				if (drone[i].i < s.dcmd[i].size()) qu.push(i);
			}
		}
	}
};

// =========== SOLVE THE PROBLEM =======

solution solve(const problem &p) {
	solution s(p);

	// TODO

	return s;
}

int main() {
	problem p = read_problem();

	solution s = solve(p);

	// simulate
	status st = status(p, s);
	st.simu();
	printf("Score: %d\n", st.score);

	s.print();
}
