#include <iostream>
#include <list>
#define CN_INFINITY 1000000
using namespace std;

#define size 15
const int M = 100;

struct Node
{
	int id;
	Node(int _id = -1) :id(_id) {}
};

struct Edge
{
	int id1;
	int id2;
	int cost;
	Edge(int _id1 = -1, int _id2 = -1, int _cost = CN_INFINITY) : id1(_id1), id2(_id2), cost(_cost) {}
};

struct Graph
{
	list<Node> nodes;
	list<Edge> edges;

	void add_node(Node node)
	{
		nodes.push_back(node);
		return;
	}

	bool add_edge(Edge edge)
	{
		edges.push_back(edge);
		return true;
	}

	list<int> get_neighbours(int id)
	{
		list<int> neighbours;
		for (Edge e : edges)
		{
			if (e.id1 == id)
				neighbours.push_back(e.id2);
		}
		return neighbours;
	}

	int cost(int id1, int id2) 
	{
		for (Edge e : edges) 
			if (e.id1 == id1 || e.id2 == id1) 
					return e.cost; 

	}
};

int min(int a, int b)
{
	if (a > b)
		return b;
	else return a;
}

void pathfinder(Graph graph)
{
	int d[size][size];
	for (Node n : graph.nodes)
	{
		list<int> neighbours = graph.get_neighbours(n.id);  
		for (int k : neighbours)
		{
			Node neighbour;
			neighbour.id = k;
			for (int j = 1;j <= size;j++)
			{
				if (j == k)
					d[n.id][j] = graph.cost(n.id, neighbour.id);   
				else d[n.id][j] = CN_INFINITY;
			}
		}
	}
	int m = 1;
	while (m <= size)
	{
		for (int i = 1;i <= size;i++)
			for (int j = 1;j <= size;j++)
				d[i][j] = min(d[i][j], d[m][j] + d[i][m]);
		m = +1;
	}
	for (int i = 1;i <= size;i++)
	{
		int j = 1;
		while (j <= size)
		{
			std::cout << "minimum cost between nodes " << i << " and " << j << " " << d[i][j] << endl;
			j++;
		}
	}
	return;
}

int main()
{
	Node start, goal;
	Graph graph;
	Node a, b, c, d, e, f, g, h, i, j, k, l, m, n, o; 
	a.id = 1; b.id = 2; c.id = 3; d.id = 4; e.id = 5; 
	f.id = 6; g.id = 7; h.id = 8; i.id = 9; j.id = 10;
	k.id = 11; l.id = 12; m.id = 13; n.id = 14; o.id = 15;
	graph.add_node(a);
	graph.add_node(b);
	graph.add_node(c);
	graph.add_node(d);
	graph.add_node(e);
	graph.add_node(f);
	graph.add_node(g);
	graph.add_node(h);
	graph.add_node(i);
	graph.add_node(j);
	graph.add_node(k);
	graph.add_node(l);
	graph.add_node(m);
	graph.add_node(n);
	graph.add_node(o);
	Edge ab(1, 2, 5), af(1, 6, 8), bc(2, 3, 1), bd(2, 4, 9), bj(2, 10, 3), cn(3, 14, 2), dg(4, 7, 4), dh(4, 8, 7),
		ea(5, 1, 1), fe(6, 5, 6), ge(7, 5, 2), ha(8, 1, 10), ie(9, 5, 2), jd(10, 4, 6), jk(10, 11, 9), ki(11, 9, 4),
		kl(11, 12, 8), lh(12, 8, 5), ml(13, 12, 3), mo(13, 15, 12), nm(14, 13, 7), nj(14, 10, 8), on(15, 14, 9);
	graph.add_edge(ab);
	graph.add_edge(af);
	graph.add_edge(bc);
	graph.add_edge(bd);
	graph.add_edge(bj);
	graph.add_edge(cn);
	graph.add_edge(dg);
	graph.add_edge(dh);
	graph.add_edge(ea);
	graph.add_edge(fe);
	graph.add_edge(ge);
	graph.add_edge(ha);
	graph.add_edge(ie);
	graph.add_edge(jd);
	graph.add_edge(jk);
	graph.add_edge(ki);
	graph.add_edge(kl);
	graph.add_edge(lh);
	graph.add_edge(ml);
	graph.add_edge(mo);
	graph.add_edge(nm);
	graph.add_edge(nj);
	graph.add_edge(on);
	pathfinder(graph);
	pathfinder(graph);
	system("pause");
	return 0;
}