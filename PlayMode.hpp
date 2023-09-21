#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} button_easy, button_hard, button_down;

	float xangle;
	float yangle;
	const glm::quat default_rotation = glm::angleAxis(glm::radians(270.0f), glm::vec3(0.0f, 0.0f, 1.0f)) 
	                              	 * glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;
	std::vector<Scene::Transform*> cubea;
	std::vector<Scene::Transform*> cubeb;
	std::vector<Scene::Transform*> cubec;
	std::vector<Scene::Transform*> cubed;
	std::vector<Scene::Transform*> cubee;
	std::vector<Scene::Transform*> cubef;
	Scene::Transform* frame;
	Scene::Transform* sphere;

	const float spawn_w = 7.0f;
	const float spawn_h = 4.0f;
	const float hit_distance = 25.0f;

	const float time_int = 60.0f/80.0f/6.0f;
	const float dist_int = 1.0f;
	const float velocity = dist_int/time_int;

	struct Block {
		glm::vec3 pos; 
		bool active = true; 
		Scene::Transform* trans;
	};
	std::vector<Block> blocks;

	struct Track {
		std::vector<int> times;
		std::vector<int> news;
	} easy, hard; 
	int difficulty = 0;

	int gamestate = 0;

	float score = 0.0f;
	glm::vec3 player_pos;

	//music coming from the tip of the leg (as a demonstration):
	std::shared_ptr< Sound::PlayingSample > leg_tip_loop;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
