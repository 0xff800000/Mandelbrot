//============================================================================
// Name        : mandelbrot.cpp
// Author      : D01000100
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
//#include <conio.h>

using namespace std;

//######################### Class Mandelbrot ##########################

class Mandelbrot{
public:
	Mandelbrot(int,int);
	void render();

private:
	int h_res;
	int v_res;
	int iterations;
	complex<float> center;
	complex<float> left_center;
	vector<vector<int>> data;
	SDL_Renderer*renderer;
	int compute_number(complex<float>);
}

Mandelbrot::Mandelbrot(int h, int v){
	h_res = h;
	v_res = v;
	iterations = 50;

	// Create window
	SDL_Window* window = SDL_CreateWindow
	(
		"Mandelbrot set", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width,
		height,
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
    left_center = {-1,0};

}

int Mandelbrot::compute_number(complex<float> c){
	complex<float> z (0,0);
	for(int i=0; i<iterations; i++){
		if(z.abs() > 2) return i;
		z = z*z + c;
	}
	return iterations;
}

void Mandelbrot::render(){
	// Get upper left corner coordinate
	float dx = abs(left_center.real() - center.real()) / h_res;
//TBD	float dy = abs();

	// Compute the image
	for(int y=0; y<v_res; y++){
		for(int x=0; x<h_res; x++){
			
		}
	}
}

void drawWorld(SDL_Renderer*renderer,GameOfLife&ant,int w,int h){
	for(int y=0;y<ant.Height;y++){
		for(int x=0;x<ant.Width;x++){
			if(ant.getWorld(x,y)){
				SDL_SetRenderDrawColor(renderer,255,255,255,255);
			}
			else{
				SDL_SetRenderDrawColor(renderer,0,0,0,0);
			}
			SDL_Rect rect;
			rect.x=x*w/ant.Width;
			rect.y=y*h/ant.Height;
			rect.w=w/ant.Width;
			rect.h=h/ant.Height;
			SDL_RenderFillRect(renderer,&rect);
		}
	}

	SDL_RenderPresent(renderer);
}

void modifyWorld(SDL_Renderer*renderer,GameOfLife&ant,int w,int h,int mouseX,int mouseY,bool state){
	ant.setWorld(mouseX*ant.Width/w,mouseY*ant.Height/h,state);
	//cout<<"Mouse:"<<mouseX*ant.Width/w<<","<<mouseY*ant.Height/h<<endl;
	if(state){
		SDL_SetRenderDrawColor(renderer,255,255,255,255);
	}
	else{
		SDL_SetRenderDrawColor(renderer,0,0,0,0);
	}
	SDL_Rect rect;
	rect.x=(mouseX*ant.Width/w)*w/ant.Width;
	rect.y=(mouseY*ant.Height/h)*h/ant.Height;
	/*
	rect.x=mouseX*w/ant.Width;
	rect.y=mouseY*h/ant.Height;
	*/
	rect.w=w/ant.Width;
	rect.h=h/ant.Height;

	SDL_RenderFillRect(renderer,&rect);

	SDL_RenderPresent(renderer);
}

void loop(SDL_Renderer*renderer,int w,int h,int gridW,int gridH){
	GameOfLife ant(gridW,gridH);
	SDL_Event ev;
	bool autoStep=false;
	bool clicked=false;
	bool state=false;
	int delayVal=0;
	int timer=0;
    for(;;){

    	if(delayVal<0)delayVal=0;

    	if(autoStep&&(timer++>=delayVal)){
    		timer=0;
    		ant.step();
    		drawWorld(renderer,ant,w,h);
    		//SDL_Delay(delayVal);
    	}

    	while(SDL_PollEvent(&ev)){
    		switch(ev.type){
				case SDL_QUIT:{
					return;
				}
				case SDL_KEYDOWN:{
					switch(ev.key.keysym.sym){
						case SDLK_ESCAPE: return;
						//Camera rotation
						case 'q':return;
						case 'w':{autoStep=!autoStep;break;}
						case 'e':{autoStep=false;ant.step();drawWorld(renderer,ant,w,h);break;}
						case 'a':{delayVal=(delayVal-500000<0)?0:delayVal-500000;break;}
						case 's':{delayVal+=500000;break;}
						case 'x':{autoStep=false;ant.killAll();drawWorld(renderer,ant,w,h);break;}
						default:{
							break;
						}
					}
					break;
				}
				case SDL_MOUSEBUTTONDOWN:{
					state=(ev.button.button==SDL_BUTTON_LEFT);
					int mouseX=ev.button.x;
					int mouseY=ev.button.y;
					modifyWorld(renderer,ant,w,h,(int)mouseX,(int)mouseY,state);
					clicked=true;
					break;
				}
				case SDL_MOUSEBUTTONUP:{
					state=(ev.button.button==SDL_BUTTON_LEFT);
					clicked=false;
					break;
				}
				case SDL_MOUSEMOTION:{
					if(clicked){
						int mouseX=ev.motion.x;
						int mouseY=ev.motion.y;
						modifyWorld(renderer,ant,w,h,(int)mouseX,(int)mouseY,state);
					}
					break;
				}
				default: break;
    		}
    	}
    }
}

int main(int argc, char** argv) {
	//freopen("CON", "w", stdout);
	// Window
	const int width=500,height=500;
	

    //Black screen
    SDL_SetRenderDrawColor(renderer,0,0,0,0);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    SDL_SetRenderDrawColor(renderer,255,255,255,255);

    int Width=100,Height=100;
    if(argc == 3){
    	Width = atoi(argv[1]);
    	Height = atoi(argv[2]);
    }
    else if(argc == 2){
    	Width = Height = atoi(argv[1]);
    }
    // cout<<"###### Game of Life ######"<<endl;
    // cout<<"Controls: q:quit, w:start/stop, e:step, x:kill all\na:slow down, s:speed up, mouse left:set, mouse right:reset"<<endl;
    // cout<<"Enter width (max:500):";cin>>Width;
    // cout<<"Enter height (max:500):";cin>>Height;

    if(Width>500)Width=500;
    if(Height>500)Height=500;

    loop(renderer,width,height,Width,Height);

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
