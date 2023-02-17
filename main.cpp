#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL_timer.h>
#include <SDL_events.h>
#include <SDL_keyboard.h>
#include <iostream>
#include <deque>
#include <vector>
#include <Windows.h>
#include <stdio.h>
#include <chrono>
#include <algorithm>
#define nScreenHeight 512
#define nScreenWidth 512
using namespace std;
bool gameRunning = true;
auto tp1 = chrono::system_clock::now();
auto tp2 = chrono::system_clock::now();
int snakeSize = 1;
enum directions{ up, down, right, left };
void events(SDL_Window* window, SDL_Renderer* renderer) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			SDL_DestroyWindow(window);
			SDL_DestroyRenderer(renderer);
			gameRunning = false;
		default:
			break;
		}
	}
}
int main() {
	SDL_Rect head = { 256, 256, 16, 16 };
	SDL_Window* window = SDL_CreateWindow("Square-io", 100, 100, nScreenWidth, nScreenHeight, 0);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	int size = 1;
	vector<SDL_Rect> apples;
	vector<SDL_Rect> mines;
	srand(time(0));
	for (int i = 0; i < 100; i++) {
		apples.push_back({ rand() % 32 * 16, rand() % 32 * 16, 16, 16 });
	}
	for (int i = 0; i < 25; i++) {
		mines.push_back({ rand() % 32 * 16, rand() % 32 * 16, 16, 16 });
	}
	directions dir = directions::forward;
	bool bInit = false;
	using Clock = std::chrono::steady_clock;
	std::chrono::time_point<std::chrono::steady_clock> start;
	std::chrono::time_point<std::chrono::steady_clock> now;
	std::chrono::milliseconds duration;
	while (gameRunning) {
		now = Clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 1);
		SDL_RenderFillRect(renderer, &snakeHead);
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 1);
		//Check for collisions.
		if (apples.back().x < head.x + head.w &&
			apples.back().x + apples.back().w > head.x &&
			apples.back().y < head.y + head.h &&
			apples.back().y + apples.back().h > head.y) {
			apples.pop_back();
			head.w += 16;
			head.h += 16;
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 1);
		for (auto mine : mines) {
			SDL_RenderFillRect(renderer, &mine);
		}
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 1);
		SDL_RenderFillRect(renderer, &apples.back());
		for(auto mine : mines){
			if (mine.x < head.x + head.w &&
				mine.x + mine.w > head.x &&
				mine.y < head.y + head.h &&
				mine.y + mine.h > head.y) {
				if (MessageBox(NULL, L"You lost! Do you wanna restart the game?", L":(", MB_YESNO) == IDYES) {
					head.x = 256;
					head.y = 256;
					head.w = 16;
					head.h = 16;
					mines.clear();
					apples.clear();
					for (int i = 0; i < 100; i++) {
						apples.push_back({ rand() % 32 * 16, rand() % 32 * 16, 16, 16 });
					}
					for (int i = 0; i < 25; i++) {
						mines.push_back({ rand() % 32 * 16, rand() % 32 * 16, 16, 16 });
					}
				}
				else {
					gameRunning = false;
					SDL_DestroyRenderer(renderer);
					SDL_DestroyWindow(window);
				}
			}
		}
		//Render the board and other elements.
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
		for (int i = 0; i <= 512; i += 16) {
			SDL_RenderDrawLine(renderer, i, 0, i, 512);
		}
		for (int j = 0; j <= 512; j += 16) {
			SDL_RenderDrawLine(renderer, 0, j, 512, j);
		}
		SDL_SetRenderDrawColor(renderer, 128, 128, 128, 1);
		events(window, renderer);
		//Get input from the player.
		if (!bInit)
		{
			start = now = Clock::now();
			bInit = true;
		}
		if (duration.count() > 125) {
			start = Clock::now();
			if (GetAsyncKeyState(VK_UP)) {
				dir = directions::up;
			}
			if (GetAsyncKeyState(VK_DOWN)) {
				dir = directions::down;
			}
			if (GetAsyncKeyState(VK_LEFT)) {
				dir = directions::left;
			}
			if (GetAsyncKeyState(VK_RIGHT)) {
				dir = directions::right;
			}
			if (dir == directions::up) {
				head.y -= 16;
			}
			if (dir == directions::down) {
				head.y += 16;
			}
			if (dir == directions::right) {
				head.x += 16;
			}
			if (dir == directions::left) {
				head.x -= 16;
			}
		}
		SDL_RenderPresent(renderer);
		SDL_RenderClear(renderer);
	}
	return 0;
}