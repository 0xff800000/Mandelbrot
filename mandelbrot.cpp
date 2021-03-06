//============================================================================
// Name        : mandelbrot.cpp
// Author      : D01000100
// Version     :
// Copyright   : Your copyright notice
// Description : Representation of mandelbrot & julia set
//============================================================================

#include <iostream>
#include <vector>
#include <complex>
#include <algorithm>
#include <SDL2/SDL.h>
#include <sys/time.h>

using namespace std;

//######################### Class Mandelbrot ##########################
enum eDirections { UP,DOWN,LEFT,RIGHT };
enum eColorMode { MONOCHROME, COLOR};
enum eMode { MANDELBROT, JULIA};

class Mandelbrot{
public:
	Mandelbrot(int,int);
	void render();
	void set_center(int,int);
	void shift(int);
	void c_move(int);
	void zoom(int);
	void resol(int);
	void toggle_cursor();
	void debug();
	void cycle_color();
	void change_mode();

private:
	int h_res;
	int v_res;
	int iterations;
	float zoom_stat;
	float max_zoom;
	complex<float> center;
	complex<float> left_center;
	complex<float> start_number;
	vector< vector<int> > data;
	SDL_Renderer*renderer;
	bool updateNeeded;
	bool cursor;
	int color_mode;
	int mode;
	struct timespec frame_period;
	int compute_number(complex<float>);
	void min_max(int&,int&);
	int map_color(int,int);
	void sweep_color(int,int&,int&,int&b);

};

Mandelbrot::Mandelbrot(int v, int h){
	h_res = h;
	v_res = v;
	iterations = 30;
	cursor = false;
	color_mode = MONOCHROME;
	mode = MANDELBROT;

	// Create window
	SDL_Window* window = SDL_CreateWindow
	(
		"Mandelbrot set", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		h_res,
		v_res,
		SDL_WINDOW_SHOWN
	);
    renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);

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
    start_number = {0,0};
    updateNeeded = true;

    // Zoom values
    zoom_stat = 1/(center.real() - left_center.real());
    max_zoom = 278339;
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

int Mandelbrot::map_color(int val, int max){
	return (int)(val * 255) / (iterations); // Absolute
}

void Mandelbrot::sweep_color(int val, int&r,int&g, int&b){
	r = 0;g = 0;b = 0;
	if(val == iterations)return;
	float wavelength = val*(750-380)/iterations+380;
	float attenuation;

	if(wavelength >= 380 && wavelength <=440){
		attenuation = 0.3 + 0.7 * (wavelength - 380) / (440 - 380);
		r = 0xff * ((-(wavelength-440)/(440-380)) * attenuation) ;
		b = 0xff * attenuation;
	}
	else if(wavelength >= 440 && wavelength <= 490){
		g = 0xff * ((wavelength - 440)/(490 - 440));
		b = 0xff;
	}
	else if(wavelength >= 490 && wavelength <= 510){
		g = 0xff;
		b = 0xff * (-(wavelength - 510) / (510 - 490));
	}
	else if(wavelength >= 510 && wavelength <= 580){
		r = 0xff * ((wavelength - 510) / (580 - 510));
		g = 0xff;
	}
	else if(wavelength >= 580 && wavelength <= 645){
		r = 0xff;
		g = 0xff * (-(wavelength - 645) / (645 - 580));
	}
	else if(wavelength >= 645 && wavelength <= 750){
		attenuation = 0.3 + 0.7 * (750 - wavelength) / (750 - 645);
		r = 0xff * attenuation;
	}
}

int Mandelbrot::compute_number(complex<float> c){
	complex<float> z = start_number;
	if(mode == JULIA){
		complex<float> tmp = c;
		c = z;
		z = tmp;
	}
	for(int i=0; i<iterations; i++){
		if(abs(z) >= 2) return i;
		z = z*z + c;
	}
	return iterations;
}

void Mandelbrot::render(){
	if(!updateNeeded) return;
	struct timespec ts,te;
	// clock_getres(CLOCK_REALTIME, &ts);
	clock_gettime(CLOCK_REALTIME, &ts);
	// Get upper left corner coordinate
	float dx = abs(left_center.real() - center.real()) / h_res * 2;
	float dy = dx;
	complex<float> up_left (left_center.real(),left_center.imag()+(v_res/2)*dy);

	// Compute the image
	#pragma omp parallel for schedule(dynamic)
	for(int y=0; y<v_res; y++){
		for(int x=0; x<h_res; x++){
			complex<float> pt;
			pt = {up_left.real() + x*dx, up_left.imag() - y*dy};
			data[y][x] = compute_number(pt);
		}
	}

	// Draw SDL monochrome image
	int min,max;
	min_max(min,max);
	for(int y=0; y<v_res; y++){
		for(int x=0; x<h_res; x++){
			int color = map_color(data[y][x], max);
			if(color_mode == MONOCHROME){
				SDL_SetRenderDrawColor(renderer,color,color,color,color);
			}
			else if(color_mode == COLOR){
				int r,g,b;
				sweep_color(data[y][x],r,g,b);
				SDL_SetRenderDrawColor(renderer,r,g,b,0);
			}
			SDL_RenderDrawPoint(renderer,x,y);
		}
	}

	// Draw cursor
	if(cursor){
		int size = 6;
		SDL_SetRenderDrawColor(renderer,0,0xff,0,0);
		SDL_RenderDrawLine(renderer,h_res/2,v_res/2-size,h_res/2,v_res/2+size);
		SDL_RenderDrawLine(renderer,h_res/2-size,v_res/2,h_res/2+size,v_res/2);
	}

	// Render frame
	SDL_RenderPresent(renderer);
	updateNeeded = false;
	// clock_getres(CLOCK_REALTIME, &te);
	clock_gettime(CLOCK_REALTIME, &te);
	frame_period.tv_sec = te.tv_sec - ts.tv_sec;
	frame_period.tv_nsec = te.tv_nsec - ts.tv_nsec;
	debug();
}

void Mandelbrot::set_center(int x, int y){
	// Get upper left corner
	float dx = abs(left_center.real() - center.real()) / h_res * 2;
	float dy = dx;
	complex<float> up_left (left_center.real(),left_center.imag()+(v_res/2)*dy);
	// Keep difference
	float diff = center.real() - left_center.real();
	// Set new center
	complex<float> click = {-dx*x,dy*y};
	click = up_left - click;
	cout << "Click at :" << click << endl;
	center = click;
	left_center = center - diff;
	updateNeeded = true;
}

void Mandelbrot::shift(int direction){
	float dx = 5*abs(left_center.real() - center.real()) / h_res;
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

void Mandelbrot::c_move(int direction){
	float dx = 5*abs(left_center.real() - center.real()) / h_res;
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
	// Update new complex number
	start_number += c;
	updateNeeded = true;
}

void Mandelbrot::zoom(int direction){
	float dx = abs(left_center.real() - center.real()) / h_res;
	bool zoom_max = (zoom_stat >= max_zoom)?true:false;

	complex<float> c;
	switch(direction){
		case UP:{
			c = {-10*dx,0};
			break;
		}
		case DOWN:{
			if(zoom_max)return;
			c = {10*dx,0};
			break;
		}
	}
	left_center += c;
	zoom_stat = 1/(center.real() - left_center.real());
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
			if(iterations <= 10) iterations = 10;
			break;
		}
	}
	updateNeeded = true;	
}

void Mandelbrot::toggle_cursor(){
	cursor = !cursor;
	updateNeeded = true;
}

void Mandelbrot::cycle_color(){
	if(color_mode == MONOCHROME){
		color_mode = COLOR;
	}
	else{
		color_mode = MONOCHROME;
	}
	updateNeeded = true;
}

void Mandelbrot::change_mode(){
	if(mode == MANDELBROT){
		mode = JULIA;
	}
	else{
		mode = MANDELBROT;
	}
	updateNeeded = true;
}

void Mandelbrot::debug(){
	cout << "Center:" << center << " ; val:" << data[v_res/2][h_res/2] << endl;
	cout << "Left:" << left_center << endl;
	cout << "Zoom status:" << zoom_stat << endl;
	cout << "Complex number:" << start_number << endl;
	cout << "Iterations:" << iterations << endl;
	cout << "Render time:" << (float)(frame_period.tv_sec * 1e9 + frame_period.tv_nsec) / 1e9 << endl;
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
						// Cursor
						case 'c':{mandel.toggle_cursor();break;}
						// Change complex number
						case 'k':{mandel.c_move(DOWN);break;}
						case 'i':{mandel.c_move(UP);break;}
						case 'j':{mandel.c_move(LEFT);break;}
						case 'l':{mandel.c_move(RIGHT);break;}
						// Color mode
						case 'v':{mandel.cycle_color();break;}
						// Change mode
						case 'm':{mandel.change_mode();break;}
						default:{
							break;
						}
					}
					break;
				case SDL_MOUSEBUTTONDOWN:{
					cout << "Click detected at :" << ev.button.x << "," << ev.button.y << endl;
					mandel.set_center(ev.button.x,ev.button.y);
				}
				}
				default: break;
    		}
    	}
		mandel.render();
    	SDL_Delay(10);
    }
}

int main(int argc, char** argv) {
    int Width=400,Height=400;
	if(argc == 3){
		Width = atoi(argv[2]);
		Height = atoi(argv[1]);
	}
    Mandelbrot mandel(Width,Height);
    loop(mandel);

    SDL_Quit();
    return 0;
}
