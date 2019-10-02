#include <SDL.h>
#include <cstdlib>
#include <glm.hpp>
#include <vector>
#include "source/defines.h"
#include <chrono>
#include <string>
#include <algorithm>

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

void RenderBox(vec2 min, vec2 max)
{
	vec2 A(min.x, min.y);
	vec2 B(max.x, min.y);
	vec2 D(Min.x - offset, 0.0f, Max.y + offset);
	vec2 C(Max.x + offset, 0.0f, Max.y + offset);
	gDebugRenderer.AddLine(DebugRenderer::PHYSICS, A, B, color);
	gDebugRenderer.AddLine(DebugRenderer::PHYSICS, B, C, color);
	gDebugRenderer.AddLine(DebugRenderer::PHYSICS, C, D, color);
	gDebugRenderer.AddLine(DebugRenderer::PHYSICS, D, A, color);
	const int n = 12;
	float t = 0.0f;
	const float dt = 2.0f * pi / float(n);
	for (int i = 0; i < n; i++)
	{
		SDL_RenderDrawLine(renderer,
			int(center.x + cos(t) * radius),
			int(center.y + sin(t) * radius),
			int(center.x + cos(t + dt) * radius),
			int(center.y + sin(t + dt) * radius));
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

struct aabb
{
	vec2 min;
	vec2 max;

	aabb() : min(FLT_MAX, FLT_MAX), max(-FLT_MAX, -FLT_MAX) {}
	aabb(vec2 min, vec2 max) : min(min), max(max) {}
	void add(const object_2d& object);
	void add(const aabb& aabb);
};

void aabb::add(const object_2d& object)
{
	vec2 ex(object.radius, object.radius);
	aabb object_bb;
	object_bb.min = object.position - ex;
	object_bb.max = object.position + ex;
	add(object_bb);
}

void aabb::add(const aabb& aabb)
{
	if (aabb.max.x > max.x)
		max.x = aabb.max.x;
	if (aabb.min.x < min.x)
		min.x = aabb.min.x;
	
	if (aabb.max.y > max.y)
		max.y = aabb.max.y;
	if (aabb.min.y < min.y)
		min.y = aabb.min.y;
}

class bvh
{
public:
	bvh(std::vector<object_2d>& bodies);
	// void get_broad_phase(const object_2d& body, std::vector<object_2d*>& broadPhase);
	void draw();
private:	
	struct bvh_node
	{
		object_2d* object		= nullptr;
		aabb bounding_box;		
		bvh_node* left			= nullptr;
		bvh_node* right			= nullptr;
	};
	void recurse(size_t from, size_t to, bvh_node* parent, int depth);
	void draw(bvh_node* node);

	//void get_broad_phase(const object_2d& body, std::vector<object_2d*>& broadPhase, int parent);

	bvh_node*				_root		= nullptr;
	std::vector<object_2d*>	_bodies;
};

bvh::bvh(std::vector<object_2d>& bodies)
{
	_bodies.reserve(bodies.size());
	for (auto& b : bodies)
		_bodies.push_back(&b);

	_root = new bvh_node();
	recurse(0, _bodies.size() - 1, _root, 0);
}

void bvh::draw()
{
	draw(_root);
}

void bvh::recurse(size_t from, size_t to, bvh_node* parent, int depth)
{
	if (from >= to)
	{
		parent->object = _bodies[from];
		parent->bounding_box.add(*parent->object);
	}
	else
	{
		static auto sort_x = [](object_2d* body0, object_2d* body1)
		{ return body0->position.x > body1->position.x; };

		static auto sort_y = [](object_2d* body0, object_2d* body1)
		{ return body0->position.y > body1->position.y; };

		if (depth % 2)
			sort(_bodies.begin() + from, _bodies.begin() + to, sort_x);
		else
			sort(_bodies.begin() + from, _bodies.begin() + to, sort_y);

		parent->left = new bvh_node();
		parent->right = new bvh_node();
		const auto len = to - from;
		recurse(from, from + len / 2, parent->left, depth + 1);
		recurse(from + len / 2 + 1, to, parent->right, depth + 1);

		parent->bounding_box.add(parent->left->bounding_box);
		parent->bounding_box.add(parent->right->bounding_box);
	}
}

void bvh::draw(bvh_node* node)
{
	node->bounding_box;

	Vector3 A(Min.x - offset, 0.0f, Min.y - offset);
	Vector3 B(Max.x + offset, 0.0f, Min.y - offset);
	Vector3 D(Min.x - offset, 0.0f, Max.y + offset);
	Vector3 C(Max.x + offset, 0.0f, Max.y + offset);
	gDebugRenderer.AddLine(DebugRenderer::PHYSICS, A, B, color);
	gDebugRenderer.AddLine(DebugRenderer::PHYSICS, B, C, color);
	gDebugRenderer.AddLine(DebugRenderer::PHYSICS, C, D, color);
	gDebugRenderer.AddLine(DebugRenderer::PHYSICS, D, A, color);

	if (node->left)
		draw(node->left);
	if (node->right)
		draw(node->right);
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
	for(int i = 0; i < 4000; i++)
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
		auto t1 = std::chrono::high_resolution_clock::now();
		auto overlap = GetOverlapping(objects, mouse);
		auto t2 = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

		string name = "Cast time: " + std::to_string(duration);
		SDL_SetWindowTitle(window, name.c_str());
		
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
