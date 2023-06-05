#include <iostream>
#include <memory>
#include <vector>
#include <list>

extern "C"
{
  #include <SDL2/SDL.h>
  #include <SDL2/SDL_image.h>
}

static SDL_Texture *tileset_texture, *background_texture;
const float BLOCK_SIZE = 20.f, PLAYER_MAX_ACCELERATION_X = 3.f, PLAYER_MAX_ACCELERATION_Y = 5.f, PLAYER_FORCE_TO_RUN = 0.3f, GRAVITY_FORCE = 0.2f;
const Uint32
  WORLD_HEIGHT = 25,
  WORLD_WIDTH = 100;

struct Tile
{
  SDL_FRect rect;
  SDL_Rect texture_rect;
  bool is_solid = true;
  float light = 1.f;
};
struct MovementContext
{
  SDL_FPoint to, acceleration;
};

bool has_intersection(SDL_Point rect, Tile tile)
{
  return tile.rect.x + tile.rect.w > rect.x && tile.rect.x < rect.x &&
      tile.rect.y + tile.rect.h > rect.y && tile.rect.y < rect.y;
}

std::list<Tile> get_tiles_collider(const SDL_FRect &rect, const std::vector<Tile> &tiles)
{
  std::list<Tile> hitted_tiles;
  for(const Tile &tile : tiles)
  {
    if(
      tile.rect.x + tile.rect.w > rect.x && tile.rect.x < rect.x + rect.w &&
      tile.rect.y + tile.rect.h > rect.y && tile.rect.y < rect.y + rect.h && tile.is_solid
    ) hitted_tiles.push_back(tile);
  }
  return hitted_tiles;
}

void move_rect_on_world(SDL_FRect &rect, MovementContext &movement, std::vector<Tile> tiles)
{
  rect.x += movement.to.x;
  std::list<Tile> ctiles = get_tiles_collider(rect, tiles);
  for(Tile tile : ctiles)
  {
    if(movement.to.x > 0)
    {
      rect.x = tile.rect.x - rect.w;
      movement.acceleration.x = 0.f;
    }
    else if(movement.to.x < 0)
    {
      rect.x = tile.rect.x + tile.rect.w;
      movement.acceleration.x = 0.f;
    }
  }

  ctiles.clear();
  rect.y += movement.to.y;
  ctiles = get_tiles_collider(rect, tiles);
  for(Tile tile : ctiles)
  {
    if(movement.to.y > 0)
    {
      rect.y = tile.rect.y - rect.h;
      movement.acceleration.y = 0;
    }
    else if(movement.to.y < 0)
    {
      rect.y = tile.rect.y + tile.rect.h;
      movement.acceleration.y = 0;
    }
  }
}

void generate_world(std::vector<Tile> &tiles, float margin_top = 500.f - WORLD_HEIGHT * 20.f) noexcept
{
  srand(time(NULL));
  int y = rand() % WORLD_HEIGHT + 10;
  for(int x = 0; x < WORLD_WIDTH; x++)
  {
    for(int i = 0; i < y; i++)
    {
      float l = 0.8f;
      if(i == y - 1)
      {
        tiles.push_back(Tile{{BLOCK_SIZE * x, margin_top + BLOCK_SIZE * WORLD_HEIGHT - BLOCK_SIZE * i++, BLOCK_SIZE, BLOCK_SIZE}, {8, 0, 8, 8}});
        if(rand() % 10 == 1)
          tiles.push_back(Tile{{BLOCK_SIZE * x, margin_top + BLOCK_SIZE * WORLD_HEIGHT - BLOCK_SIZE * i++, BLOCK_SIZE, BLOCK_SIZE}, {8 * 5, 8 * 2, 8, 8}, false});
        else if(rand() % 30 == 1)
        {
          tiles.push_back(Tile{{BLOCK_SIZE * x, margin_top + BLOCK_SIZE * WORLD_HEIGHT - BLOCK_SIZE * i++, BLOCK_SIZE, BLOCK_SIZE}, {8 * 3, 8 * 4, 8, 8}});
          int ly = 0;
          for(int y = 0; y < 4; y++)
          {
            tiles.push_back(Tile{{BLOCK_SIZE * x, margin_top + BLOCK_SIZE * WORLD_HEIGHT - BLOCK_SIZE * i++, BLOCK_SIZE, BLOCK_SIZE}, {8 * 3, 8 * 3, 8, 8}});
            ly = i;
          }
          for(int h = 0; h < 5; h++)
          {
            for(int k = 0; k < 5; k++)
              tiles.push_back(Tile{{BLOCK_SIZE * x - BLOCK_SIZE * k + (2 * BLOCK_SIZE), margin_top + BLOCK_SIZE * WORLD_HEIGHT - BLOCK_SIZE * ly, BLOCK_SIZE, BLOCK_SIZE}, {8 * 1, 8 * 5, 8, 8}});
            ly++;
          }
        }
        else if(rand() % 15 == 1)
        {
          tiles.push_back(Tile{{BLOCK_SIZE * x, margin_top + BLOCK_SIZE * WORLD_HEIGHT - BLOCK_SIZE * i++, BLOCK_SIZE, BLOCK_SIZE}, {8 * 2, 8 * 5, 8, 8}, false});
        }
        else if(rand() % 15 == 1)
        {
          tiles.push_back(Tile{{BLOCK_SIZE * x, margin_top + BLOCK_SIZE * WORLD_HEIGHT - BLOCK_SIZE * i++, BLOCK_SIZE, BLOCK_SIZE}, {8 * 2, 8 * 4, 8, 8}, false});
        }
        continue;
      }
      if(i < y - 4)
      {
        if(rand() % 80 == 1)
          tiles.push_back(Tile{{BLOCK_SIZE * x, margin_top + BLOCK_SIZE * WORLD_HEIGHT - BLOCK_SIZE * i, BLOCK_SIZE, BLOCK_SIZE}, {8 * 6, 8 * 3, 8, 8}, true, 1.f - l / (i - y)});
        else if(rand() % 50 == 1)
          tiles.push_back(Tile{{BLOCK_SIZE * x, margin_top + BLOCK_SIZE * WORLD_HEIGHT - BLOCK_SIZE * i, BLOCK_SIZE, BLOCK_SIZE}, {8 * 8, 8 * 0, 8, 8}, true, 1.f - l / (i - y)});
        else if(rand() % 40 == 1)
          tiles.push_back(Tile{{BLOCK_SIZE * x, margin_top + BLOCK_SIZE * WORLD_HEIGHT - BLOCK_SIZE * i, BLOCK_SIZE, BLOCK_SIZE}, {8 * 8, 8 * 1, 8, 8}, true, 1.f - l / (i - y)});
        else if(rand() % 30 == 1)
          tiles.push_back(Tile{{BLOCK_SIZE * x, margin_top + BLOCK_SIZE * WORLD_HEIGHT - BLOCK_SIZE * i, BLOCK_SIZE, BLOCK_SIZE}, {8 * 8, 8 * 2, 8, 8}, true, 1.f - l / (i - y)});
        else
          tiles.push_back(Tile{{BLOCK_SIZE * x, margin_top + BLOCK_SIZE * WORLD_HEIGHT - BLOCK_SIZE * i, BLOCK_SIZE, BLOCK_SIZE}, {8 * 6, 8 * 2, 8, 8}, true, 1.f - l / (i - y)});
      }
      else if(i >= y / 2)
        tiles.push_back(Tile{{BLOCK_SIZE * x, margin_top + BLOCK_SIZE * WORLD_HEIGHT - BLOCK_SIZE * i, BLOCK_SIZE, BLOCK_SIZE}, {8, 8, 8, 8}, true, (i < y - 2) ? 1.f - l / (i - y) : 1.f});
    }
    if(rand() % 3 != 1)
      y += (rand() % 2) == 1 ? 1 : -1;
    if(y <= 8) y++;
  }
}

Tile get_tile_from_id(const Uint32 ID, SDL_Point scrolled) noexcept
{
  Tile tile;
  switch(ID)
  {
    case 1:
      tile = Tile{{
        (float) std::round((scrolled.x - (int) (BLOCK_SIZE / 2)) / BLOCK_SIZE) * BLOCK_SIZE,
        (float) std::round((scrolled.y - (int) (BLOCK_SIZE / 2)) / BLOCK_SIZE) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE}, {8 * 6, 8 * 2, 8, 8}};
      break;
    case 2:
      tile = Tile{{
        (float) std::round((scrolled.x - (int) (BLOCK_SIZE / 2)) / BLOCK_SIZE) * BLOCK_SIZE,
        (float) std::round((scrolled.y - (int) (BLOCK_SIZE / 2)) / BLOCK_SIZE) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE}, {8 * 7, 8 * 2, 8, 8}};
      break;
    case 3:
      tile = Tile{{
        (float) std::round((scrolled.x - (int) (BLOCK_SIZE / 2)) / BLOCK_SIZE) * BLOCK_SIZE,
        (float) std::round((scrolled.y - (int) (BLOCK_SIZE / 2)) / BLOCK_SIZE) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE}, {8 * 4, 8 * 4, 8, 8}};
      break;
    case 4:
      tile = Tile{{
        (float) std::round((scrolled.x - (int) (BLOCK_SIZE / 2)) / BLOCK_SIZE) * BLOCK_SIZE,
        (float) std::round((scrolled.y - (int) (BLOCK_SIZE / 2)) / BLOCK_SIZE) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE}, {8 * 5, 8 * 4, 8, 8}};
      break;
    case 5:
      tile = Tile{{
        (float) std::round((scrolled.x - (int) (BLOCK_SIZE / 2)) / BLOCK_SIZE) * BLOCK_SIZE,
        (float) std::round((scrolled.y - (int) (BLOCK_SIZE / 2)) / BLOCK_SIZE) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE}, {8 * 6, 8 * 4, 8, 8}};
      break;
    case 6:
      tile = Tile{{
        (float) std::round((scrolled.x - (int) (BLOCK_SIZE / 2)) / BLOCK_SIZE) * BLOCK_SIZE,
        (float) std::round((scrolled.y - (int) (BLOCK_SIZE / 2)) / BLOCK_SIZE) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE}, {8 * 7, 8 * 3, 8, 8}};
      break;
    case 7:
      tile = Tile{{
        (float) std::round((scrolled.x - (int) (BLOCK_SIZE / 2)) / BLOCK_SIZE) * BLOCK_SIZE,
        (float) std::round((scrolled.y - (int) (BLOCK_SIZE / 2)) / BLOCK_SIZE) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE}, {8 * 7, 8 * 4, 8, 8}};
      break;
    case 8:
      tile = Tile{{
        (float) std::round((scrolled.x - (int) (BLOCK_SIZE / 2)) / BLOCK_SIZE) * BLOCK_SIZE,
        (float) std::round((scrolled.y - (int) (BLOCK_SIZE / 2)) / BLOCK_SIZE) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE}, {8 * 8, 8 * 3, 8, 8}};
      break;
  }
  return tile;
}

int main(void)
{
  std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window(
    SDL_CreateWindow("Terraria 2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 500, SDL_WINDOW_SHOWN),
    SDL_DestroyWindow
  );
  std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer(
    SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_PRESENTVSYNC),
    SDL_DestroyRenderer
  );
  SDL_Event event;
  ::tileset_texture = IMG_LoadTexture(renderer.get(), "assets/tileset.png");
  ::background_texture = IMG_LoadTexture(renderer.get(), "assets/background.png");
  bool player_move_right = false, player_move_left = false;

  std::vector<Tile> world_tilemap;
  std::vector<Tile> world_tilemap_background;
  SDL_Point mouse_position;

  SDL_FPoint camera_scroll = {0.f, 0.f};
  SDL_FRect player_rect = {100.f, 30.f, 30.f / 2.f, 70.f / 2.f};
  SDL_Rect player_texture_rect = {8 * 7, 0, 8, 8};
  SDL_RendererFlip player_flip = SDL_FLIP_NONE;

  generate_world(world_tilemap);
  MovementContext player_movement;
  memset(&player_movement, 0, sizeof(MovementContext));
  Uint32 current_block = 0, eye_frame = 3;
  bool dig = false, place = false;
  Uint32 delay_action = SDL_GetTicks(), eye_tick = SDL_GetTicks();
  SDL_FRect background_rect = {-300.f, -200.f, 1000.f, 700.f};
  Tile sun = Tile{{250.f + 25.f, 0.f, 50.f, 50.f}, {8 * 3, 8 * 5, 8, 8}};

  while(window.get() && renderer.get())
  {
    SDL_GetMouseState(&mouse_position.x, &mouse_position.y);

    camera_scroll.x += (500.f / 2.f - player_rect.x - camera_scroll.x) / 15.f;
    camera_scroll.y += (500.f / 2.f - player_rect.y - camera_scroll.y) / 15.f;
    SDL_Point scrolled = {
      mouse_position.x - (int) camera_scroll.x,
      mouse_position.y - (int) camera_scroll.y
    };

    while(SDL_PollEvent(&event))
    {
      if(event.type == SDL_QUIT)
        window.~unique_ptr();
      if(event.type == SDL_MOUSEBUTTONDOWN)
      {
        if(event.button.button == SDL_BUTTON_RIGHT)
        {
          place = true;
          
        }
        if(event.button.button == SDL_BUTTON_LEFT)
        {
          dig = true;
        }
      }
      if(event.type == SDL_MOUSEBUTTONUP)
      {
        if(event.button.button == SDL_BUTTON_RIGHT)
        {
          place = false;
        }
        if(event.button.button == SDL_BUTTON_LEFT)
        {
          dig = false;
        }
      }


      if(event.type == SDL_KEYDOWN)
      {
        if(event.key.keysym.sym == SDLK_a)
          player_move_left = true;
        if(event.key.keysym.sym == SDLK_d)
          player_move_right = true;
        if(event.key.keysym.sym == SDLK_r)
        {
          world_tilemap.clear();
          world_tilemap_background.clear();
          generate_world(world_tilemap);
        }
        if(event.key.keysym.sym >= SDLK_0 && event.key.keysym.sym <= SDLK_9)
        {
          current_block = event.key.keysym.sym - SDLK_0;
        }

        if(event.key.keysym.sym == SDLK_SPACE)
          player_movement.acceleration.y = -5.f;
      }
      else if(event.type == SDL_KEYUP)
      {
        if(event.key.keysym.sym == SDLK_a)
          player_move_left = false;
        if(event.key.keysym.sym == SDLK_d)
          player_move_right = false;
      }
    }
    if(dig)
    {
      for(int i = 0; i < world_tilemap.size(); i++)
      {
        if(has_intersection(scrolled, world_tilemap[i]))
        {
          world_tilemap.erase(world_tilemap.begin() + i);
        }
      }
    }
    if(place)
    {
      if(delay_action + 50 < SDL_GetTicks())
      {
        if(current_block >= 6 && current_block <= 8)
          world_tilemap_background.push_back(get_tile_from_id(current_block, scrolled));
        else
          world_tilemap.push_back(get_tile_from_id(current_block, scrolled));
        delay_action = SDL_GetTicks();
      }
    }

    if(eye_tick + 500 < SDL_GetTicks())
    {
      sun.texture_rect.x = 8 * (eye_frame += eye_frame >= 9 ? -eye_frame + 3 : 1);
      eye_tick = SDL_GetTicks();
    }
    player_movement.to = SDL_FPoint{
      player_move_left ? player_movement.acceleration.x -= PLAYER_FORCE_TO_RUN : player_move_right ? player_movement.acceleration.x += PLAYER_FORCE_TO_RUN : player_movement.acceleration.x,
      player_movement.acceleration.y += player_movement.acceleration.y > PLAYER_MAX_ACCELERATION_Y ? 0.f : GRAVITY_FORCE
    };
    if(player_move_left) player_flip = SDL_FLIP_HORIZONTAL;
    else if(player_move_right) player_flip = SDL_FLIP_NONE;
    if((!player_move_left && !player_move_right) || std::abs(player_movement.acceleration.x) > PLAYER_MAX_ACCELERATION_X)
    {
      player_movement.acceleration.x += player_movement.acceleration.x < 0.f ? PLAYER_FORCE_TO_RUN : player_movement.acceleration.x > 0.f ? -PLAYER_FORCE_TO_RUN : 0.f;
    }

    move_rect_on_world(player_rect, player_movement, world_tilemap);
    

    SDL_SetRenderDrawColor(renderer.get(), 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer.get());

    SDL_FRect background_scrolled = background_rect;
    background_scrolled.x += camera_scroll.x / 7.f;
    background_scrolled.y += camera_scroll.y / 7.f;
    SDL_RenderCopyF(renderer.get(), background_texture, NULL, &background_scrolled);
    
    SDL_FRect sun_scrolled = sun.rect;
    sun_scrolled.x += camera_scroll.x / 4.f;
    sun_scrolled.y += camera_scroll.y / 4.f;

    SDL_RenderCopyExF(renderer.get(), tileset_texture, &sun.texture_rect, &sun_scrolled, 0, NULL, SDL_FLIP_NONE);

    for(Tile tile : world_tilemap_background)
    {
      tile.rect.x += camera_scroll.x;
      tile.rect.y += camera_scroll.y;
      SDL_RenderCopyF(renderer.get(), ::tileset_texture, &tile.texture_rect, &tile.rect);
    }
    for(Tile tile : world_tilemap)
    {
      tile.rect.x += camera_scroll.x;
      tile.rect.y += camera_scroll.y;
      SDL_SetTextureColorMod(::tileset_texture, (Uint8) std::round(255 * tile.light), (Uint8) std::round(255 * tile.light), (Uint8) std::round(255 * tile.light));
      SDL_RenderCopyF(renderer.get(), ::tileset_texture, &tile.texture_rect, &tile.rect);
    }

    SDL_FRect player_rect_scrolled = player_rect;
    player_rect_scrolled.x += camera_scroll.x;
    player_rect_scrolled.y += camera_scroll.y;

    SDL_RenderCopyExF(renderer.get(), ::tileset_texture, &player_texture_rect, &player_rect_scrolled, 0, NULL, player_flip);

    Tile tile = get_tile_from_id(current_block, SDL_Point{20, 20});
    SDL_RenderCopyF(renderer.get(), ::tileset_texture, &tile.texture_rect, &tile.rect);


    SDL_RenderPresent(renderer.get());
  }

  SDL_DestroyTexture(tileset_texture);
  return EXIT_SUCCESS;
}