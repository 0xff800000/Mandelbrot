//============================================================================
// Name        : mandelbrot.cpp
// Author      : D01000100
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <vector>
#include <complex>
#include <algorithm>
#include <SDL2/SDL.h>

using namespace std;

//######################### Class Mandelbrot ##########################
enum eDirections { UP,DOWN,LEFT,RIGHT };

class Mandelbrot{
public:
	Mandelbrot(int,int);
	void render();
	void set_center(float,float);
	void shift(int);
	void zoom(int);
	void resol(int);
	void debug();

private:
	int h_res;
	int v_res;
	int iterations;
	complex<float> center;
	complex<float> left_center;
	vector< vector<int> > data;
	SDL_Renderer*renderer;
	bool updateNeeded;
	int compute_number(complex<float>);
	void min_max(int&,int&);
	int map_color(int,int,int);
};

Mandelbrot::Mandelbrot(int h, int v){
	h_res = h;
	v_res = v;
	iterations = 30;

	// Create window
	SDL_Window* window = SDL_CreateWindow
	(
		"Mandelbrot set", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		v_res,
		h_res,
		SDL_WINDOW_SHOWN
	);
    renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);

    // Black screen
    SDL_SetRenderDrawColor(renderer,0,0,0,0);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    // Init data
    for(int y=0; y<v_res; y++){
    	vector<int> line;
    	for(int x=0; x<h_res; x++){
    		line.push_back(0);
    	}
    	data.push_back(line);
    }

    // Init complex
    center = {0,0};
    left_center = {-2,0};
    updateNeeded = true;
}

void Mandelbrot::min_max(int&min,int&max){
	int mins = iterations,maxs = 0;
	for(int y=0; y<v_res; y++){
		for(int x=0; x<h_res; x++){
			if(data[y][x]>max)max = data[y][x];
			if(data[y][x]<min)min = data[y][x];
		}
	}
	min = mins;
	max = maxs;
}

int Mandelbrot::map_color(int val, int min, int max){
	return (int)(val * 255) / (iterations); // Absolute
	//return (max > 0)?((val-min)*255) / (max-min):0; // Relative

	//return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int Mandelbrot::compute_number(complex<float> c){
	complex<float> z (0,0);
	for(int i=0; i<iterations; i++){
		if(abs(z) >= 2) return i;
		z = z*z + c;
	}
	return iterations;
}

void Mandelbrot::render(){
	if(!updateNeeded) return;
	// Get upper left corner coordinate
	float dx = abs(left_center.real() - center.real()) / h_res * 2;
	float dy = dx;
	complex<float> up_left (left_center.real(),left_center.imag()+(v_res/2)*dy);

	// Compute the image
	for(int y=0; y<v_res; y++){
		for(int x=0; x<h_res; x++){
			complex<float> pt;
			pt = {up_left.real() + x*dx, up_left.imag() - y*dy};
			data[y][x] = compute_number(pt);
		}
	}

	// Render SDL monochrome image
	int min,max;
	min_max(min,max);
	for(int y=0; y<v_res; y++){
		for(int x=0; x<h_res; x++){
			int color = map_color(data[y][x], min, max);
			SDL_SetRenderDrawColor(renderer,color,color,color,color);
			SDL_RenderDrawPoint(renderer,x,y);
		}
	}
	SDL_RenderPresent(renderer);
	updateNeeded = false;
	debug();
}

void Mandelbrot::set_center(float x, float y){
	float dx = center.real() - left_center.real();
	center = {x,y};
	left_center = center;
	left_center -= dx;
}

void Mandelbrot::shift(int direction){
	float dx = abs(left_center.real() - center.real()) / h_res;
	complex<float> c;
	switch(direction){
		case UP:{
			c = {0,-dx};
			break;
		}
		case DOWN:{
			c = {0,dx};
			break;
		}
		case LEFT:{
			c = {-dx,0};
			break;
		}
		case RIGHT:{
			c = {dx,0};
			break;
		}
	}
	// Update new center
	center += c;
	left_center += c;
	updateNeeded = true;
}

void Mandelbrot::zoom(int direction){
	float dx = abs(left_center.real() - center.real()) / h_res;
	complex<float> c;
	switch(direction){
		case UP:{
			c = {-5*dx,0};
			break;
		}
		case DOWN:{
			c = {5*dx,0};
			break;
		}
	}
	left_center += c;
	updateNeeded = true;
}

void Mandelbrot::resol(int direction){
	switch(direction){
		case UP:{
			iterations += 10;
			break;
		}
		case DOWN:{
			iterations -= 10;
			break;
		}
	}
	updateNeeded = true;	
}

void Mandelbrot::debug(){
	cout << "Center:" << center << endl;
	cout << "Left:" << left_center << endl;
	cout << "Iterations:" << iterations << endl;
	cout << "*******" << endl;
}

//######################### Class Mandelbrot ##########################

void loop(Mandelbrot&mandel){
	SDL_Event ev;
    for(;;){
    	while(SDL_PollEvent(&ev)){
    		switch(ev.type){
				case SDL_QUIT:{
					return;
				}
				case SDL_KEYDOWN:{
					switch(ev.key.keysym.sym){
						case SDLK_ESCAPE: return;
						// Zoom
						case 'q':{mandel.zoom(UP);break;}
						case 'e':{mandel.zoom(DOWN);break;}
						// Movement
						case 'w':{mandel.shift(DOWN);break;}
						case 's':{mandel.shift(UP);break;}
						case 'a':{mandel.shift(LEFT);break;}
						case 'd':{mandel.shift(RIGHT);break;}
						// Resolution
						case 'x':{mandel.resol(UP);break;}
						case 'y':{mandel.resol(DOWN);break;}
						default:{
							break;
						}
					}
					break;
				}
				default: break;
    		}
    	}
		mandel.render();
    	SDL_Delay(10);
    }
}

int main(int argc, char** argv) {
    int Width=500,Height=500;
    Mandelbrot mandel(Width,Height);
    loop(mandel);

    SDL_Quit();
    return 0;
}