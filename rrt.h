
#ifndef RRT_H
#define RRT_H

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <vector>
#include <math.h>
#include <random>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;
using namespace nlohmann;

#define MAX_NODES 5000

typedef struct {
  int x;
  int y;
} coordinates;


struct Node {
  Node* parent;
  vector <Node*> children;
  coordinates position;
};

class RRT {
  public:
    RRT();
    bool within_bounds(int x, int y);
    void find_node_by_color(Node *&node, int r, int g, int b);
    void set_color(coordinates &n, int offset, int b, int g, int r);
    float calculate_distance(coordinates &p, coordinates &q);
    int find_nearest_node(Node *q_rand);
    coordinates take_step(coordinates &q_near, coordinates &q_rand);
    bool check_for_obstruction(int x, int y);
    bool check_path_along_height(coordinates &q1, coordinates &q2);
    bool check_path_along_width(coordinates &q1, coordinates &q2);
    void draw_path();
    void add_node(int exp);

    Node* start;
    Node* goal;
    Node* nodes[MAX_NODES];
    int total_nodes;
    bool reached;
    int exploration_factor;
    int step_size;
    int screen_width;
    int screen_height;
    Mat image;
    json config;
};

#endif
