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
		p.wh.push_back(w);
	}

	scanf("%d\n", &p.O);
	for (int i = 0; i < p.O; i++) {
		order o;
		scanf("%d %d\n%d\n", &o.x, &o.y, &o.P);
		o.ip.resize(o.P);
		for (int j = 0; j < o.P; j++) {
			scanf("%d ", &o.ip[j]);
		}
		p.orders.push_back(o);
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

	solution(const problem &_p) : p(_p), dcmd(p.D, vector<cmd>()) {}

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
		int dt = ceil(sqrt(dx*dx+dy*dy)); 
		fprintf(stderr, "%d %d -> %d %d (%d + %d)\n", x, y, xx, yy, t, dt);
		t += dt;
		x = xx;
		y = yy;
	}
	void load(const problem &p, int pi, int q) {
		// weight verification
		u += q * p.up[pi];
		if (u > p.U) fprintf(stderr, "Too much load (%d/%d)\n", u, p.U);
		// do stuff
		t++;
		fprintf(stderr, "load->%d\n", t);
		qp[pi] += q;
	}
	void unload(const problem &p, int pi, int q) {
		u -= q * p.up[pi];
		if (q > qp[pi]) fprintf(stderr, "Not enough %d on drone (have %d/%d)\n", pi, qp[pi], q);

		t++;
		fprintf(stderr, "unload->%d\n", t);
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

	void simu(int t_lim, bool stop_when_free) {
		while (true) {
			int t = t_lim;
			for (int i = 0; i < p.D; i++) {
				fprintf(stderr, "(%d %d %d %d %s)", i, drone[i].t, drone[i].i, s.dcmd[i].size(), (stop_when_free?"!":"."));
				if (drone[i].i == s.dcmd[i].size() && stop_when_free) {
					fprintf(stderr, "Stopping simu, %i is free\n", i);
					return;
				}
				if (drone[i].i < s.dcmd[i].size()) {
					if (t == -1 || drone[i].t < t) t = drone[i].t;
				}
			}
			if (t >= t_lim) {
				fprintf(stderr, "Stopping simu, we are at t=%d\n", t);
				return;
			}

			vector<int> di;
			for (int i = 0; i < p.D; i++) {
				if (drone[i].t == t && drone[i].i < s.dcmd[i].size()) di.push_back(i);
			}
			fprintf(stderr, "Simu at %d : %d actions [", t, di.size());
			for (auto i: di) fprintf(stderr, "%d ", i);
			fprintf(stderr, "]\n");

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
		}
	}

	void simu_until_any_free(){
		simu(p.T, true);
	}

	void simu_all() {
		simu(p.T, false);

		for (int i = 0; i < p.D; i++) {
			if (drone[i].i < s.dcmd[i].size()) fprintf(stderr, "Drone %d still has commands...\n", i);
		}
	}
	
	vector<int> avail_drones() {
		vector<int> ret;
		for (int i = 0; i < p.D; i++) {
			if (drone[i].i == s.dcmd[i].size()) ret.push_back(i);
		}
		return ret;
	}
};

// =========== SOLVE THE PROBLEM =======

solution solve(const problem &p) {
	solution s(p);

	status st = status(p, s);

	vector<bool> order_done(p.O, false);

	vector<vector<int>> wh_free;
	for (int i = 0; i < p.W; i++) {
		wh_free.push_back(st.wh[i].qp);
	}

	while (true) {
		st.simu_until_any_free();
		vector<int> a = st.avail_drones();
		if (a.size() == 0) {
			fprintf(stderr, "Nobody free...\n");
			break;
		}

		int di = a.front();

		int best_dist = -1;
		int best_o = -1;
		int best_w = -1;

		for (int io = 0; io < p.O; io++) {
			if (order_done[io]) continue;

			// check order can be done by only one go
			int w = 0;
			for (auto i: p.orders[io].ip) w += p.up[i];
			if (w > p.U) {
				order_done[io] = true;
				continue;
			}

			for (int iw = 0; iw < p.W; iw++) {
				// check warehouse iw can satisfy order io
				bool ok = true;
				for (int i = 0; i < p.P; i++) if (wh_free[iw][i] < st.order[io].qp[i]) ok = false;
				if (!ok) continue;

				int dx1 = p.wh[iw].x - st.drone[di].x, dy1 = p.wh[iw].y - st.drone[di].y;
				int dx2 = p.wh[iw].x - p.orders[io].x, dy2 = p.wh[iw].y - p.orders[io].y;
				int dist = ceil(sqrt(dx1*dx1+dy1*dy1))+ceil(sqrt(dx2*dx2+dy2*dy2));
				if (best_dist == -1 || dist < best_dist) {
					best_dist = dist;
					best_o = io;
					best_w = iw;
				}
			}
		}

		if (best_dist == -1) break;

		// Deliver order best_o by using drone di picking stuff from warehouse best_w
		fprintf(stderr, "Doing order %d by wh %d drone %d\n", best_o, best_w, di);
		for (int i = 0; i < p.P; i++) {
			if (st.order[best_o].qp[i] > 0) s.dcmd[di].push_back(cmd::load(best_w, i, st.order[best_o].qp[i]));
			wh_free[best_w][i] -= st.order[best_o].qp[i];
		}


		for (int i = 0; i < p.P; i++) {
			if (st.order[best_o].qp[i] > 0) s.dcmd[di].push_back(cmd::deliver(best_o, i, st.order[best_o].qp[i]));
		}
		order_done[best_o] = true;
	}

	st.simu_all();
	// Remove commands that go beyond the allocated time
	for (int i = 0; i < p.D; i++) {
		if (st.drone[i].t >= p.T) {
			while(s.dcmd[i].size() >= st.drone[i].i) s.dcmd[i].pop_back();
		}
	}

	return s;
}

solution solve2(const problem &p) {
	solution s(p);

	status st = status(p, s);

	vector<bool> order_done(p.O, false);

	vector<vector<int>> wh_free;
	for (int i = 0; i < p.W; i++) {
		wh_free.push_back(st.wh[i].qp);
	}
	vector<vector<int>> dr_cmd_loaded(p.D, vector<int>());
	vector<int> dr_load(p.D, 0);

	// remove orders we cannot do in one go
	for (int io = 0; io < p.O; io++) {
		int w = 0;
		for (auto i: p.orders[io].ip) w += p.up[i];
		if (w > p.U) {
			order_done[io] = true;
			continue;
		}
	}

	// do stuff
	while (true) {
		st.simu_until_any_free();
		vector<int> a = st.avail_drones();
		if (a.size() == 0) break;

		// check the nearest thing we can do between loading an order and delivering an order
		int c_dist = -1;
		int c_deliver = -1;
		int c_load = -1;
		int c_di = -1;
		for (auto di: a) {
			for (auto o: dr_cmd_loaded[di]) {
				int dx = p.orders[o].x - st.drone[di].x, dy = p.orders[o].y - st.drone[di].y;
				int dist = ceil(sqrt(dx*dx+dy*dy));
				if (dist < c_dist || c_dist == -1) {
					c_dist = dist;
					c_deliver = o;
					c_di = di;
				}
			}
		}
		int c_subdist = -1;
		int c_load_w = -1;
		for (auto di: a) {
			for (int io = 0; io < p.O; io++) {
				if (order_done[io]) continue;

				// check order can be loaded by drone
				int w = dr_load[di];
				for (auto i: p.orders[io].ip) w += p.up[i];
				if (w > p.U) {
					continue;
				}

				for (int iw = 0; iw < p.W; iw++) {
					// check warehouse iw can satisfy order io
					bool ok = true;
					for (int i = 0; i < p.P; i++) if (wh_free[iw][i] < st.order[io].qp[i]) ok = false;
					if (!ok) continue;

					int dx1 = p.wh[iw].x - st.drone[di].x, dy1 = p.wh[iw].y - st.drone[di].y;
					int dx2 = p.wh[iw].x - p.orders[io].x, dy2 = p.wh[iw].y - p.orders[io].y;
					int dist = ceil(sqrt(dx1*dx1+dy1*dy1));
					int subdist = ceil(sqrt(dx2*dx2+dy2*dy2));
					if (c_dist == -1 || dist < c_dist || (dist == c_dist && (c_subdist == -1 || subdist < c_subdist))) {
						c_dist = dist;
						c_subdist = subdist;
						c_deliver = -1;
						c_load = io;
						c_load_w = iw;
						c_di = di;
					}
				}
			}
		}
		int di = c_di;

		// if we chose to deliver
		if (c_deliver != -1) {
			fprintf(stderr, "%d deliver %d\n", di, c_deliver);
			for (int i = 0; i < p.P; i++) {
				if (st.order[c_deliver].qp[i] > 0) {
					s.dcmd[di].push_back(cmd::deliver(c_deliver, i, st.order[c_deliver].qp[i]));
					dr_load[di] -= st.order[c_deliver].qp[i] * p.up[i];
				}
			}
			for (int i = 0; i < dr_cmd_loaded[di].size(); i++) {
				if (dr_cmd_loaded[di][i] == c_deliver) {
					dr_cmd_loaded[di][i] = dr_cmd_loaded[di].back();
					dr_cmd_loaded[di].pop_back();
					break;
				}
			}
		} else if (c_load != -1) {
			fprintf(stderr, "%d pick up %d at %d\n", di, c_load, c_load_w);
			for (int i = 0; i < p.P; i++) {
				if (st.order[c_load].qp[i] > 0) s.dcmd[di].push_back(cmd::load(c_load_w, i, st.order[c_load].qp[i]));
				wh_free[c_load_w][i] -= st.order[c_load].qp[i];
				dr_load[di] += st.order[c_load].qp[i] * p.up[i];
			}
			dr_cmd_loaded[di].push_back(c_load);
			order_done[c_load] = true;
		} else {
			break;
		}
	}

	st.simu_all();
	// Remove commands that go beyond the allocated time
	for (int i = 0; i < p.D; i++) {
		if (st.drone[i].t >= p.T) {
			while(s.dcmd[i].size() >= st.drone[i].i) s.dcmd[i].pop_back();
		}
	}

	return s;
}

int main() {
	problem p = read_problem();

	solution s = solve2(p);

	// simulate
	fprintf(stderr, "Checking solution...\n");
	status st = status(p, s);
	st.simu_all();
	fprintf(stderr, "Score: %d\n", st.score);

	s.print();
}
