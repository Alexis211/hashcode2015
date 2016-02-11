#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>

#include "../../util.hpp"

using namespace std;	// pour la concision, bordel

struct drone {
	int U;			// max payload of the drone
};

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
	vector<drone> d;// each of the drone (max payload, items carried)

	int P;			// number of products
	vector<int> wp;	// weight of each product

	int W;			// the number of warehouses
	vector<warehouse> wh;	// the content of the warehouses

	int O;			// the number of orders
	vector<order> orders;	// each order
};


problem read_problem() {
	problem p;

	scanf("%d %d %d %d %d\n", &p.X, &p.Y, &p.D, &p.T, &p.U);

	scanf("%d\n", &p.P);
	p.wp.resize(p.P);
	for (int i = 0; i < p.P; i++) {
		scanf("%d ", &p.wp[i]);
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

int main() {
	problem p = read_problem();
}
