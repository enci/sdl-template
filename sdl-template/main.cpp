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

SDL_Renderer* renderer = nullptr;

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

void RenderLine(vec2 a, vec2 b)
{
	SDL_RenderDrawLine(renderer, int(a.x), int(a.y), int(b.x), int(b.y));
}

void RenderBox(vec2 min, vec2 max)
{
	const vec2 A(min.x, min.y);
	const vec2 B(max.x, min.y);
	const vec2 D(min.x, max.y);
	const vec2 C(max.x, max.y);
	RenderLine(A, B);
	RenderLine(B, C);
	RenderLine(C, D);
	RenderLine(D, A);
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
	bool overlap(const vec2& vec);
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

bool aabb::overlap(const vec2& vec)
{
	return
		(min.x <= vec.x) && (min.y <= vec.y) &&
		(max.x >= vec.x) && (max.y >= vec.y);
}

class bvh
{
public:
	bvh(std::vector<object_2d>& bodies);
	std::vector<object_2d*> get_overlap(vec2 pos);
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

	void get_overlap(vec2 pos, std::vector<object_2d*>& overlap, bvh_node* node);

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

std::vector<object_2d*> bvh::get_overlap(vec2 pos)
{
	std::vector<object_2d*> overlap;
	get_overlap(pos, overlap, _root);
	return overlap;
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
	RenderBox(node->bounding_box.min, node->bounding_box.max);
	
	if (node->left)
		draw(node->left);
	if (node->right)
		draw(node->right);
}

void bvh::get_overlap(vec2 pos, std::vector<object_2d*>& overlap, bvh_node* node)
{
	if(node->bounding_box.overlap(pos))
	{
		RenderBox(node->bounding_box.min, node->bounding_box.max);
		if (node->left)
			get_overlap(pos, overlap, node->left);		
		if (node->right)
			get_overlap(pos, overlap, node->right);
		if (node->object)
			overlap.push_back(node->object);
	}
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

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_RenderClear(renderer);

	std::vector<object_2d> objects;
	for(int i = 0; i < 24; i++)
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

	bvh bvh_tree(objects);
	
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
		//auto overlap = GetOverlapping(objects, mouse);
		auto overlap = bvh_tree.get_overlap(mouse);
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
