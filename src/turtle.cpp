#include "turtle.h"
#include <cmath>
#include <iostream>
#include <functional>

#define PI 3.14159265

void Turtle::Update() {
  update_steps++;
  if (update_steps >= speed_steps)
  {

    SDL_Point prev_cell{
        static_cast<int>(head_x),
        static_cast<int>(head_y)}; // We first capture the head's cell before updating.

    /* Steps to perform:
    perform motion (state rotate or move straight)
    if rotate check if goal reached
    if straight check for timeout and for not allowed area
    if timeout start random generator for new motion and new timeout
    */
    switch (state)
    {
    case State::Sleep:
      Sleep();
      break;

    case State::Turn:
      UpdateRotation();
      if(rotation == target_rotation_){
        NewWalk();
      }
      break;

    case State::Walk:
      UpdateHead();
        // check if walked enough
      if(steps >= steps_to_go){
        NewDirection(Wall::None);
        steps = 0;
      }
      else
      {
        DetectWall();
      }
      break;

    case State::Feed:
      auto instruction = motion_path.front();
      /*Walking*/
      if(instruction.action_ == State::Walk){
        steps_to_go = instruction.steps_;
        UpdateHead();
        if(steps >= steps_to_go){
          instruction.completed = true;
          steps = 0;
          std::cout << steps_to_go << " Steps completed! \n";
        }else{
          DetectWall();
        }
      }else if (instruction.action_ == State::Turn){
        target_rotation_ = instruction.steps_;
        /* Turn */
        UpdateRotation();
        if(rotation == target_rotation_){
          instruction.completed = true;
          std::cout << " Turn to " << target_rotation_ << " completed! \n";
        }
      }else{
        std::cout << "Invalid Instructio! \n";
      }
      if(instruction.completed == true){
        // remove completed instruction from path
        motion_path.pop_front();
        // if complete walk new way.
        if (motion_path.empty()){
          NewDirection(Wall::None);
        }
      }
      break;
    }

    SDL_Point current_cell{
        static_cast<int>(head_x),
        static_cast<int>(head_y)}; // Capture the head's cell after updating.

    //GoSleep();
    // Update all of the body vector items if the snake head has moved to a new
    // cell.
    //if (current_cell.x != prev_cell.x || current_cell.y != prev_cell.y) {
    //  UpdateBody(current_cell, prev_cell);
    //}
    update_steps = 0;
  }
}

// States ************************************************************


void Turtle::UpdateRotation() {
  int rot_step = 5;
  // decide to turn left or right.
  int diff = target_rotation_ - rotation;
  if(diff > 180 || (diff > -180 && diff < 0))
  {
    rotation = rotation - rot_step;
  }
  else
  {
    rotation = rotation + rot_step;
  }
  // keep rotation below 360
  if (rotation >= 360){
    rotation = rotation - 360;
  } else if (rotation < 0){
    rotation = rotation + 360;
  }
}

void Turtle::UpdateHead() {
  switch (rotation) {
    case 0:
      head_y -= speed;
      break;

    case 45:
      head_x += speed;
      head_y -= speed;
      break;

    case 90:
      head_x += speed;
      break;

    case 135:
      head_x += speed;
      head_y += speed;
      break;

    case 180:
      head_y += speed;
      break;

    case 225:
      head_x -= speed;
      head_y += speed;
      break;

    case 270:
      head_x -= speed;
      break;

    case 315:
      head_x -= speed;
      head_y -= speed;
      break;
  }
  steps++;


  // Wrap the Snake around to the beginning if going off of the screen.
  //head_x = fmod(head_x + grid_width, grid_width);
  //head_y = fmod(head_y + grid_height, grid_height);
}

// Transitions ************************************************************

void Turtle::DetectWall() {
  // TODO make sure to have turtle always at safe distance from wall (own function?)
  bool wall = false;
  int security_dist = size;
  Wall wall_dir = Wall::None;

  // Right wall
  if (head_x >= grid_width - security_dist)
  {
    head_x = grid_width - security_dist;
    if (rotation == 90 || rotation == 135 || rotation == 45){
      wall = true;
      wall_dir = Wall::Right;
    }
  }
  // Left wall
  if (head_x <= 0){
    head_x = 0;
    if (rotation == 270 || rotation == 315 || rotation == 225){
      wall = true;
      wall_dir = Wall::Left;
    }
  }
  // Bottom Wall
  if (head_y >= grid_width -security_dist){
    head_y = grid_width - security_dist;
    if (rotation == 180 || rotation == 225 || rotation == 135){
      wall = true;
      wall_dir = Wall::Bottom;
    }
  }
  // Top Wall
  if (head_y <= 0){
    head_y = 0;
    if (rotation == 0 || rotation == 315 || rotation == 45){
      wall = true;
      wall_dir = Wall::Top;
    }
  }
  // Rotation change
  if (wall){
    motion_path.erase(motion_path.begin(), motion_path.end());
    // new direction
    NewDirection(wall_dir);
    wall = false;
    std::cout << "Wall Detected!\n";
  }
}

void Turtle::NewWalk(){
 // Seed with a real random value, if available
  std::random_device r;

  // Choose a random mean between 1 and 6
  //std::default_random_engine e1(r());
  std::mt19937 e2(r());
  //TODO better maximal length of walk
  std::uniform_int_distribution<int> uniform_dist(10, grid_width/2);
  steps_to_go = uniform_dist(e2);

  state = State::Walk;
  std::cout << "Going to Walk " << steps_to_go << " Steps!\n";
  //std::cout << "Head x: " << head_x << " y: " << head_y << " \n";

}

Turtle::TargetVector Turtle::GetTargetVector(int x_start, int y_start, int x_end, int y_end){
    // definition of normal up
    int x1 = 0;
    int y1 = -1;
    // get direction to target
    int x2 = x_end - x_start;
    int y2 = y_end - y_start;
    int dot = x1 * x2 + y1 * y2;
    int det = x1 * y2 - x2 * y1;
    float dir_rad = atan2(det, dot);
    // convert to degree
    float dir_deg = dir_rad * (180/3.1415);
    int dir_deg_rnd = static_cast<int>(round(dir_deg));
    // address negative directions.
    if (dir_deg_rnd < 0){
      dir_deg_rnd = dir_deg_rnd + 360;
    }

    // distance
    float dist = sqrt(pow(x_end - x_start, 2) + pow(y_end - y_start, 2) * 1.0);
    int dist_rnd = static_cast<int>(round(dist));

    return TargetVector(dir_deg_rnd, dist_rnd);
}

  void Turtle::ConvertToTurtleVector(TargetVector *target){
      // get nearest walkable rotation
      std::cout << "Converting " << target->dir << " to ";
      int rot_steps = nearbyint(static_cast<float>(target->dir) / 45.0);
      // protect from larger than 360 deg
      if (rot_steps >= 8){
        rot_steps = 0;
      }
      // Write in 45 deg steps
      target->dir = rot_steps * 45;

      std::cout << target->dir << " \n";

      // check for diagonal walking direction
      if(rot_steps%2 != 0){
        target->dist = target->dist / sqrt(2);
      }
  }

Turtle::Coordinate Turtle::GetTargetCoordinate(int x, int y, int dir, int dist){
    int x2 = 0;
    int y2 = 0;
    // x is horitonal top
    // y is vertical left
    // diagonal steps are one unit of dist each as well

    switch (dir) {
    case 0:
      x2 = x;
      y2 = y - dist;
      break;

    case 45:
      x2 = x + dist;
      y2 = y - dist;
      break;

    case 90:
      x2 = x + dist;
      y2 = y;
      break;

    case 135:
      x2 = x + dist;
      y2 = y + dist;
      break;

    case 180:
      x2 = x;
      y2 = y + dist;
      break;

    case 225:
      x2 = x - dist;
      y2 = y + dist;
      break;

    case 270:
      x2 = x - dist;
      y2 = y;      break;

    case 315:
      x2 = x - dist;
      y2 = y - dist;
      break;
    default:
      std::cout << "INVALID FUNCTION CALL! -GetTargetXY- \n";
    }

    return Turtle::Coordinate(x2,y2);
  }

// TODO FOOD
void Turtle::CheckForFood(int x, int y){
  if (state == State::Turn){

    TargetVector target =  GetTargetVector(head_x, head_y, x, y);

    // create view angle of turtle
    int range = 15;
    int max_range = rotation + range;
    int min_range = rotation - range;
    bool in_range = false;
    if (min_range < 0 || max_range > 359)
    {
      min_range = min_range + 90;
      max_range = max_range + 90;
      int target_dir_90 = target.dir + 90;
      if (target_dir_90 > 360){
        target_dir_90 = target_dir_90 - 360;
      }
      in_range = (target_dir_90 <= max_range && target_dir_90 >= min_range);
    }else{
      in_range = (target.dir <= max_range && target.dir >= min_range);
    }
    // check if turtle can see target
    if (in_range) {

      //state = State::Walk;
      steps_to_go = target.dist;
      std::cout << "I see Food at " << target.dir << " deg in " << target.dist << " steps \n";

      if(false){
       // set starting direction
       ConvertToTurtleVector(&target);

       // calculate new path
       // based on a right triangle
       float beta_signed = rotation - target.dir;
       float beta = abs(beta_signed);
       float alpha = 90 - beta; // 90 - 180 - alpha
       float gamma = 90;

       int b = static_cast<int>(round((target.dist * sin(beta * PI / 180)) / sin(gamma * PI / 180)));
       int a = static_cast<int>(round((target.dist * sin(alpha * PI / 180)) / sin(gamma * PI / 180)));
       // check which direction to turn
       int turn = 0;
       if (beta_signed > 0)
       {
         turn = rotation - 90;
      }
      else
      {
        turn = rotation + 90;
      }
      if (turn >= 360){
        turn = turn - 360;
      }else if (turn < 0){
        turn = turn + 360;
      }

      int second_steps = abs(b);
      int first_steps = abs(a);

      motion_path.push_back( Instruction(State::Walk, first_steps) );
      std::cout << "IN: Walk " << first_steps << " steps \n";

      if(second_steps != 0){
        motion_path.push_back( Instruction(State::Turn, turn) );
        std::cout << "IN: Turn " << turn << " steps \n";

        motion_path.push_back( Instruction(State::Walk, second_steps) );
        std::cout << "IN: Walk " << second_steps << " steps \n";
      }
      }else{

        ConvertToTurtleVector(&target);

        motion_path.push_back( Instruction(State::Turn, target.dir) );
        std::cout << "IN: Turn " << target.dir << " steps \n";
        motion_path.push_back( Instruction(State::Walk, target.dist) );
        std::cout << "IN: Walk " << target.dist << " steps \n";

        // get new position
        Coordinate targetXY = GetTargetCoordinate( head_x, head_y, target.dir, target.dist);

        int distance = target.dist;
        while (distance >= size)
        {
          // set next walking path
          TargetVector new_target =  GetTargetVector(targetXY.x, targetXY.y, x, y);
          ConvertToTurtleVector(&new_target);
          motion_path.push_back( Instruction(State::Turn, new_target.dir) );
          std::cout << "IN: Turn " << new_target.dir << " steps \n";
          motion_path.push_back( Instruction(State::Walk, new_target.dist) );
          std::cout << "IN: Turn " << new_target.dist << " steps \n";
          // get new position
          targetXY = GetTargetCoordinate( targetXY.x, targetXY.y, new_target.dir, new_target.dist);
          distance = new_target.dist;
        }
      }

      state = State::Feed;
      steps = 0;
      steps_to_go = 0;
    }
  }
}

void Turtle::NewDirection(Wall wall){
  // Seed with a real random value, if available
  std::random_device r;
  bool clear = false;
  int new_dir = 0;

  // Choose a random mean between 1 and 6
  //std::default_random_engine e1(r());
  std::mt19937 e2(r());
  std::uniform_int_distribution<int> uniform_dist(0, 6);

  while(!clear){
    int mean = uniform_dist(e2);

    // new rotation
    new_dir = (45 * mean);
    if(new_dir >= 360){
      new_dir = new_dir - 360;
    }

    switch (wall)
    {
      case Wall::Top :
        if(new_dir > 45 && new_dir < 315){
          clear = true;
        }
        break;

      case Wall::Right :
        if(new_dir > 135 || new_dir < 45){
          clear = true;
        }
        break;

      case Wall::Bottom :
        if(new_dir > 225 || new_dir < 135){
          clear = true;
        }
        break;

      case Wall::Left :
        if(new_dir > 315 || new_dir < 225){
          clear = true;
        }
        break;
    
      default:
        clear = true;
        break;
    }
  }

  target_rotation_ = new_dir;
  state = State::Turn;
  std::cout << "Turning to " << target_rotation_ << "!\n";
}

void Turtle::GoSleep(){
  //std::random_device r;
  //std::mt19937 e2(r());
  //std::uniform_int_distribution<int> uniform_dist(0, 1000);
  //int mean = uniform_dist(e2);
  static int cnt = 0;
  cnt++;
  if (cnt >= 500)
  {
    state = State::Sleep;
    std::cout << "Going to Sleep!\n";
    cnt = 0;
  }
}

void Turtle::Sleep(){
  counter++;
  if (counter >= sleeepcycle)
  {
    NewDirection(Wall::None);
    counter = 0;
  }
}


// Inefficient method to check if cell is occupied by snake.
bool Turtle::TurtleCell(int x, int y) {
  /*if (x == static_cast<int>(head_x) && y == static_cast<int>(head_y)) {
    return true;
  }*/
  if ((x >= head_x) && (x <= (head_x + size)) && ((y >= head_y) && (y <= (head_y + size)))){
    return true;
  }
  return false;
}