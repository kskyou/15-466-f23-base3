#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <queue>
#include <sstream>
#include <fstream>

GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("real.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("real.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("beep.wav"));
});

PlayMode::PlayMode() : scene(*hexapod_scene) {
	//get pointers to leg for convenience:

	srand(time(NULL));

	// https://stackoverflow.com/questions/1894886/parsing-a-comma-delimited-stdstring
	{
		std::string line;
		std::ifstream f(data_path("easy"));
		std::getline(f, line);
		std::stringstream ss(line);
		for (int i; ss >> i;) {
			easy.times.push_back(i);    
			if (ss.peek() == ',')
				ss.ignore();
		}
		std::getline(f, line);
		ss = std::stringstream (line);
		for (int i; ss >> i;) {
			easy.news.push_back(i);    
			if (ss.peek() == ',')
				ss.ignore();
		}
	}
	{
		std::string line;
		std::ifstream f(data_path("hard"));
		std::getline(f, line);
		std::stringstream ss(line);
		for (int i; ss >> i;) {
			hard.times.push_back(i);    
			if (ss.peek() == ',')
				ss.ignore();
		}
		std::getline(f, line);
		ss = std::stringstream (line);
		for (int i; ss >> i;) {
			hard.news.push_back(i);    
			if (ss.peek() == ',')
				ss.ignore();
		}
	}

	for (auto &transform : scene.transforms) {

		if (transform.name.substr(0, 5) == "Frame"){
			frame = &transform;
			frame->position = glm::vec3(0.0f, 0.0f, 0.0f);
		} else if (transform.name.substr(0, 5) == "Spher"){
			sphere = &transform;
		} else {
			transform.position = glm::vec3(-hit_distance -10.0f, 0.0f, 0.0f);
				if (transform.name.substr(0, 5) == "CubeA"){
				cubea.push_back(&transform);
			} else if (transform.name.substr(0, 5) == "CubeB"){
				cubeb.push_back(&transform);
			} else if (transform.name.substr(0, 5) == "CubeC"){
				cubec.push_back(&transform);
			} else if (transform.name.substr(0, 5) == "CubeD"){
				cubed.push_back(&transform);
			} else if (transform.name.substr(0, 5) == "CubeE"){
				cubee.push_back(&transform);
			} else if (transform.name.substr(0, 5) == "CubeF"){
				cubef.push_back(&transform);
			}
		}
	}

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	camera->transform->rotation = default_rotation;
	camera->transform->position = glm::vec3(-hit_distance, 0.0f, 0.0f);
	
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_e) {
			button_easy.downs += 1;
			button_easy.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_h) {
			button_hard.downs += 1;
			button_hard.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_z) {
			button_down.downs += 1;
			button_down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_x) {
			button_down.downs += 1;
			button_down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_e) {
			button_easy.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_h) {
			button_hard.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_z) {
			button_down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_x) {
			button_down.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			xangle = xangle - motion.x;
			yangle = yangle + motion.y;
			const float xb = 0.3f;
			const float yb = 0.18f;
			if (xangle > xb){
				xangle = xb;
			} else if (xangle < -xb){
				xangle = -xb;
			}
			if (yangle > yb){
				yangle = yb;
			} else if (yangle < -yb){
				yangle = -yb;
			}

			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	auto randf = [] () {return static_cast<float> (rand()) / (static_cast<float> (RAND_MAX));};

	if (gamestate == 0){
	
		if (button_easy.pressed || button_hard.pressed){
			gamestate = 1;
			std::vector<int> times = button_easy.pressed ? easy.times : hard.times;
			std::vector<int> news = button_easy.pressed ? easy.news : hard.news;
			float d1 = button_easy.pressed ? 3.0f : 5.0f;
			float d2 = button_easy.pressed ? 4.0f : 7.0f;

			std::queue<int> diff;
			int count = 0;
			for (uint32_t i=1; i<news.size(); i++){
				if (news[i]){
					diff.push(count);
					count = 0;
				} else {
					count ++;
				}
			}
			diff.push(count);

			count = 0;
			int currdiff = 0;
			int ca = 0, cb = 0, cc = 0, cd = 0, ce = 0, cf = 0;
			float currx = 0.0f;
			glm::vec2 curryz = glm::vec2(0.0f), nextyz = glm::vec2(0.0f); 
			for (uint32_t i=0; i<news.size(); i++){
				currx += times[i] * dist_int; 
				if (news[i]){
					currdiff = diff.front();
					diff.pop();
					count = 0;
				} else {
					count += 1;
				}
				Block newblock;

				if (count == 0){
					glm::vec2 prevyz = nextyz; 
					curryz = glm::vec2(-spawn_w + 2.0f*spawn_w * randf(), -spawn_h + 2.0f*spawn_h * randf());
					nextyz = glm::vec2(-spawn_w + 2.0f*spawn_w * randf(), -spawn_h + 2.0f*spawn_h * randf());
					while (glm::length(curryz - nextyz) > 3.0f * (currdiff+1) || glm::length(curryz - prevyz) > 4.0f){
						curryz = glm::vec2(-spawn_w + 2.0f*spawn_w * randf(), -spawn_h + 2.0f*spawn_h * randf());
						nextyz = glm::vec2(-spawn_w + 2.0f*spawn_w * randf(), -spawn_h + 2.0f*spawn_h * randf());
					}
					newblock.trans = cubea[ca];
					ca++;
				} else if (count == 1){
					newblock.trans = cubeb[cb];
					cb++;
				} else if (count == 2){
					newblock.trans = cubec[cc];
					cc++;
				} else if (count == 3){
					newblock.trans = cubed[cd];
					cd++;
				} else if (count == 4){
					newblock.trans = cubee[ce];
					ce++;
				} else if (count == 5){
					newblock.trans = cubef[cf];
					cf++;
				}
				float scalec = float(currdiff - count) / float(currdiff), scalen = float(count) / float(currdiff);
				newblock.pos = glm::vec3(currx, curryz.x * scalec + nextyz.x * scalen, curryz.y * scalec + nextyz.y * scalen); 
				blocks.push_back(newblock);
			}
			leg_tip_loop = Sound::play(*dusty_floor_sample, 1.0f, 0.0f);
		}

	} else {
		for (uint32_t i=0; i<blocks.size(); i++){
			if (blocks[i].active){
				blocks[i].trans->position = blocks[i].pos;
				blocks[i].pos.x -= velocity * elapsed; 

				if (blocks[i].pos.x < -dist_int/2.0f) {
					blocks[i].active = false;
					blocks[i].trans->position = glm::vec3(-hit_distance -10.0f, 0.0f, 0.0f);
					// Miss
				} else if (blocks[i].pos.x < dist_int/2.0f){
					if (button_down.pressed && abs(player_pos.y-blocks[i].pos.y) < 0.5f && abs(player_pos.z-blocks[i].pos.z) < 0.5f){
						score += 100 * exp(-2*abs(player_pos.x-blocks[i].pos.x));
						blocks[i].active = false;
						blocks[i].trans->position = glm::vec3(-hit_distance -10.0f, 0.0f, 0.0f);
					}
					// Hit
				}
			} 
		}

	}

	//move camera:
	{
		camera->transform->rotation = glm::angleAxis(yangle, glm::vec3(sin(xangle),-cos(xangle),0.0f))
									* glm::angleAxis(xangle, glm::vec3(0.0f, 0.0f, 1.0f)) 
									* default_rotation;

		glm::vec3 dir = glm::vec3(hit_distance*cos(xangle)*cos(yangle), hit_distance*sin(xangle)*cos(yangle), hit_distance*sin(yangle));
		player_pos = dir * (hit_distance / dir.x) - glm::vec3(hit_distance, 0.0f, 0.0f); 
		sphere->position = player_pos;
	}

	//reset button press counters:
	button_down.downs = 0;
	button_easy.downs = 0;
	button_hard.downs = 0;

}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		float ofs = 2.0f / drawable_size.y;
		if (gamestate == 1){
			lines.draw_text("Click with z or x. Your score is " + std::to_string(int(score)),
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		} else {
			lines.draw_text("Select difficulty, e for easy or h for hard.",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		}

	}
	GL_ERRORS();
}


