
#include "libs/json.hpp"
#include "rrt.h"
#include <typeinfo>

string config_filename = "config.json";

json load_config() {
    ifstream i(config_filename);
    json config;
    i >> config;

    return config;
}

RRT::RRT() {
    config = load_config();
    total_nodes = 0;
    reached = false;

    step_size = config["step_size"].get<int>();
    exploration_factor = config["exploration_factor"].get<int>();

    image = imread("./map.png", CV_LOAD_IMAGE_COLOR);
    screen_width = image.cols;
    screen_height = image.rows;

    start = new Node;
    find_node_by_color(start, config["start_color"]["r"].get<int>(), config["start_color"]["g"].get<int>(), config["start_color"]["b"].get<int>());
    start->parent = NULL;
    // set_color(start->position, 5, 255, 0, 0); // set start node to blue
    nodes[total_nodes++] = start;

    goal = new Node;
    find_node_by_color(goal, config["goal_color"]["r"].get<int>(), config["goal_color"]["g"].get<int>(), config["goal_color"]["b"].get<int>());
    // set_color(goal->position, 10, 255, 255, 255);   // set goal node to green

    srand(time(NULL));  // calling time with NULL returns the current system time
}

void RRT::find_node_by_color(Node* &node, int r, int g, int b) {
    for(int i = 0; i < image.cols; i++) {
      for(int j = 0; j < image.rows; j++) {
        if ((image.at<Vec3b>(i,j)[0] == b) && (image.at<Vec3b>(i,j)[1] == g) && (image.at<Vec3b>(i,j)[2] == r)) {
            node->position.x = (i-1) % screen_width;
            node->position.y = (j-1) % screen_height;

            return ;
        }
      }
    }
}

bool RRT::within_bounds(int x, int y) {
    // check if coordinates within bounds of the image
    if ((x < 0) || (x > screen_width) || (y < 0) || (y > screen_height))
      return false;

    return true;
}

void RRT::set_color(coordinates &node, int offset, int b, int g, int r) {
  // set BGR values of offset height and width to demarcate nodes
  for(int i = node.x - offset; i < node.x + offset; i++) {
    for (int j = node.x - offset; j < node.y + offset; j++) {
        if (!within_bounds(i, j))
          continue;

        image.at<Vec3b>(i,j)[0] = b;
        image.at<Vec3b>(i,j)[1] = g;
        image.at<Vec3b>(i,j)[2] = r;
    }
  }
}

float RRT::calculate_distance(coordinates &p, coordinates &q) {
    // calculate distance betwen two nodes
    return sqrt(pow(p.x - q.x, 2) + pow(p.y - q.y, 2));
}

int RRT::find_nearest_node(Node *q_rand) {
    // find nearest node in graph to q_rand
    int q_near_index = 0;
    float min_distance = 999.9, distance = calculate_distance(start->position, q_rand->position);

    for(int i = 0; i < total_nodes; i++) {
      distance = calculate_distance(nodes[i]->position, q_rand->position);

      if (distance < min_distance) {
        min_distance = distance;
        q_near_index = i;
      }
    }

    return q_near_index;
}

coordinates RRT::take_step(coordinates &q_near, coordinates &q_rand) {
    // take a step from q_near to q_rand
    coordinates dist_vector, q_step;
    float dist_magnitude = 0.0, unit_x = 0.0, unit_y = 0.0;

    dist_vector.x = (q_rand.x - q_near.x);
    dist_vector.y = (q_rand.y - q_near.y);

    dist_magnitude = sqrt(pow(dist_vector.x, 2) + pow(dist_vector.y, 2));
    unit_x = (float)(dist_vector.x / dist_magnitude);
    unit_y = (float)(dist_vector.y / dist_magnitude);

    q_step.x = (int)(q_near.x + step_size*unit_x);
    q_step.y = (int)(q_near.y + step_size*unit_y);

    return q_step;
}

bool RRT::check_for_obstruction(int x, int y) {
    // checks in the image if color is not white (hence not free)
    if ( ((int)image.at<Vec3b>(x, y)[0] < 250) &&
          ((int)image.at<Vec3b>(x, y)[1] < 250) &&
          ((int)image.at<Vec3b>(x, y)[2] < 250) )
      return true;

    return false;
}

bool RRT::check_path_along_height(coordinates &q1, coordinates &q2) {
    // check the straight line between q_near and q_rand
    // to see if it passes through any obstacles
    coordinates low, high;
    float slope;

    if (q1.x > q2.x) {
      high = q1;
      low = q2;
    } else {
      high = q2;
      low = q1;
    }

    if (high.x == low.x)
      return false;

    slope = ((float)(high.y - low.y)) / ((float)(high.x - low.x));

    for(int x1 = low.x + 1; x1 < high.x; x1++) {
      // loop along the x-axis, using slope to calculate corresponding y-axis value
      int y1 = (int)( (slope*x1) - (slope*low.x) + low.y );
      int y2 = y1 + 1;
      // checks if index is out of bounds
      if ( (x1 < 0) || (x1 > screen_width) || (y1 < 0) || (y1 > screen_height) || (y2 < 0) || (y2 > screen_height) )
        continue;
      // checks if image color is white (free) or not
      if (check_for_obstruction(x1, y1))
        return false;
      if (check_for_obstruction(x1, y2))
        return false;
    }

    return true;
}

bool RRT::check_path_along_width(coordinates &q1, coordinates &q2) {
    // check the straight line between q_near and q_rand
    // to see if it passes through any obstacles
    coordinates low, high;
    float slope;

    if (q1.y > q2.y) {
      high = q1;
      low = q2;
    } else {
      high = q2;
      low = q1;
    }

    if (high.x == low.x)
      return false;

    slope = ((float)(high.y - low.y)) / ((float)(high.x - low.x));

    for(int y1 = low.y + 1; y1 < high.y; y1++) {
      // loop along the x-axis, using slope to calculate corresponding y-axis value
      int x1 = (int)( ((y1 - low.y)/slope) + low.x );
      int x2 = x1 + 1;
      // checks if index is out of bounds
      if ( (y1 < 0) || (y1 > screen_height) || (x1 < 0) || (x1 > screen_width) || (x2 < 0) || (x2 > screen_width) )
        continue;
      // checks if image color is white (free) or not
      if (check_for_obstruction(x1, y1))
        return false;
      if (check_for_obstruction(x2, y1))
        return false;
    }

    return true;
}

void RRT::draw_path() {
    // begin from goal node and trace a way back to start node
    Node *top, *bottom;
    bottom = goal;
    top = goal->parent;

    // loop until we reach start node
    while(top->parent) {
      line(image, Point(top->position.y, top->position.x), Point(bottom->position.y, bottom->position.x), Scalar(0, 245, 245), 2, 8); // yellow final path

      bottom = bottom->parent;
      top = top->parent;
    }
}

void RRT::add_node(int exp) {
    // generate node at random position, take a step towards it from nearest node in tree
    // and add to tree if straight line between step node and near node free of obstacles
    Node* q_rand = new Node;
    Node* q_step = new Node;

    // sample the goal as q_rand with (100 - exploration_factor) % probability
    if (exp > exploration_factor) {
      q_rand = goal;
    } else {
      q_rand->position.x = rand() % screen_width + 1;
      q_rand->position.y = rand() % screen_height + 1;
    }

    int q_near_index = find_nearest_node(q_rand);
    if (calculate_distance(q_rand->position, nodes[q_near_index]->position) < step_size)
      return;
    else
      q_step->position = take_step(nodes[q_near_index]->position, q_rand->position);

    if ( check_path_along_height(nodes[q_near_index]->position, q_rand->position) && check_path_along_width(nodes[q_near_index]->position, q_rand->position) ) {
      nodes[total_nodes++] = q_step;
      q_step->parent = nodes[q_near_index];
      nodes[q_near_index]->children.push_back(q_step);
      // red branches of the tree
      line(image, Point(q_step->position.y, q_step->position.x), Point(nodes[q_near_index]->position.y, nodes[q_near_index]->position.x), Scalar(0, 0, 255), 2, 8);

      // set_color(q_step->position, 2.0, 0, 255, 0);  // yellow marking of step nodes

      if( check_path_along_height(q_step->position, goal->position) && check_path_along_width(q_step->position, goal->position) && (calculate_distance(q_step->position, goal->position) < step_size) ) {
        reached = true;
        nodes[total_nodes++] = goal;
        goal->parent = q_step;
        nodes[total_nodes-1]->children.push_back(goal);
        draw_path();
      }
    }
}


int main() {
    RRT rrt = RRT();

    random_device rd;  //Will be used to obtain a seed for the random number engine
    mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    uniform_int_distribution<> unif_dist(1, 100);

    while(!rrt.reached)
      rrt.add_node(unif_dist(gen));

    namedWindow("RRT Path Generation", WINDOW_AUTOSIZE);
    imshow("RRT Path Generation", rrt.image);
    imwrite("rrt_path.png", rrt.image);
    waitKey();

    return 0;
}
