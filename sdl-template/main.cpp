#include <SDL.h>
#include <cstdlib>
#include <glm.hpp>
#include <vector>
#include "source/defines.h"

const float pi = 3.14159265359f;
const int w = 1280;
const int h = 720;

using namespace glm;
using namespace std;

struct object_2d
{
	vec2 position;
	float radius;
	SDL_Color color;
};

void RenderDrawCircle(SDL_Renderer * renderer,
	vec2 center,
	float radius)
{
	const int n = 12;
	float t = 0.0f;
	const float dt = 2.0f * pi / float(n);
	for(int i = 0; i < n; i++)
	{
		SDL_RenderDrawLine(renderer,
			int(center.x + cos(t) * radius),
			int(center.y + sin(t) * radius),
			int(center.x + cos(t+dt) * radius),
			int(center.y + sin(t+dt) * radius));
		t += dt;
	}
}

std::vector<object_2d*> GetOverlapping(std::vector<object_2d>& objects, vec2& point)
{
	std::vector<object_2d*> overlap;
	for(auto& o : objects)
	{
		vec2 d = point - o.position;
		if (length(d) < o.radius)
			overlap.push_back(&o);
	}
	return overlap;
}

int main(int argc, char** argv)
{
	SDL_Window* window = NULL;
	window = SDL_CreateWindow
	(
		"Template Window", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		w,
		h,
		SDL_WINDOW_SHOWN
	);

	SDL_Renderer* renderer = NULL;
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_RenderClear(renderer);

	std::vector<object_2d> objects;
	for(int i = 0; i < 255; i++)
	{
		const Uint8 r = Uint8(rand_in_range(128, 255));
		const Uint8 g = Uint8(rand_in_range(128, 255));
		const Uint8 b = Uint8(rand_in_range(128, 255));

		object_2d ob =
		{
			{
				rand_in_range(0.0f, float(w)),
				rand_in_range(0.0f, float(h))
			},
			rand_in_range(5.0f, 20.0f),
			{r,g,b, 255}
		};
		objects.push_back(ob);
	}

	SDL_Event event;
	bool quit = false;

	while (!quit)
	{
		while (SDL_PollEvent(&event))
			if (event.type == SDL_QUIT)
				quit = true;

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		for (auto& o : objects)
		{
			SDL_SetRenderDrawColor(renderer, o.color.r, o.color.g, o.color.b, 255);
			RenderDrawCircle(renderer, o.position, o.radius);
		}

		int x;
		int y;
		SDL_GetMouseState(&x, &y);
		auto mouse = vec2(x, y);

		// Measure the time for this call and make it faster. You can output the value in the title of the window
		auto overlap = GetOverlapping(objects, mouse);
		
		for (auto& o : overlap)
		{
			SDL_SetRenderDrawColor(renderer, o->color.r, o->color.g, o->color.b, 255);
			RenderDrawCircle(renderer, o->position, o->radius+0.8f);
			RenderDrawCircle(renderer, o->position, o->radius-0.8f);
			RenderDrawCircle(renderer, o->position, o->radius+1.6f);
			RenderDrawCircle(renderer, o->position, o->radius-1.6f);
		}
		
		RenderDrawCircle(renderer, mouse,6.0f);
		
		SDL_RenderPresent(renderer);
		SDL_Delay(1000 / 30);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return EXIT_SUCCESS;
}
