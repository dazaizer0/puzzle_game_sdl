#include "config.h"
#include "game_elements/skarabeusz.hpp"
#include "game_elements/connection.hpp"

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 768;
bool END = false;

class Scene {
protected:
    SDL_Renderer* renderer;
    SDL_Window* window;
    bool fullscreen;
    bool quit;

public:
    Scene(SDL_Renderer* rend, SDL_Window* win) : renderer(rend), window(win), quit(false) {}

    virtual void start() {}
    virtual void handleEvents(SDL_Event& event) {}
    virtual void update() {}
    virtual void render() {}

    void quitScene() {
        fullscreen = false;
        quit = true;
    }

    virtual void run() {
        start();

        while (!quit) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                handleEvents(event);
            }

            update();
            
            render();
        }
    }

    virtual ~Scene() {}
};

class Game : public Scene {
protected:
    render::texture bg = render::texture("res/bg.png", { 512, 384 }, { 1024, 768 }, 0.0f, renderer);
    render::texture flap = render::texture("res/x.png", { 512, 384 }, { 1024, 1024 }, 0.0f, renderer);
    render::texture key = render::texture("res/key.png", { 512, 384 }, { 200, 100 }, 0.0f, renderer);

    render::texture sm_btn = render::texture("res/sm_btn.png", { 100, 730 }, { 150, 40 }, 0.0f, renderer);
    render::texture restart_btn = render::texture("res/restart_btn.png", { 270, 730 }, { 150, 40 }, 0.0f, renderer);
    render::texture lost_image = render::texture("res/lost_image.png", { 512, 384 }, { 550, 150 }, 0.0f, renderer);

    Skarabeusz skarabeusze[26] = {
        Skarabeusz(renderer, {198, 168}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {334, 168}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {511, 130}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {690, 166}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {130, 246}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {265, 243}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {410, 248}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {510, 235}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {614, 248}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {758, 248}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {245, 393}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {440, 394}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {509, 330}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {582, 398}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {782, 393}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {275, 565}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {414, 562}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {509, 486}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {513, 613}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {612, 557}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {749, 562}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {880, 566}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {341, 642}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {682, 636}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {824, 642}, {80, 95}, 0.0f),
        Skarabeusz(renderer, {2000, 2000}, {1, 1}, 0.0f) // tester
    };
    size_t skarabeusze_size = sizeof(skarabeusze) / sizeof(skarabeusze[0]);

    std::vector<Line> connection_lines;
    std::vector<Line> connection_lines_light;
    std::vector<std::string> connections;

    mathy::vec2<int> mouse_pos = mathy::vec2<int>::ZERO();

    int actual_skarabeusz_index = 25;
    int previous_skarabeusz_index = 25;
    Uint8 mouse_state = 0;
    int move_nr = 0;

    bool first_move = true;
    bool mouse_left_down = false;
    bool can_move = true;
    bool finished = false;
    bool key_collected = false;

    //Mix_Music* music;
    Mix_Chunk* put_sound;
    Mix_Chunk* finish_sound;
    bool finish_played = false;
public:
    Game(SDL_Renderer* rend, SDL_Window* wind) : Scene(rend, wind) {}

    virtual void start() override {
        //music = Mix_LoadMUS("res/music.mp3");
        put_sound = Mix_LoadWAV("res/select.wav");
        finish_sound = Mix_LoadWAV("res/finish.wav");
        //if (!music) {
        //    printf("Mix_LoadMUS Error: %s\n", Mix_GetError());
       // }

        //if (Mix_PlayMusic(music, -1) == -1) {
        //    printf("Mix_PlayMusic Error: %s\n", Mix_GetError());
       // }

        flap.enabled = false;
        key.enabled = false;
        lost_image.enabled = false;

        skarabeusze[0].neighbours_indexes = { 4, 5 };
        skarabeusze[1].neighbours_indexes = { 5, 6 };
        skarabeusze[2].neighbours_indexes = { 6, 8 };
        skarabeusze[3].neighbours_indexes = { 8, 9 };
        skarabeusze[4].neighbours_indexes = { 0, 5 };
        skarabeusze[5].neighbours_indexes = { 0, 1, 4, 6, 10 };
        skarabeusze[6].neighbours_indexes = { 1, 2, 7, 11, 10, 5 };
        skarabeusze[7].neighbours_indexes = { 6, 8 };
        skarabeusze[8].neighbours_indexes = { 2, 3, 7, 9, 13, 14 };
        skarabeusze[9].neighbours_indexes = { 3, 8, 14 };
        skarabeusze[10].neighbours_indexes = { 5, 6, 11, 15 };
        skarabeusze[11].neighbours_indexes = { 10, 6, 12, 17, 16, 15 };
        skarabeusze[12].neighbours_indexes = { 11, 13 };
        skarabeusze[13].neighbours_indexes = { 12, 17, 19, 20, 14, 8 };
        skarabeusze[14].neighbours_indexes = { 13, 8, 9, 20 };
        skarabeusze[15].neighbours_indexes = { 10, 11, 16, 22 };
        skarabeusze[16].neighbours_indexes = { 15, 11, 18, 22 };
        skarabeusze[17].neighbours_indexes = { 11, 13 };
        skarabeusze[18].neighbours_indexes = { 16, 19 };
        skarabeusze[19].neighbours_indexes = { 18, 13, 20, 23 };
        skarabeusze[20].neighbours_indexes = { 19, 13, 14, 21, 24, 23 };
        skarabeusze[21].neighbours_indexes = { 20, 24 };
        skarabeusze[22].neighbours_indexes = { 15, 16 };
        skarabeusze[23].neighbours_indexes = { 19, 20 };
        skarabeusze[24].neighbours_indexes = { 20, 21 };

        for (int i = 0; i < skarabeusze_size; i++) {
            skarabeusze[i].state = selected;
        }
    }

    virtual void handleEvents(SDL_Event& event) override {
        if (event.type == SDL_QUIT) {
            quitScene();
            END = true;
        }
        else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            quitScene();
            END = true;
        }
        else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_f) {
            finished = true;
        }

        if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                mouse_left_down = true;
            }
        }
        else if (event.type == SDL_MOUSEBUTTONUP) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                mouse_left_down = false;
            }
        }
    }

    virtual void update() override {
        mouse_state = SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
        can_move = false;

        if (mathy::distance(mouse_pos, sm_btn.position) < ((sm_btn.size.x + sm_btn.size.y) / 5) && mouse_state && mouse_left_down) {
            if (!fullscreen) {
                SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
                fullscreen = true;
            }
            else {
                SDL_SetWindowFullscreen(window, 0);
                fullscreen = false;
            }
        }

        if (mathy::distance(mouse_pos, restart_btn.position) < ((restart_btn.size.x + restart_btn.size.y) / 5) && mouse_state && mouse_left_down) {
            quitScene();
        }

        if (mathy::distance(mouse_pos, key.position) < ((key.size.x + key.size.y) / 5) && mouse_state && mouse_left_down && finished) {
            key.position = mouse_pos;
            key.update_destination_rect();
            key_collected = true;
        }

        for (int i = 0; i < skarabeusze_size; i++) {
            if (mathy::distance(mouse_pos, skarabeusze[i].position) < ((skarabeusze[i].size.x + skarabeusze[i].size.y) / 8) && mouse_state && mouse_left_down && !key_collected) {
                if (skarabeusze[i].can_select || first_move) {
                    if (i != previous_skarabeusz_index && !finished) {
                        int temp_actual_skarabeusz_index = actual_skarabeusz_index;
                        int temp_previous_skarabeusz_index = previous_skarabeusz_index;
                        previous_skarabeusz_index = actual_skarabeusz_index;
                        actual_skarabeusz_index = i;
                        std::string actual_connection1 = std::to_string(previous_skarabeusz_index) + std::to_string(actual_skarabeusz_index);
                        std::string actual_connection2 = std::to_string(actual_skarabeusz_index) + std::to_string(previous_skarabeusz_index);

                        bool connection_exists = false;
                        for (const auto& connection : connections) {
                            if (actual_connection1 == connection || actual_connection2 == connection) {
                                connection_exists = true;
                                break;
                            }
                        }

                        if (!connection_exists) {
                            if (!first_move) {
                                connection_lines.push_back(Line(skarabeusze[actual_skarabeusz_index].position, skarabeusze[previous_skarabeusz_index].position, mathy::colorRGBA::BLUE(), 16));
                                connection_lines_light.push_back(Line(skarabeusze[actual_skarabeusz_index].position, skarabeusze[previous_skarabeusz_index].position, mathy::colorRGBA{ 75, 150, 225, 255 }, 8));
                            }

                            skarabeusze[i].state = confirmed;
                            first_move = false;
                            skarabeusze[i].was_confirmed = true;
                            connections.push_back(actual_connection1);
                            connections.push_back(actual_connection2);

                            for (int j = 0; j < skarabeusze_size; j++) {
                                skarabeusze[j].can_select = false;
                                if (!skarabeusze[j].was_confirmed) {
                                    skarabeusze[j].state = empty;
                                }
                                else {
                                    skarabeusze[j].state = confirmed;
                                }
                            }
                        }
                        else {
                            actual_skarabeusz_index = temp_actual_skarabeusz_index;
                            previous_skarabeusz_index = temp_previous_skarabeusz_index;
                        }

                        move_nr += 1;
                        std::cout << "move: " << move_nr << '\n';
                        Mix_PlayChannel(1, put_sound, 0);
                    }
                }
            }
        }

        for (const auto& actual_neighbour_index : skarabeusze[actual_skarabeusz_index].neighbours_indexes) {
            std::string actual_connection1 = std::to_string(actual_neighbour_index) + std::to_string(actual_skarabeusz_index);
            std::string actual_connection2 = std::to_string(actual_skarabeusz_index) + std::to_string(actual_neighbour_index);

            bool connection_exists = false;
            for (const auto& connection : connections) {
                if (connection == actual_connection1 || connection == actual_connection2) {
                    connection_exists = true;
                    break;
                }
            }

            if (!connection_exists) {
                if (actual_neighbour_index != previous_skarabeusz_index) {
                    skarabeusze[actual_neighbour_index].state = selected;
                    skarabeusze[actual_neighbour_index].can_select = true;
                    can_move = true;
                }
            }
        }

        if (!can_move && !first_move) {
            lost_image.enabled = true;
        }

        bool all_confirmed_or_selected = true;
        for (int i = 0; i < skarabeusze_size - 1; i++) {
            if (skarabeusze[i].state != confirmed && skarabeusze[i].state != selected) {
                all_confirmed_or_selected = false;
                break;
            }
        }

        if (all_confirmed_or_selected && !first_move) {
            finished = true;
        }

        if (finished) {
            flap.enabled = true;
            key.enabled = true;
            skarabeusze[12].enabled = false;

            if (key.size.y < 150) {
                key.size = mathy::vec2<int>{ key.size.x += 1, key.size.y += 1 };
                key.position.y -= 1;
                key.update_destination_rect();
            }

            if (!finish_played) {
                Mix_PlayChannel(2, finish_sound, 0);
                finish_played = true;
            }
        }
    }

    virtual void render() override {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        bg.render_texture();

        for (auto& line : connection_lines) {
            line.render_line(renderer);
        }

        for (auto& line : connection_lines_light) {
            line.render_line(renderer);
        }

        flap.render_texture();

        for (int i = 0; i < skarabeusze_size; i++) {
            skarabeusze[i].render();
        }

        restart_btn.render_texture();
        key.render_texture();
        sm_btn.render_texture();
        lost_image.render_texture();

        SDL_RenderPresent(renderer);
    }
};

int main(int argc, char* args[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
        printf("Mix_OpenAudio Error: %s\n", Mix_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Saqqarah: The Pyramid of Puzzles", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    Scene* currentScene;

    while (!END) {
        Game gameScene(renderer, window);
        currentScene = &gameScene;
        currentScene->run();
    }

    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

